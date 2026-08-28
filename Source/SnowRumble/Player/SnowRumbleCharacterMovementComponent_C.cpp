// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleCharacterMovementComponent_C.h"

#include "GameFramework/Character.h"
#include "GameFramework/GameStateBase.h"

namespace
{
bool AreEnvironmentalDriftStatesEquivalent(
	const FEnvironmentalDriftState_C& A,
	const FEnvironmentalDriftState_C& B)
{
	if (A.bActive != B.bActive)
	{
		return false;
	}

	if (!A.bActive)
	{
		return true;
	}

	return A.SourceGeneration == B.SourceGeneration
		&& A.Direction.Equals(B.Direction, KINDA_SMALL_NUMBER)
		&& FMath::IsNearlyEqual(A.Acceleration, B.Acceleration)
		&& FMath::IsNearlyEqual(A.GroundMaxDriftSpeed, B.GroundMaxDriftSpeed)
		&& FMath::IsNearlyEqual(A.FallingMaxDriftSpeed, B.FallingMaxDriftSpeed)
		&& FMath::IsNearlyEqual(A.StartServerTime, B.StartServerTime)
		&& FMath::IsNearlyEqual(A.EndServerTime, B.EndServerTime)
		&& FMath::IsNearlyEqual(A.RampUpSeconds, B.RampUpSeconds)
		&& FMath::IsNearlyEqual(A.RampDownSeconds, B.RampDownSeconds);
}

class FSavedMove_SnowRumbleCharacter_C : public FSavedMove_Character
{
	using Super = FSavedMove_Character;

public:
	virtual void Clear() override
	{
		Super::Clear();

		SavedEnvironmentalDriftState = FEnvironmentalDriftState_C();
		SavedEnvironmentalDriftServerTime = 0.0f;
	}

	virtual bool CanCombineWith(
		const FSavedMovePtr& NewMove,
		ACharacter* InCharacter,
		float MaxDelta) const override
	{
		const FSavedMove_SnowRumbleCharacter_C* NewSnowRumbleMove =
			static_cast<const FSavedMove_SnowRumbleCharacter_C*>(NewMove.Get());
		if (!NewSnowRumbleMove
			|| !AreEnvironmentalDriftStatesEquivalent(
				SavedEnvironmentalDriftState,
				NewSnowRumbleMove->SavedEnvironmentalDriftState))
		{
			return false;
		}

		return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
	}

	virtual void SetMoveFor(
		ACharacter* Character,
		float InDeltaTime,
		const FVector& NewAccel,
		FNetworkPredictionData_Client_Character& ClientData) override
	{
		Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

		const USnowRumbleCharacterMovementComponent_C* MovementComponent =
			Character
				? Cast<USnowRumbleCharacterMovementComponent_C>(
					Character->GetCharacterMovement())
				: nullptr;
		if (!MovementComponent)
		{
			SavedEnvironmentalDriftState = FEnvironmentalDriftState_C();
			SavedEnvironmentalDriftServerTime = 0.0f;
			return;
		}

		SavedEnvironmentalDriftState =
			MovementComponent->GetEnvironmentalDriftState();
		SavedEnvironmentalDriftServerTime =
			MovementComponent->GetServerWorldTimeSeconds();
	}

	virtual void PrepMoveFor(ACharacter* Character) override
	{
		if (USnowRumbleCharacterMovementComponent_C* MovementComponent =
			Character
				? Cast<USnowRumbleCharacterMovementComponent_C>(
					Character->GetCharacterMovement())
				: nullptr)
		{
			MovementComponent->RestoreEnvironmentalDriftForSavedMove(
				SavedEnvironmentalDriftState,
				SavedEnvironmentalDriftServerTime);
		}

		Super::PrepMoveFor(Character);
	}

private:
	FEnvironmentalDriftState_C SavedEnvironmentalDriftState;
	float SavedEnvironmentalDriftServerTime = 0.0f;
};

class FNetworkPredictionData_Client_SnowRumbleCharacter_C
	: public FNetworkPredictionData_Client_Character
{
	using Super = FNetworkPredictionData_Client_Character;

public:
	explicit FNetworkPredictionData_Client_SnowRumbleCharacter_C(
		const UCharacterMovementComponent& ClientMovement)
		: Super(ClientMovement)
	{
	}

	virtual FSavedMovePtr AllocateNewMove() override
	{
		return FSavedMovePtr(new FSavedMove_SnowRumbleCharacter_C());
	}
};
}

void USnowRumbleCharacterMovementComponent_C::SetEnvironmentalDrift(
	const FEnvironmentalDriftState_C& NewDriftState)
{
	FEnvironmentalDriftState_C SanitizedState = NewDriftState;
	SanitizedState.Direction.Z = 0.0f;
	SanitizedState.Direction = SanitizedState.Direction.GetSafeNormal();
	SanitizedState.Acceleration = FMath::Max(0.0f, SanitizedState.Acceleration);
	SanitizedState.GroundMaxDriftSpeed =
		FMath::Max(0.0f, SanitizedState.GroundMaxDriftSpeed);
	SanitizedState.FallingMaxDriftSpeed =
		FMath::Max(0.0f, SanitizedState.FallingMaxDriftSpeed);
	SanitizedState.RampUpSeconds =
		FMath::Max(0.0f, SanitizedState.RampUpSeconds);
	SanitizedState.RampDownSeconds =
		FMath::Max(0.0f, SanitizedState.RampDownSeconds);
	SanitizedState.bActive =
		SanitizedState.bActive
		&& !SanitizedState.Direction.IsNearlyZero()
		&& SanitizedState.Acceleration > 0.0f
		&& SanitizedState.EndServerTime > SanitizedState.StartServerTime;

	EnvironmentalDriftState = SanitizedState;
}

void USnowRumbleCharacterMovementComponent_C::ClearEnvironmentalDrift(
	int32 SourceGeneration)
{
	if (SourceGeneration != INDEX_NONE
		&& EnvironmentalDriftState.bActive
		&& EnvironmentalDriftState.SourceGeneration != SourceGeneration)
	{
		return;
	}

	EnvironmentalDriftState = FEnvironmentalDriftState_C();
}

const FEnvironmentalDriftState_C&
USnowRumbleCharacterMovementComponent_C::GetEnvironmentalDriftState() const
{
	return EnvironmentalDriftState;
}

void USnowRumbleCharacterMovementComponent_C::RestoreEnvironmentalDriftForSavedMove(
	const FEnvironmentalDriftState_C& SavedDriftState,
	float SavedServerTimeSeconds)
{
	EnvironmentalDriftState = SavedDriftState;
	bUseSavedMoveServerTimeForEnvironmentalDrift = true;
	SavedMoveEnvironmentalDriftServerTime = SavedServerTimeSeconds;
}

void USnowRumbleCharacterMovementComponent_C::CalcVelocity(
	float DeltaTime,
	float Friction,
	bool bFluid,
	float BrakingDeceleration)
{
	Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
	ApplyEnvironmentalDrift(DeltaTime);
}

void USnowRumbleCharacterMovementComponent_C::PerformMovement(float DeltaTime)
{
	Super::PerformMovement(DeltaTime);

	bUseSavedMoveServerTimeForEnvironmentalDrift = false;
	SavedMoveEnvironmentalDriftServerTime = 0.0f;
}

FNetworkPredictionData_Client*
USnowRumbleCharacterMovementComponent_C::GetPredictionData_Client() const
{
	if (!ClientPredictionData)
	{
		USnowRumbleCharacterMovementComponent_C* MutableThis =
			const_cast<USnowRumbleCharacterMovementComponent_C*>(this);
		MutableThis->ClientPredictionData =
			new FNetworkPredictionData_Client_SnowRumbleCharacter_C(*this);
	}

	return ClientPredictionData;
}

bool USnowRumbleCharacterMovementComponent_C::HasValidEnvironmentalDrift() const
{
	return EnvironmentalDriftState.bActive
		&& !EnvironmentalDriftState.Direction.IsNearlyZero()
		&& EnvironmentalDriftState.Acceleration > 0.0f
		&& EnvironmentalDriftState.EndServerTime
			> EnvironmentalDriftState.StartServerTime;
}

float USnowRumbleCharacterMovementComponent_C::GetEnvironmentalDriftMaxSpeedForCurrentMode()
	const
{
	if (MovementMode == MOVE_Walking || MovementMode == MOVE_NavWalking)
	{
		return EnvironmentalDriftState.GroundMaxDriftSpeed;
	}

	if (MovementMode == MOVE_Falling)
	{
		return EnvironmentalDriftState.FallingMaxDriftSpeed;
	}

	return 0.0f;
}

float USnowRumbleCharacterMovementComponent_C::CalculateEnvironmentalDriftStrengthAlpha()
	const
{
	const float ServerTimeSeconds = bUseSavedMoveServerTimeForEnvironmentalDrift
		? SavedMoveEnvironmentalDriftServerTime
		: GetServerWorldTimeSeconds();
	return CalculateEnvironmentalDriftStrengthAlphaAtTime(ServerTimeSeconds);
}

float USnowRumbleCharacterMovementComponent_C::
	CalculateEnvironmentalDriftStrengthAlphaAtTime(float ServerTimeSeconds) const
{
	if (!HasValidEnvironmentalDrift()
		|| ServerTimeSeconds < EnvironmentalDriftState.StartServerTime
		|| ServerTimeSeconds >= EnvironmentalDriftState.EndServerTime)
	{
		return 0.0f;
	}

	const float DriftElapsedSeconds = FMath::Max(
		0.0f,
		ServerTimeSeconds - EnvironmentalDriftState.StartServerTime);
	const float DriftRemainingSeconds = FMath::Max(
		0.0f,
		EnvironmentalDriftState.EndServerTime - ServerTimeSeconds);

	float RampUpStrength = 1.0f;
	if (EnvironmentalDriftState.RampUpSeconds > KINDA_SMALL_NUMBER)
	{
		const float RampAlpha = FMath::Clamp(
			DriftElapsedSeconds / EnvironmentalDriftState.RampUpSeconds,
			0.0f,
			1.0f);
		RampUpStrength = RampAlpha * RampAlpha;
	}

	float RampDownStrength = 1.0f;
	if (EnvironmentalDriftState.RampDownSeconds > KINDA_SMALL_NUMBER)
	{
		RampDownStrength = FMath::Clamp(
			DriftRemainingSeconds / EnvironmentalDriftState.RampDownSeconds,
			0.0f,
			1.0f);
	}

	return FMath::Clamp(
		FMath::Min(RampUpStrength, RampDownStrength),
		0.0f,
		1.0f);
}

float USnowRumbleCharacterMovementComponent_C::GetServerWorldTimeSeconds() const
{
	const UWorld* World = GetWorld();
	const AGameStateBase* GameState = World
		? World->GetGameState<AGameStateBase>()
		: nullptr;
	return GameState ? GameState->GetServerWorldTimeSeconds()
		: World ? World->GetTimeSeconds() : 0.0f;
}

void USnowRumbleCharacterMovementComponent_C::ApplyEnvironmentalDrift(
	float DeltaTime)
{
	if (DeltaTime <= KINDA_SMALL_NUMBER || !HasValidEnvironmentalDrift())
	{
		return;
	}

	const float MaxDriftSpeed = GetEnvironmentalDriftMaxSpeedForCurrentMode();
	if (MaxDriftSpeed <= 0.0f)
	{
		return;
	}

	const float StrengthAlpha = CalculateEnvironmentalDriftStrengthAlpha();
	if (StrengthAlpha <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float TargetDriftSpeed = MaxDriftSpeed * StrengthAlpha;
	const FVector CurrentHorizontalVelocity(Velocity.X, Velocity.Y, 0.0f);
	const float CurrentSpeedAlongDrift = FVector::DotProduct(
		CurrentHorizontalVelocity,
		EnvironmentalDriftState.Direction);
	const float MissingDriftSpeed =
		TargetDriftSpeed - CurrentSpeedAlongDrift;
	if (MissingDriftSpeed <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float DriftSpeedToAdd = FMath::Min(
		EnvironmentalDriftState.Acceleration * DeltaTime,
		MissingDriftSpeed);
	const float PreviousZ = Velocity.Z;
	Velocity += EnvironmentalDriftState.Direction * DriftSpeedToAdd;
	Velocity.Z = PreviousZ;
}

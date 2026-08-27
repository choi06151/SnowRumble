// Copyright Epic Games, Inc. All Rights Reserved.

#include "IceGlacierWindGustActor_J.h"

#include "../Game/SnowRumbleGameState_C.h"
#include "../Player/SnowRumbleCharacter.h"
#include "../Player/SnowRumbleCharacterMovementComponent_C.h"
#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"

AIceGlacierWindGustActor::AIceGlacierWindGustActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);
	SetNetUpdateFrequency(10.0f);
	SetMinNetUpdateFrequency(2.0f);

	RootSceneComponent =
		CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootSceneComponent);

	WindDirections.Add(FVector::ForwardVector);
	WindDirections.Add(-FVector::ForwardVector);
	WindDirections.Add(FVector::RightVector);
	WindDirections.Add(-FVector::RightVector);
}

void AIceGlacierWindGustActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		UpdateServerWind(DeltaSeconds);
	}
	RefreshLocalEnvironmentalDrift();
}

void AIceGlacierWindGustActor::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AIceGlacierWindGustActor, WindReplicatedState);
}

EIceGlacierWindGustState
AIceGlacierWindGustActor::GetCurrentWindState() const
{
	return WindReplicatedState.CurrentWindState;
}

FVector AIceGlacierWindGustActor::GetCurrentWindDirection() const
{
	return WindReplicatedState.CurrentWindDirection;
}

void AIceGlacierWindGustActor::OnRep_WindReplicatedState()
{
	NotifyWindStateChanged(
		LastNotifiedWindState,
		WindReplicatedState.CurrentWindState);
	LastNotifiedWindState = WindReplicatedState.CurrentWindState;
	RefreshLocalEnvironmentalDrift();
}

void AIceGlacierWindGustActor::UpdateServerWind(float DeltaSeconds)
{
	if (!IsMatchPlayActive())
	{
		ResetWindForInactiveMatch();
		return;
	}

	if (!bWindScheduleStarted)
	{
		ScheduleNextWind();
	}

	const float CurrentServerTime = GetServerWorldTimeSeconds();
	switch (WindReplicatedState.CurrentWindState)
	{
	case EIceGlacierWindGustState::Idle:
		if (CurrentServerTime >= NextWindStartServerTime)
		{
			StartWindWarning();
		}
		break;
	case EIceGlacierWindGustState::Warning:
		if (CurrentServerTime >= WindReplicatedState.CurrentStateEndServerTime)
		{
			StartWindGust();
		}
		break;
	case EIceGlacierWindGustState::Gust:
		if (CurrentServerTime >= WindReplicatedState.CurrentStateEndServerTime)
		{
			EndWindGust();
		}
		else
		{
			ApplyEnvironmentalDriftToServerCharacters();
		}
		break;
	default:
		break;
	}
}

bool AIceGlacierWindGustActor::IsMatchPlayActive() const
{
	const UWorld* World = GetWorld();
	const ASnowRumbleGameState* SnowRumbleGameState = World
		? World->GetGameState<ASnowRumbleGameState>()
		: nullptr;
	return SnowRumbleGameState && !SnowRumbleGameState->IsMatchInputLocked();
}

float AIceGlacierWindGustActor::GetServerWorldTimeSeconds() const
{
	const UWorld* World = GetWorld();
	const AGameStateBase* GameState = World
		? World->GetGameState<AGameStateBase>()
		: nullptr;
	return GameState ? GameState->GetServerWorldTimeSeconds()
		: World ? World->GetTimeSeconds() : 0.0f;
}

void AIceGlacierWindGustActor::ScheduleNextWind()
{
	const float SafeMinInterval = FMath::Max(0.0f, MinWindIntervalSeconds);
	const float SafeMaxInterval = FMath::Max(
		SafeMinInterval,
		MaxWindIntervalSeconds);
	const float IntervalSeconds =
		FMath::FRandRange(SafeMinInterval, SafeMaxInterval);
	NextWindStartServerTime = GetServerWorldTimeSeconds() + IntervalSeconds;
	bWindScheduleStarted = true;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[IceWind] Schedule NextWind Interval=%.2f"),
		IntervalSeconds);
}

void AIceGlacierWindGustActor::StartWindWarning()
{
	WindReplicatedState.CurrentWindDirection = ChooseRandomWindDirection();
	WindReplicatedState.CurrentWindState = EIceGlacierWindGustState::Warning;
	WindReplicatedState.CurrentGustStartServerTime = 0.0f;
	WindReplicatedState.CurrentStateEndServerTime =
		GetServerWorldTimeSeconds() + FMath::Max(0.0f, WarningDurationSeconds);
	++WindReplicatedState.WindGeneration;
	bWindScheduleStarted = false;

	NotifyWindStateChanged(
		LastNotifiedWindState,
		WindReplicatedState.CurrentWindState);
	LastNotifiedWindState = WindReplicatedState.CurrentWindState;
	ForceNetUpdate();

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[IceWind] Warning Direction=%s"),
		*WindReplicatedState.CurrentWindDirection.ToCompactString());
}

void AIceGlacierWindGustActor::StartWindGust()
{
	WindReplicatedState.CurrentWindState = EIceGlacierWindGustState::Gust;
	WindReplicatedState.CurrentGustStartServerTime = GetServerWorldTimeSeconds();
	WindReplicatedState.CurrentStateEndServerTime =
		WindReplicatedState.CurrentGustStartServerTime
		+ FMath::Max(0.0f, WindDurationSeconds);

	NotifyWindStateChanged(
		LastNotifiedWindState,
		WindReplicatedState.CurrentWindState);
	LastNotifiedWindState = WindReplicatedState.CurrentWindState;
	ApplyEnvironmentalDriftToServerCharacters();
	ForceNetUpdate();

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[IceWind] GustStart Direction=%s GroundDrift=%.2f FallingDrift=%.2f"),
		*WindReplicatedState.CurrentWindDirection.ToCompactString(),
		GroundWindMaxDriftSpeed,
		FallingWindMaxDriftSpeed);
}

void AIceGlacierWindGustActor::EndWindGust()
{
	const int32 EndingWindGeneration = WindReplicatedState.WindGeneration;
	WindReplicatedState.CurrentWindState = EIceGlacierWindGustState::Idle;
	WindReplicatedState.CurrentGustStartServerTime = 0.0f;
	WindReplicatedState.CurrentStateEndServerTime = 0.0f;

	NotifyWindStateChanged(
		LastNotifiedWindState,
		WindReplicatedState.CurrentWindState);
	LastNotifiedWindState = WindReplicatedState.CurrentWindState;
	ClearEnvironmentalDriftFromServerCharacters(EndingWindGeneration);
	ForceNetUpdate();

	UE_LOG(LogTemp, Log, TEXT("[IceWind] GustEnd"));

	ScheduleNextWind();
}

void AIceGlacierWindGustActor::ResetWindForInactiveMatch()
{
	bWindScheduleStarted = false;
	NextWindStartServerTime = 0.0f;
	WindReplicatedState.CurrentGustStartServerTime = 0.0f;
	WindReplicatedState.CurrentStateEndServerTime = 0.0f;

	if (WindReplicatedState.CurrentWindState == EIceGlacierWindGustState::Idle)
	{
		return;
	}

	const int32 EndingWindGeneration = WindReplicatedState.WindGeneration;
	const EIceGlacierWindGustState PreviousState =
		WindReplicatedState.CurrentWindState;
	WindReplicatedState.CurrentWindState = EIceGlacierWindGustState::Idle;
	NotifyWindStateChanged(
		PreviousState,
		WindReplicatedState.CurrentWindState);
	LastNotifiedWindState = WindReplicatedState.CurrentWindState;
	ClearEnvironmentalDriftFromServerCharacters(EndingWindGeneration);
	ForceNetUpdate();

	if (PreviousState == EIceGlacierWindGustState::Gust)
	{
		UE_LOG(LogTemp, Log, TEXT("[IceWind] GustEnd"));
	}
}

FVector AIceGlacierWindGustActor::ChooseRandomWindDirection() const
{
	TArray<FVector> ValidDirections;
	ValidDirections.Reserve(WindDirections.Num());
	for (const FVector& WindDirection : WindDirections)
	{
		const FVector NormalizedDirection = NormalizeWindDirection(WindDirection);
		if (!NormalizedDirection.IsNearlyZero())
		{
			ValidDirections.Add(NormalizedDirection);
		}
	}

	if (ValidDirections.IsEmpty())
	{
		return FVector::ForwardVector;
	}

	const int32 DirectionIndex = FMath::RandHelper(ValidDirections.Num());
	return ValidDirections[DirectionIndex];
}

FVector AIceGlacierWindGustActor::NormalizeWindDirection(
	FVector Direction) const
{
	Direction.Z = 0.0f;
	return Direction.GetSafeNormal();
}

FEnvironmentalDriftState_C AIceGlacierWindGustActor::BuildEnvironmentalDriftState()
	const
{
	FEnvironmentalDriftState_C DriftState;
	DriftState.bActive =
		WindReplicatedState.CurrentWindState == EIceGlacierWindGustState::Gust;
	DriftState.Direction = NormalizeWindDirection(
		WindReplicatedState.CurrentWindDirection);
	DriftState.Acceleration = WindAcceleration;
	DriftState.GroundMaxDriftSpeed = GroundWindMaxDriftSpeed;
	DriftState.FallingMaxDriftSpeed = FallingWindMaxDriftSpeed;
	DriftState.StartServerTime =
		WindReplicatedState.CurrentGustStartServerTime;
	DriftState.EndServerTime = WindReplicatedState.CurrentStateEndServerTime;
	DriftState.RampUpSeconds = WindRampUpSeconds;
	DriftState.RampDownSeconds = WindRampDownSeconds;
	DriftState.SourceGeneration = WindReplicatedState.WindGeneration;
	return DriftState;
}

void AIceGlacierWindGustActor::ApplyEnvironmentalDriftToServerCharacters() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FEnvironmentalDriftState_C DriftState = BuildEnvironmentalDriftState();
	for (TActorIterator<ASnowRumbleCharacter> It(World); It; ++It)
	{
		ASnowRumbleCharacter* Character = *It;
		if (ShouldAffectCharacter(Character))
		{
			ApplyEnvironmentalDriftToCharacter(Character, DriftState);
		}
		else
		{
			ClearEnvironmentalDriftFromCharacter(
				Character,
				DriftState.SourceGeneration);
		}
	}
}

void AIceGlacierWindGustActor::ClearEnvironmentalDriftFromServerCharacters(
	int32 WindGeneration) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ASnowRumbleCharacter> It(World); It; ++It)
	{
		ClearEnvironmentalDriftFromCharacter(*It, WindGeneration);
	}
}

void AIceGlacierWindGustActor::RefreshLocalEnvironmentalDrift() const
{
	const UWorld* World = GetWorld();
	const APlayerController* LocalPlayerController = World
		? World->GetFirstPlayerController()
		: nullptr;
	ASnowRumbleCharacter* LocalCharacter =
		LocalPlayerController
			? Cast<ASnowRumbleCharacter>(LocalPlayerController->GetPawn())
			: nullptr;
	if (!LocalCharacter || !LocalCharacter->IsLocallyControlled())
	{
		return;
	}

	const FEnvironmentalDriftState_C DriftState = BuildEnvironmentalDriftState();
	const float CurrentServerTime = GetServerWorldTimeSeconds();
	const bool bWindVfxLeadActive =
		WindReplicatedState.CurrentWindState
			== EIceGlacierWindGustState::Warning
		&& CurrentServerTime
			>= WindReplicatedState.CurrentStateEndServerTime
			- FMath::Max(0.0f, LocalSnowEffectWindLeadSeconds);
	const bool bWindVfxDirectionActive =
		DriftState.bActive || bWindVfxLeadActive;
	LocalCharacter->SetLocalSnowEffectWindDirection(
		bWindVfxDirectionActive
			? DriftState.Direction
			: FVector(0.0f, 0.0f, -1.0f));
	float WindStrengthAlpha = DriftState.bActive ? 1.0f : 0.0f;
	if (bWindVfxLeadActive)
	{
		const float LeadSeconds =
			FMath::Max(0.0f, LocalSnowEffectWindLeadSeconds);
		const float SecondsUntilGust =
			FMath::Max(
				0.0f,
				WindReplicatedState.CurrentStateEndServerTime
					- CurrentServerTime);
		WindStrengthAlpha = LeadSeconds > KINDA_SMALL_NUMBER
			? FMath::Clamp(1.0f - SecondsUntilGust / LeadSeconds, 0.0f, 1.0f)
			: 1.0f;
	}
	LocalCharacter->SetLocalSnowEffectWindStrength(FMath::Lerp(
		LocalSnowEffectDefaultStrength,
		LocalSnowEffectGustStrength,
		WindStrengthAlpha));
	if (DriftState.bActive && ShouldAffectCharacter(LocalCharacter))
	{
		ApplyEnvironmentalDriftToCharacter(LocalCharacter, DriftState);
	}
	else
	{
		ClearEnvironmentalDriftFromCharacter(
			LocalCharacter,
			DriftState.SourceGeneration);
	}
}

void AIceGlacierWindGustActor::ApplyEnvironmentalDriftToCharacter(
	ASnowRumbleCharacter* Character,
	const FEnvironmentalDriftState_C& DriftState) const
{
	USnowRumbleCharacterMovementComponent_C* MovementComponent =
		Character
			? Cast<USnowRumbleCharacterMovementComponent_C>(
				Character->GetCharacterMovement())
			: nullptr;
	if (MovementComponent)
	{
		MovementComponent->SetEnvironmentalDrift(DriftState);
	}
}

void AIceGlacierWindGustActor::ClearEnvironmentalDriftFromCharacter(
	ASnowRumbleCharacter* Character,
	int32 WindGeneration) const
{
	USnowRumbleCharacterMovementComponent_C* MovementComponent =
		Character
			? Cast<USnowRumbleCharacterMovementComponent_C>(
				Character->GetCharacterMovement())
			: nullptr;
	if (MovementComponent)
	{
		MovementComponent->ClearEnvironmentalDrift(WindGeneration);
	}
}

float AIceGlacierWindGustActor::CalculateCurrentWindStrengthAlpha() const
{
	if (WindReplicatedState.CurrentWindState != EIceGlacierWindGustState::Gust)
	{
		return 0.0f;
	}

	if (WindReplicatedState.CurrentGustStartServerTime <= 0.0f
		|| WindReplicatedState.CurrentStateEndServerTime
			<= WindReplicatedState.CurrentGustStartServerTime)
	{
		return 0.0f;
	}

	const float CurrentServerTime = GetServerWorldTimeSeconds();
	if (CurrentServerTime < WindReplicatedState.CurrentGustStartServerTime
		|| CurrentServerTime
			>= WindReplicatedState.CurrentStateEndServerTime)
	{
		return 0.0f;
	}

	const float GustElapsedSeconds = FMath::Max(
		0.0f,
		CurrentServerTime - WindReplicatedState.CurrentGustStartServerTime);
	const float GustRemainingSeconds = FMath::Max(
		0.0f,
		WindReplicatedState.CurrentStateEndServerTime - CurrentServerTime);

	float RampUpStrength = 1.0f;
	if (WindRampUpSeconds > KINDA_SMALL_NUMBER)
	{
		const float RampAlpha = FMath::Clamp(
			GustElapsedSeconds / WindRampUpSeconds,
			0.0f,
			1.0f);
		RampUpStrength = RampAlpha * RampAlpha;
	}

	float RampDownStrength = 1.0f;
	if (WindRampDownSeconds > KINDA_SMALL_NUMBER)
	{
		RampDownStrength = FMath::Clamp(
			GustRemainingSeconds / WindRampDownSeconds,
			0.0f,
			1.0f);
	}

	return FMath::Clamp(
		FMath::Min(RampUpStrength, RampDownStrength),
		0.0f,
		1.0f);
}

bool AIceGlacierWindGustActor::ShouldAffectCharacter(
	const ASnowRumbleCharacter* Character) const
{
	return IsValid(Character)
		&& Character->IsPlayerControlled()
		&& !Character->IsFrozen()
		&& !Character->IsDead();
}

void AIceGlacierWindGustActor::NotifyWindStateChanged(
	EIceGlacierWindGustState PreviousState,
	EIceGlacierWindGustState NewState)
{
	if (PreviousState == NewState)
	{
		return;
	}

	if (NewState == EIceGlacierWindGustState::Warning)
	{
		OnWindWarningStarted(WindReplicatedState.CurrentWindDirection);
	}
	else if (NewState == EIceGlacierWindGustState::Gust)
	{
		OnWindGustStarted(
			WindReplicatedState.CurrentWindDirection,
			GroundWindMaxDriftSpeed);
	}
	else if (PreviousState == EIceGlacierWindGustState::Gust)
	{
		OnWindGustEnded();
	}
}

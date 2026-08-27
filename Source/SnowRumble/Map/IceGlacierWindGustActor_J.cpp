// Copyright Epic Games, Inc. All Rights Reserved.

#include "IceGlacierWindGustActor_J.h"

#include "../Game/SnowRumbleGameState_C.h"
#include "../Player/SnowRumbleCharacter.h"
#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
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
	else
	{
		ApplyPredictedWindDriftToLocalPlayer(DeltaSeconds);
	}

	UpdateLocalSnowVfxWindDirection();
}

void AIceGlacierWindGustActor::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AIceGlacierWindGustActor, CurrentWindDirection);
	DOREPLIFETIME(AIceGlacierWindGustActor, CurrentWindState);
}

EIceGlacierWindGustState
AIceGlacierWindGustActor::GetCurrentWindState() const
{
	return CurrentWindState;
}

FVector AIceGlacierWindGustActor::GetCurrentWindDirection() const
{
	return CurrentWindDirection;
}

void AIceGlacierWindGustActor::OnRep_CurrentWindState()
{
	NotifyWindStateChanged(LastNotifiedWindState, CurrentWindState);
	LastNotifiedWindState = CurrentWindState;
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
	switch (CurrentWindState)
	{
	case EIceGlacierWindGustState::Idle:
		if (CurrentServerTime >= NextWindStartServerTime)
		{
			StartWindWarning();
		}
		break;
	case EIceGlacierWindGustState::Warning:
		if (CurrentServerTime >= CurrentStateEndServerTime)
		{
			StartWindGust();
		}
		break;
	case EIceGlacierWindGustState::Gust:
		if (CurrentServerTime >= CurrentStateEndServerTime)
		{
			EndWindGust();
		}
		else
		{
			ApplyWindDriftToPlayers(DeltaSeconds);
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
	CurrentWindDirection = ChooseRandomWindDirection();
	CurrentWindState = EIceGlacierWindGustState::Warning;
	CurrentStateEndServerTime =
		GetServerWorldTimeSeconds() + FMath::Max(0.0f, WarningDurationSeconds);
	bWindScheduleStarted = false;

	NotifyWindStateChanged(LastNotifiedWindState, CurrentWindState);
	LastNotifiedWindState = CurrentWindState;
	ForceNetUpdate();

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[IceWind] Warning Direction=%s"),
		*CurrentWindDirection.ToCompactString());
}

void AIceGlacierWindGustActor::StartWindGust()
{
	CurrentWindState = EIceGlacierWindGustState::Gust;
	CurrentGustStartServerTime = GetServerWorldTimeSeconds();
	CurrentStateEndServerTime =
		CurrentGustStartServerTime + FMath::Max(0.0f, WindDurationSeconds);

	NotifyWindStateChanged(LastNotifiedWindState, CurrentWindState);
	LastNotifiedWindState = CurrentWindState;
	ForceNetUpdate();

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[IceWind] GustStart Direction=%s GroundDrift=%.2f FallingDrift=%.2f"),
		*CurrentWindDirection.ToCompactString(),
		GroundWindMaxDriftSpeed,
		FallingWindMaxDriftSpeed);
}

void AIceGlacierWindGustActor::EndWindGust()
{
	CurrentWindState = EIceGlacierWindGustState::Idle;

	NotifyWindStateChanged(LastNotifiedWindState, CurrentWindState);
	LastNotifiedWindState = CurrentWindState;
	ForceNetUpdate();

	UE_LOG(LogTemp, Log, TEXT("[IceWind] GustEnd"));

	ScheduleNextWind();
}

void AIceGlacierWindGustActor::ResetWindForInactiveMatch()
{
	bWindScheduleStarted = false;
	NextWindStartServerTime = 0.0f;
	CurrentGustStartServerTime = 0.0f;
	CurrentStateEndServerTime = 0.0f;

	if (CurrentWindState == EIceGlacierWindGustState::Idle)
	{
		return;
	}

	const EIceGlacierWindGustState PreviousState = CurrentWindState;
	CurrentWindState = EIceGlacierWindGustState::Idle;
	NotifyWindStateChanged(PreviousState, CurrentWindState);
	LastNotifiedWindState = CurrentWindState;
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

void AIceGlacierWindGustActor::ApplyWindDriftToPlayers(float DeltaSeconds)
{
	UWorld* World = GetWorld();
	const FVector WindDirection = NormalizeWindDirection(CurrentWindDirection);
	if (!World
		|| DeltaSeconds <= KINDA_SMALL_NUMBER
		|| WindAcceleration <= 0.0f
		|| WindDirection.IsNearlyZero())
	{
		return;
	}

	const float StrengthAlpha = CalculateCurrentWindStrengthAlpha();
	if (StrengthAlpha <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	for (TActorIterator<ASnowRumbleCharacter> It(World); It; ++It)
	{
		ASnowRumbleCharacter* Character = *It;
		if (!ShouldAffectCharacter(Character))
		{
			continue;
		}

		ApplyWindDriftToCharacter(
			*Character,
			DeltaSeconds,
			WindDirection,
			StrengthAlpha);
	}
}

void AIceGlacierWindGustActor::ApplyPredictedWindDriftToLocalPlayer(
	float DeltaSeconds) const
{
	if (CurrentWindState != EIceGlacierWindGustState::Gust
		|| DeltaSeconds <= KINDA_SMALL_NUMBER
		|| WindAcceleration <= 0.0f)
	{
		return;
	}

	UWorld* World = GetWorld();
	const FVector WindDirection = NormalizeWindDirection(CurrentWindDirection);
	if (!World || WindDirection.IsNearlyZero())
	{
		return;
	}

	const float StrengthAlpha = CalculateCurrentWindStrengthAlpha();
	if (StrengthAlpha <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	for (FConstPlayerControllerIterator It =
			World->GetPlayerControllerIterator();
		It;
		++It)
	{
		const APlayerController* PlayerController = It->Get();
		ASnowRumbleCharacter* LocalCharacter =
			PlayerController
				? Cast<ASnowRumbleCharacter>(PlayerController->GetPawn())
				: nullptr;
		if (LocalCharacter
			&& LocalCharacter->IsLocallyControlled()
			&& ShouldAffectCharacter(LocalCharacter))
		{
			ApplyWindDriftToCharacter(
				*LocalCharacter,
				DeltaSeconds,
				WindDirection,
				StrengthAlpha);
		}
	}
}

void AIceGlacierWindGustActor::ApplyWindDriftToCharacter(
	ASnowRumbleCharacter& Character,
	float DeltaSeconds,
	const FVector& WindDirection,
	float StrengthAlpha) const
{
	UCharacterMovementComponent* MovementComponent =
		Character.GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	const float MaxWindDriftSpeed =
		GetMaxWindDriftSpeedForMovementMode(*MovementComponent);
	if (MaxWindDriftSpeed <= 0.0f)
	{
		return;
	}

	const float TargetWindDriftSpeed = MaxWindDriftSpeed * StrengthAlpha;
	const FVector CurrentVelocity = MovementComponent->Velocity;
	const FVector CurrentHorizontalVelocity(
		CurrentVelocity.X,
		CurrentVelocity.Y,
		0.0f);
	const float CurrentSpeedAlongWind = FVector::DotProduct(
		CurrentHorizontalVelocity,
		WindDirection);
	const float MissingWindDriftSpeed =
		TargetWindDriftSpeed - CurrentSpeedAlongWind;
	if (MissingWindDriftSpeed <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float WindDriftSpeedToAdd = FMath::Min(
		WindAcceleration * DeltaSeconds,
		MissingWindDriftSpeed);
	FVector NewVelocity =
		CurrentVelocity + WindDirection * WindDriftSpeedToAdd;
	NewVelocity.Z = CurrentVelocity.Z;
	MovementComponent->Velocity = NewVelocity;
}

void AIceGlacierWindGustActor::UpdateLocalSnowVfxWindDirection() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector SnowEffectDirection =
		CurrentWindState == EIceGlacierWindGustState::Gust
			? NormalizeWindDirection(CurrentWindDirection)
			: FVector::DownVector;

	for (FConstPlayerControllerIterator It =
			World->GetPlayerControllerIterator();
		It;
		++It)
	{
		const APlayerController* PlayerController = It->Get();
		ASnowRumbleCharacter* LocalCharacter =
			PlayerController
				? Cast<ASnowRumbleCharacter>(PlayerController->GetPawn())
				: nullptr;
		if (LocalCharacter && LocalCharacter->IsLocallyControlled())
		{
			LocalCharacter->SetLocalSnowEffectWindDirection(
				SnowEffectDirection);
		}
	}
}

float AIceGlacierWindGustActor::GetMaxWindDriftSpeedForMovementMode(
	const UCharacterMovementComponent& MovementComponent) const
{
	if (MovementComponent.IsMovingOnGround())
	{
		return GroundWindMaxDriftSpeed;
	}

	if (MovementComponent.IsFalling())
	{
		return FallingWindMaxDriftSpeed;
	}

	return 0.0f;
}

float AIceGlacierWindGustActor::CalculateCurrentWindStrengthAlpha() const
{
	if (CurrentWindState != EIceGlacierWindGustState::Gust)
	{
		return 0.0f;
	}

	const float CurrentServerTime = GetServerWorldTimeSeconds();
	const float GustElapsedSeconds = FMath::Max(
		0.0f,
		CurrentServerTime - CurrentGustStartServerTime);
	const float GustRemainingSeconds = FMath::Max(
		0.0f,
		CurrentStateEndServerTime - CurrentServerTime);

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
		OnWindWarningStarted(CurrentWindDirection);
	}
	else if (NewState == EIceGlacierWindGustState::Gust)
	{
		OnWindGustStarted(CurrentWindDirection, GroundWindMaxDriftSpeed);
	}
	else if (PreviousState == EIceGlacierWindGustState::Gust)
	{
		OnWindGustEnded();
	}
}

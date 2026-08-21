// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleIceGlacierCollapseActor_J.h"

#include "../Game/SnowRumbleGameState_C.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Net/UnrealNetwork.h"

ASnowRumbleIceGlacierCollapseActor::ASnowRumbleIceGlacierCollapseActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false);

	RootSceneComponent =
		CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootSceneComponent);
}

void ASnowRumbleIceGlacierCollapseActor::BeginPlay()
{
	Super::BeginPlay();

	InitializeCollapseTargets();
	ApplyCollapseState(GetRoundElapsedSeconds());
}

void ASnowRumbleIceGlacierCollapseActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float RoundElapsedSeconds = GetRoundElapsedSeconds();
	if (HasAuthority())
	{
		const ESnowRumbleIceGlacierCollapsePhase NewPhase =
			CalculateCollapsePhase(RoundElapsedSeconds);
		if (CurrentCollapsePhase != NewPhase)
		{
			CurrentCollapsePhase = NewPhase;
			ForceNetUpdate();
		}
	}

	ApplyCollapseState(RoundElapsedSeconds);
}

void ASnowRumbleIceGlacierCollapseActor::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASnowRumbleIceGlacierCollapseActor, CurrentCollapsePhase);
}

ESnowRumbleIceGlacierCollapsePhase
ASnowRumbleIceGlacierCollapseActor::GetCurrentCollapsePhase() const
{
	return CurrentCollapsePhase;
}

float ASnowRumbleIceGlacierCollapseActor::GetGroup1Alpha() const
{
	return CalculateGroup1Alpha(GetRoundElapsedSeconds());
}

float ASnowRumbleIceGlacierCollapseActor::GetGroup2Alpha() const
{
	return CalculateGroup2Alpha(GetRoundElapsedSeconds());
}

void ASnowRumbleIceGlacierCollapseActor::OnRep_CollapsePhase()
{
	ApplyCollapseState(GetRoundElapsedSeconds());
}

void ASnowRumbleIceGlacierCollapseActor::InitializeCollapseTargets()
{
	Group1TargetStates.Reset();
	Group2TargetStates.Reset();

	InitializeCollapseGroup(Group1Pieces, Group1TargetStates);
	InitializeCollapseGroup(Group2Pieces, Group2TargetStates);
}

void ASnowRumbleIceGlacierCollapseActor::InitializeCollapseGroup(
	const TArray<FIceGlacierCollapsePiece>& SourcePieces,
	TArray<FIceGlacierCollapseTargetState>& OutTargetStates)
{
	for (const FIceGlacierCollapsePiece& Piece : SourcePieces)
	{
		AActor* TargetActor = Piece.TargetActor.Get();
		if (!IsValid(TargetActor))
		{
			continue;
		}

		FIceGlacierCollapseTargetState TargetState;
		TargetState.Actor = TargetActor;
		TargetState.PieceSettings.StartDelaySeconds = Piece.StartDelaySeconds;
		TargetState.PieceSettings.WarningDurationSeconds =
			Piece.WarningDurationSeconds;
		TargetState.PieceSettings.ShakeAmplitude = Piece.ShakeAmplitude;
		TargetState.PieceSettings.ShakeFrequency = Piece.ShakeFrequency;
		TargetState.PieceSettings.FallDurationSeconds = Piece.FallDurationSeconds;
		TargetState.PieceSettings.SinkDistance = Piece.SinkDistance;
		TargetState.InitialTransform = TargetActor->GetActorTransform();

		TArray<UPrimitiveComponent*> PrimitiveComponents;
		TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
		for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
		{
			if (!PrimitiveComponent)
			{
				continue;
			}

			FIceGlacierCollapseComponentCollisionState CollisionState;
			CollisionState.Component = PrimitiveComponent;
			CollisionState.CollisionEnabled =
				PrimitiveComponent->GetCollisionEnabled();
			TargetState.CollisionStates.Add(CollisionState);
		}

		OutTargetStates.Add(TargetState);
	}
}

ESnowRumbleIceGlacierCollapsePhase
ASnowRumbleIceGlacierCollapseActor::CalculateCollapsePhase(
	float RoundElapsedSeconds) const
{
	if (RoundElapsedSeconds < Group1StartSeconds)
	{
		return ESnowRumbleIceGlacierCollapsePhase::Stable;
	}
	if (RoundElapsedSeconds < Group1EndSeconds)
	{
		return ESnowRumbleIceGlacierCollapsePhase::Group1Sinking;
	}
	if (RoundElapsedSeconds < Group2StartSeconds)
	{
		return ESnowRumbleIceGlacierCollapsePhase::Group1Complete;
	}
	if (RoundElapsedSeconds < Group2EndSeconds)
	{
		return ESnowRumbleIceGlacierCollapsePhase::Group2Sinking;
	}

	return ESnowRumbleIceGlacierCollapsePhase::Complete;
}

float ASnowRumbleIceGlacierCollapseActor::CalculateAlpha(
	float RoundElapsedSeconds,
	float StartSeconds,
	float EndSeconds) const
{
	const float DurationSeconds =
		FMath::Max(KINDA_SMALL_NUMBER, EndSeconds - StartSeconds);
	return FMath::Clamp(
		(RoundElapsedSeconds - StartSeconds) / DurationSeconds,
		0.0f,
		1.0f);
}

float ASnowRumbleIceGlacierCollapseActor::CalculateGroup1Alpha(
	float RoundElapsedSeconds) const
{
	if (RoundElapsedSeconds < Group1StartSeconds)
	{
		return 0.0f;
	}

	switch (CurrentCollapsePhase)
	{
	case ESnowRumbleIceGlacierCollapsePhase::Group1Sinking:
		return CalculateAlpha(
			RoundElapsedSeconds,
			Group1StartSeconds,
			Group1EndSeconds);
	case ESnowRumbleIceGlacierCollapsePhase::Group1Complete:
	case ESnowRumbleIceGlacierCollapsePhase::Group2Sinking:
	case ESnowRumbleIceGlacierCollapsePhase::Complete:
		return 1.0f;
	case ESnowRumbleIceGlacierCollapsePhase::Stable:
	default:
		return 0.0f;
	}
}

float ASnowRumbleIceGlacierCollapseActor::CalculateGroup2Alpha(
	float RoundElapsedSeconds) const
{
	if (RoundElapsedSeconds < Group2StartSeconds)
	{
		return 0.0f;
	}

	switch (CurrentCollapsePhase)
	{
	case ESnowRumbleIceGlacierCollapsePhase::Group2Sinking:
		return CalculateAlpha(
			RoundElapsedSeconds,
			Group2StartSeconds,
			Group2EndSeconds);
	case ESnowRumbleIceGlacierCollapsePhase::Complete:
		return 1.0f;
	case ESnowRumbleIceGlacierCollapsePhase::Stable:
	case ESnowRumbleIceGlacierCollapsePhase::Group1Sinking:
	case ESnowRumbleIceGlacierCollapsePhase::Group1Complete:
	default:
		return 0.0f;
	}
}

void ASnowRumbleIceGlacierCollapseActor::ApplyCollapseState(
	float RoundElapsedSeconds)
{
	float Group1TimeSeconds = Group1StartSeconds - 1.0f;
	float Group2TimeSeconds = Group2StartSeconds - 1.0f;

	switch (CurrentCollapsePhase)
	{
	case ESnowRumbleIceGlacierCollapsePhase::Group1Sinking:
		Group1TimeSeconds = RoundElapsedSeconds;
		break;
	case ESnowRumbleIceGlacierCollapsePhase::Group1Complete:
		Group1TimeSeconds = Group1EndSeconds;
		break;
	case ESnowRumbleIceGlacierCollapsePhase::Group2Sinking:
		Group1TimeSeconds = Group1EndSeconds;
		Group2TimeSeconds = RoundElapsedSeconds;
		break;
	case ESnowRumbleIceGlacierCollapsePhase::Complete:
		Group1TimeSeconds = Group1EndSeconds;
		Group2TimeSeconds = Group2EndSeconds;
		break;
	case ESnowRumbleIceGlacierCollapsePhase::Stable:
	default:
		break;
	}

	ApplyCollapseGroup(
		Group1TargetStates,
		Group1TimeSeconds,
		Group1StartSeconds,
		Group1EndSeconds);
	ApplyCollapseGroup(
		Group2TargetStates,
		Group2TimeSeconds,
		Group2StartSeconds,
		Group2EndSeconds);
}

void ASnowRumbleIceGlacierCollapseActor::ApplyCollapseGroup(
	TArray<FIceGlacierCollapseTargetState>& TargetStates,
	float RoundElapsedSeconds,
	float GroupStartSeconds,
	float GroupEndSeconds)
{
	for (FIceGlacierCollapseTargetState& TargetState : TargetStates)
	{
		float WarningAlpha = 0.0f;
		float FallAlpha = 0.0f;
		const EIceGlacierCollapsePieceState PieceState = CalculatePieceState(
			TargetState.PieceSettings,
			RoundElapsedSeconds,
			GroupStartSeconds,
			GroupEndSeconds,
			WarningAlpha,
			FallAlpha);

		switch (PieceState)
		{
		case EIceGlacierCollapsePieceState::Warning:
		{
			const float WarningStartSeconds = GroupStartSeconds +
				FMath::Max(0.0f, TargetState.PieceSettings.StartDelaySeconds);
			const float WarningElapsedSeconds =
				FMath::Max(0.0f, RoundElapsedSeconds - WarningStartSeconds);
			ApplyWarningTransform(
				TargetState,
				WarningElapsedSeconds,
				WarningAlpha);
			RestoreTargetCollision(TargetState);
			break;
		}
		case EIceGlacierCollapsePieceState::Falling:
			ApplyFallingTransform(TargetState, FallAlpha);
			RestoreTargetCollision(TargetState);
			break;
		case EIceGlacierCollapsePieceState::Done:
			ApplyFallingTransform(TargetState, 1.0f);
			if (bDisableCollisionAfterFullySunk)
			{
				DisableTargetCollision(TargetState);
			}
			else
			{
				RestoreTargetCollision(TargetState);
			}
			break;
		case EIceGlacierCollapsePieceState::Idle:
		default:
			ApplyInitialTransform(TargetState);
			RestoreTargetCollision(TargetState);
			break;
		}
	}
}

EIceGlacierCollapsePieceState
ASnowRumbleIceGlacierCollapseActor::CalculatePieceState(
	const FIceGlacierCollapsePieceRuntimeSettings& PieceSettings,
	float RoundElapsedSeconds,
	float GroupStartSeconds,
	float GroupEndSeconds,
	float& OutWarningAlpha,
	float& OutFallAlpha) const
{
	OutWarningAlpha = 0.0f;
	OutFallAlpha = 0.0f;

	if (RoundElapsedSeconds >= GroupEndSeconds)
	{
		OutFallAlpha = 1.0f;
		return EIceGlacierCollapsePieceState::Done;
	}

	const float WarningStartSeconds = GroupStartSeconds +
		FMath::Max(0.0f, PieceSettings.StartDelaySeconds);
	if (RoundElapsedSeconds < WarningStartSeconds)
	{
		return EIceGlacierCollapsePieceState::Idle;
	}

	const float WarningDurationSeconds =
		FMath::Max(0.0f, PieceSettings.WarningDurationSeconds);
	const float WarningEndSeconds =
		WarningStartSeconds + WarningDurationSeconds;
	if (WarningDurationSeconds > KINDA_SMALL_NUMBER &&
		RoundElapsedSeconds < WarningEndSeconds)
	{
		OutWarningAlpha = FMath::Clamp(
			(RoundElapsedSeconds - WarningStartSeconds) /
				WarningDurationSeconds,
			0.0f,
			1.0f);
		return EIceGlacierCollapsePieceState::Warning;
	}

	OutWarningAlpha = 1.0f;

	const float FallDurationSeconds =
		FMath::Max(0.0f, PieceSettings.FallDurationSeconds);
	if (FallDurationSeconds <= KINDA_SMALL_NUMBER)
	{
		OutFallAlpha = 1.0f;
		return EIceGlacierCollapsePieceState::Done;
	}

	const float FallEndSeconds = WarningEndSeconds + FallDurationSeconds;
	if (RoundElapsedSeconds < FallEndSeconds)
	{
		OutFallAlpha = FMath::Clamp(
			(RoundElapsedSeconds - WarningEndSeconds) / FallDurationSeconds,
			0.0f,
			1.0f);
		return EIceGlacierCollapsePieceState::Falling;
	}

	OutFallAlpha = 1.0f;
	return EIceGlacierCollapsePieceState::Done;
}

void ASnowRumbleIceGlacierCollapseActor::ApplyWarningTransform(
	FIceGlacierCollapseTargetState& TargetState,
	float WarningElapsedSeconds,
	float WarningAlpha)
{
	AActor* TargetActor = TargetState.Actor.Get();
	if (!IsValid(TargetActor))
	{
		return;
	}

	const float ShakeAmplitude = TargetState.PieceSettings.ShakeAmplitude;
	const float ShakeFrequency = TargetState.PieceSettings.ShakeFrequency;
	if (ShakeAmplitude <= KINDA_SMALL_NUMBER ||
		ShakeFrequency <= KINDA_SMALL_NUMBER)
	{
		ApplyInitialTransform(TargetState);
		return;
	}

	const float ClampedWarningAlpha = FMath::Clamp(WarningAlpha, 0.0f, 1.0f);
	const float ShakeStrength = ClampedWarningAlpha * ClampedWarningAlpha;
	const float ShakePhase = WarningElapsedSeconds * ShakeFrequency * 2.0f * PI;

	FTransform NewTransform = TargetState.InitialTransform;
	FVector NewLocation = TargetState.InitialTransform.GetLocation();
	NewLocation.X += FMath::Sin(ShakePhase) * ShakeAmplitude * 0.65f * ShakeStrength;
	NewLocation.Y += FMath::Cos(ShakePhase * 1.31f) * ShakeAmplitude * 0.45f * ShakeStrength;
	NewTransform.SetLocation(NewLocation);

	const float RotationAmplitudeDegrees =
		FMath::Min(1.5f, ShakeAmplitude * 0.08f) * ShakeStrength;
	FRotator NewRotation = TargetState.InitialTransform.GetRotation().Rotator();
	NewRotation.Roll +=
		FMath::Sin(ShakePhase * 0.87f) * RotationAmplitudeDegrees;
	NewRotation.Pitch +=
		FMath::Cos(ShakePhase * 1.11f) * RotationAmplitudeDegrees;
	NewTransform.SetRotation(NewRotation.Quaternion());

	TargetActor->SetActorTransform(
		NewTransform,
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
}

void ASnowRumbleIceGlacierCollapseActor::ApplyFallingTransform(
	FIceGlacierCollapseTargetState& TargetState,
	float Alpha)
{
	AActor* TargetActor = TargetState.Actor.Get();
	if (!IsValid(TargetActor))
	{
		return;
	}

	FTransform NewTransform = TargetState.InitialTransform;
	FVector NewLocation = TargetState.InitialTransform.GetLocation();
	NewLocation.Z -= TargetState.PieceSettings.SinkDistance *
		FMath::Clamp(Alpha, 0.0f, 1.0f);
	NewTransform.SetLocation(NewLocation);
	TargetActor->SetActorTransform(
		NewTransform,
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
}

void ASnowRumbleIceGlacierCollapseActor::ApplyInitialTransform(
	FIceGlacierCollapseTargetState& TargetState)
{
	AActor* TargetActor = TargetState.Actor.Get();
	if (!IsValid(TargetActor))
	{
		return;
	}

	TargetActor->SetActorTransform(
		TargetState.InitialTransform,
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
}

void ASnowRumbleIceGlacierCollapseActor::DisableTargetCollision(
	FIceGlacierCollapseTargetState& TargetState)
{
	if (TargetState.bCollisionDisabledAfterSink)
	{
		return;
	}

	for (const FIceGlacierCollapseComponentCollisionState& CollisionState :
		TargetState.CollisionStates)
	{
		if (UPrimitiveComponent* PrimitiveComponent =
			CollisionState.Component.Get())
		{
			PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	TargetState.bCollisionDisabledAfterSink = true;
}

void ASnowRumbleIceGlacierCollapseActor::RestoreTargetCollision(
	FIceGlacierCollapseTargetState& TargetState)
{
	if (!TargetState.bCollisionDisabledAfterSink)
	{
		return;
	}

	for (const FIceGlacierCollapseComponentCollisionState& CollisionState :
		TargetState.CollisionStates)
	{
		if (UPrimitiveComponent* PrimitiveComponent =
			CollisionState.Component.Get())
		{
			PrimitiveComponent->SetCollisionEnabled(
				CollisionState.CollisionEnabled);
		}
	}

	TargetState.bCollisionDisabledAfterSink = false;
}

float ASnowRumbleIceGlacierCollapseActor::GetRoundElapsedSeconds() const
{
	const UWorld* World = GetWorld();
	const ASnowRumbleGameState* SnowRumbleGameState = World
		? World->GetGameState<ASnowRumbleGameState>()
		: nullptr;
	return SnowRumbleGameState
		? SnowRumbleGameState->GetRoundElapsedSeconds()
		: 0.0f;
}

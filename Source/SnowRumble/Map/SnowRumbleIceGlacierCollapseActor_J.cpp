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

	InitializeCollapseGroup(CollapseGroup1, Group1TargetStates);
	InitializeCollapseGroup(CollapseGroup2, Group2TargetStates);
}

void ASnowRumbleIceGlacierCollapseActor::InitializeCollapseGroup(
	const TArray<TObjectPtr<AActor>>& SourceActors,
	TArray<FIceGlacierCollapseTargetState>& OutTargetStates)
{
	for (const TObjectPtr<AActor>& TargetActorReference : SourceActors)
	{
		AActor* TargetActor = TargetActorReference.Get();
		if (!IsValid(TargetActor))
		{
			continue;
		}

		FIceGlacierCollapseTargetState TargetState;
		TargetState.Actor = TargetActor;
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
	ApplyCollapseGroup(
		Group1TargetStates,
		CalculateGroup1Alpha(RoundElapsedSeconds));
	ApplyCollapseGroup(
		Group2TargetStates,
		CalculateGroup2Alpha(RoundElapsedSeconds));
}

void ASnowRumbleIceGlacierCollapseActor::ApplyCollapseGroup(
	TArray<FIceGlacierCollapseTargetState>& TargetStates,
	float Alpha)
{
	for (FIceGlacierCollapseTargetState& TargetState : TargetStates)
	{
		ApplyTargetTransform(TargetState, Alpha);

		if (Alpha >= 1.0f && bDisableCollisionAfterFullySunk)
		{
			DisableTargetCollision(TargetState);
		}
		else
		{
			RestoreTargetCollision(TargetState);
		}
	}
}

void ASnowRumbleIceGlacierCollapseActor::ApplyTargetTransform(
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
	NewLocation.Z -= SinkDistance * FMath::Clamp(Alpha, 0.0f, 1.0f);
	NewTransform.SetLocation(NewLocation);
	TargetActor->SetActorTransform(
		NewTransform,
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

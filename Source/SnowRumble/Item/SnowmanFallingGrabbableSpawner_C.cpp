// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowmanFallingGrabbableSpawner_C.h"

#include "../Game/SnowmanModeGameState_K.h"
#include "GrabbablePhysicsObject_C.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "NavigationSystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogSnowmanFallingGrab, Log, All);

ASnowmanFallingGrabbableSpawner::ASnowmanFallingGrabbableSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
	GrabbableObjectClass = AGrabbablePhysicsObject::StaticClass();
}

void ASnowmanFallingGrabbableSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority() || !GetWorld())
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		SnowmanModeStateTimerHandle,
		this,
		&ASnowmanFallingGrabbableSpawner::UpdateSnowmanModeState,
		0.25f,
		true);
}

void ASnowmanFallingGrabbableSpawner::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SnowmanModeStateTimerHandle);
		World->GetTimerManager().ClearTimer(FirstDropTimerHandle);
		World->GetTimerManager().ClearTimer(DropTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void ASnowmanFallingGrabbableSpawner::UpdateSnowmanModeState()
{
	ASnowmanModeGameState* SnowmanGameState =
		GetWorld() ? GetWorld()->GetGameState<ASnowmanModeGameState>() : nullptr;
	if (!SnowmanGameState)
	{
		return;
	}

	if (SnowmanGameState->IsSnowmanModeEnded())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(SnowmanModeStateTimerHandle);
			World->GetTimerManager().ClearTimer(FirstDropTimerHandle);
			World->GetTimerManager().ClearTimer(DropTimerHandle);
		}
		return;
	}

	if (bDropSequenceStarted || !SnowmanGameState->IsSnowmanModeTimerActive())
	{
		return;
	}

	bDropSequenceStarted = true;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			FirstDropTimerHandle,
			this,
			&ASnowmanFallingGrabbableSpawner::SpawnFallingGrabbableObject,
			FMath::Max(0.0f, FirstDropDelaySeconds),
			false);
	}
}

bool ASnowmanFallingGrabbableSpawner::IsSnowmanModeFinished() const
{
	const ASnowmanModeGameState* SnowmanGameState =
		GetWorld() ? GetWorld()->GetGameState<ASnowmanModeGameState>() : nullptr;
	return !SnowmanGameState
		|| SnowmanGameState->IsSnowmanModeEnded()
		|| !SnowmanGameState->IsSnowmanModeTimerActive();
}

void ASnowmanFallingGrabbableSpawner::SpawnFallingGrabbableObject()
{
	if (!HasAuthority() || !GetWorld() || IsSnowmanModeFinished())
	{
		return;
	}

	if (CandidateStaticMeshes.IsEmpty())
	{
		UE_LOG(LogSnowmanFallingGrab, Warning,
			TEXT("No candidate Static Mesh is configured."));
	}
	else
	{
		UNavigationSystemV1* NavigationSystem =
			FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		if (!NavigationSystem)
		{
			UE_LOG(LogSnowmanFallingGrab, Warning,
				TEXT("Navigation system is missing."));
		}
		else
		{
			FNavLocation NavLocation;
			const bool bFoundLocation =
				NavigationSystem->GetRandomReachablePointInRadius(
					GetActorLocation(),
					NavMeshSearchRadius,
					NavLocation);
			if (!bFoundLocation)
			{
				UE_LOG(LogSnowmanFallingGrab, Warning,
					TEXT("No reachable NavMesh point found near the spawner."));
			}
			else if (!*GrabbableObjectClass)
			{
				UE_LOG(LogSnowmanFallingGrab, Warning,
					TEXT("Grabbable object class is missing."));
			}
			else
			{
				const int32 CandidateIndex =
					FMath::RandRange(0, CandidateStaticMeshes.Num() - 1);
				UStaticMesh* CandidateMesh = CandidateStaticMeshes[CandidateIndex];
				if (CandidateMesh)
				{
					FActorSpawnParameters SpawnParameters;
					SpawnParameters.SpawnCollisionHandlingOverride =
						ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
					AGrabbablePhysicsObject* SpawnedObject =
						GetWorld()->SpawnActor<AGrabbablePhysicsObject>(
							GrabbableObjectClass,
							NavLocation.Location + FVector::UpVector * DropHeightOffset,
							FRotator::ZeroRotator,
							SpawnParameters);
					if (SpawnedObject)
					{
						TArray<UMaterialInterface*> EmptyMaterials;
						SpawnedObject->ConfigureReplicatedVisuals(
							CandidateMesh,
							EmptyMaterials);
						SpawnedObject->ConfigureInteractionSettings(
							PlayerPushStrength,
							InteractionsToBreak,
							InteractionBreakEffect,
							InteractionBreakSound,
							InteractionBreakSoundAttenuation);
						SpawnedObject->SetReplicates(true);
						SpawnedObject->SetReplicateMovement(true);
						SpawnedObject->ForceNetUpdate();
					}
				}
			}
		}
	}

	if (UWorld* World = GetWorld(); World && !IsSnowmanModeFinished())
	{
		World->GetTimerManager().SetTimer(
			DropTimerHandle,
			this,
			&ASnowmanFallingGrabbableSpawner::SpawnFallingGrabbableObject,
			FMath::Max(0.1f, DropIntervalSeconds),
			false);
	}
}

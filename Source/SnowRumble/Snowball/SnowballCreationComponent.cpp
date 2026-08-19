// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowballCreationComponent.h"

#include "../Player/SnowRumbleCharacter.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "SnowballItem.h"
#include "SnowballEquipmentComponent.h"
#include "TimerManager.h"

USnowballCreationComponent::USnowballCreationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void USnowballCreationComponent::StartCreatingSnowball()
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn
		|| (!OwningPawn->HasAuthority() && !OwningPawn->IsLocallyControlled()))
	{
		return;
	}

	const UCameraComponent* FollowCamera =
		OwningPawn->FindComponentByClass<UCameraComponent>();
	if (!FollowCamera)
	{
		return;
	}

	const FVector ViewLocation = FollowCamera->GetComponentLocation();
	const FVector ViewDirection = FollowCamera->GetForwardVector();

	if (bIsCreating)
	{
		return;
	}

	if (OwningPawn->HasAuthority())
	{
		ServerStartCreatingSnowball_Implementation(
			ViewLocation,
			ViewDirection);
		return;
	}

	ServerStartCreatingSnowball(ViewLocation, ViewDirection);
}

void USnowballCreationComponent::CancelCreatingSnowball()
{
	const APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn
		|| (!OwningPawn->HasAuthority() && !OwningPawn->IsLocallyControlled()))
	{
		return;
	}

	if (OwningPawn->HasAuthority())
	{
		ServerCancelCreatingSnowball_Implementation();
		return;
	}

	ServerCancelCreatingSnowball();
}

bool USnowballCreationComponent::IsCreatingSnowball() const
{
	return bIsCreating;
}

float USnowballCreationComponent::GetCreationProgress() const
{
	const ASnowRumbleCharacter* Character =
		Cast<ASnowRumbleCharacter>(GetOwner());
	const float EffectiveCreationDuration =
		CreationDuration
		* (Character
			? Character->GetSnowballCreationDurationMultiplier()
			: 1.0f);
	if (!bIsCreating || EffectiveCreationDuration <= 0.0f)
	{
		return 0.0f;
	}

	const UWorld* World = GetWorld();
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	const float CurrentServerTime =
		GameState
			? GameState->GetServerWorldTimeSeconds()
			: World
				? World->GetTimeSeconds()
				: CreationStartServerTime;

	return FMath::Clamp(
		(CurrentServerTime - CreationStartServerTime)
			/ EffectiveCreationDuration,
		0.0f,
		1.0f);
}

void USnowballCreationComponent::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USnowballCreationComponent, bIsCreating);
	DOREPLIFETIME(USnowballCreationComponent, CreationStartServerTime);
}

void USnowballCreationComponent::ServerStartCreatingSnowball_Implementation(
	FVector_NetQuantize ViewLocation,
	FVector_NetQuantizeNormal ViewDirection)
{
	ASnowRumbleCharacter* Character = Cast<ASnowRumbleCharacter>(GetOwner());
	USnowballEquipmentComponent* Equipment =
		Character
			? Character->FindComponentByClass<USnowballEquipmentComponent>()
			: nullptr;
	FHitResult SurfaceHit;
	const bool bCameraOriginValid =
		Character
		&& FVector::DistSquared(ViewLocation, Character->GetActorLocation())
			<= FMath::Square(MaxCameraOriginDistance);
	const bool bSurfaceHit =
		Character
		&& FindSnowSurface(ViewLocation, ViewDirection, SurfaceHit);
	const bool bCanStartCreation =
		Character
		&& !bIsCreating
		&& !Character->IsFrozen()
		&& !Character->IsAiming()
		&& (!Equipment || !Equipment->HasHeldSnowball())
		&& SnowballItemClass
		&& bCameraOriginValid
		&& bSurfaceHit;

	if (!bCanStartCreation)
	{
		return;
	}

	CreationSurfaceActor = SurfaceHit.GetActor();
	CreationSurfacePoint = SurfaceHit.ImpactPoint;
	CreationSurfaceNormal = SurfaceHit.ImpactNormal.GetSafeNormal();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	CreationStartServerTime = World->GetTimeSeconds();
	SetCreatingState(true);
	const float EffectiveCreationDuration =
		CreationDuration
		* Character->GetSnowballCreationDurationMultiplier();
	World->GetTimerManager().SetTimer(
		CreationTimerHandle,
		this,
		&USnowballCreationComponent::CompleteCreation,
		FMath::Max(0.01f, EffectiveCreationDuration),
		false);
	Character->ForceNetUpdate();
}

void USnowballCreationComponent::ServerCancelCreatingSnowball_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CreationTimerHandle);
	}

	SetCreatingState(false);

	if (AActor* OwningActor = GetOwner())
	{
		OwningActor->ForceNetUpdate();
	}

}

void USnowballCreationComponent::OnRep_IsCreating()
{
	OnCreatingChanged.Broadcast(bIsCreating);
}

void USnowballCreationComponent::CompleteCreation()
{
	ASnowRumbleCharacter* Character = Cast<ASnowRumbleCharacter>(GetOwner());
	USnowballEquipmentComponent* Equipment =
		Character
			? Character->FindComponentByClass<USnowballEquipmentComponent>()
			: nullptr;
	if (!Character
		|| Character->IsFrozen()
		|| Character->IsAiming()
		|| (Equipment && Equipment->HasHeldSnowball())
		|| !SnowballItemClass
		|| !CreationSurfaceActor.IsValid()
		|| !CreationSurfaceActor->ActorHasTag(SnowSurfaceTag))
	{
		SetCreatingState(false);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		SetCreatingState(false);
		return;
	}

	const FVector ForwardLocation =
		Character->GetActorLocation()
		+ Character->GetActorForwardVector() * CreationForwardDistance;
	const FVector SpawnLocation =
		FVector::PointPlaneProject(
			ForwardLocation,
			CreationSurfacePoint,
			CreationSurfaceNormal)
		+ CreationSurfaceNormal * 20.0f;
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Character;
	SpawnParameters.Instigator = Character;
	SpawnParameters.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASnowballItem* CreatedSnowball = World->SpawnActor<ASnowballItem>(
		SnowballItemClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParameters);

	if (CreatedSnowball)
	{
		CreatedSnowball->IgnoreActorTemporarily(
			Character,
			CreatedSnowballOwnerCollisionIgnoreSeconds);
	}

	if (CreatedSnowball
		&& Character->HasEquippedSnowDuckMaker()
		&& Equipment
		&& Equipment->EquipCreatedSnowballFromServer(CreatedSnowball))
	{
		SetCreatingState(false);
		Character->ForceNetUpdate();
		return;
	}

	SetCreatingState(false);
	Character->ForceNetUpdate();
}

bool USnowballCreationComponent::FindSnowSurface(
	const FVector& ViewLocation,
	const FVector& ViewDirection,
	FHitResult& OutHit) const
{
	const ASnowRumbleCharacter* Character = Cast<ASnowRumbleCharacter>(GetOwner());
	UWorld* World = GetWorld();
	if (!Character || !World || ViewDirection.IsNearlyZero())
	{
		return false;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SnowballCreationTrace), false, Character);
	QueryParams.AddIgnoredActor(Character);

	TArray<AActor*> AttachedActors;
	Character->GetAttachedActors(AttachedActors, true, true);
	QueryParams.AddIgnoredActors(AttachedActors);

	const float EffectiveTraceDistance =
		CreationTraceDistance
		+ FVector::Distance(ViewLocation, Character->GetActorLocation());
	const FVector TraceEnd =
		ViewLocation + ViewDirection.GetSafeNormal() * EffectiveTraceDistance;
	const bool bHit = World->LineTraceSingleByChannel(
		OutHit,
		ViewLocation,
		TraceEnd,
		ECC_Visibility,
		QueryParams);

	const bool bHitSnowSurface =
		bHit
		&& OutHit.GetActor()
		&& OutHit.GetActor()->ActorHasTag(SnowSurfaceTag);

	return bHitSnowSurface;
}

void USnowballCreationComponent::SetCreatingState(bool bNewCreating)
{
	if (bIsCreating == bNewCreating)
	{
		return;
	}

	bIsCreating = bNewCreating;
	if (!bIsCreating)
	{
		CreationStartServerTime = 0.0f;
		CreationSurfaceActor.Reset();
		CreationSurfacePoint = FVector::ZeroVector;
		CreationSurfaceNormal = FVector::UpVector;
	}
	OnRep_IsCreating();
}

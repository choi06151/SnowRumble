// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowballCreationComponent.h"

#include "../Player/SnowRumbleCharacter.h"
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
	const APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn
		|| (!OwningPawn->HasAuthority() && !OwningPawn->IsLocallyControlled())
		|| bIsCreating)
	{
		return;
	}

	if (OwningPawn->HasAuthority())
	{
		ServerStartCreatingSnowball_Implementation();
		return;
	}

	ServerStartCreatingSnowball();
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
	if (!bIsCreating || CreationDuration <= 0.0f)
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
		(CurrentServerTime - CreationStartServerTime) / CreationDuration,
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

void USnowballCreationComponent::ServerStartCreatingSnowball_Implementation()
{
	ASnowRumbleCharacter* Character = Cast<ASnowRumbleCharacter>(GetOwner());
	const USnowballEquipmentComponent* Equipment =
		Character
			? Character->FindComponentByClass<USnowballEquipmentComponent>()
			: nullptr;
	FHitResult SurfaceHit;
	if (!Character
		|| Character->IsFrozen()
		|| Character->IsAiming()
		|| (Equipment && Equipment->HasHeldSnowball())
		|| !SnowballItemClass
		|| !FindSnowSurface(SurfaceHit))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	CreationStartServerTime = World->GetTimeSeconds();
	SetCreatingState(true);
	World->GetTimerManager().SetTimer(
		CreationTimerHandle,
		this,
		&USnowballCreationComponent::CompleteCreation,
		CreationDuration,
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
	const USnowballEquipmentComponent* Equipment =
		Character
			? Character->FindComponentByClass<USnowballEquipmentComponent>()
			: nullptr;
	FHitResult SurfaceHit;
	if (!Character
		|| Character->IsFrozen()
		|| Character->IsAiming()
		|| (Equipment && Equipment->HasHeldSnowball())
		|| !SnowballItemClass
		|| !FindSnowSurface(SurfaceHit))
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

	const FVector SpawnLocation = SurfaceHit.ImpactPoint + SurfaceHit.ImpactNormal * 20.0f;
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Character;
	SpawnParameters.Instigator = Character;
	SpawnParameters.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	World->SpawnActor<ASnowballItem>(
		SnowballItemClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParameters);

	SetCreatingState(false);
	Character->ForceNetUpdate();
}

bool USnowballCreationComponent::FindSnowSurface(FHitResult& OutHit) const
{
	const ASnowRumbleCharacter* Character = Cast<ASnowRumbleCharacter>(GetOwner());
	const AController* Controller = Character ? Character->GetController() : nullptr;
	UWorld* World = GetWorld();
	if (!Character || !Controller || !World)
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SnowballCreationTrace), false, Character);
	const bool bHit = World->LineTraceSingleByChannel(
		OutHit,
		ViewLocation,
		ViewLocation + ViewRotation.Vector() * CreationTraceDistance,
		ECC_Visibility,
		QueryParams);

	return bHit
		&& OutHit.GetActor()
		&& OutHit.GetActor()->ActorHasTag(SnowSurfaceTag);
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
	}
	OnRep_IsCreating();
}

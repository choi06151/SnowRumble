#include "GrabbableStaticMeshBootstrapActor_C.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GrabbablePhysicsObject_C.h"
#include "Kismet/GameplayStatics.h"

AGrabbableStaticMeshBootstrapActor::AGrabbableStaticMeshBootstrapActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	GrabbableObjectClass = AGrabbablePhysicsObject::StaticClass();
	GrabbableTags.Add(TEXT("grabbable"));
	GrabbableTags.Add(TEXT("grabable"));
}

void AGrabbableStaticMeshBootstrapActor::BeginPlay()
{
	Super::BeginPlay();

	ConvertTaggedStaticMeshes();
}

void AGrabbableStaticMeshBootstrapActor::ConvertTaggedStaticMeshes()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<AActor*> LevelActors;
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), LevelActors);

	for (AActor* SourceActor : LevelActors)
	{
		if (!IsValid(SourceActor)
			|| SourceActor == this
			|| SourceActor->IsA<AGrabbablePhysicsObject>()
			|| SourceActor->IsA<AGrabbableStaticMeshBootstrapActor>())
		{
			continue;
		}

		TArray<UStaticMeshComponent*> StaticMeshComponents;
		SourceActor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);

		for (UStaticMeshComponent* SourceComponent : StaticMeshComponents)
		{
			if (!ShouldConvertStaticMeshComponent(SourceActor, SourceComponent))
			{
				continue;
			}

			if (HasAuthority())
			{
				ConvertStaticMeshComponent(SourceActor, SourceComponent);
			}

			ApplyOriginalMeshSuppression(SourceActor, SourceComponent);
		}
	}
}

bool AGrabbableStaticMeshBootstrapActor::ShouldConvertStaticMeshComponent(
	const AActor* SourceActor,
	const UStaticMeshComponent* SourceComponent) const
{
	if (!IsValid(SourceActor) || !IsValid(SourceComponent) || !SourceComponent->GetStaticMesh())
	{
		return false;
	}

	for (const FName& Tag : GrabbableTags)
	{
		if (!Tag.IsNone()
			&& (SourceComponent->ComponentHasTag(Tag)
				|| (bConvertActorTaggedStaticMeshes && SourceActor->ActorHasTag(Tag))))
		{
			return true;
		}
	}

	return false;
}

void AGrabbableStaticMeshBootstrapActor::ConvertStaticMeshComponent(
	AActor* SourceActor,
	UStaticMeshComponent* SourceComponent) const
{
	if (!IsValid(SourceActor)
		|| !IsValid(SourceComponent)
		|| !SourceComponent->GetStaticMesh()
		|| !*GrabbableObjectClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FTransform SourceTransform = SourceComponent->GetComponentTransform();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = SourceActor->GetOwner();
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AGrabbablePhysicsObject* SpawnedObject = World->SpawnActor<AGrabbablePhysicsObject>(
		GrabbableObjectClass,
		SourceTransform,
		SpawnParameters);

	if (!IsValid(SpawnedObject))
	{
		return;
	}

	UStaticMeshComponent* SpawnedMeshComponent = Cast<UStaticMeshComponent>(SpawnedObject->GetPhysicsComponent());
	if (!IsValid(SpawnedMeshComponent))
	{
		SpawnedObject->Destroy();
		return;
	}

	TArray<UMaterialInterface*> SourceMaterials;
	const int32 MaterialCount = SourceComponent->GetNumMaterials();
	SourceMaterials.Reserve(MaterialCount);
	for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
	{
		SourceMaterials.Add(SourceComponent->GetMaterial(MaterialIndex));
	}
	SpawnedObject->ConfigureReplicatedVisuals(
		SourceComponent->GetStaticMesh(),
		SourceMaterials);

	SpawnedMeshComponent->SetWorldTransform(SourceTransform, false, nullptr, ETeleportType::TeleportPhysics);
	SpawnedMeshComponent->SetMobility(EComponentMobility::Movable);
	SpawnedMeshComponent->SetSimulatePhysics(true);
	SpawnedMeshComponent->SetEnableGravity(true);
	SpawnedMeshComponent->SetNotifyRigidBodyCollision(true);

	SpawnedObject->ConfigureInteractionSettings(
		ConvertedPlayerPushStrength,
		ConvertedInteractionsToBreak,
		ConvertedInteractionBreakEffect,
		ConvertedInteractionBreakSound,
		ConvertedInteractionBreakSoundAttenuation);
	SpawnedObject->SetReplicates(true);
	SpawnedObject->SetReplicateMovement(true);
	SpawnedObject->ForceNetUpdate();
}

void AGrabbableStaticMeshBootstrapActor::ApplyOriginalMeshSuppression(
	AActor* SourceActor,
	UStaticMeshComponent* SourceComponent) const
{
	if (!IsValid(SourceActor) || !IsValid(SourceComponent))
	{
		return;
	}

	if (bDisableOriginalCollision)
	{
		SourceComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (bHideOriginalMeshes)
	{
		SourceComponent->SetHiddenInGame(true);
		SourceActor->SetActorHiddenInGame(true);
	}
}

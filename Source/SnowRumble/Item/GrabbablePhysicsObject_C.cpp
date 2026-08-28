// Copyright Epic Games, Inc. All Rights Reserved.

#include "GrabbablePhysicsObject_C.h"

#include "../Audio/SnowRumbleAudioHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "NiagaraFunctionLibrary.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"

AGrabbablePhysicsObject::AGrabbablePhysicsObject()
{
	bReplicates = true;
	SetReplicateMovement(true);
	SetNetUpdateFrequency(30.0f);
	SetMinNetUpdateFrequency(15.0f);

	PhysicsComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PhysicsComponent"));
	PhysicsComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	PhysicsComponent->SetNotifyRigidBodyCollision(true);
	PhysicsComponent->SetSimulatePhysics(true);
	PhysicsComponent->SetEnableGravity(true);
	PhysicsComponent->OnComponentHit.AddDynamic(this, &AGrabbablePhysicsObject::HandleComponentHit);
	RootComponent = PhysicsComponent;
}

UPrimitiveComponent* AGrabbablePhysicsObject::GetPhysicsComponent() const
{
	return PhysicsComponent;
}

bool AGrabbablePhysicsObject::CanBeGrabbed() const
{
	return bCanBeGrabbed && IsValid(PhysicsComponent);
}

int32 AGrabbablePhysicsObject::GetInteractionCount() const
{
	return InteractionCount;
}

void AGrabbablePhysicsObject::ConfigureInteractionSettings(
	float NewPlayerPushStrength,
	int32 NewInteractionsToBreak,
	UNiagaraSystem* NewInteractionBreakEffect,
	USoundBase* NewInteractionBreakSound,
	USoundAttenuation* NewInteractionBreakSoundAttenuation)
{
	PlayerPushStrength = FMath::Max(NewPlayerPushStrength, 0.0f);
	InteractionsToBreak = FMath::Max(NewInteractionsToBreak, 1);
	InteractionBreakEffect = NewInteractionBreakEffect;
	InteractionBreakSound = NewInteractionBreakSound;
	InteractionBreakSoundAttenuation = NewInteractionBreakSoundAttenuation;
}

void AGrabbablePhysicsObject::ConfigureReplicatedVisuals(
	UStaticMesh* NewStaticMesh,
	const TArray<UMaterialInterface*>& NewMaterials)
{
	if (!HasAuthority())
	{
		return;
	}

	ReplicatedStaticMesh = NewStaticMesh;
	ReplicatedMaterials.Reset(NewMaterials.Num());
	for (UMaterialInterface* Material : NewMaterials)
	{
		ReplicatedMaterials.Add(Material);
	}

	ApplyReplicatedVisuals();
	ForceNetUpdate();
}

void AGrabbablePhysicsObject::OnRep_ReplicatedStaticMesh()
{
	ApplyReplicatedVisuals();
}

void AGrabbablePhysicsObject::ApplyReplicatedVisuals()
{
	if (!IsValid(PhysicsComponent))
	{
		return;
	}

	PhysicsComponent->SetStaticMesh(ReplicatedStaticMesh);
	PhysicsComponent->SetMaterial(0, nullptr);
	for (int32 MaterialIndex = 0; MaterialIndex < ReplicatedMaterials.Num(); ++MaterialIndex)
	{
		PhysicsComponent->SetMaterial(MaterialIndex, ReplicatedMaterials[MaterialIndex]);
	}
}

void AGrabbablePhysicsObject::RegisterInteraction()
{
	if (!HasAuthority() || IsActorBeingDestroyed())
	{
		return;
	}

	const UWorld* World = GetWorld();
	const float CurrentTime = World ? World->GetTimeSeconds() : 0.0f;
	if (CurrentTime - LastInteractionTimeSeconds < InteractionCooldownSeconds)
	{
		return;
	}

	LastInteractionTimeSeconds = CurrentTime;
	InteractionCount = FMath::Min(
		InteractionCount + 1,
		FMath::Max(InteractionsToBreak, 1));
	ForceNetUpdate();

	if (InteractionCount >= FMath::Max(InteractionsToBreak, 1))
	{
		const FVector BreakLocation = GetActorLocation();
		MulticastPlayInteractionBreakEffect(BreakLocation);
		MulticastPlayInteractionBreakSound(BreakLocation);
		OnInteractionBreak();
		Destroy();
	}
}

void AGrabbablePhysicsObject::MulticastPlayInteractionBreakEffect_Implementation(
	FVector_NetQuantize Location)
{
	if (InteractionBreakEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			InteractionBreakEffect,
			Location,
			GetActorRotation());
	}
}

void AGrabbablePhysicsObject::MulticastPlayInteractionBreakSound_Implementation(
	FVector_NetQuantize Location)
{
	SnowRumbleAudio::PlaySoundAtLocation(
		this,
		InteractionBreakSound,
		ESnowRumbleAudioMixChannel::Gameplay,
		Location,
		1.0f,
		1.0f,
		InteractionBreakSoundAttenuation);
}

void AGrabbablePhysicsObject::PushCharacterFromGrabMotion(
	ACharacter* Character,
	FVector MotionDirection)
{
	if (!HasAuthority() || PlayerPushStrength <= 0.0f || !Character)
	{
		return;
	}

	FVector PushDirection = MotionDirection.GetSafeNormal2D();
	if (PushDirection.IsNearlyZero())
	{
		PushDirection =
			(Character->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	}
	if (!PushDirection.IsNearlyZero())
	{
		Character->LaunchCharacter(PushDirection * PlayerPushStrength, true, false);
		RegisterInteraction();
	}
}

void AGrabbablePhysicsObject::HandleGrabbedByCharacter(ACharacter* /*Grabber*/)
{
	bIsHeldByCharacter = true;
}

void AGrabbablePhysicsObject::HandleReleasedByCharacter(ACharacter* /*Grabber*/)
{
	bIsHeldByCharacter = false;
}

void AGrabbablePhysicsObject::TickGrabbedByCharacter(
	ACharacter* /*Grabber*/,
	FVector /*HeldMotion*/,
	float /*DeltaTime*/)
{
}

void AGrabbablePhysicsObject::HandleComponentHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!HasAuthority()
		|| !bIsHeldByCharacter
		|| PlayerPushStrength <= 0.0f
		|| !OtherActor)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character)
	{
		return;
	}

	const FVector PushDirection = PhysicsComponent
		? PhysicsComponent->GetPhysicsLinearVelocity().GetSafeNormal2D()
		: FVector::ZeroVector;
	PushCharacterFromGrabMotion(Character, PushDirection);
}

void AGrabbablePhysicsObject::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGrabbablePhysicsObject, InteractionCount);
	DOREPLIFETIME(AGrabbablePhysicsObject, ReplicatedStaticMesh);
	DOREPLIFETIME(AGrabbablePhysicsObject, ReplicatedMaterials);
}

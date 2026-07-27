// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowballItem.h"

#include "../Player/SnowRumbleCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ASnowballItem::ASnowballItem()
{
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(18.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	CollisionComponent->SetSimulatePhysics(true);
	CollisionComponent->SetEnableGravity(true);
	CollisionComponent->OnComponentHit.AddDynamic(this, &ASnowballItem::HandleCollision);
	RootComponent = CollisionComponent;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 0.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->ProjectileGravityScale = 0.25f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->Deactivate();
}

void ASnowballItem::BeginPlay()
{
	Super::BeginPlay();

	InitialActorScale = GetActorScale3D();
	ApplyGrowthScale();
}

bool ASnowballItem::TrySetHeldBy(
	ASnowRumbleCharacter* NewHolder,
	USceneComponent* HoldPoint)
{
	if (!HasAuthority()
		|| !CanBePickedUp()
		|| !NewHolder
		|| !HoldPoint)
	{
		return false;
	}

	Holder = NewHolder;
	ItemState = ESnowballItemState::Held;
	SetOwner(NewHolder);
	RefreshStatePresentation();
	ForceNetUpdate();
	return true;
}

bool ASnowballItem::Throw(const FVector& ThrowDirection, float ThrowSpeed)
{
	if (!HasAuthority()
		|| ItemState != ESnowballItemState::Held
		|| !Holder
		|| !ProjectileMovement
		|| ThrowDirection.IsNearlyZero()
		|| ThrowSpeed <= 0.0f)
	{
		return false;
	}

	ASnowRumbleCharacter* PreviousHolder = Holder;
	ItemState = ESnowballItemState::Thrown;
	Holder = nullptr;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	CollisionComponent->SetSimulatePhysics(false);
	CollisionComponent->SetEnableGravity(false);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->IgnoreActorWhenMoving(PreviousHolder, true);
	SetReplicateMovement(true);

	ProjectileMovement->Velocity = ThrowDirection.GetSafeNormal() * ThrowSpeed;
	ProjectileMovement->Activate(true);
	SetLifeSpan(MaximumThrownLifeSeconds);
	ForceNetUpdate();
	return true;
}

bool ASnowballItem::DropToGround()
{
	if (!HasAuthority()
		|| ItemState != ESnowballItemState::Held
		|| !Holder)
	{
		return false;
	}

	ItemState = ESnowballItemState::Ground;
	Holder = nullptr;
	SetOwner(nullptr);

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	RefreshStatePresentation();
	ForceNetUpdate();
	return true;
}

bool ASnowballItem::TryStartRolling(ASnowRumbleCharacter* NewRoller)
{
	if (!HasAuthority()
		|| !CanBePickedUp()
		|| !NewRoller)
	{
		return false;
	}

	ItemState = ESnowballItemState::Rolling;
	Roller = NewRoller;
	SetOwner(NewRoller);
	LastRollingLocation = GetActorLocation();
	CollisionComponent->IgnoreActorWhenMoving(NewRoller, true);
	RefreshStatePresentation();
	ForceNetUpdate();
	return true;
}

bool ASnowballItem::StopRolling()
{
	if (!HasAuthority()
		|| ItemState != ESnowballItemState::Rolling)
	{
		return false;
	}

	ASnowRumbleCharacter* PreviousRoller = Roller;
	ItemState = ESnowballItemState::Ground;
	Roller = nullptr;
	SetOwner(nullptr);
	CollisionComponent->IgnoreActorWhenMoving(PreviousRoller, false);
	RefreshStatePresentation();
	ForceNetUpdate();
	return true;
}

void ASnowballItem::MoveRollingSnowball(const FVector& TargetLocation)
{
	if (!HasAuthority()
		|| ItemState != ESnowballItemState::Rolling
		|| !Roller
		|| !CollisionComponent
		|| TargetLocation.ContainsNaN())
	{
		return;
	}

	const FVector PreviousLocation = GetActorLocation();
	FHitResult SweepHit;
	SetActorLocation(
		TargetLocation,
		true,
		&SweepHit,
		ETeleportType::None);

	const FVector MovedDelta = GetActorLocation() - PreviousLocation;
	const float MovedDistance = MovedDelta.Size2D();
	if (MovedDistance > KINDA_SMALL_NUMBER)
	{
		const FVector RollDirection = MovedDelta.GetSafeNormal2D();
		const FVector RollAxis = FVector::CrossProduct(FVector::UpVector, RollDirection);
		const float Radius = FMath::Max(CollisionComponent->GetScaledSphereRadius(), 1.0f);
		AddActorWorldRotation(FQuat(RollAxis, MovedDistance / Radius));
	}
}

void ASnowballItem::UpdateRollingGrowth()
{
	if (!HasAuthority()
		|| ItemState != ESnowballItemState::Rolling
		|| DistanceForMaximumGrowth <= 0.0f)
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	AccumulatedRollingDistance += FVector::Dist2D(
		CurrentLocation,
		LastRollingLocation);
	LastRollingLocation = CurrentLocation;

	const float NewGrowthProgress = FMath::Clamp(
		AccumulatedRollingDistance / DistanceForMaximumGrowth,
		0.0f,
		1.0f);
	if (!FMath::IsNearlyEqual(GrowthProgress, NewGrowthProgress, 0.001f))
	{
		GrowthProgress = NewGrowthProgress;
		OnRep_GrowthProgress();
		ForceNetUpdate();
	}
}

float ASnowballItem::GetGrowthProgress() const
{
	return GrowthProgress;
}

bool ASnowballItem::CanBePickedUp() const
{
	return ItemState == ESnowballItemState::Ground && !Holder;
}

ESnowballItemState ASnowballItem::GetItemState() const
{
	return ItemState;
}

ASnowRumbleCharacter* ASnowballItem::GetHolder() const
{
	return Holder;
}

void ASnowballItem::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASnowballItem, ItemState);
	DOREPLIFETIME(ASnowballItem, Holder);
	DOREPLIFETIME(ASnowballItem, Roller);
	DOREPLIFETIME(ASnowballItem, GrowthProgress);
}

void ASnowballItem::OnRep_ItemState()
{
	RefreshStatePresentation();
}

void ASnowballItem::OnRep_Holder()
{
	RefreshStatePresentation();
}

void ASnowballItem::OnRep_GrowthProgress()
{
	ApplyGrowthScale();
}

void ASnowballItem::ApplyGrowthScale()
{
	const float PreviousRadius = CollisionComponent
		? CollisionComponent->GetScaledSphereRadius()
		: 0.0f;
	const float ScaleMultiplier = FMath::Lerp(
		1.0f,
		MaximumScaleMultiplier,
		FMath::Clamp(GrowthProgress, 0.0f, 1.0f));
	SetActorScale3D(InitialActorScale * ScaleMultiplier);

	if (HasAuthority()
		&& ItemState == ESnowballItemState::Rolling
		&& CollisionComponent)
	{
		const float NewRadius = CollisionComponent->GetScaledSphereRadius();
		const float RadiusIncrease = NewRadius - PreviousRadius;
		if (RadiusIncrease > KINDA_SMALL_NUMBER)
		{
			AddActorWorldOffset(
				FVector::UpVector * RadiusIncrease,
				false,
				nullptr,
				ETeleportType::TeleportPhysics);
		}
	}
}

void ASnowballItem::RefreshStatePresentation()
{
	if (ItemState == ESnowballItemState::Held && Holder)
	{
		if (USceneComponent* HoldPoint = Holder->GetSnowballHoldPoint())
		{
			if (ProjectileMovement)
			{
				ProjectileMovement->Deactivate();
				ProjectileMovement->Velocity = FVector::ZeroVector;
			}

			CollisionComponent->SetSimulatePhysics(false);
			CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SetReplicateMovement(false);
			AttachToComponent(
				HoldPoint,
				FAttachmentTransformRules(
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::KeepWorld,
					false));
			CollisionComponent->SetRelativeLocationAndRotation(
				FVector::ZeroVector,
				FRotator::ZeroRotator);
		}
		return;
	}

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (ItemState == ESnowballItemState::Rolling)
	{
		if (ProjectileMovement)
		{
			ProjectileMovement->Deactivate();
			ProjectileMovement->Velocity = FVector::ZeroVector;
		}

		CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CollisionComponent->SetSimulatePhysics(false);
		CollisionComponent->SetEnableGravity(false);
		SetReplicateMovement(true);
		return;
	}

	if (ItemState == ESnowballItemState::Thrown)
	{
		CollisionComponent->SetSimulatePhysics(false);
		CollisionComponent->SetEnableGravity(false);
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SetReplicateMovement(true);
		return;
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->Deactivate();
		ProjectileMovement->Velocity = FVector::ZeroVector;
	}

	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetSimulatePhysics(true);
	CollisionComponent->SetEnableGravity(true);
	CollisionComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
	CollisionComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	CollisionComponent->WakeAllRigidBodies();
	SetReplicateMovement(true);
}

void ASnowballItem::HandleCollision(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!HasAuthority()
		|| ItemState != ESnowballItemState::Thrown
		|| !OtherActor
		|| OtherActor == this
		|| OtherActor == GetOwner())
	{
		return;
	}

	UGameplayStatics::ApplyDamage(
		OtherActor,
		Damage,
		GetInstigatorController(),
		this,
		UDamageType::StaticClass());

	Destroy();
}

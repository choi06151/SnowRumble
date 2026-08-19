// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowballItem.h"

#include "../Player/SnowRumbleCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

ASnowballItem::ASnowballItem()
{
	bReplicates = true;
	SetReplicateMovement(true);
	NetUpdateFrequency = 30.0f;
	MinNetUpdateFrequency = 15.0f;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(18.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	CollisionComponent->SetSimulatePhysics(false);
	CollisionComponent->SetEnableGravity(false);
	CollisionComponent->OnComponentHit.AddDynamic(this, &ASnowballItem::HandleCollision);
	RootComponent = CollisionComponent;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 0.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->ProjectileGravityScale = 0.25f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->bSweepCollision = true;
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->OnProjectileStop.AddDynamic(
		this,
		&ASnowballItem::HandleProjectileStopped);
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
	bIsSettledOnGround = false;
	SetOwner(NewHolder);
	RefreshStatePresentation();
	ForceNetUpdate();
	return true;
}

bool ASnowballItem::Throw(
	const FVector& ThrowDirection,
	float ThrowSpeed,
	float ThrowChargeProgress)
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
	bIsSettledOnGround = false;
	bHasProcessedThrownImpact = false;
	CurrentThrowChargeProgress = FMath::Clamp(ThrowChargeProgress, 0.0f, 1.0f);

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	CollisionComponent->SetSimulatePhysics(false);
	CollisionComponent->SetEnableGravity(false);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->IgnoreActorWhenMoving(PreviousHolder, true);
	SetReplicateMovement(true);

	ProjectileMovement->ProjectileGravityScale =
		IsFullyGrown()
			? LargeSnowballProjectileGravityScale
			: SmallSnowballProjectileGravityScale;
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

	ASnowRumbleCharacter* PreviousHolder = Holder;
	ItemState = ESnowballItemState::Ground;
	Holder = nullptr;
	bIsSettledOnGround = true;
	SetOwner(nullptr);

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	TrySettleOnGroundBelow(PreviousHolder);
	RefreshStatePresentation();
	ForceNetUpdate();
	return true;
}

void ASnowballItem::IgnoreActorTemporarily(
	AActor* ActorToIgnore,
	float DurationSeconds)
{
	if (!HasAuthority()
		|| !CollisionComponent
		|| !ActorToIgnore
		|| DurationSeconds <= 0.0f)
	{
		return;
	}

	TemporarilyIgnoredActors.Add(ActorToIgnore);
	CollisionComponent->IgnoreActorWhenMoving(ActorToIgnore, true);
	if (!bTemporarilyIgnoringPawnCollision)
	{
		CachedPawnCollisionResponse =
			CollisionComponent->GetCollisionResponseToChannel(ECC_Pawn);
		bTemporarilyIgnoringPawnCollision = true;
		CollisionComponent->SetCollisionResponseToChannel(
			ECC_Pawn,
			ECR_Ignore);
	}

	FTimerHandle RestoreTimerHandle;
	FTimerDelegate RestoreDelegate;
	RestoreDelegate.BindUObject(
		this,
		&ASnowballItem::RestoreTemporarilyIgnoredActor,
		TWeakObjectPtr<AActor>(ActorToIgnore));
	GetWorldTimerManager().SetTimer(
		RestoreTimerHandle,
		RestoreDelegate,
		DurationSeconds,
		false);
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
	bIsSettledOnGround = false;
	SetOwner(NewRoller);
	LastRollingLocation = GetActorLocation();
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
	bIsSettledOnGround = true;
	SetOwner(nullptr);
	TrySettleOnGroundBelow(PreviousRoller);
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
	SetActorLocation(
		TargetLocation,
		false,
		nullptr,
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

void ASnowballItem::SettleOnGroundFromSurface(
	const FVector& SurfacePoint,
	const FVector& SurfaceNormal)
{
	if (!HasAuthority()
		|| !CollisionComponent
		|| SurfacePoint.ContainsNaN()
		|| SurfaceNormal.ContainsNaN())
	{
		return;
	}

	const FVector SafeSurfaceNormal =
		SurfaceNormal.IsNearlyZero()
			? FVector::UpVector
			: SurfaceNormal.GetSafeNormal();
	const float Radius = CollisionComponent->GetScaledSphereRadius();
	const FVector SettledLocation =
		SurfacePoint
		+ SafeSurfaceNormal * (Radius + GroundSettleExtraClearance);

	bIsSettledOnGround = true;
	SetActorLocation(
		SettledLocation,
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
	RefreshStatePresentation();
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

	const float NewGrowthProgress =
		AccumulatedRollingDistance >= DistanceForMaximumGrowth
			? 1.0f
			: FMath::Clamp(
				AccumulatedRollingDistance / DistanceForMaximumGrowth,
				0.0f,
				1.0f);
	const bool bReachedMaximumGrowth =
		NewGrowthProgress >= 1.0f
		&& GrowthProgress < 1.0f;
	if (bReachedMaximumGrowth
		|| !FMath::IsNearlyEqual(GrowthProgress, NewGrowthProgress, 0.001f))
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

bool ASnowballItem::IsFullyGrown() const
{
	return GrowthProgress >= 0.999f;
}

float ASnowballItem::GetRollingCollisionRadius() const
{
	return CollisionComponent
		? CollisionComponent->GetScaledSphereRadius()
		: 1.0f;
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
	DOREPLIFETIME(ASnowballItem, bIsSettledOnGround);
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

void ASnowballItem::OnRep_IsSettledOnGround()
{
	RefreshStatePresentation();
}

bool ASnowballItem::TrySettleOnGroundBelow(AActor* ActorToIgnore)
{
	if (!HasAuthority() || !CollisionComponent)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector TraceStart =
		CurrentLocation + FVector::UpVector * GroundSettleTraceUpDistance;
	const FVector TraceEnd =
		CurrentLocation - FVector::UpVector * GroundSettleTraceDownDistance;

	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(SnowballGroundSettleTrace),
		false,
		this);
	QueryParams.AddIgnoredActor(this);
	if (ActorToIgnore)
	{
		QueryParams.AddIgnoredActor(ActorToIgnore);
	}

	FHitResult GroundHit;
	const bool bHitGround = World->LineTraceSingleByChannel(
		GroundHit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams);
	if (!bHitGround || !GroundHit.bBlockingHit)
	{
		return false;
	}

	SettleOnGroundFromSurface(
		GroundHit.ImpactPoint,
		GroundHit.ImpactNormal);
	return true;
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
		if (USceneComponent* HoldPoint =
			Holder->GetSnowballHoldPointForSnowball(this))
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

		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
	CollisionComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
	CollisionComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	CollisionComponent->SetEnableGravity(false);
	CollisionComponent->SetSimulatePhysics(false);
	SetReplicateMovement(true);
}

void ASnowballItem::HandleCollision(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	if (ItemState == ESnowballItemState::Ground
		&& !bIsSettledOnGround
		&& Hit.ImpactNormal.Z > 0.5f)
	{
		bIsSettledOnGround = true;
		RefreshStatePresentation();
		ForceNetUpdate();
		return;
	}

	if (ItemState != ESnowballItemState::Thrown
		|| OtherActor == this
		|| OtherActor == GetOwner())
	{
		return;
	}

	HandleThrownImpact(OtherActor, Hit);
}

void ASnowballItem::HandleProjectileStopped(const FHitResult& Hit)
{
	if (!HasAuthority() || ItemState != ESnowballItemState::Thrown)
	{
		return;
	}

	AActor* HitActor = Hit.GetActor();
	if (HitActor == this || HitActor == GetOwner())
	{
		return;
	}

	HandleThrownImpact(HitActor, Hit);
}

void ASnowballItem::HandleThrownImpact(
	AActor* OtherActor,
	const FHitResult& Hit)
{
	if (!HasAuthority()
		|| ItemState != ESnowballItemState::Thrown
		|| bHasProcessedThrownImpact)
	{
		return;
	}

	bHasProcessedThrownImpact = true;

	if (OtherActor)
	{
		const ASnowRumbleCharacter* ThrowingCharacter =
			Cast<ASnowRumbleCharacter>(GetOwner());
		const float ItemDamageMultiplier = ThrowingCharacter
			? ThrowingCharacter->GetSnowballDamageMultiplier()
			: 1.0f;
		const float ChargedDamage = Damage * FMath::Lerp(
			FMath::Clamp(MinimumDamageMultiplier, 0.0f, 1.0f),
			1.0f,
			CurrentThrowChargeProgress)
			* ItemDamageMultiplier;
		UGameplayStatics::ApplyDamage(
			OtherActor,
			ChargedDamage,
			GetInstigatorController(),
			this,
			UDamageType::StaticClass());

		if (ASnowRumbleCharacter* HitCharacter =
			Cast<ASnowRumbleCharacter>(OtherActor))
		{
			FVector KnockbackDirection =
				ProjectileMovement
					? ProjectileMovement->Velocity.GetSafeNormal2D()
					: FVector::ZeroVector;
			if (KnockbackDirection.IsNearlyZero())
			{
				KnockbackDirection =
					(HitCharacter->GetActorLocation() - GetActorLocation())
					.GetSafeNormal2D();
			}

			if (!KnockbackDirection.IsNearlyZero())
			{
				const bool bLargeSnowball = IsFullyGrown();
				const float MinimumKnockback = FMath::Max(
					0.0f,
					bLargeSnowball
						? LargeSnowballMinimumKnockback
						: SmallSnowballMinimumKnockback);
				const float MaximumKnockback = FMath::Max(
					MinimumKnockback,
					bLargeSnowball
						? LargeSnowballMaximumKnockback
						: SmallSnowballMaximumKnockback);
				const float KnockbackStrength = FMath::Lerp(
					MinimumKnockback,
					MaximumKnockback,
					CurrentThrowChargeProgress);
				KnockbackDirection = FVector(
					KnockbackDirection.X,
					KnockbackDirection.Y,
					KnockbackUpwardRatio).GetSafeNormal();
				HitCharacter->LaunchCharacter(
					KnockbackDirection * KnockbackStrength,
					true,
					true);
			}
		}
	}

	const FVector HitImpactPoint(
		Hit.ImpactPoint.X,
		Hit.ImpactPoint.Y,
		Hit.ImpactPoint.Z);
	const FVector ImpactPoint =
		Hit.ImpactPoint.IsNearlyZero()
			? GetActorLocation()
			: HitImpactPoint;
	const FVector HitImpactNormal(
		Hit.ImpactNormal.X,
		Hit.ImpactNormal.Y,
		Hit.ImpactNormal.Z);
	const FVector ImpactNormal =
		Hit.ImpactNormal.IsNearlyZero()
			? FVector::UpVector
			: HitImpactNormal.GetSafeNormal();

	MulticastPlayImpactEffect(ImpactPoint, ImpactNormal);

	Destroy();
}

void ASnowballItem::RestoreTemporarilyIgnoredActor(
	TWeakObjectPtr<AActor> IgnoredActor)
{
	AActor* ActorToRestore = IgnoredActor.Get();
	if (!CollisionComponent || !ActorToRestore)
	{
		return;
	}

	TemporarilyIgnoredActors.Remove(IgnoredActor);
	CollisionComponent->IgnoreActorWhenMoving(ActorToRestore, false);
	if (TemporarilyIgnoredActors.IsEmpty() && bTemporarilyIgnoringPawnCollision)
	{
		CollisionComponent->SetCollisionResponseToChannel(
			ECC_Pawn,
			CachedPawnCollisionResponse);
		bTemporarilyIgnoringPawnCollision = false;
	}
}

void ASnowballItem::MulticastPlayImpactEffect_Implementation(
	FVector_NetQuantize ImpactPoint,
	FVector_NetQuantizeNormal ImpactNormal)
{
	PlayImpactEffect(ImpactPoint, ImpactNormal);
}

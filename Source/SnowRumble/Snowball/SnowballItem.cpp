// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowballItem.h"

#include "../Audio/SnowRumbleAudioHelpers.h"
#include "../Game/SnowRumblePlayerState.h"
#include "../Player/SnowRumbleCharacter.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

namespace
{
bool AreSnowballCharactersOnSameTeam(
	const ASnowRumbleCharacter* ThrowingCharacter,
	const ASnowRumbleCharacter* HitCharacter)
{
	const ASnowRumblePlayerState* ThrowerPlayerState = ThrowingCharacter
		? ThrowingCharacter->GetPlayerState<ASnowRumblePlayerState>()
		: nullptr;
	const ASnowRumblePlayerState* HitPlayerState = HitCharacter
		? HitCharacter->GetPlayerState<ASnowRumblePlayerState>()
		: nullptr;
	if (!ThrowerPlayerState || !HitPlayerState)
	{
		return false;
	}

	const ESnowRumbleTeam ThrowerTeam = ThrowerPlayerState->GetLobbyTeam();
	return ThrowerTeam != ESnowRumbleTeam::None
		&& ThrowerTeam == HitPlayerState->GetLobbyTeam();
}
}

ASnowballItem::ASnowballItem()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	SetNetUpdateFrequency(30.0f);
	SetMinNetUpdateFrequency(15.0f);

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

	// Movement replication can apply the server's grown scale before client
	// BeginPlay. Keep the actual spawn scale on the server as the baseline.
	if (HasAuthority())
	{
		InitialActorScale = GetActorScale3D();
	}
	ApplyGrowthScale();
}

void ASnowballItem::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	(void)DeltaSeconds;

	if (HasAuthority() && bIsThrownRolling)
	{
		UpdateThrownRolling();
	}

	if (HasAuthority()
		&& bIsFallingSnowball
		&& bIsFallingSnowballPhysicsActive
		&& ItemState == ESnowballItemState::Thrown
		&& CollisionComponent
		&& CollisionComponent->IsSimulatingPhysics())
	{
		const float RestSpeed = 35.0f;
		const float RestDuration = 0.35f;
		const float CurrentSpeed =
			CollisionComponent->GetPhysicsLinearVelocity().Size();
		if (CurrentSpeed <= RestSpeed)
		{
			FallingSnowballRestTime += DeltaSeconds;
		}
		else
		{
			FallingSnowballRestTime = 0.0f;
		}

		if (FallingSnowballRestTime >= RestDuration)
		{
			ItemState = ESnowballItemState::Ground;
			bIsFallingSnowball = false;
			bIsFallingSnowballPhysicsActive = false;
			bIsSettledOnGround = true;
			if (!TrySettleOnGroundBelow(nullptr))
			{
				RefreshStatePresentation();
			}
			ForceNetUpdate();
		}
	}
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
	float ThrowChargeProgress,
	float ThrowDamageMultiplier)
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
	bIsThrownRolling = false;
	AccumulatedThrownRollingDistance = 0.0f;
	LastThrownRollingLocation = GetActorLocation();
	ThrownRollingDirection = ThrowDirection.GetSafeNormal2D();
	ThrownRollingCollisionCount = 0;
	ThrownRollingHitCharacters.Reset();
	CurrentThrowChargeProgress = FMath::Clamp(ThrowChargeProgress, 0.0f, 1.0f);
	CurrentThrowDamageMultiplier = FMath::Max(0.0f, ThrowDamageMultiplier);
	FallingSnowballDamage = 0.0f;
	bIsFallingSnowball = false;
	bIsFallingSnowballPhysicsActive = false;
	FallingSnowballRestTime = 0.0f;

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

void ASnowballItem::InitializeFallingLargeSnowball(
	float ImpactDamage,
	const FVector& InitialVelocity)
{
	if (!HasAuthority()
		|| !CollisionComponent
		|| !ProjectileMovement)
	{
		return;
	}

	ItemState = ESnowballItemState::Thrown;
	Holder = nullptr;
	Roller = nullptr;
	GrowthProgress = 1.0f;
	bIsSettledOnGround = false;
	bHasProcessedThrownImpact = false;
	bIsThrownRolling = false;
	AccumulatedThrownRollingDistance = 0.0f;
	LastThrownRollingLocation = GetActorLocation();
	ThrownRollingDirection = FVector::DownVector;
	ThrownRollingCollisionCount = 0;
	ThrownRollingHitCharacters.Reset();
	CurrentThrowChargeProgress = 1.0f;
	CurrentThrowDamageMultiplier = 1.0f;
	FallingSnowballDamage = FMath::Max(0.0f, ImpactDamage);
	bIsFallingSnowball = true;
	bIsFallingSnowballPhysicsActive = false;
	FallingSnowballRestTime = 0.0f;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	CollisionComponent->SetSimulatePhysics(false);
	CollisionComponent->SetEnableGravity(false);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SetReplicateMovement(true);
	ApplyGrowthScale();

	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->ProjectileGravityScale = LargeSnowballProjectileGravityScale;
	ProjectileMovement->Velocity = InitialVelocity;
	ProjectileMovement->Activate(true);
	ForceNetUpdate();
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
	const FVector& SurfaceNormal,
	bool bIsSnowSurface)
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
	const float SizeBasedZOffset = bIsSnowSurface
		? (IsFullyGrown()
			? LargeSnowballGroundSettleZOffset
			: SmallSnowballGroundSettleZOffset)
		: 0.0f;
	const FVector SettledLocation =
		SurfacePoint
		+ SafeSurfaceNormal * (Radius + GroundSettleExtraClearance)
		+ FVector::UpVector * SizeBasedZOffset;

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

void ASnowballItem::StartThrownRolling()
{
	if (!HasAuthority()
		|| !IsFullyGrown()
		|| bIsFallingSnowball
		|| bIsThrownRolling
		|| !CollisionComponent)
	{
		return;
	}

	const FVector RollingVelocity = ProjectileMovement
		? ProjectileMovement->Velocity
		: FVector::ZeroVector;
	if (ProjectileMovement)
	{
		ProjectileMovement->Deactivate();
		ProjectileMovement->Velocity = FVector::ZeroVector;
	}

	bIsThrownRolling = true;
	AccumulatedThrownRollingDistance = 0.0f;
	LastThrownRollingLocation = GetActorLocation();
	ThrownRollingDirection = RollingVelocity.GetSafeNormal2D();
	ThrownRollingCollisionCount = 0;
	ThrownRollingHitCharacters.Reset();
	SetLifeSpan(0.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetEnableGravity(true);
	CollisionComponent->SetSimulatePhysics(true);
	CollisionComponent->SetPhysicsLinearVelocity(RollingVelocity);
	SetReplicateMovement(true);
	ForceNetUpdate();
}

void ASnowballItem::StartFallingSnowballPhysics(
	const FVector& InitialVelocity)
{
	if (!HasAuthority()
		|| !CollisionComponent
		|| !bIsFallingSnowball)
	{
		return;
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->Deactivate();
	}

	bIsFallingSnowballPhysicsActive = true;
	FallingSnowballRestTime = 0.0f;
	bIsSettledOnGround = false;
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetEnableGravity(true);
	CollisionComponent->SetSimulatePhysics(true);
	CollisionComponent->SetLinearDamping(FallingSnowballLinearDamping);
	CollisionComponent->SetAngularDamping(FallingSnowballAngularDamping);
	CollisionComponent->SetPhysicsLinearVelocity(InitialVelocity);
	CollisionComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	SetReplicateMovement(true);
	ForceNetUpdate();
}

void ASnowballItem::UpdateThrownRolling()
{
	if (!HasAuthority() || !bIsThrownRolling || !CollisionComponent)
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector CurrentVelocity = CollisionComponent->GetPhysicsLinearVelocity();
	if (MinimumThrownRollingSpeed > 0.0f
		&& CurrentVelocity.SizeSquared2D()
			<= FMath::Square(MinimumThrownRollingSpeed))
	{
		DestroyThrownRolling(FHitResult());
		return;
	}
	if (CurrentVelocity.SizeSquared2D() > KINDA_SMALL_NUMBER)
	{
		ThrownRollingDirection = CurrentVelocity.GetSafeNormal2D();
	}
	AccumulatedThrownRollingDistance += FVector::Dist2D(
		CurrentLocation,
		LastThrownRollingLocation);
	LastThrownRollingLocation = CurrentLocation;

	if (DistanceForThrownLargeSnowballToDissolve <= 0.0f)
	{
		return;
	}

	const float NewGrowthProgress = FMath::Clamp(
		1.0f - (AccumulatedThrownRollingDistance
			/ DistanceForThrownLargeSnowballToDissolve),
		0.0f,
		1.0f);
	if (!FMath::IsNearlyEqual(GrowthProgress, NewGrowthProgress, 0.001f))
	{
		GrowthProgress = NewGrowthProgress;
		OnRep_GrowthProgress();
		ForceNetUpdate();
	}

	if (NewGrowthProgress <= KINDA_SMALL_NUMBER)
	{
		DestroyThrownRolling(FHitResult());
	}
}

void ASnowballItem::DestroyThrownRolling(const FHitResult& Hit)
{
	if (!HasAuthority() || !bIsThrownRolling)
	{
		return;
	}

	bIsThrownRolling = false;
	if (CollisionComponent)
	{
		CollisionComponent->SetSimulatePhysics(false);
	}

	const FVector HitImpactPoint(Hit.ImpactPoint);
	const FVector ImpactPoint = HitImpactPoint.IsNearlyZero()
		? GetActorLocation()
		: HitImpactPoint;
	const FVector HitImpactNormal(Hit.ImpactNormal);
	const FVector ImpactNormal = HitImpactNormal.IsNearlyZero()
		? FVector::UpVector
		: HitImpactNormal.GetSafeNormal();
	MulticastPlayImpactEffect(ImpactPoint, ImpactNormal);
	Destroy();
}

bool ASnowballItem::IsFullyGrown() const
{
	return GrowthProgress >= FMath::Clamp(LargeSnowballGrowthThreshold, 0.0f, 1.0f);
}

float ASnowballItem::GetRollingCollisionRadius() const
{
	return CollisionComponent
		? CollisionComponent->GetScaledSphereRadius()
		: 1.0f;
}

float ASnowballItem::GetProjectileGravityScale() const
{
	return IsFullyGrown()
		? LargeSnowballProjectileGravityScale
		: SmallSnowballProjectileGravityScale;
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

bool ASnowballItem::IsSnowSurfaceActor(const AActor* SurfaceActor) const
{
	return SurfaceActor
		&& !GroundSnowSurfaceTag.IsNone()
		&& SurfaceActor->ActorHasTag(GroundSnowSurfaceTag);
}

void ASnowballItem::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASnowballItem, ItemState);
	DOREPLIFETIME(ASnowballItem, Holder);
	DOREPLIFETIME(ASnowballItem, Roller);
	DOREPLIFETIME(ASnowballItem, GrowthProgress);
	DOREPLIFETIME(ASnowballItem, InitialActorScale);
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

void ASnowballItem::OnRep_InitialActorScale()
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

	TArray<FHitResult> GroundHits;
	const bool bHitGround = World->LineTraceMultiByChannel(
		GroundHits,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams);
	if (!bHitGround)
	{
		return false;
	}

	FHitResult GroundHit;
	FHitResult SnowSurfaceFallbackHit;
	bool bHasSnowSurfaceFallback = false;
	for (const FHitResult& Hit : GroundHits)
	{
		if (!Hit.bBlockingHit)
		{
			continue;
		}

		const AActor* HitActor = Hit.GetActor();
		const bool bIsSnowSurface =
			HitActor
			&& !GroundSnowSurfaceTag.IsNone()
			&& HitActor->ActorHasTag(GroundSnowSurfaceTag);
		if (bIsSnowSurface)
		{
			if (!bHasSnowSurfaceFallback)
			{
				SnowSurfaceFallbackHit = Hit;
				bHasSnowSurfaceFallback = true;
			}
			continue;
		}

		GroundHit = Hit;
		break;
	}

	if (!GroundHit.bBlockingHit && bHasSnowSurfaceFallback)
	{
		GroundHit = SnowSurfaceFallbackHit;
	}
	if (!GroundHit.bBlockingHit)
	{
		return false;
	}

	const bool bIsSnowSurface =
		GroundHit.GetActor()
		&& !GroundSnowSurfaceTag.IsNone()
		&& GroundHit.GetActor()->ActorHasTag(GroundSnowSurfaceTag);
	SettleOnGroundFromSurface(
		GroundHit.ImpactPoint,
		GroundHit.ImpactNormal,
		bIsSnowSurface);
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
	if (!HasAuthority()
		|| ItemState != ESnowballItemState::Thrown
		|| bIsThrownRolling)
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
	if (!HasAuthority() || ItemState != ESnowballItemState::Thrown)
	{
		return;
	}

	ASnowRumbleCharacter* HitCharacter = Cast<ASnowRumbleCharacter>(OtherActor);
	if (bIsFallingSnowball)
	{
		if (!HitCharacter)
		{
			if (!bIsFallingSnowballPhysicsActive)
			{
				const FVector InitialVelocity = ProjectileMovement
					? ProjectileMovement->Velocity
					: FVector(0.0f, 0.0f, -250.0f);
				StartFallingSnowballPhysics(InitialVelocity);
			}
			return;
		}

		if (ThrownRollingHitCharacters.Contains(HitCharacter))
		{
			return;
		}
		ThrownRollingHitCharacters.Add(HitCharacter);
	}

	if (bIsThrownRolling)
	{
		++ThrownRollingCollisionCount;
		const bool bShouldDestroyAfterImpact =
			MaximumThrownRollingCollisionCount > 0
			&& ThrownRollingCollisionCount
			>= MaximumThrownRollingCollisionCount;
		if (!HitCharacter)
		{
			if (bShouldDestroyAfterImpact)
			{
				DestroyThrownRolling(Hit);
			}
			return;
		}

		if (ThrownRollingHitCharacters.Contains(HitCharacter))
		{
			if (bShouldDestroyAfterImpact)
			{
				DestroyThrownRolling(Hit);
			}
			return;
		}
		ThrownRollingHitCharacters.Add(HitCharacter);
	}
	else if (bHasProcessedThrownImpact)
	{
		return;
	}

	if (!bIsThrownRolling && !bIsFallingSnowball)
	{
		bHasProcessedThrownImpact = true;
	}

	if (OtherActor)
	{
		const ASnowRumbleCharacter* ThrowingCharacter =
			Cast<ASnowRumbleCharacter>(GetOwner());
		const bool bIgnoreFriendlyFire =
			HitCharacter
			&& AreSnowballCharactersOnSameTeam(
				ThrowingCharacter,
				HitCharacter);
		if (!bIgnoreFriendlyFire)
		{
			const float ItemDamageMultiplier = ThrowingCharacter
				? ThrowingCharacter->GetSnowballDamageMultiplier()
				: 1.0f;
			const float ChargedDamage = Damage * FMath::Lerp(
				FMath::Clamp(MinimumDamageMultiplier, 0.0f, 1.0f),
				1.0f,
				CurrentThrowChargeProgress)
				* ItemDamageMultiplier;
			const float GrowthDamageMultiplier = FMath::Lerp(
				1.0f,
				FMath::Max(1.0f, MaximumGrowthDamageMultiplier),
				FMath::Clamp(GrowthProgress, 0.0f, 1.0f));
			const float FinalDamage = bIsFallingSnowball
				? FallingSnowballDamage
				: ChargedDamage
				* GrowthDamageMultiplier
				* CurrentThrowDamageMultiplier;
			UGameplayStatics::ApplyDamage(
				OtherActor,
				FinalDamage,
				GetInstigatorController(),
				this,
				UDamageType::StaticClass());

			if (HitCharacter)
			{
				FVector KnockbackDirection =
					bIsThrownRolling
						? ThrownRollingDirection
						: (ProjectileMovement
						? ProjectileMovement->Velocity.GetSafeNormal2D()
						: FVector::ZeroVector);
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
	}

	if (bIsThrownRolling)
	{
		if (MaximumThrownRollingCollisionCount > 0
			&& ThrownRollingCollisionCount
			>= MaximumThrownRollingCollisionCount)
		{
			DestroyThrownRolling(Hit);
		}
		return;
	}

	if (bIsFallingSnowball)
	{
		if (!bIsFallingSnowballPhysicsActive && ProjectileMovement)
		{
			ProjectileMovement->Activate(true);
			ProjectileMovement->Velocity = FVector(0.0f, 0.0f, -250.0f);
		}
		return;
	}

	if (IsFullyGrown())
	{
		StartThrownRolling();
		return;
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
	USoundBase* SoundToPlay = IsFullyGrown() && LargeImpactSound
		? LargeImpactSound
		: ImpactSound;
	SnowRumbleAudio::PlaySoundAtLocation(
		this,
		SoundToPlay,
		ESnowRumbleAudioMixChannel::Gameplay,
		ImpactPoint,
		1.0f,
		1.0f,
		ImpactSoundAttenuation);
	PlayImpactEffect(ImpactPoint, ImpactNormal);
}

void ASnowballItem::PlayRollingSound()
{
	if (HasAuthority())
	{
		MulticastPlayRollingSound(GetActorLocation());
	}
}

void ASnowballItem::StopRollingSound()
{
	if (HasAuthority())
	{
		MulticastStopRollingSound();
	}
}

void ASnowballItem::MulticastPlayRollingSound_Implementation(
	FVector_NetQuantize Location)
{
	if (RollingAudioComponent)
	{
		RollingAudioComponent->Stop();
		RollingAudioComponent = nullptr;
	}

	if (!RollingSound)
	{
		return;
	}

	RollingAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
		this,
		RollingSound,
		Location,
		FRotator::ZeroRotator,
		SnowRumbleAudio::GetEffectiveVolume(
			this,
			ESnowRumbleAudioMixChannel::Gameplay),
		1.0f,
		0.0f,
		RollingSoundAttenuation);
}

void ASnowballItem::MulticastStopRollingSound_Implementation()
{
	if (RollingAudioComponent)
	{
		RollingAudioComponent->Stop();
		RollingAudioComponent = nullptr;
	}
}

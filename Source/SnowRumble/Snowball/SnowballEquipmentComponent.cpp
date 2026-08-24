// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowballEquipmentComponent.h"

#include "../Player/SnowRumbleCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"
#include "SnowballItem.h"

USnowballEquipmentComponent::USnowballEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void USnowballEquipmentComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ASnowRumbleCharacter* Character = Cast<ASnowRumbleCharacter>(GetOwner());
	if (!Character
		|| !Character->HasAuthority()
		|| !RollingSnowball)
	{
		return;
	}

	if (Character->IsFrozen()
		|| Character->IsPickingUpItem())
	{
		ServerStopRollingSnowball_Implementation();
		return;
	}

	const FVector MovementDirection = Character->GetVelocity().GetSafeNormal2D();
	if (!MovementDirection.IsNearlyZero())
	{
		LastRollingMovementDirection = MovementDirection;
	}

	const float RollingCollisionRadius =
		RollingSnowball->GetRollingCollisionRadius();
	const FVector TargetLocation =
		BuildRollingSnowballTargetLocation(Character, RollingSnowball);

	FHitResult RollingHit;
	bool bBlockedByObstacle =
		Character->MoveRollingSnowballCollision(
			TargetLocation,
			RollingCollisionRadius,
			RollingHit);
	if (bBlockedByObstacle)
	{
		Character->EnableRollingSnowballCollision(
			TargetLocation,
			RollingCollisionRadius);
		bBlockedByObstacle = false;
	}
	RollingSnowball->MoveRollingSnowball(
		Character->GetRollingSnowballCollisionLocation());
	RollingSnowball->UpdateRollingGrowth();
	RollingSnowball->ForceNetUpdate();
}

void USnowballEquipmentComponent::TryPickupSnowball()
{
	const APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn || !OwningPawn->IsLocallyControlled())
	{
		return;
	}

	ServerTryPickupSnowball();
}

bool USnowballEquipmentComponent::HasHeldSnowball() const
{
	return IsValid(HeldSnowball);
}

ASnowballItem* USnowballEquipmentComponent::GetHeldSnowball() const
{
	return HeldSnowball;
}

bool USnowballEquipmentComponent::EquipCreatedSnowballFromServer(
	ASnowballItem* CreatedSnowball)
{
	ASnowRumbleCharacter* Character = Cast<ASnowRumbleCharacter>(GetOwner());
	if (!Character
		|| !Character->HasAuthority()
		|| Character->IsFrozen()
		|| Character->IsPickingUpItem()
		|| HasHeldSnowball()
		|| !CreatedSnowball)
	{
		return false;
	}

	if (!CreatedSnowball->TrySetHeldBy(
		Character,
		Character->GetSnowballHoldPointForSnowball(CreatedSnowball)))
	{
		return false;
	}

	HeldSnowball = CreatedSnowball;
	Character->NotifySnowballPickupSucceeded(IsHoldingLargeSnowball());
	OnRep_HeldSnowball();
	Character->ForceNetUpdate();
	return true;
}

bool USnowballEquipmentComponent::IsHoldingLargeSnowball() const
{
	return HeldSnowball
		&& HeldSnowball->IsFullyGrown();
}

float USnowballEquipmentComponent::GetLargeSnowballCarryWalkSpeed() const
{
	return LargeSnowballCarryWalkSpeed;
}

ASnowballItem* USnowballEquipmentComponent::FindClosestPickupCandidate() const
{
	const AActor* OwningActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwningActor || !World || HasHeldSnowball())
	{
		return nullptr;
	}

	const FVector OwnerLocation = OwningActor->GetActorLocation();
	const float MaximumDistanceSquared = FMath::Square(PickupRadius);
	float ClosestDistanceSquared = MaximumDistanceSquared;
	ASnowballItem* ClosestSnowball = nullptr;

	for (TActorIterator<ASnowballItem> Iterator(World); Iterator; ++Iterator)
	{
		ASnowballItem* Candidate = *Iterator;
		if (!Candidate || !Candidate->CanBePickedUp())
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(
			OwnerLocation,
			Candidate->GetActorLocation());
		if (DistanceSquared <= ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			ClosestSnowball = Candidate;
		}
	}

	return ClosestSnowball;
}

void USnowballEquipmentComponent::SetAiming(bool bNewAiming)
{
	const APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn
		|| (!OwningPawn->HasAuthority() && !OwningPawn->IsLocallyControlled()))
	{
		return;
	}

	const ASnowRumbleCharacter* Character = Cast<ASnowRumbleCharacter>(OwningPawn);
	const bool bValidatedAiming =
		bNewAiming
		&& Character
		&& !Character->IsFrozen()
		&& !Character->IsPickingUpItem()
		&& HasHeldSnowball();

	if (!bValidatedAiming && bIsCharging)
	{
		CancelCharging();
	}

	if (bIsAiming != bValidatedAiming)
	{
		bIsAiming = bValidatedAiming;
		OnRep_IsAiming();
	}

	if (!OwningPawn->HasAuthority())
	{
		ServerSetAiming(bNewAiming);
	}
}

bool USnowballEquipmentComponent::IsAiming() const
{
	return bIsAiming && HasHeldSnowball();
}

bool USnowballEquipmentComponent::CanThrowHeldSnowball() const
{
	const ASnowRumbleCharacter* Character =
		Cast<ASnowRumbleCharacter>(GetOwner());
	const bool bCanUseDuckMakerDirectly =
		Character && Character->HasEquippedSnowDuckMaker();
	return HasHeldSnowball()
		&& (bIsAiming || bCanUseDuckMakerDirectly)
		&& !bHasPendingThrow;
}

void USnowballEquipmentComponent::StartCharging()
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn
		|| (!OwningPawn->HasAuthority() && !OwningPawn->IsLocallyControlled())
		|| !CanThrowHeldSnowball()
		|| bIsCharging)
	{
		return;
	}

	if (OwningPawn->HasAuthority())
	{
		ServerStartCharging_Implementation();
		return;
	}

	ChargeStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : -1.0;
	SetChargingState(true);
	ServerStartCharging();
}

void USnowballEquipmentComponent::ReleaseChargedSnowball()
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn
		|| (!OwningPawn->HasAuthority() && !OwningPawn->IsLocallyControlled())
		|| !bIsCharging)
	{
		return;
	}

	FVector ViewLocation = FVector::ZeroVector;
	FVector ViewDirection = FVector::ForwardVector;
	if (!BuildCurrentThrowView(ViewLocation, ViewDirection))
	{
		CancelCharging();
		return;
	}

	if (OwningPawn->HasAuthority())
	{
		ServerReleaseChargedSnowball_Implementation(
			ViewLocation,
			ViewDirection);
		return;
	}

	SetChargingState(false);
	ServerReleaseChargedSnowball(ViewLocation, ViewDirection);
}

void USnowballEquipmentComponent::ConfirmPendingThrowFromAnimationNotify()
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn
		|| (!OwningPawn->HasAuthority() && !OwningPawn->IsLocallyControlled()))
	{
		return;
	}

	FVector ViewLocation = FVector::ZeroVector;
	FVector ViewDirection = FVector::ForwardVector;
	if (!BuildCurrentThrowView(ViewLocation, ViewDirection))
	{
		return;
	}

	if (OwningPawn->HasAuthority())
	{
		ServerConfirmPendingThrowFromAnimationNotify_Implementation(
			ViewLocation,
			ViewDirection);
		return;
	}

	ServerConfirmPendingThrowFromAnimationNotify(ViewLocation, ViewDirection);
}

void USnowballEquipmentComponent::CancelCharging()
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn
		|| (!OwningPawn->HasAuthority() && !OwningPawn->IsLocallyControlled())
		|| !bIsCharging)
	{
		return;
	}

	ClearPendingThrow();
	SetChargingState(false);

	if (!OwningPawn->HasAuthority())
	{
		ServerCancelCharging();
	}
}

void USnowballEquipmentComponent::InterruptThrowStateFromServer()
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority())
	{
		return;
	}

	const bool bHadInterruptedThrowState = bIsCharging || bHasPendingThrow;
	ClearPendingThrow();
	SetChargingState(false);

	if (bHadInterruptedThrowState)
	{
		OwningActor->ForceNetUpdate();
	}
}

bool USnowballEquipmentComponent::IsCharging() const
{
	return bIsCharging && CanThrowHeldSnowball();
}

float USnowballEquipmentComponent::GetChargeProgress() const
{
	const UWorld* World = GetWorld();
	const float CurrentMaximumChargeSeconds = GetCurrentMaximumChargeSeconds();
	if (!World
		|| !IsCharging()
		|| ChargeStartTime < 0.0
		|| CurrentMaximumChargeSeconds <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(
		static_cast<float>(
			(World->GetTimeSeconds() - ChargeStartTime)
			/ CurrentMaximumChargeSeconds),
		0.0f,
		1.0f);
}

void USnowballEquipmentComponent::DropHeldSnowball()
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn
		|| (!OwningPawn->HasAuthority() && !OwningPawn->IsLocallyControlled()))
	{
		return;
	}

	if (OwningPawn->HasAuthority())
	{
		ServerDropHeldSnowball_Implementation();
		return;
	}

	ServerDropHeldSnowball();
}

void USnowballEquipmentComponent::StartRollingSnowball()
{
	const APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn
		|| (!OwningPawn->HasAuthority() && !OwningPawn->IsLocallyControlled()))
	{
		return;
	}

	if (OwningPawn->HasAuthority())
	{
		ServerStartRollingSnowball_Implementation();
		return;
	}

	ServerStartRollingSnowball();
}

void USnowballEquipmentComponent::StopRollingSnowball()
{
	const APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn
		|| (!OwningPawn->HasAuthority() && !OwningPawn->IsLocallyControlled()))
	{
		return;
	}

	if (OwningPawn->HasAuthority())
	{
		ServerStopRollingSnowball_Implementation();
		return;
	}

	ServerStopRollingSnowball();
}

bool USnowballEquipmentComponent::IsRollingSnowball() const
{
	return IsValid(RollingSnowball);
}

ASnowballItem* USnowballEquipmentComponent::GetRollingSnowball() const
{
	return RollingSnowball;
}

float USnowballEquipmentComponent::GetRollingWalkSpeed() const
{
	return RollingSnowball
		? FMath::Lerp(
			SmallSnowballRollingWalkSpeed,
			LargeSnowballRollingWalkSpeed,
			RollingSnowball->GetGrowthProgress())
		: SmallSnowballRollingWalkSpeed;
}

void USnowballEquipmentComponent::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USnowballEquipmentComponent, HeldSnowball);
	DOREPLIFETIME(USnowballEquipmentComponent, bIsAiming);
	DOREPLIFETIME(USnowballEquipmentComponent, bIsCharging);
	DOREPLIFETIME(USnowballEquipmentComponent, RollingSnowball);
}

void USnowballEquipmentComponent::ServerTryPickupSnowball_Implementation()
{
	ASnowRumbleCharacter* Character = Cast<ASnowRumbleCharacter>(GetOwner());
	if (!Character
		|| Character->IsFrozen()
		|| HasHeldSnowball())
	{
		return;
	}

	ASnowballItem* PickupCandidate = FindClosestPickupCandidate();
	if (!PickupCandidate)
	{
		return;
	}

	if (PickupCandidate->TrySetHeldBy(
		Character,
		Character->GetSnowballHoldPointForSnowball(PickupCandidate)))
	{
		HeldSnowball = PickupCandidate;
		Character->NotifySnowballPickupSucceeded(IsHoldingLargeSnowball());
		OnRep_HeldSnowball();
		Character->ForceNetUpdate();
	}
}

void USnowballEquipmentComponent::ServerSetAiming_Implementation(bool bNewAiming)
{
	SetAiming(bNewAiming);

	if (AActor* OwningActor = GetOwner())
	{
		OwningActor->ForceNetUpdate();
	}
}

void USnowballEquipmentComponent::ServerStartCharging_Implementation()
{
	const ASnowRumbleCharacter* Character = Cast<ASnowRumbleCharacter>(GetOwner());
	if (!Character
		|| Character->IsFrozen()
		|| !CanThrowHeldSnowball()
		|| bIsCharging)
	{
		return;
	}

	ChargeStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : -1.0;
	SetChargingState(true);

	if (AActor* OwningActor = GetOwner())
	{
		OwningActor->ForceNetUpdate();
	}
}

void USnowballEquipmentComponent::ServerReleaseChargedSnowball_Implementation(
	FVector_NetQuantize ViewLocation,
	FVector_NetQuantizeNormal ViewDirection)
{
	ASnowRumbleCharacter* Character = Cast<ASnowRumbleCharacter>(GetOwner());
	UWorld* World = GetWorld();
	if (!Character
		|| !World
		|| Character->IsFrozen()
		|| !IsCharging()
		|| ViewLocation.ContainsNaN()
		|| ViewDirection.ContainsNaN()
		|| FVector::DistSquared(
			ViewLocation,
			Character->GetActorLocation())
			> FMath::Square(MaximumAimViewOriginDistance)
		|| FVector::DotProduct(
			ViewDirection.GetSafeNormal(),
			Character->GetBaseAimRotation().Vector()) < 0.7f)
	{
		CancelCharging();
		return;
	}

	FVector AimTarget = FVector::ZeroVector;
	if (!FindServerAimTarget(ViewLocation, ViewDirection, AimTarget))
	{
		CancelCharging();
		return;
	}

	const bool bThrowingLargeSnowball = IsHoldingLargeSnowball();
	const bool bUseAirborneThrowBonus =
		!bThrowingLargeSnowball && Character->IsInAir();
	const float CurrentMaximumChargeSeconds = GetCurrentMaximumChargeSeconds();
	const float ChargeProgress = FMath::Clamp(
		static_cast<float>(
			(World->GetTimeSeconds() - ChargeStartTime)
			/ CurrentMaximumChargeSeconds),
		0.0f,
		1.0f);
	const float ThrowSpeed = FMath::Lerp(
		bThrowingLargeSnowball
			? LargeSnowballMinimumThrowSpeed
			: MinimumThrowSpeed,
		bThrowingLargeSnowball
			? LargeSnowballMaximumThrowSpeed
			: MaximumThrowSpeed,
		ChargeProgress);
	const float FinalThrowSpeed =
		ThrowSpeed
		* (bUseAirborneThrowBonus
			? FMath::Max(0.0f, AirborneThrowSpeedMultiplier)
			: 1.0f);
	const float ThrowDamageMultiplier =
		bUseAirborneThrowBonus
			? FMath::Max(0.0f, AirborneThrowDamageMultiplier)
			: 1.0f;
	const FVector AimDirection =
		(AimTarget - HeldSnowball->GetActorLocation()).GetSafeNormal();
	if (AimDirection.IsNearlyZero())
	{
		CancelCharging();
		return;
	}

	SetChargingState(false);

	bHasPendingThrow = true;
	PendingThrowSpeed = FinalThrowSpeed;
	PendingThrowChargeProgress = ChargeProgress;
	PendingThrowDamageMultiplier = ThrowDamageMultiplier;
	Character->NotifySnowballThrowSucceeded(bThrowingLargeSnowball);
	Character->ForceNetUpdate();
}

void USnowballEquipmentComponent::ServerConfirmPendingThrowFromAnimationNotify_Implementation(
	FVector_NetQuantize ViewLocation,
	FVector_NetQuantizeNormal ViewDirection)
{
	ExecutePendingThrowFromServer(ViewLocation, ViewDirection);
}

void USnowballEquipmentComponent::ServerCancelCharging_Implementation()
{
	ClearPendingThrow();
	SetChargingState(false);

	if (AActor* OwningActor = GetOwner())
	{
		OwningActor->ForceNetUpdate();
	}
}

void USnowballEquipmentComponent::ServerDropHeldSnowball_Implementation()
{
	ASnowRumbleCharacter* Character = Cast<ASnowRumbleCharacter>(GetOwner());
	if (!Character
		|| Character->IsFrozen()
		|| Character->IsPickingUpItem()
		|| !HasHeldSnowball())
	{
		return;
	}

	SetChargingState(false);
	SetAiming(false);
	ClearPendingThrow();

	ASnowballItem* SnowballToDrop = HeldSnowball;
	if (!SnowballToDrop || !SnowballToDrop->DropToGround())
	{
		return;
	}

	HeldSnowball = nullptr;
	OnRep_HeldSnowball();
	Character->ForceNetUpdate();
}

void USnowballEquipmentComponent::ServerStartRollingSnowball_Implementation()
{
	ASnowRumbleCharacter* Character = Cast<ASnowRumbleCharacter>(GetOwner());
	if (!Character
		|| Character->IsFrozen()
		|| Character->IsPickingUpItem()
		|| HasHeldSnowball()
		|| RollingSnowball)
	{
		return;
	}

	ASnowballItem* RollingCandidate = FindClosestPickupCandidate();
	if (!RollingCandidate
		|| !RollingCandidate->TryStartRolling(Character))
	{
		return;
	}

	RollingSnowball = RollingCandidate;
	Character->EnableRollingSnowballCollision(
		RollingSnowball->GetActorLocation(),
		RollingSnowball->GetRollingCollisionRadius());
	LastRollingMovementDirection =
		Character->GetActorForwardVector().GetSafeNormal2D();
	if (LastRollingMovementDirection.IsNearlyZero())
	{
		LastRollingMovementDirection = FVector::ForwardVector;
	}
	const FVector InitialTargetLocation =
		BuildRollingSnowballTargetLocation(Character, RollingSnowball);
	FHitResult InitialRollingHit;
	Character->MoveRollingSnowballCollision(
		InitialTargetLocation,
		RollingSnowball->GetRollingCollisionRadius(),
		InitialRollingHit);
	RollingSnowball->MoveRollingSnowball(
		Character->GetRollingSnowballCollisionLocation());
	RollingSnowball->PlayRollingSound();
	Character->ForceNetUpdate();
	RollingSnowball->ForceNetUpdate();
}

void USnowballEquipmentComponent::ServerStopRollingSnowball_Implementation()
{
	if (RollingSnowball)
	{
		if (ASnowRumbleCharacter* Character = Cast<ASnowRumbleCharacter>(GetOwner()))
		{
			Character->DisableRollingSnowballCollision();
		}

		RollingSnowball->StopRollingSound();
		RollingSnowball->StopRolling();
		RollingSnowball = nullptr;
	}

	if (AActor* OwningActor = GetOwner())
	{
		OwningActor->ForceNetUpdate();
	}
}

void USnowballEquipmentComponent::OnRep_HeldSnowball()
{
	if (!HasHeldSnowball() && bIsCharging)
	{
		bIsCharging = false;
		OnRep_IsCharging();
	}

	if (!HasHeldSnowball() && bIsAiming)
	{
		bIsAiming = false;
		OnRep_IsAiming();
	}

	if (ASnowRumbleCharacter* Character =
		Cast<ASnowRumbleCharacter>(GetOwner()))
	{
		Character->RefreshHeldEquipmentMovementState();
	}

	OnHeldSnowballChanged.Broadcast(HeldSnowball);
}

void USnowballEquipmentComponent::OnRep_IsAiming()
{
	OnAimingChanged.Broadcast(IsAiming());
}

void USnowballEquipmentComponent::OnRep_IsCharging()
{
	OnChargingChanged.Broadcast(IsCharging());
}

float USnowballEquipmentComponent::GetCurrentMaximumChargeSeconds() const
{
	return IsHoldingLargeSnowball()
		? LargeSnowballMaximumChargeSeconds
		: MaximumChargeSeconds;
}

FVector USnowballEquipmentComponent::BuildRollingSnowballTargetLocation(
	const ASnowRumbleCharacter* Character,
	const ASnowballItem* Snowball) const
{
	if (!Character || !Snowball)
	{
		return FVector::ZeroVector;
	}

	FVector RollingDirection = LastRollingMovementDirection.GetSafeNormal2D();
	if (RollingDirection.IsNearlyZero())
	{
		RollingDirection = Character->GetActorForwardVector().GetSafeNormal2D();
	}
	if (RollingDirection.IsNearlyZero())
	{
		RollingDirection = FVector::ForwardVector;
	}

	const UCapsuleComponent* CapsuleComponent =
		Character->GetCapsuleComponent();
	const float CharacterRadius = CapsuleComponent
		? CapsuleComponent->GetScaledCapsuleRadius()
		: 0.0f;
	const float RollingCollisionRadius = Snowball->GetRollingCollisionRadius();
	const float MinimumRollingDistance =
		CharacterRadius + RollingCollisionRadius + 2.0f;
	const float EffectiveRollingDistance =
		FMath::Max(RollingDistance, MinimumRollingDistance);

	FVector TargetLocation =
		Character->GetActorLocation()
		+ RollingDirection * EffectiveRollingDistance;
	TargetLocation.Z = Snowball->GetActorLocation().Z;
	return TargetLocation;
}

bool USnowballEquipmentComponent::FindServerAimTarget(
	const FVector& ViewLocation,
	const FVector& ViewDirection,
	FVector& OutAimTarget) const
{
	const UWorld* World = GetWorld();
	const ASnowRumbleCharacter* Character =
		Cast<ASnowRumbleCharacter>(GetOwner());
	if (!World
		|| !Character
		|| !HeldSnowball
		|| ViewDirection.IsNearlyZero()
		|| AimTraceDistance <= 0.0f)
	{
		return false;
	}

	const FVector TraceEnd =
		ViewLocation + ViewDirection.GetSafeNormal() * AimTraceDistance;
	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(SnowballAimTrace),
		false,
		Character);
	QueryParams.AddIgnoredActor(HeldSnowball);

	FHitResult AimHit;
	const bool bHit = World->LineTraceSingleByChannel(
		AimHit,
		ViewLocation,
		TraceEnd,
		ECC_Visibility,
		QueryParams);
	OutAimTarget = bHit ? AimHit.ImpactPoint : TraceEnd;
	return true;
}

bool USnowballEquipmentComponent::BuildCurrentThrowView(
	FVector& OutViewLocation,
	FVector& OutViewDirection) const
{
	const APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn)
	{
		return false;
	}

	const UCameraComponent* FollowCamera =
		OwningPawn->FindComponentByClass<UCameraComponent>();
	OutViewLocation =
		FollowCamera
			? FollowCamera->GetComponentLocation()
			: OwningPawn->GetPawnViewLocation();
	OutViewDirection =
		FollowCamera
			? FollowCamera->GetForwardVector()
			: OwningPawn->GetBaseAimRotation().Vector();
	return !OutViewLocation.ContainsNaN()
		&& !OutViewDirection.ContainsNaN()
		&& !OutViewDirection.IsNearlyZero();
}

void USnowballEquipmentComponent::ExecutePendingThrowFromServer(
	const FVector& ViewLocation,
	const FVector& ViewDirection)
{
	ASnowRumbleCharacter* Character = Cast<ASnowRumbleCharacter>(GetOwner());
	if (!Character
		|| !Character->HasAuthority()
		|| Character->IsFrozen()
		|| !bHasPendingThrow
		|| !HeldSnowball
		|| ViewLocation.ContainsNaN()
		|| ViewDirection.ContainsNaN()
		|| FVector::DistSquared(
			ViewLocation,
			Character->GetActorLocation())
			> FMath::Square(MaximumAimViewOriginDistance)
		|| FVector::DotProduct(
			ViewDirection.GetSafeNormal(),
			Character->GetBaseAimRotation().Vector()) < 0.7f)
	{
		ClearPendingThrow();
		return;
	}

	FVector AimTarget = FVector::ZeroVector;
	if (!FindServerAimTarget(ViewLocation, ViewDirection, AimTarget))
	{
		ClearPendingThrow();
		return;
	}

	const FVector AimDirection =
		(AimTarget - HeldSnowball->GetActorLocation()).GetSafeNormal();
	if (AimDirection.IsNearlyZero())
	{
		ClearPendingThrow();
		return;
	}

	ASnowballItem* SnowballToThrow = HeldSnowball;
	const FVector ThrowDirection =
		IsHoldingLargeSnowball()
			? (AimDirection
				+ FVector::UpVector * LargeSnowballArcLift).GetSafeNormal()
			: AimDirection;
	const float ThrowSpeed = PendingThrowSpeed;
	const float ThrowChargeProgress = PendingThrowChargeProgress;
	const float ThrowDamageMultiplier = PendingThrowDamageMultiplier;

	ClearPendingThrow();

	if (!SnowballToThrow->Throw(
		ThrowDirection,
		ThrowSpeed,
		ThrowChargeProgress,
		ThrowDamageMultiplier))
	{
		return;
	}

	HeldSnowball = nullptr;
	bIsAiming = false;
	OnRep_HeldSnowball();
	OnRep_IsAiming();
	Character->ForceNetUpdate();
}

void USnowballEquipmentComponent::ClearPendingThrow()
{
	bHasPendingThrow = false;
	PendingThrowSpeed = 0.0f;
	PendingThrowChargeProgress = 0.0f;
	PendingThrowDamageMultiplier = 1.0f;
}

void USnowballEquipmentComponent::SetChargingState(bool bNewCharging)
{
	const bool bValidatedCharging = bNewCharging && CanThrowHeldSnowball();
	if (bIsCharging == bValidatedCharging)
	{
		return;
	}

	bIsCharging = bValidatedCharging;
	ChargeStartTime =
		bIsCharging && GetWorld()
			? GetWorld()->GetTimeSeconds()
			: -1.0;
	OnRep_IsCharging();
}

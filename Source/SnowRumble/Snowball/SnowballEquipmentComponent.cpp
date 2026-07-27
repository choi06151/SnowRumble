// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowballEquipmentComponent.h"

#include "../Player/SnowRumbleCharacter.h"
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
		|| Character->IsPickingUpItem()
		|| FVector::DistSquared2D(
			Character->GetActorLocation(),
			RollingSnowball->GetActorLocation())
			> FMath::Square(MaximumRollingSeparation))
	{
		ServerStopRollingSnowball_Implementation();
		return;
	}

	const FVector MovementDirection = Character->GetVelocity().GetSafeNormal2D();
	if (MovementDirection.IsNearlyZero())
	{
		return;
	}

	FVector TargetLocation =
		Character->GetActorLocation()
		+ MovementDirection * RollingDistance;
	TargetLocation.Z = RollingSnowball->GetActorLocation().Z;
	RollingSnowball->MoveRollingSnowball(TargetLocation);
	RollingSnowball->UpdateRollingGrowth();
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
	return HasHeldSnowball() && bIsAiming;
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

	const FVector ThrowDirection = OwningPawn->GetBaseAimRotation().Vector();
	if (OwningPawn->HasAuthority())
	{
		ServerReleaseChargedSnowball_Implementation(ThrowDirection);
		return;
	}

	SetChargingState(false);
	ServerReleaseChargedSnowball(ThrowDirection);
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

	SetChargingState(false);

	if (!OwningPawn->HasAuthority())
	{
		ServerCancelCharging();
	}
}

bool USnowballEquipmentComponent::IsCharging() const
{
	return bIsCharging && CanThrowHeldSnowball();
}

float USnowballEquipmentComponent::GetChargeProgress() const
{
	const UWorld* World = GetWorld();
	if (!World || !IsCharging() || ChargeStartTime < 0.0 || MaximumChargeSeconds <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(
		static_cast<float>((World->GetTimeSeconds() - ChargeStartTime) / MaximumChargeSeconds),
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
		Character->GetSnowballHoldPoint()))
	{
		HeldSnowball = PickupCandidate;
		Character->NotifyItemPickupSucceeded();
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
	FVector_NetQuantizeNormal ThrowDirection)
{
	ASnowRumbleCharacter* Character = Cast<ASnowRumbleCharacter>(GetOwner());
	UWorld* World = GetWorld();
	if (!Character
		|| !World
		|| Character->IsFrozen()
		|| !IsCharging()
		|| ThrowDirection.ContainsNaN()
		|| FVector::DotProduct(
			ThrowDirection.GetSafeNormal(),
			Character->GetBaseAimRotation().Vector()) < 0.7f)
	{
		CancelCharging();
		return;
	}

	const float ChargeProgress = FMath::Clamp(
		static_cast<float>((World->GetTimeSeconds() - ChargeStartTime) / MaximumChargeSeconds),
		0.0f,
		1.0f);
	const float ThrowSpeed = FMath::Lerp(
		MinimumThrowSpeed,
		MaximumThrowSpeed,
		ChargeProgress);

	ASnowballItem* SnowballToThrow = HeldSnowball;
	SetChargingState(false);

	if (!SnowballToThrow || !SnowballToThrow->Throw(ThrowDirection, ThrowSpeed))
	{
		return;
	}

	HeldSnowball = nullptr;
	bIsAiming = false;
	OnRep_HeldSnowball();
	OnRep_IsAiming();
	Character->ForceNetUpdate();
}

void USnowballEquipmentComponent::ServerCancelCharging_Implementation()
{
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
	Character->GetCapsuleComponent()->IgnoreActorWhenMoving(
		RollingSnowball,
		true);
	Character->ForceNetUpdate();
}

void USnowballEquipmentComponent::ServerStopRollingSnowball_Implementation()
{
	if (RollingSnowball)
	{
		if (ASnowRumbleCharacter* Character = Cast<ASnowRumbleCharacter>(GetOwner()))
		{
			Character->GetCapsuleComponent()->IgnoreActorWhenMoving(
				RollingSnowball,
				false);
		}

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

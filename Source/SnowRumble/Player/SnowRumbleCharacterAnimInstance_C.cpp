// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleCharacterAnimInstance_C.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PawnMovementComponent.h"

void USnowRumbleCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CachedCharacter = Cast<ASnowRumbleCharacter>(TryGetPawnOwner());
	RefreshFromOwnerCharacter();
}

void USnowRumbleCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CachedCharacter)
	{
		CachedCharacter = Cast<ASnowRumbleCharacter>(TryGetPawnOwner());
	}

	RefreshFromOwnerCharacter();
}

bool USnowRumbleCharacterAnimInstance::HasUpperBodyOverride() const
{
	return UpperBodyAnimState != ESnowRumbleUpperBodyAnimState::None;
}

bool USnowRumbleCharacterAnimInstance::HasFullBodyOverride() const
{
	return FullBodyAnimState != ESnowRumbleFullBodyAnimState::None;
}

void USnowRumbleCharacterAnimInstance::RefreshFromOwnerCharacter()
{
	if (!CachedCharacter)
	{
		ResetAnimationState();
		return;
	}

	const UPawnMovementComponent* MovementComponent =
		CachedCharacter->GetMovementComponent();
	const FVector Velocity = MovementComponent
		? MovementComponent->Velocity
		: CachedCharacter->GetVelocity();

	GroundSpeed = Velocity.Size2D();
	bIsMoving = CachedCharacter->IsMoving();
	bIsInAir = CachedCharacter->IsInAir();
	bIsSprinting = CachedCharacter->IsSprinting();
	bIsFrozen = CachedCharacter->IsFrozen();
	bIsDead = CachedCharacter->IsDead();
	bIsAiming = CachedCharacter->IsAiming();
	bIsChargingSnowball = CachedCharacter->IsChargingSnowball();
	bIsCreatingSnowball = CachedCharacter->IsCreatingSnowball();
	bIsPickingUpItem = CachedCharacter->IsPickingUpItem();
	bIsInteractingWithItem = CachedCharacter->IsInteractingWithItem();
	bIsHitReacting = CachedCharacter->IsHitReacting();
	bIsGrabReaching = CachedCharacter->IsGrabReaching();
	bIsGrabbingCharacter = CachedCharacter->IsGrabbingCharacter();
	bIsGrabAttached = CachedCharacter->IsGrabAttached();
	bIsHangingFromWorldGrab = CachedCharacter->IsHangingFromWorldGrab();
	bIsGrabbedByCharacter = CachedCharacter->IsGrabbedByCharacter();
	GrabAttachedWorldLocation =
		CachedCharacter->GetGrabAttachedWorldLocation();
	GrabbedByCharacterWorldLocation =
		CachedCharacter->GetGrabbedByCharacterWorldLocation();
	GrabbedByCharacterComponentLocation = FVector::ZeroVector;
	if (bIsGrabbedByCharacter)
	{
		if (const USkeletalMeshComponent* MeshComponent = GetSkelMeshComponent())
		{
			GrabbedByCharacterComponentLocation =
				MeshComponent->GetComponentTransform().InverseTransformPosition(
					GrabbedByCharacterWorldLocation);
		}
	}
	RightHandGrabTargetLocation =
		CachedCharacter->GetRightHandGrabTargetLocation();
	LeftHandGrabTargetLocation =
		CachedCharacter->GetLeftHandGrabTargetLocation();
	GrabReachAlpha =
		FMath::Clamp(CachedCharacter->GetGrabReachAlpha(), 0.0f, 1.0f);
	ViewPitchDegrees = CachedCharacter->GetViewPitchDegrees();
	ViewPitchAlpha =
		FMath::Clamp(CachedCharacter->GetViewPitchAlpha(), 0.0f, 1.0f);
	ViewYawDegrees = CachedCharacter->GetViewYawDegrees();
	ViewYawAlpha =
		FMath::Clamp(CachedCharacter->GetViewYawAlpha(), -0.5f, 0.5f);
	SnowballCarryState = CachedCharacter->GetSnowballCarryState();
	HeldAnimationState = CachedCharacter->GetHeldAnimationState();
	SnowballActionState = CachedCharacter->GetSnowballActionState();
	TimedActionState = CachedCharacter->GetTimedActionState();
	SnowballChargeProgress =
		FMath::Clamp(CachedCharacter->GetSnowballChargeProgress(), 0.0f, 1.0f);
	SnowballCreationProgress =
		FMath::Clamp(CachedCharacter->GetSnowballCreationProgress(), 0.0f, 1.0f);
	RefreshDerivedAnimationStates();
}

void USnowRumbleCharacterAnimInstance::RefreshDerivedAnimationStates()
{
	if (bIsInAir)
	{
		LocomotionAnimState = ESnowRumbleLocomotionAnimState::InAir;
	}
	else if (bIsSprinting)
	{
		LocomotionAnimState = ESnowRumbleLocomotionAnimState::Sprint;
	}
	else if (bIsMoving)
	{
		LocomotionAnimState = ESnowRumbleLocomotionAnimState::Walk;
	}
	else
	{
		LocomotionAnimState = ESnowRumbleLocomotionAnimState::Idle;
	}

	if (bIsChargingSnowball)
	{
		switch (HeldAnimationState)
		{
		case ESnowRumbleHeldAnimationState::LargeSnowball:
			UpperBodyAnimState =
				ESnowRumbleUpperBodyAnimState::LargeSnowballCharge;
			break;
		case ESnowRumbleHeldAnimationState::SnowShovel:
			UpperBodyAnimState =
				ESnowRumbleUpperBodyAnimState::SnowShovelCharge;
			break;
		case ESnowRumbleHeldAnimationState::SnowDuckMaker:
			UpperBodyAnimState =
				ESnowRumbleUpperBodyAnimState::SnowDuckMakerCharge;
			break;
		case ESnowRumbleHeldAnimationState::SmallSnowball:
		default:
			UpperBodyAnimState =
				ESnowRumbleUpperBodyAnimState::SmallSnowballCharge;
			break;
		}
	}
	else if (bIsAiming)
	{
		switch (HeldAnimationState)
		{
		case ESnowRumbleHeldAnimationState::LargeSnowball:
			UpperBodyAnimState =
				ESnowRumbleUpperBodyAnimState::LargeSnowballAim;
			break;
		case ESnowRumbleHeldAnimationState::SnowShovel:
			UpperBodyAnimState =
				ESnowRumbleUpperBodyAnimState::SnowShovelAim;
			break;
		case ESnowRumbleHeldAnimationState::SnowDuckMaker:
			UpperBodyAnimState =
				ESnowRumbleUpperBodyAnimState::SnowDuckMakerAim;
			break;
		case ESnowRumbleHeldAnimationState::SmallSnowball:
		default:
			UpperBodyAnimState =
				ESnowRumbleUpperBodyAnimState::SmallSnowballAim;
			break;
		}
	}
	else if (SnowballCarryState == ESnowballCarryState::LargeSnowball)
	{
		UpperBodyAnimState = ESnowRumbleUpperBodyAnimState::LargeSnowball;
	}
	else if (SnowballCarryState == ESnowballCarryState::SmallSnowball)
	{
		UpperBodyAnimState = ESnowRumbleUpperBodyAnimState::SmallSnowball;
	}
	else if (HeldAnimationState == ESnowRumbleHeldAnimationState::SnowShovel)
	{
		UpperBodyAnimState = ESnowRumbleUpperBodyAnimState::SnowShovel;
	}
	else if (
		HeldAnimationState == ESnowRumbleHeldAnimationState::SnowDuckMaker)
	{
		UpperBodyAnimState = ESnowRumbleUpperBodyAnimState::SnowDuckMaker;
	}
	else
	{
		UpperBodyAnimState = ESnowRumbleUpperBodyAnimState::None;
	}

	if (bIsDead)
	{
		FullBodyAnimState = ESnowRumbleFullBodyAnimState::Dead;
	}
	else if (bIsFrozen)
	{
		FullBodyAnimState = ESnowRumbleFullBodyAnimState::Frozen;
	}
	else if (bIsHitReacting)
	{
		FullBodyAnimState = ESnowRumbleFullBodyAnimState::HitReact;
	}
	else if (bIsInteractingWithItem)
	{
		FullBodyAnimState = ESnowRumbleFullBodyAnimState::ItemInteraction;
	}
	else if (bIsPickingUpItem)
	{
		FullBodyAnimState = ESnowRumbleFullBodyAnimState::Pickup;
	}
	else if (SnowballActionState == ESnowballActionState::RollingSnowball)
	{
		FullBodyAnimState = ESnowRumbleFullBodyAnimState::RollSnowball;
	}
	else if (bIsCreatingSnowball)
	{
		FullBodyAnimState = ESnowRumbleFullBodyAnimState::CreateSnowball;
	}
	else
	{
		FullBodyAnimState = ESnowRumbleFullBodyAnimState::None;
	}
}

void USnowRumbleCharacterAnimInstance::ResetAnimationState()
{
	GroundSpeed = 0.0f;
	bIsMoving = false;
	bIsInAir = false;
	bIsSprinting = false;
	bIsFrozen = false;
	bIsDead = false;
	bIsAiming = false;
	bIsChargingSnowball = false;
	bIsCreatingSnowball = false;
	bIsPickingUpItem = false;
	bIsInteractingWithItem = false;
	bIsHitReacting = false;
	bIsGrabReaching = false;
	bIsGrabbingCharacter = false;
	bIsGrabAttached = false;
	bIsHangingFromWorldGrab = false;
	bIsGrabbedByCharacter = false;
	GrabAttachedWorldLocation = FVector::ZeroVector;
	GrabbedByCharacterWorldLocation = FVector::ZeroVector;
	GrabbedByCharacterComponentLocation = FVector::ZeroVector;
	RightHandGrabTargetLocation = FVector::ZeroVector;
	LeftHandGrabTargetLocation = FVector::ZeroVector;
	GrabReachAlpha = 0.0f;
	ViewPitchDegrees = 0.0f;
	ViewPitchAlpha = 0.5f;
	ViewYawDegrees = 0.0f;
	ViewYawAlpha = 0.0f;
	SnowballCarryState = ESnowballCarryState::Normal;
	HeldAnimationState = ESnowRumbleHeldAnimationState::BareHands;
	SnowballActionState = ESnowballActionState::None;
	TimedActionState = ESnowRumbleTimedActionState::None;
	SnowballChargeProgress = 0.0f;
	SnowballCreationProgress = 0.0f;
	LocomotionAnimState = ESnowRumbleLocomotionAnimState::Idle;
	UpperBodyAnimState = ESnowRumbleUpperBodyAnimState::None;
	FullBodyAnimState = ESnowRumbleFullBodyAnimState::None;
}

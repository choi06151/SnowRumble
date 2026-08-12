// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleCharacterAnimInstance_C.h"

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

UAnimSequenceBase* USnowRumbleCharacterAnimInstance::GetPrimaryAnimation() const
{
	if (bIsDead && DeadAnimation)
	{
		return DeadAnimation;
	}
	if (bIsFrozen && FrozenAnimation)
	{
		return FrozenAnimation;
	}
	if (bIsPickingUpItem && PickupAnimation)
	{
		return PickupAnimation;
	}
	if (SnowballActionState == ESnowballActionState::RollingSnowball
		&& RollSnowballAnimation)
	{
		return RollSnowballAnimation;
	}
	if (bIsCreatingSnowball && CreateSnowballAnimation)
	{
		return CreateSnowballAnimation;
	}
	if (bIsChargingSnowball && SnowballChargeAnimation)
	{
		return SnowballChargeAnimation;
	}
	if (bIsAiming)
	{
		if (bIsMoving && AimWalkAnimation)
		{
			return AimWalkAnimation;
		}
		if (AimIdleAnimation)
		{
			return AimIdleAnimation;
		}
	}
	if (SnowballCarryState == ESnowballCarryState::LargeSnowball
		&& LargeSnowballHoldAnimation)
	{
		return LargeSnowballHoldAnimation;
	}
	if (SnowballCarryState == ESnowballCarryState::SmallSnowball
		&& SmallSnowballHoldAnimation)
	{
		return SmallSnowballHoldAnimation;
	}
	if (bIsInAir && JumpOrFallAnimation)
	{
		return JumpOrFallAnimation;
	}
	if (bIsSprinting && SprintAnimation)
	{
		return SprintAnimation;
	}
	if (bIsMoving && WalkAnimation)
	{
		return WalkAnimation;
	}

	return IdleAnimation;
}

void USnowRumbleCharacterAnimInstance::RefreshFromOwnerCharacter()
{
	if (!CachedCharacter)
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
		SnowballCarryState = ESnowballCarryState::Normal;
		SnowballActionState = ESnowballActionState::None;
		TimedActionState = ESnowRumbleTimedActionState::None;
		SnowballChargeProgress = 0.0f;
		SnowballCreationProgress = 0.0f;
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
	SnowballCarryState = CachedCharacter->GetSnowballCarryState();
	SnowballActionState = CachedCharacter->GetSnowballActionState();
	TimedActionState = CachedCharacter->GetTimedActionState();
	SnowballChargeProgress =
		FMath::Clamp(CachedCharacter->GetSnowballChargeProgress(), 0.0f, 1.0f);
	SnowballCreationProgress =
		FMath::Clamp(CachedCharacter->GetSnowballCreationProgress(), 0.0f, 1.0f);
}

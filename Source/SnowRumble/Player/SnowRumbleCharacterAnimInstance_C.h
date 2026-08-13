// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SnowRumbleCharacter.h"
#include "SnowRumbleCharacterAnimInstance_C.generated.h"

class UAnimSequenceBase;

UCLASS(Blueprintable)
class SNOWRUMBLE_API USnowRumbleCharacterAnimInstance
	: public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/** 현재 상태 우선순위에 맞는 주 애니메이션 슬롯을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	UAnimSequenceBase* GetPrimaryAnimation() const;

	/** ABP가 이동 BlendSpace 등에 사용할 지상 이동 속도다. */
	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|State")
	float GroundSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|State")
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|State")
	bool bIsInAir = false;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|State")
	bool bIsSprinting = false;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|State")
	bool bIsFrozen = false;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|State")
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|State")
	bool bIsAiming = false;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|State")
	bool bIsChargingSnowball = false;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|State")
	bool bIsCreatingSnowball = false;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|State")
	bool bIsPickingUpItem = false;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|State")
	bool bIsInteractingWithItem = false;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|State")
	bool bIsHitReacting = false;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|State")
	ESnowballCarryState SnowballCarryState = ESnowballCarryState::Normal;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|State")
	ESnowRumbleHeldAnimationState HeldAnimationState =
		ESnowRumbleHeldAnimationState::BareHands;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|State")
	ESnowballActionState SnowballActionState = ESnowballActionState::None;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|State")
	ESnowRumbleTimedActionState TimedActionState =
		ESnowRumbleTimedActionState::None;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|State")
	float SnowballChargeProgress = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|State")
	float SnowballCreationProgress = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation|Slots|Locomotion")
	TObjectPtr<UAnimSequenceBase> IdleAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation|Slots|Locomotion")
	TObjectPtr<UAnimSequenceBase> WalkAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation|Slots|Locomotion")
	TObjectPtr<UAnimSequenceBase> SprintAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation|Slots|Locomotion")
	TObjectPtr<UAnimSequenceBase> JumpOrFallAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation|Slots|Aim")
	TObjectPtr<UAnimSequenceBase> AimIdleAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation|Slots|Aim")
	TObjectPtr<UAnimSequenceBase> AimWalkAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation|Slots|Snowball")
	TObjectPtr<UAnimSequenceBase> SmallSnowballHoldAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation|Slots|Snowball")
	TObjectPtr<UAnimSequenceBase> LargeSnowballHoldAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation|Slots|Tool")
	TObjectPtr<UAnimSequenceBase> SnowShovelHoldAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation|Slots|Tool")
	TObjectPtr<UAnimSequenceBase> SnowDuckMakerHoldAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation|Slots|Snowball")
	TObjectPtr<UAnimSequenceBase> SnowballChargeAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation|Slots|Snowball")
	TObjectPtr<UAnimSequenceBase> CreateSnowballAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation|Slots|Snowball")
	TObjectPtr<UAnimSequenceBase> RollSnowballAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation|Slots|Action")
	TObjectPtr<UAnimSequenceBase> PickupAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation|Slots|Action")
	TObjectPtr<UAnimSequenceBase> ItemInteractionAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation|Slots|Action")
	TObjectPtr<UAnimSequenceBase> HitReactAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation|Slots|Health")
	TObjectPtr<UAnimSequenceBase> FrozenAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation|Slots|Health")
	TObjectPtr<UAnimSequenceBase> DeadAnimation;

protected:
	void RefreshFromOwnerCharacter();

	UPROPERTY(Transient, BlueprintReadOnly, Category = "SnowRumble|Animation")
	TObjectPtr<ASnowRumbleCharacter> CachedCharacter;
};

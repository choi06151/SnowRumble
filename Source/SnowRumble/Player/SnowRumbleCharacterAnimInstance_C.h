// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SnowRumbleCharacter.h"
#include "SnowRumbleCharacterAnimationTypes_C.h"
#include "SnowRumbleCharacterAnimInstance_C.generated.h"

UENUM(BlueprintType)
enum class ESnowRumbleLocomotionAnimState : uint8
{
	Idle,
	Walk,
	Sprint,
	InAir
};

UENUM(BlueprintType)
enum class ESnowRumbleUpperBodyAnimState : uint8
{
	None,
	Aim,
	SmallSnowball,
	LargeSnowball,
	SnowShovel,
	SnowDuckMaker,
	ChargeSnowball
};

UENUM(BlueprintType)
enum class ESnowRumbleFullBodyAnimState : uint8
{
	None,
	CreateSnowball,
	RollSnowball,
	Pickup,
	ItemInteraction,
	HitReact,
	Frozen,
	Dead
};

UCLASS(Blueprintable)
class SNOWRUMBLE_API USnowRumbleCharacterAnimInstance
	: public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/** 상체 전용 pose를 현재 locomotion 위에 섞어야 하는지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation", meta = (BlueprintThreadSafe))
	bool HasUpperBodyOverride() const;

	/** 전체 몸 action pose가 최종 pose를 덮어써야 하는지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation", meta = (BlueprintThreadSafe))
	bool HasFullBodyOverride() const;

	/** 서버가 확정한 one-shot 동작을 ABP 몽타주/슬롯으로 재생한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Animation|Trigger")
	void OnAnimationTriggerRequested(ESnowRumbleCharacterAnimTrigger Trigger);

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

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|Derived State")
	ESnowRumbleLocomotionAnimState LocomotionAnimState =
		ESnowRumbleLocomotionAnimState::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|Derived State")
	ESnowRumbleUpperBodyAnimState UpperBodyAnimState =
		ESnowRumbleUpperBodyAnimState::None;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Animation|Derived State")
	ESnowRumbleFullBodyAnimState FullBodyAnimState =
		ESnowRumbleFullBodyAnimState::None;

protected:
	void RefreshFromOwnerCharacter();
	void RefreshDerivedAnimationStates();
	void ResetAnimationState();

	UPROPERTY(Transient, BlueprintReadOnly, Category = "SnowRumble|Animation")
	TObjectPtr<ASnowRumbleCharacter> CachedCharacter;
};

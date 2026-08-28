// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerGrabComponent_C.generated.h"

class ASnowRumbleCharacter;
class AGrabbablePhysicsObject;
class ASnowballItem;
class UPhysicsConstraintComponent;
class UPrimitiveComponent;
class USphereComponent;
class USkeletalMeshComponent;
class USoundAttenuation;
class USoundBase;

UENUM(BlueprintType)
enum class ESnowRumbleGrabHand : uint8
{
	Right,
	Left
};

UENUM(BlueprintType)
enum class ESnowRumbleGrabAttachmentType : uint8
{
	None,
	Character,
	World,
	PhysicsObject
};

UCLASS(ClassGroup = (SnowRumble), meta = (BlueprintSpawnableComponent))
class SNOWRUMBLE_API UPlayerGrabComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerGrabComponent();

	/** 소유 플레이어가 팔 뻗기 잡기 상태를 시작하도록 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Grab")
	void StartGrabReach();

	/** 소유 플레이어가 팔 뻗기 잡기 상태와 현재 잡기를 해제하도록 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Grab")
	void StopGrabReach();

	/** Control Rig가 팔을 앞으로 뻗어야 하는지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	bool IsGrabReaching() const;

	/** 현재 서버가 다른 캐릭터 Mesh를 잡은 상태인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	bool IsGrabbingCharacter() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	bool IsCarryingOpposingFrozenCharacter() const;

	/** 현재 손이 캐릭터나 월드 지형에 붙은 상태인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	bool IsGrabAttached() const;

	/** 현재 Grabable 물건을 잡고 있는지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	bool IsGrabbingPhysicsObject() const;

	/** 현재 손이 벽이나 월드 오브젝트에 붙어 매달린 상태인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	bool IsHangingFromWorldGrab() const;

	/** Control Rig가 붙은 손을 고정할 월드 위치를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	FVector GetGrabAttachedWorldLocation() const;

	/** Control Rig가 사용할 오른손 월드 목표 위치를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	FVector GetRightHandGrabTargetLocation() const;

	/** Control Rig가 사용할 왼손 월드 목표 위치를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	FVector GetLeftHandGrabTargetLocation() const;

	/** 잡기 손 IK와 AnimDynamics 보간에 사용할 0~1 alpha다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	float GetGrabReachAlpha() const;

	/** 잡기 제한 시간이 남은 비율을 1에서 0으로 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	float GetGrabRemainingTimeProgress() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void ServerStartGrabReach();

	UFUNCTION(Server, Reliable)
	void ServerStopGrabReach();

	UFUNCTION()
	void OnRep_IsGrabReaching();

	UFUNCTION()
	void OnRep_GrabbedCharacter();

	UFUNCTION()
	void OnRep_GrabAttachmentType();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayGrabSound(
		FVector_NetQuantize Location,
		bool bReleased);

	/** 서버가 잡기 입력과 현재 상태를 검사한다. */
	bool CanStartGrabReach() const;

	/** 손 목표 위치 기준으로 손이 붙을 수 있는 캐릭터 Mesh 또는 월드 표면을 찾는다. */
	bool FindGrabCandidate(
		ASnowRumbleCharacter*& OutCharacter,
		USkeletalMeshComponent*& OutMesh,
		AGrabbablePhysicsObject*& OutPhysicsObject,
		UPrimitiveComponent*& OutPhysicsComponent,
		ASnowballItem*& OutSnowball,
		FName& OutBoneName,
		FVector& OutAttachedWorldLocation,
		ESnowRumbleGrabAttachmentType& OutAttachmentType,
		bool bAllowWorldAttachment) const;

	/** 서버가 현재 손과 대상 Mesh를 물리 constraint로 연결한다. */
	void AttachGrabConstraint(
		ASnowRumbleCharacter* TargetCharacter,
		USkeletalMeshComponent* TargetMesh,
		FName TargetBoneName,
		FVector AttachedWorldLocation);

	void AttachPhysicsObject(
		AGrabbablePhysicsObject* TargetObject,
		UPrimitiveComponent* TargetComponent,
		FVector AttachedWorldLocation);

	/** 서버가 현재 손을 월드 표면에 붙은 상태로 확정한다. */
	void AttachWorldGrab(FVector AttachedWorldLocation);

	/** 서버와 클라이언트 표현 상태에서 현재 잡기를 해제한다. */
	void ClearGrabConstraint();

	/** 서버가 잡힌 캐릭터 몸을 잡은 손 위치 근처로 부드럽게 끌어당긴다. */
	void UpdateGrabbedCharacterTether(float DeltaTime);

	/** 서버가 벽에 붙은 손 위치 기준으로 소유 캐릭터 몸을 매달린 위치에 유지한다. */
	void UpdateWorldGrabTether(float DeltaTime);

	/** 서버가 물리 물건 전용 앵커를 손 위치로 갱신한다. */
	void UpdatePhysicsObjectGrabTether(float DeltaTime);

	/** 물리 물건 Grab 중 플레이어가 카메라 Yaw를 따라 회전하도록 전환한다. */
	void ApplyPhysicsObjectRotationMode();

	/** 물리 물건 Grab 전 회전 설정을 복원한다. */
	void ClearPhysicsObjectRotationMode();

	/** 팔 뻗기 목표 위치를 캐릭터 기준으로 계산한다. */
	FVector BuildHandGrabTargetLocation(ESnowRumbleGrabHand Hand) const;

	/** 실제 Mesh 손 bone/socket 위치를 우선 사용해 잡힌 대상을 끌 기준점을 계산한다. */
	FVector BuildHandGrabAnchorLocation(ESnowRumbleGrabHand Hand) const;

	/** 좌클릭 Grab reach 중 카메라 Yaw를 따라 캐릭터가 회전하도록 설정한다. */
	void ApplyGrabReachRotationMode();

	/** Grab reach 종료 시 기존 캐릭터 회전 설정을 복원한다. */
	void ClearGrabReachRotationMode();

	/** 플레이어를 붙잡고 있는 동안 잡는 캐릭터가 카메라 Yaw를 따라 돌도록 보정한다. */
	void UpdateGrabOwnerRotationToControlYaw(float DeltaTime);

	ASnowRumbleCharacter* GetOwnerCharacter() const;

	/** 서버 시각을 가져와 잡기 제한과 회복을 같은 시간축으로 계산한다. */
	float GetCurrentServerTimeSeconds() const;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_IsGrabReaching, Category = "SnowRumble|Grab")
	bool bIsGrabReaching = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_GrabbedCharacter, Category = "SnowRumble|Grab")
	TObjectPtr<ASnowRumbleCharacter> GrabbedCharacter;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_GrabbedPhysicsObject, Category = "SnowRumble|Grab")
	TObjectPtr<AGrabbablePhysicsObject> GrabbedPhysicsObject;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_GrabAttachmentType, Category = "SnowRumble|Grab")
	ESnowRumbleGrabAttachmentType GrabAttachmentType =
		ESnowRumbleGrabAttachmentType::None;

	UFUNCTION()
	void OnRep_GrabbedPhysicsObject();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "SnowRumble|Grab")
	FVector GrabAttachedWorldLocation = FVector::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "SnowRumble|Grab")
	FName GrabbedTargetBoneName = NAME_None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "SnowRumble|Grab")
	ESnowRumbleGrabHand ActiveGrabHand = ESnowRumbleGrabHand::Right;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "SnowRumble|Grab")
	float GrabReachStartedServerTime = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "SnowRumble|Grab")
	float GrabAttachmentStartedServerTime = 0.0f;

	/** 현재 잡기 게이지의 저장된 잔량이다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "SnowRumble|Grab|Limit")
	float GrabHoldProgress = 1.0f;

	/** 현재 연결을 시작할 때의 게이지 잔량이다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "SnowRumble|Grab|Limit")
	float GrabProgressAtAttachmentStart = 1.0f;

	/** 잡기 해제 후 회복을 시작할 서버 시각이다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "SnowRumble|Grab|Limit")
	float GrabRecoveryStartedServerTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Limit", meta = (ClampMin = "0.0"))
	float MaximumGrabHoldSeconds = 5.0f;

	/** 잡기 해제 후 게이지가 회복되기 전 대기 시간이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Limit", meta = (ClampMin = "0.0"))
	float GrabRecoveryDelaySeconds = 1.0f;

	/** 대기 시간이 지난 뒤 게이지가 완전히 회복되는 시간이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Limit", meta = (ClampMin = "0.0"))
	float GrabRecoverySeconds = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Control Rig", meta = (ClampMin = "0.0"))
	float GrabReachForwardDistance = 95.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Control Rig", meta = (ClampMin = "0.0"))
	float GrabReachRaiseInterpSpeed = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Control Rig", meta = (ClampMin = "0.0"))
	float GrabReachLowerInterpSpeed = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Control Rig")
	float GrabReachUpOffset = 35.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Control Rig")
	float GrabReachSideOffset = 28.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Trace", meta = (ClampMin = "0.0"))
	float GrabTraceRadius = 18.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Trace", meta = (ClampMin = "0.0"))
	float GrabTraceBacktrackDistance = 28.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Trace", meta = (ClampMin = "0.0"))
	float GrabTraceForwardDistance = 22.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Trace|Debug")
	bool bDrawGrabTraceDebug = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Trace|Debug", meta = (ClampMin = "0.0"))
	float GrabTraceDebugDrawSeconds = 0.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Trace", meta = (ClampMin = "0.0"))
	float WorldGrabMinReachHoldSeconds = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Trace", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinGrabReachAlphaForAttachment = 0.65f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Trace", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WorldGrabMaxSurfaceNormalZ = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Trace")
	float WorldGrabMinAttachHeightFromActor = -10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Trace")
	float WorldGrabMaxAttachHeightFromActor = 180.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Socket")
	FName RightGrabHandBoneName = TEXT("hand_r");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Socket")
	FName LeftGrabHandBoneName = TEXT("hand_l");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Audio")
	TObjectPtr<USoundBase> GrabSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Audio")
	TObjectPtr<USoundBase> ReleaseGrabSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Audio")
	TObjectPtr<USoundAttenuation> GrabSoundAttenuation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0"))
	float GrabTetherSlackDistance = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0"))
	float GrabTetherPullStrength = 7.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0"))
	float GrabTetherMaxPullSpeed = 520.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GrabTetherVelocityDamping = 0.45f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GrabbedCharacterInputVelocityRetention = 0.55f;

	/** 잡힌 캐릭터 당김 속도가 순간적으로 바뀌지 않도록 보간하는 속도다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0"))
	float GrabbedCharacterTetherVelocityInterpSpeed = 8.0f;

	/** 잡힌 캐릭터가 손 위치를 따라갈 때 사용할 위치 보간 속도다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0"))
	float GrabbedCharacterLocationInterpSpeed = 14.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0"))
	float GrabbedCharacterFacingInterpSpeed = 14.0f;

	/** 플레이어를 잡은 캐릭터가 카메라 Yaw를 따라 회전하는 보간 속도다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0"))
	float GrabOwnerControlYawInterpSpeed = 24.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0"))
	float WorldGrabBodyBackOffset = 42.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0"))
	float WorldGrabBodyDownOffset = 96.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0"))
	float WorldGrabTetherSlackDistance = 12.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0"))
	float WorldGrabTetherPullStrength = 9.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0"))
	float WorldGrabTetherMaxPullSpeed = 620.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0"))
	float WorldGrabTetherMaxUpwardSpeed = 120.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WorldGrabInputVelocityRetention = 0.55f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0"))
	float WorldGrabFacingInterpSpeed = 14.0f;

	UPROPERTY(Transient)
	TObjectPtr<UPhysicsConstraintComponent> GrabConstraintComponent;

	UPROPERTY(Transient)
	TObjectPtr<UPhysicsConstraintComponent> GrabCollisionConstraintComponent;

	UPROPERTY(Transient)
	TObjectPtr<USphereComponent> GrabPhysicsAnchorComponent;

	UPROPERTY(Transient)
	TObjectPtr<UPrimitiveComponent> GrabbedPhysicsComponent;

	UPROPERTY(Transient)
	bool bGrabbedPhysicsWasSimulating = false;

	UPROPERTY(Transient)
	bool bGrabbedPhysicsGravityEnabled = true;

	UPROPERTY(Transient)
	FVector LastGrabbedPhysicsLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FTransform GrabbedPhysicsRelativeTransform = FTransform::Identity;

	/** 물리 물건을 잡은 순간의 시점 Pitch다. 이후 Pitch 변화만 물건에 적용한다. */
	UPROPERTY(Transient)
	float GrabbedPhysicsGrabViewPitchDegrees = 0.0f;

	UPROPERTY(Transient)
	bool bHasPhysicsObjectRotationOverride = false;

	UPROPERTY(Transient)
	bool bHasGrabReachRotationOverride = false;

	UPROPERTY(Transient)
	bool bUseControllerRotationYawBeforeGrabReach = false;

	UPROPERTY(Transient)
	bool bOrientRotationToMovementBeforeGrabReach = true;

	UPROPERTY(Transient)
	bool bUseControllerRotationYawBeforePhysicsObjectGrab = false;

	UPROPERTY(Transient)
	bool bOrientRotationToMovementBeforePhysicsObjectGrab = true;

	UPROPERTY(Transient)
	float CurrentGrabReachAlpha = 0.0f;

	UPROPERTY(Transient)
	double GrabReachStartedTimeSeconds = 0.0;

	UPROPERTY(Transient)
	FVector GrabbedActorLocationOffsetFromAttachedPoint = FVector::ZeroVector;
};

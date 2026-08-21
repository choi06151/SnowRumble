// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerGrabComponent_C.generated.h"

class ASnowRumbleCharacter;
class UPhysicsConstraintComponent;
class USkeletalMeshComponent;

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
	World
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

	/** 현재 손이 캐릭터나 월드 지형에 붙은 상태인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	bool IsGrabAttached() const;

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

	/** 서버가 잡기 입력과 현재 상태를 검사한다. */
	bool CanStartGrabReach() const;

	/** 손 목표 위치 기준으로 손이 붙을 수 있는 캐릭터 Mesh 또는 월드 표면을 찾는다. */
	bool FindGrabCandidate(
		ASnowRumbleCharacter*& OutCharacter,
		USkeletalMeshComponent*& OutMesh,
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

	/** 서버가 현재 손을 월드 표면에 붙은 상태로 확정한다. */
	void AttachWorldGrab(FVector AttachedWorldLocation);

	/** 서버와 클라이언트 표현 상태에서 현재 잡기를 해제한다. */
	void ClearGrabConstraint();

	/** 서버가 잡힌 캐릭터 몸을 잡은 손 위치 근처로 부드럽게 끌어당긴다. */
	void UpdateGrabbedCharacterTether(float DeltaTime);

	/** 서버가 벽에 붙은 손 위치 기준으로 소유 캐릭터 몸을 매달린 위치에 유지한다. */
	void UpdateWorldGrabTether(float DeltaTime);

	/** 팔 뻗기 목표 위치를 캐릭터 기준으로 계산한다. */
	FVector BuildHandGrabTargetLocation(ESnowRumbleGrabHand Hand) const;

	/** 실제 Mesh 손 bone/socket 위치를 우선 사용해 잡힌 대상을 끌 기준점을 계산한다. */
	FVector BuildHandGrabAnchorLocation(ESnowRumbleGrabHand Hand) const;

	ASnowRumbleCharacter* GetOwnerCharacter() const;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_IsGrabReaching, Category = "SnowRumble|Grab")
	bool bIsGrabReaching = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_GrabbedCharacter, Category = "SnowRumble|Grab")
	TObjectPtr<ASnowRumbleCharacter> GrabbedCharacter;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_GrabAttachmentType, Category = "SnowRumble|Grab")
	ESnowRumbleGrabAttachmentType GrabAttachmentType =
		ESnowRumbleGrabAttachmentType::None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "SnowRumble|Grab")
	FVector GrabAttachedWorldLocation = FVector::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "SnowRumble|Grab")
	FName GrabbedTargetBoneName = NAME_None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "SnowRumble|Grab")
	ESnowRumbleGrabHand ActiveGrabHand = ESnowRumbleGrabHand::Right;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0"))
	float GrabTetherSlackDistance = 18.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0"))
	float GrabTetherPullStrength = 7.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0"))
	float GrabTetherMaxPullSpeed = 520.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GrabTetherVelocityDamping = 0.45f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GrabbedCharacterInputVelocityRetention = 0.55f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Constraint", meta = (ClampMin = "0.0"))
	float GrabbedCharacterFacingInterpSpeed = 14.0f;

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
	float CurrentGrabReachAlpha = 0.0f;

	UPROPERTY(Transient)
	double GrabReachStartedTimeSeconds = 0.0;

	UPROPERTY(Transient)
	FVector GrabbedActorLocationOffsetFromAttachedPoint = FVector::ZeroVector;
};

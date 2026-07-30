// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SnowRumbleCharacter.generated.h"

class UCameraComponent;
class UDamageType;
class UInputAction;
class UInputMappingContext;
class UNiagaraComponent;
class UOutlineComponent;
class USceneComponent;
class USnowRumbleHealthComponent;
class USnowballCreationComponent;
class USnowballEquipmentComponent;
class USpringArmComponent;
class AController;
struct FDamageEvent;
struct FInputActionValue;

UENUM(BlueprintType)
enum class ESnowballCarryState : uint8
{
	Normal,
	SmallSnowball,
	LargeSnowball
};

UENUM(BlueprintType)
enum class ESnowballActionState : uint8
{
	None,
	RollingSnowball
};

UCLASS()
class SNOWRUMBLE_API ASnowRumbleCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASnowRumbleCharacter();

	virtual void Tick(float DeltaSeconds) override;

	/** Animation Blueprint에서 수평 이동 중인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsMoving() const;

	/** Animation Blueprint에서 공중 상태인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsInAir() const;

	/** Animation Blueprint에서 실제 스프린트 이동 중인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsSprinting() const;

	/** Animation Blueprint에서 얼기 상태인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsFrozen() const;

	/** Animation Blueprint와 UI에서 눈덩이 장착 여부를 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsHoldingSnowball() const;

	/** Animation Blueprint에서 평소·작은 눈·큰 눈 운반 상태를 구분한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	ESnowballCarryState GetSnowballCarryState() const;

	/** Animation Blueprint에서 운반 상태와 별개인 눈덩이 행동 상태를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	ESnowballActionState GetSnowballActionState() const;

	/** Animation Blueprint에서 눈덩이 조준 상태인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsAiming() const;

	/** Animation Blueprint에서 눈덩이 투척 충전 중인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsChargingSnowball() const;

	/** UI에서 사용할 0~1 정규화된 눈덩이 충전량을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	float GetSnowballChargeProgress() const;

	/** Animation Blueprint에서 눈덩이 제작 중인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsCreatingSnowball() const;

	/** Animation Blueprint에서 아이템 획득 동작 중인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsPickingUpItem() const;

	/** UI에서 사용할 0~1 정규화된 눈덩이 제작 진행도를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	float GetSnowballCreationProgress() const;

	/** 눈덩이를 부착할 캐릭터의 조정 가능한 장착 위치를 반환한다. */
	USceneComponent* GetSnowballHoldPoint() const;

	/** 서버에서 아이템 획득 성공 애니메이션 상태를 시작한다. */
	void NotifyItemPickupSucceeded();

	/** 보유 장비가 바뀌면 스프린트와 현재 최대 이동속도를 다시 적용한다. */
	void RefreshHeldEquipmentMovementState();

	/** 서버에서 받은 피해를 HP 컴포넌트에 전달한다. */
	virtual float TakeDamage(
		float DamageAmount,
		FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** 이동 입력을 카메라의 수평 방향 기준 이동으로 변환한다. */
	void Move(const FInputActionValue& Value);

	/** 마우스 입력을 로컬 플레이어의 카메라 회전에 적용한다. */
	void Look(const FInputActionValue& Value);

	/** 점프 입력이 시작되면 캐릭터 점프를 요청한다. */
	void StartJump();

	/** 점프 입력이 끝나면 점프 요청을 해제한다. */
	void StopJump();

	/** 스프린트 입력이 시작되면 로컬 예측과 서버 요청을 시작한다. */
	void HandleSprintStarted();

	/** 스프린트 입력이 끝나면 로컬 예측과 서버 상태를 해제한다. */
	void HandleSprintCompleted();

	/** 상호작용 입력의 누름 또는 해제 상태를 Blueprint에 전달한다. */
	void HandleInteractStarted();

	/** 상호작용 입력 해제 상태를 Blueprint에 전달한다. */
	void HandleInteractCompleted();

	/** 조준 입력 누름 상태를 Blueprint에 전달한다. */
	void HandleAimStarted();

	/** 조준 입력 해제 상태를 Blueprint에 전달한다. */
	void HandleAimCompleted();

	/** 상황별 행동 입력 누름 상태를 Blueprint에 전달한다. */
	void HandleActionStarted();

	/** 상황별 행동 입력 해제 상태를 Blueprint에 전달한다. */
	void HandleActionCompleted();

	/** 장비 내려놓기 입력을 Blueprint에 전달한다. */
	void HandleDropEquipment();

	/** 이모션 입력 누름 상태를 Blueprint에 전달한다. */
	void HandleEmoteStarted();

	/** 이모션 입력 해제 상태를 Blueprint에 전달한다. */
	void HandleEmoteCompleted();

	/** 로컬 플레이어에게 현재 캐릭터의 입력 매핑 컨텍스트를 적용한다. */
	void ApplyInputMappingContext();

	/** 로컬 PlayerCameraManager에 안전한 상하 시야각을 적용한다. */
	void ApplyCameraPitchLimits();

	/** 자신이 조종하는 캐릭터의 카메라에서만 눈 VFX를 활성화한다. */
	void RefreshLocalSnowEffect();

	/** 얼기 상태에 따라 캐릭터 이동을 중지하거나 복구한다. */
	UFUNCTION()
	void HandleFrozenChanged(bool bIsFrozen);

	/** 조준 상태에 따라 로컬 카메라와 모든 화면의 이동속도를 갱신한다. */
	UFUNCTION()
	void HandleSnowballAimingChanged(bool bNewAiming);

	/** 현재 캐릭터가 이동과 일반 행동을 수행할 수 있는지 확인한다. */
	bool CanPerformGameplayAction() const;

	/** 스프린트 상태에 맞는 최대 이동속도를 CharacterMovement에 적용한다. */
	void ApplyMovementSpeed();

	/** 서버가 소유 클라이언트의 스프린트 상태 요청을 검사하고 확정한다. */
	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bNewSprinting);

	/** 복제된 스프린트 상태를 다른 화면의 이동속도와 표현에 적용한다. */
	UFUNCTION()
	void OnRep_IsSprinting();

	/** 서버에서 아이템 획득 애니메이션 상태를 종료한다. */
	void FinishPickupAnimationState();

	/** 복제된 획득 상태에 따라 이동 잠금과 복구를 적용한다. */
	UFUNCTION()
	void OnRep_IsPickingUpItem();

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Input")
	void OnInteractInput(bool bPressed);

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Input")
	void OnAimInput(bool bPressed);

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Input")
	void OnActionInput(bool bPressed);

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Input")
	void OnDropEquipmentInput();

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Input")
	void OnEmoteInput(bool bPressed);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Camera")
	TObjectPtr<UNiagaraComponent> LocalSnowEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Health")
	TObjectPtr<USnowRumbleHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball")
	TObjectPtr<USnowballEquipmentComponent> SnowballEquipmentComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball")
	TObjectPtr<USnowballCreationComponent> SnowballCreationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball")
	TObjectPtr<USceneComponent> SnowballHoldPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Interaction")
	TObjectPtr<UOutlineComponent> OutlineComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputMappingContext> PlayerMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> ActionAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> DropEquipmentAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> EmoteAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Movement", meta = (ClampMin = "0.0"))
	float WalkSpeed = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Movement", meta = (ClampMin = "0.0"))
	float SprintSpeed = 750.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Movement", meta = (ClampMin = "0.0"))
	float AimWalkSpeed = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera", meta = (ClampMin = "5.0", ClampMax = "170.0"))
	float AimFieldOfView = 75.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera", meta = (ClampMin = "0.0"))
	float AimFieldOfViewInterpSpeed = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera", meta = (ClampMin = "0.0"))
	float DefaultShoulderOffset = 75.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera", meta = (ClampMin = "0.0"))
	float AimShoulderOffset = 90.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera", meta = (ClampMin = "0.0"))
	float AimCameraArmLength = 340.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera", meta = (ClampMin = "0.0"))
	float CameraPositionInterpSpeed = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera", meta = (ClampMin = "0.0"))
	float PostThrowCameraHoldSeconds = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera")
	float CameraPivotHeight = 65.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera", meta = (ClampMin = "-89.0", ClampMax = "0.0"))
	float CameraViewPitchMin = -65.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera", meta = (ClampMin = "0.0", ClampMax = "89.0"))
	float CameraViewPitchMax = 55.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation", meta = (ClampMin = "0.01"))
	float PickupAnimationStateDuration = 0.6f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_IsSprinting, Category = "SnowRumble|Movement")
	bool bIsSprinting = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_IsPickingUpItem, Category = "SnowRumble|Animation")
	bool bIsPickingUpItem = false;

	float DefaultFieldOfView = 90.0f;
	FVector DefaultCameraSocketOffset = FVector::ZeroVector;
	float DefaultCameraArmLength = 400.0f;
	float CameraShoulderSide = 1.0f;
	double PostThrowAimCameraEndTime = -1.0;

	FTimerHandle PickupAnimationTimerHandle;

	bool bIsInteractHeld = false;
	bool bUsedInteractForRolling = false;

};

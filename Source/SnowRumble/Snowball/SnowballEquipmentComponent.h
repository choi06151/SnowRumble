// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SnowballEquipmentComponent.generated.h"

class ASnowballItem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnHeldSnowballChanged,
	ASnowballItem*,
	HeldSnowball);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnSnowballAimingChanged,
	bool,
	bIsAiming);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnSnowballChargingChanged,
	bool,
	bIsCharging);

UCLASS(ClassGroup = (SnowRumble), meta = (BlueprintSpawnableComponent))
class SNOWRUMBLE_API USnowballEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USnowballEquipmentComponent();

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** 소유 플레이어가 가까운 바닥 눈덩이 획득을 서버에 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Snowball")
	void TryPickupSnowball();

	/** 현재 플레이어가 눈덩이를 들고 있는지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	bool HasHeldSnowball() const;

	/** 현재 들고 있는 눈덩이를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	ASnowballItem* GetHeldSnowball() const;

	/** 현재 최대 성장 큰 눈덩이를 들고 있는지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	bool IsHoldingLargeSnowball() const;

	/** 최대 성장 큰 눈덩이 운반 중 적용할 이동속도를 반환한다. */
	float GetLargeSnowballCarryWalkSpeed() const;

	/** 획득 범위 안에서 가장 가까운 바닥 눈덩이를 찾는다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	ASnowballItem* FindClosestPickupCandidate() const;

	/** 소유 플레이어의 조준 요청을 로컬에 예측하고 서버에 전달한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Snowball")
	void SetAiming(bool bNewAiming);

	/** 현재 눈덩이를 들고 조준 중인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	bool IsAiming() const;

	/** 서버가 현재 보유 눈덩이를 던질 수 있는 상태인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	bool CanThrowHeldSnowball() const;

	/** 조준 중인 소유 플레이어가 투척 충전을 시작한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Snowball")
	void StartCharging();

	/** 소유 플레이어가 충전을 끝내고 화면 중앙 방향으로 투척을 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Snowball")
	void ReleaseChargedSnowball();

	/** 투척 없이 진행 중인 충전을 취소한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Snowball")
	void CancelCharging();

	/** 현재 투척 충전 중인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	bool IsCharging() const;

	/** 소유 화면 또는 서버에서 0~1 정규화된 현재 충전량을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	float GetChargeProgress() const;

	/** 소유 플레이어가 현재 눈덩이를 바닥에 내려놓도록 서버에 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Snowball")
	void DropHeldSnowball();

	/** 소유 플레이어가 가까운 바닥 눈덩이 굴리기를 요청한다. */
	void StartRollingSnowball();

	/** 소유 플레이어가 현재 눈덩이 굴리기 종료를 요청한다. */
	void StopRollingSnowball();

	/** 현재 플레이어가 눈덩이를 굴리는 중인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	bool IsRollingSnowball() const;

	/** 현재 굴리고 있는 눈덩이를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	ASnowballItem* GetRollingSnowball() const;

	/** 현재 성장 크기를 반영한 굴리기 이동속도를 반환한다. */
	float GetRollingWalkSpeed() const;

	UPROPERTY(BlueprintAssignable, Category = "SnowRumble|Snowball")
	FOnHeldSnowballChanged OnHeldSnowballChanged;

	UPROPERTY(BlueprintAssignable, Category = "SnowRumble|Snowball")
	FOnSnowballAimingChanged OnAimingChanged;

	UPROPERTY(BlueprintAssignable, Category = "SnowRumble|Snowball")
	FOnSnowballChargingChanged OnChargingChanged;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 서버가 현재 플레이어 상태와 거리를 다시 검사해 획득을 확정한다. */
	UFUNCTION(Server, Reliable)
	void ServerTryPickupSnowball();

	/** 서버가 보유·행동 가능 상태를 검사해 조준 상태를 확정한다. */
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bNewAiming);

	UFUNCTION(Server, Reliable)
	void ServerStartCharging();

	UFUNCTION(Server, Reliable)
	void ServerReleaseChargedSnowball(FVector_NetQuantizeNormal ThrowDirection);

	UFUNCTION(Server, Reliable)
	void ServerCancelCharging();

	/** 서버가 실제 보유 상태를 검사하고 캐릭터 앞에 눈덩이를 내려놓는다. */
	UFUNCTION(Server, Reliable)
	void ServerDropHeldSnowball();

	/** 서버가 거리와 상태를 검사해 가까운 눈덩이 굴리기를 시작한다. */
	UFUNCTION(Server, Reliable)
	void ServerStartRollingSnowball();

	/** 서버가 현재 굴리기 대상의 바닥 물리를 복구한다. */
	UFUNCTION(Server, Reliable)
	void ServerStopRollingSnowball();

	/** 복제된 보유 눈덩이 변경을 Animation Blueprint와 UI 연결 지점에 알린다. */
	UFUNCTION()
	void OnRep_HeldSnowball();

	/** 복제된 조준 상태 변경을 캐릭터 표현과 UI 연결 지점에 알린다. */
	UFUNCTION()
	void OnRep_IsAiming();

	UFUNCTION()
	void OnRep_IsCharging();

	/** 현재 보유한 눈덩이 크기에 맞는 최대 충전시간을 반환한다. */
	float GetCurrentMaximumChargeSeconds() const;

	void SetChargingState(bool bNewCharging);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball", meta = (ClampMin = "0.0"))
	float PickupRadius = 180.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_HeldSnowball, Category = "SnowRumble|Snowball")
	TObjectPtr<ASnowballItem> HeldSnowball;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_IsAiming, Category = "SnowRumble|Snowball")
	bool bIsAiming = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_IsCharging, Category = "SnowRumble|Snowball")
	bool bIsCharging = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "SnowRumble|Snowball")
	TObjectPtr<ASnowballItem> RollingSnowball;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw", meta = (ClampMin = "0.1"))
	float MaximumChargeSeconds = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw", meta = (ClampMin = "0.0"))
	float MinimumThrowSpeed = 900.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw", meta = (ClampMin = "0.0"))
	float MaximumThrowSpeed = 2400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Large", meta = (ClampMin = "0.1"))
	float LargeSnowballMaximumChargeSeconds = 3.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Large", meta = (ClampMin = "0.0"))
	float LargeSnowballMinimumThrowSpeed = 700.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Large", meta = (ClampMin = "0.0"))
	float LargeSnowballMaximumThrowSpeed = 1400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Large", meta = (ClampMin = "0.0"))
	float LargeSnowballArcLift = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Rolling", meta = (ClampMin = "0.0"))
	float MaximumRollingSeparation = 250.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Rolling", meta = (ClampMin = "0.0"))
	float RollingDistance = 90.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Rolling", meta = (ClampMin = "0.0"))
	float SmallSnowballRollingWalkSpeed = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Rolling", meta = (ClampMin = "0.0"))
	float LargeSnowballRollingWalkSpeed = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Rolling", meta = (ClampMin = "0.0"))
	float RollingObstaclePushSpeed = 120.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Carry", meta = (ClampMin = "0.0"))
	float LargeSnowballCarryWalkSpeed = 200.0f;

	double ChargeStartTime = -1.0;
};

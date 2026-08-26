// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SnowballEquipmentComponent.generated.h"

class ASnowballItem;
class ASnowRumbleCharacter;
class AActor;
class USplineComponent;
class USplineMeshComponent;
class UStaticMesh;
class UMaterialInterface;
class UNiagaraComponent;
class UNiagaraSystem;

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

	/** 서버가 새로 만든 눈덩이를 즉시 손에 장착한다. */
	bool EquipCreatedSnowballFromServer(ASnowballItem* CreatedSnowball);

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

	/** 던지기 몽타주의 AnimNotify 시점에 현재 카메라 조준으로 보류 중인 투척을 실제 발사한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Snowball")
	void ConfirmPendingThrowFromAnimationNotify();

	/** 로컬 소유자의 큰 눈덩이 투척 궤적과 착탄 표시를 즉시 숨긴다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Snowball")
	void HideThrowTrajectoryPreview();

	/** 투척 없이 진행 중인 충전을 취소한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Snowball")
	void CancelCharging();

	/** 피격 등으로 던지기 연출이 끊겼을 때 보유 눈덩이는 유지하고 충전/보류 투척만 정리한다. */
	void InterruptThrowStateFromServer();

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

	/** 서버가 카메라 조준 정보를 검사하고 조준점 방향으로 충전 투척을 확정한다. */
	UFUNCTION(Server, Reliable)
	void ServerReleaseChargedSnowball(
		FVector_NetQuantize ViewLocation,
		FVector_NetQuantizeNormal ViewDirection);

	UFUNCTION(Server, Reliable)
	void ServerConfirmPendingThrowFromAnimationNotify(
		FVector_NetQuantize ViewLocation,
		FVector_NetQuantizeNormal ViewDirection);

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

	/** 굴리는 동안 서버가 눈덩이를 유지할 캐릭터 앞 위치를 계산한다. */
	FVector BuildRollingSnowballTargetLocation(
		const ASnowRumbleCharacter* Character,
		const ASnowballItem* Snowball) const;

	/** 서버가 검증한 카메라 Line Trace로 화면 중앙의 월드 조준점을 찾는다. */
	bool FindServerAimTarget(
		const FVector& ViewLocation,
		const FVector& ViewDirection,
		FVector& OutAimTarget) const;

	/** 로컬 소유자의 현재 카메라 또는 Pawn 시야 정보를 얻는다. */
	bool BuildCurrentThrowView(
		FVector& OutViewLocation,
		FVector& OutViewDirection) const;

	/** 서버가 저장해 둔 속도·충전값과 Notify 시점 조준으로 실제 눈덩이 투척을 처리한다. */
	void ExecutePendingThrowFromServer(
		const FVector& ViewLocation,
		const FVector& ViewDirection);

	/** 로컬 조준 화면에 큰 눈덩이의 포물선 미리보기를 갱신한다. */
	void UpdateThrowTrajectoryPreview();

	/** 로컬 조준 화면의 포물선 미리보기를 비운다. */
	void ClearThrowTrajectoryPreview();

	/** 보류 중인 던지기 값을 모두 초기화한다. */
	void ClearPendingThrow();

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

	/** 공중에서 작은 눈덩이를 던질 때 적용할 피해 배율이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Airborne", meta = (ClampMin = "0.0"))
	float AirborneThrowDamageMultiplier = 1.5f;

	/** 공중에서 작은 눈덩이를 던질 때 적용할 속도 배율이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Airborne", meta = (ClampMin = "0.0"))
	float AirborneThrowSpeedMultiplier = 1.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Aim", meta = (ClampMin = "0.0"))
	float AimTraceDistance = 10000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Aim", meta = (ClampMin = "0.0"))
	float MaximumAimViewOriginDistance = 1000.0f;

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

	/** SnowSurface 위에서 굴릴 때 공 중심에 적용할 월드 Z 오프셋이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Rolling")
	float RollingSnowballSnowSurfaceZOffset = 0.0f;

	/** 일반 지면 위에서 굴릴 때 공 위치에 적용할 월드 Z 오프셋이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Rolling")
	float RollingSnowballGroundZOffset = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Rolling", meta = (ClampMin = "0.0"))
	float RollingGroundTraceUpDistance = 120.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Rolling", meta = (ClampMin = "0.0"))
	float RollingGroundTraceDownDistance = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Rolling", meta = (ClampMin = "0.0"))
	float SmallSnowballRollingWalkSpeed = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Rolling", meta = (ClampMin = "0.0"))
	float LargeSnowballRollingWalkSpeed = 150.0f;

	/** 큰 눈덩이 포물선 미리보기에서 사용할 샘플 개수다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Trajectory", meta = (ClampMin = "2", ClampMax = "64"))
	int32 TrajectoryPreviewSampleCount = 24;

	/** 큰 눈덩이 포물선 미리보기의 샘플 시간 간격이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Trajectory", meta = (ClampMin = "0.01"))
	float TrajectoryPreviewTimeStep = 0.08f;

	/** 큰 눈덩이 포물선 미리보기를 화면에 표시할지 여부다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Trajectory")
	bool bDrawTrajectoryPreview = true;

	/** 패키징에서도 표시할 큰 눈덩이 포물선용 Static Mesh다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Trajectory")
	TObjectPtr<UStaticMesh> TrajectoryPreviewMesh;

	/** 큰 눈덩이 포물선 메쉬에 적용할 Material이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Trajectory")
	TObjectPtr<UMaterialInterface> TrajectoryPreviewMaterial;

	/** 큰 눈덩이 포물선 메쉬에 적용할 색상이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Trajectory")
	FLinearColor TrajectoryPreviewColor = FLinearColor(0.2f, 0.75f, 1.0f, 1.0f);

	/** 큰 눈덩이 포물선 메쉬에서 우선 적용할 머티리얼 슬롯 이름이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Trajectory")
	FName TrajectoryPreviewMaterialSlotName = TEXT("TransformGizmoMaterial");

	/** 포물선 Spline Mesh의 단면 스케일이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Trajectory", meta = (ClampMin = "0.01"))
	float TrajectoryPreviewMeshScale = 1.0f;

	/** 큰 눈덩이 착탄 예상 위치에 표시할 액터 블루프린트다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Trajectory")
	TSubclassOf<AActor> TrajectoryPreviewLandingActorClass;

	/** 큰 눈덩이 궤적 표시를 Niagara로도 출력할지 여부다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Trajectory|Niagara")
	bool bUseTrajectoryPreviewNiagara = true;

	/** BP에서 플레이어에 미리 붙여둔 궤적 Niagara 컴포넌트를 지정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnowRumble|Snowball|Throw|Trajectory|Niagara")
	TObjectPtr<UNiagaraComponent> TrajectoryPreviewNiagaraComponent;

	/** 지정 컴포넌트가 없을 때 런타임 생성에 사용할 궤적 Niagara 시스템이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Trajectory|Niagara")
	TObjectPtr<UNiagaraSystem> TrajectoryPreviewNiagaraSystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Trajectory|Niagara")
	FName TrajectoryPreviewNiagaraPointsParameter = TEXT("User.TrajectoryPoints");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Trajectory|Niagara")
	FName TrajectoryPreviewNiagaraPointCountParameter = TEXT("User.TrajectoryPointCount");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Trajectory|Niagara")
	FName TrajectoryPreviewNiagaraColorParameter = TEXT("User.TrajectoryColor");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Trajectory|Niagara")
	FName TrajectoryPreviewNiagaraStartParameter = TEXT("User.TrajectoryStart");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Trajectory|Niagara")
	FName TrajectoryPreviewNiagaraEndParameter = TEXT("User.TrajectoryEnd");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Rolling", meta = (ClampMin = "0.0"))
	float RollingObstaclePushSpeed = 120.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Carry", meta = (ClampMin = "0.0"))
	float LargeSnowballCarryWalkSpeed = 200.0f;

	UPROPERTY(Transient)
	TObjectPtr<USplineComponent> ThrowTrajectorySpline;

	UPROPERTY(Transient)
	TArray<TObjectPtr<USplineMeshComponent>> ThrowTrajectorySplineMeshes;

	/** 착탄 예상 위치 표시용 로컬 액터다. */
	UPROPERTY(Transient)
	TObjectPtr<AActor> TrajectoryPreviewLandingActor;

	bool bHideTrajectoryPreviewForCurrentThrow = false;

	double ChargeStartTime = -1.0;
	bool bHasPendingThrow = false;
	FVector LastRollingMovementDirection = FVector::ForwardVector;
	float PendingThrowSpeed = 0.0f;
	float PendingThrowChargeProgress = 0.0f;
	float PendingThrowDamageMultiplier = 1.0f;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SnowIslandWaterPressureActor_J.generated.h"

class ASnowRumbleCharacter;
class ACampfire;
class USceneComponent;

UENUM(BlueprintType)
enum class ESnowIslandWaterPressureStage : uint8
{
	Stable,
	OuterFlood,
	CentralFlood,
	Overtime
};

struct FWaterSubmersionState
{
	float DamageProgressSeconds = 0.0f;
	float TimeSinceExitSeconds = 0.0f;
};

UCLASS(Blueprintable)
class SNOWRUMBLE_API ASnowIslandWaterPressureActor : public AActor
{
	GENERATED_BODY()

public:
	ASnowIslandWaterPressureActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 현재 물 상승 단계를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snow Island Water")
	ESnowIslandWaterPressureStage GetCurrentWaterStage() const;

	/** 현재 Water Surface World Z를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snow Island Water")
	float GetCurrentWaterZ() const;

	/** PvP GameMode의 맵 축소 이벤트에 맞춰 물 상승 단계를 시작한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Snow Island Water")
	void StartWaterPressureFromMapShrink(
		int32 ShrinkStage,
		float RoundElapsedSeconds,
		float ShrinkDurationSeconds);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 복제된 물 상태를 클라이언트의 기존 Water Actor 표현에 반영한다. */
	UFUNCTION()
	void OnRep_WaterPressureState();

	/** Runtime에서 실제로 움직일 유일한 Water Actor다. Water2/Water3는 목표 높이 참고용으로만 둔다. */
	UPROPERTY(EditInstanceOnly, Category = "SnowRumble|Snow Island Water|Water")
	TObjectPtr<AActor> ControlledWaterActor;

	/** 이동시킬 기존 Water Component 이름이다. 비어 있거나 찾지 못하면 RootComponent를 사용한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Water")
	FName WaterComponentName = TEXT("StaticMeshComponent0");

	/** Water Actor의 현재 World Z를 시작 수위로 사용한다. 꺼져 있으면 ManualInitialWaterZ를 사용한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Water")
	bool bUseControlledWaterInitialZ = true;

	/** Water Actor 참조가 없거나 시작 수위를 직접 지정할 때 사용할 World Z다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Water")
	float ManualInitialWaterZ = 0.0f;

	/** 안정 단계가 끝나는 경기 시간이다. 이 시간 전에는 물 Damage도 비활성화한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Timing", meta = (ClampMin = "0.0"))
	float StableEndSeconds = 240.0f;

	/** 외곽 침수 단계가 끝나는 경기 시간이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Timing", meta = (ClampMin = "0.0"))
	float OuterFloodEndSeconds = 300.0f;

	/** 중앙 축소 단계가 끝나는 경기 시간이다. 이후는 연장전이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Timing", meta = (ClampMin = "0.0"))
	float CentralFloodEndSeconds = 360.0f;

	/** 240~300초 구간 종료 시 도달할 Water Surface World Z다. Water2 높이를 참고해 Editor에서 설정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Tuning")
	float OuterFloodWaterZ = 0.0f;

	/** 300~360초 구간 종료 시 도달할 Water Surface World Z다. Water3 높이를 참고해 Editor에서 설정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Tuning")
	float CentralFloodWaterZ = 0.0f;

	/** 연장전에서 매초 추가 상승할 World Z 거리다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Tuning")
	float OvertimeRiseSpeed = 0.0f;

	/** 서버가 Character별 침수 상태를 확인하는 주기다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Damage", meta = (ClampMin = "0.01"))
	float DamageCheckIntervalSeconds = 0.1f;

	/** Character별 유효 침수시간이 이 값에 도달하면 Damage를 1회 적용한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Damage", meta = (ClampMin = "0.01"))
	float DamageApplyIntervalSeconds = 1.0f;

	/** 침수 중인 플레이어에게 서버가 한 번에 적용할 Damage다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Damage", meta = (ClampMin = "0.0"))
	float DamagePerTick = 8.0f;

	/** 이 시간 이상 물 밖에 있어야 침수 누적시간을 초기화한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Damage", meta = (ClampMin = "0.0"))
	float ExitGraceSeconds = 0.5f;

	/** PIE 진단용 Water Damage 로그를 출력한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Debug")
	bool bLogWaterDamageDebug = false;

	/** Capsule 하단 World Z에 더할 보정값이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Submersion")
	float SubmersionSampleOffsetZ = 0.0f;

	/** 발바닥 기준 Sample Z가 Water Surface보다 이 값 이상 낮을 때 침수로 본다. 0이면 물이 발에 닿는 즉시 판정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Submersion", meta = (ClampMin = "0.0"))
	float RequiredSubmersionDepth = 0.0f;

	/** 침수 중인 캐릭터를 수면 근처로 약하게 띄울지 정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Buoyancy")
	bool bApplyWaterBuoyancy = true;

	/** Capsule 하단이 수면보다 이 정도 아래에 머물도록 부력을 적용한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Buoyancy", meta = (ClampMin = "0.0"))
	float BuoyancyTargetSubmersionDepth = 45.0f;

	/** 수면 목표 높이와 현재 침수 sample 차이에 곱할 상승 보정 강도다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Buoyancy", meta = (ClampMin = "0.0"))
	float BuoyancyCorrectionSpeed = 3.5f;

	/** 부력 적용 시 보장할 최소 상승 속도다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Buoyancy", meta = (ClampMin = "0.0"))
	float BuoyancyMinimumUpwardVelocity = 80.0f;

	/** 부력 적용 시 허용할 최대 상승 속도다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Buoyancy", meta = (ClampMin = "0.0"))
	float BuoyancyMaximumUpwardVelocity = 360.0f;

	/** 물속에서 일정한 부력 위에 약한 통통 튀는 상승 펄스를 더할지 정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Buoyancy")
	bool bApplyWaterBounce = true;

	/** 물속 통통 튐이 1초에 반복되는 횟수다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Buoyancy", meta = (ClampMin = "0.0"))
	float WaterBounceFrequency = 1.35f;

	/** 물속 통통 튐으로 추가할 최대 상승 속도다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Island Water|Buoyancy", meta = (ClampMin = "0.0"))
	float WaterBounceUpwardVelocity = 120.0f;

private:
	/** 지정 Actor에서 움직일 Water Component를 찾는다. */
	USceneComponent* ResolveWaterComponent(AActor* WaterActor) const;

	/** 실제 Water Component와 시작 World Z를 초기화한다. */
	void InitializeControlledWater();

	/** 현재 경기 시간에 대응하는 물 상승 단계다. */
	ESnowIslandWaterPressureStage CalculateWaterStage(float RoundElapsedSeconds) const;

	/** 현재 경기 시간에 대응하는 Water Surface World Z다. */
	float CalculateWaterZ(float RoundElapsedSeconds) const;

	/** 맵 축소 단계에 대응하는 물 상승 단계다. */
	ESnowIslandWaterPressureStage CalculateWaterStageFromShrinkStage(
		int32 ShrinkStage) const;

	/** 맵 축소 단계가 끝났을 때 도달할 Water Surface World Z다. */
	float CalculateWaterTargetZFromShrinkStage(int32 ShrinkStage) const;

	/** 맵 축소 이벤트 기반 현재 Water Surface World Z다. */
	float CalculateEventDrivenWaterZ() const;

	/** 현재 Water Surface World Z를 기존 Water Actor에 적용한다. */
	void ApplyWaterZ(float WaterZ);

	/** 서버 Damage Timer에서 침수된 플레이어를 찾아 피해를 요청한다. */
	void HandleDamageTimerElapsed();

	/** 서버 Damage Timer에서 물에 닿은 모닥불을 즉시 끈다. */
	void ExtinguishSubmergedCampfires();

	/** 모닥불 기준 위치가 현재 물에 닿았는지 서버 기준으로 판정한다. */
	bool IsCampfireSubmerged(const ACampfire* Campfire) const;

	/** Character Capsule 하단 기준 침수 Sample World Z를 계산한다. */
	float CalculateSubmersionSampleZ(
		const ASnowRumbleCharacter* Character,
		float& OutCapsuleHalfHeight) const;

	/** Character가 현재 물에 잠긴 상태인지 서버 기준으로 판정한다. */
	bool IsCharacterSubmerged(const ASnowRumbleCharacter* Character) const;

	/** 침수 중인 Character가 수면 근처에서 둥둥 뜨도록 서버 이동 속도를 보정한다. */
	void ApplyWaterBuoyancyToCharacter(
		ASnowRumbleCharacter* Character,
		float SubmersionSampleZ) const;

	/** 기존 Damage 시스템으로 Hazard Damage를 요청한다. C-07 완료 후 이 함수 내부만 교체한다. */
	void RequestHazardDamage(AActor* Target, float DamageAmount);

	/** GameState의 서버 동기화 경기 시간을 읽는다. */
	float GetRoundElapsedSeconds() const;

	TWeakObjectPtr<USceneComponent> ControlledWaterComponent;
	float InitialWaterZ = 0.0f;
	TMap<TWeakObjectPtr<ASnowRumbleCharacter>, FWaterSubmersionState> SubmersionStates;

	UPROPERTY(ReplicatedUsing = OnRep_WaterPressureState)
	ESnowIslandWaterPressureStage CurrentWaterStage = ESnowIslandWaterPressureStage::Stable;

	UPROPERTY(ReplicatedUsing = OnRep_WaterPressureState)
	float CurrentWaterZ = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_WaterPressureState)
	bool bUseMapShrinkEventTiming = false;

	UPROPERTY(ReplicatedUsing = OnRep_WaterPressureState)
	float MapShrinkWaterStartServerTime = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_WaterPressureState)
	float MapShrinkWaterDurationSeconds = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_WaterPressureState)
	float MapShrinkWaterStartZ = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_WaterPressureState)
	float MapShrinkWaterTargetZ = 0.0f;

	FTimerHandle DamageTimerHandle;
};

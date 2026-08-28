// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IceGlacierWindGustActor_J.generated.h"

class ASnowRumbleCharacter;
class UCharacterMovementComponent;
class USceneComponent;

UENUM(BlueprintType)
enum class EIceGlacierWindGustState : uint8
{
	Idle,
	Warning,
	Gust
};

UCLASS(Blueprintable)
class SNOWRUMBLE_API AIceGlacierWindGustActor : public AActor
{
	GENERATED_BODY()

public:
	AIceGlacierWindGustActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 현재 서버가 확정하고 복제한 강풍 상태다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Ice Glacier Wind")
	EIceGlacierWindGustState GetCurrentWindState() const;

	/** 현재 Warning 또는 Gust에 사용할 월드 기준 강풍 방향이다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Ice Glacier Wind")
	FVector GetCurrentWindDirection() const;

protected:
	/** 강풍 Warning이 시작될 때 Blueprint 표현을 연결한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Ice Glacier Wind")
	void OnWindWarningStarted(FVector WindDirection);

	/** 실제 강풍이 시작될 때 Blueprint 표현을 연결한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Ice Glacier Wind")
	void OnWindGustStarted(FVector WindDirection, float Strength);

	/** 실제 강풍이 끝날 때 Blueprint 표현을 정리한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Ice Glacier Wind")
	void OnWindGustEnded();

	/** 실제 플레이 시작 후 다음 강풍까지 서버가 랜덤으로 기다릴 최소 시간이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Ice Glacier Wind|Timing", meta = (ClampMin = "0.0"))
	float MinWindIntervalSeconds = 15.0f;

	/** 실제 플레이 시작 후 다음 강풍까지 서버가 랜덤으로 기다릴 최대 시간이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Ice Glacier Wind|Timing", meta = (ClampMin = "0.0"))
	float MaxWindIntervalSeconds = 30.0f;

	/** 강풍 전에 방향을 예고할 시간이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Ice Glacier Wind|Timing", meta = (ClampMin = "0.0"))
	float WarningDurationSeconds = 1.0f;

	/** 실제 강풍이 지속되는 시간이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Ice Glacier Wind|Timing", meta = (ClampMin = "0.0"))
	float WindDurationSeconds = 2.5f;

	/** Gust 시작 후 최대 강도까지 부드럽게 올라가는 시간이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Ice Glacier Wind|Timing", meta = (ClampMin = "0.0"))
	float WindRampUpSeconds = 0.8f;

	/** Gust 종료 직전 강도가 부드럽게 줄어드는 시간이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Ice Glacier Wind|Timing", meta = (ClampMin = "0.0"))
	float WindRampDownSeconds = 0.3f;

	/** 바람 방향으로 부족한 수평 드리프트 속도를 매초 보충하는 가속도다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Ice Glacier Wind|Drift", meta = (ClampMin = "0.0"))
	float WindAcceleration = 180.0f;

	/** 지상 이동 상태에서 바람이 만들어내려는 최대 수평 드리프트 속도다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Ice Glacier Wind|Drift", meta = (ClampMin = "0.0"))
	float GroundWindMaxDriftSpeed = 180.0f;

	/** 공중 이동 상태에서 바람이 만들어내려는 최대 수평 드리프트 속도다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Ice Glacier Wind|Drift", meta = (ClampMin = "0.0"))
	float FallingWindMaxDriftSpeed = 110.0f;

	/** 서버가 이 목록 중 하나를 골라 수평 방향으로 정규화한 뒤 사용한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Ice Glacier Wind|Force")
	TArray<FVector> WindDirections;

	/** 루트 Scene Component다. */
	UPROPERTY(VisibleDefaultsOnly, Category = "SnowRumble|Ice Glacier Wind")
	TObjectPtr<USceneComponent> RootSceneComponent;

private:
	/** 복제된 상태 변경을 Blueprint 표현 이벤트로 전달한다. */
	UFUNCTION()
	void OnRep_CurrentWindState();

	/** 서버가 Match 상태와 강풍 상태 머신을 갱신한다. */
	void UpdateServerWind(float DeltaSeconds);

	/** 실제 플레이가 시작되어 강풍 스케줄을 진행해도 되는지 확인한다. */
	bool IsMatchPlayActive() const;

	/** 서버 동기화 World 시간을 반환한다. */
	float GetServerWorldTimeSeconds() const;

	/** 다음 강풍 Warning 시작 시간을 서버에서 랜덤 예약한다. */
	void ScheduleNextWind();

	/** 서버가 강풍 Warning 상태로 전환한다. */
	void StartWindWarning();

	/** 서버가 실제 강풍 상태로 전환한다. */
	void StartWindGust();

	/** 서버가 강풍을 끝내고 다음 스케줄을 예약한다. */
	void EndWindGust();

	/** Match가 비활성화되면 진행 중인 강풍 상태와 예약을 정리한다. */
	void ResetWindForInactiveMatch();

	/** 서버가 강풍 방향 후보 중 하나를 선택한다. */
	FVector ChooseRandomWindDirection() const;

	/** 디자이너가 입력한 방향을 강풍에 사용할 수평 단위 방향으로 변환한다. */
	FVector NormalizeWindDirection(FVector Direction) const;

	/** Gust 중인 서버가 플레이어 캐릭터에 제한된 수평 드리프트 속도를 적용한다. */
	void ApplyWindDriftToPlayers(float DeltaSeconds);

	/** 비권한 클라이언트가 로컬 캐릭터에 같은 바람 드리프트를 예측 적용한다. */
	void ApplyPredictedWindDriftToLocalPlayer(float DeltaSeconds) const;

	/** 캐릭터 하나에 바람 드리프트 속도를 적용한다. */
	void ApplyWindDriftToCharacter(
		ASnowRumbleCharacter& Character,
		float DeltaSeconds,
		const FVector& WindDirection,
		float StrengthAlpha) const;

	/** 복제된 강풍 상태를 로컬 플레이어 눈 VFX 방향 파라미터에 반영한다. */
	void UpdateLocalSnowVfxWindDirection() const;

	/** 현재 이동 모드에 맞는 최대 바람 드리프트 속도를 반환한다. */
	float GetMaxWindDriftSpeedForMovementMode(
		const UCharacterMovementComponent& MovementComponent) const;

	/** 현재 Gust 경과시간과 남은 시간 기준으로 0~1 강도 Alpha를 계산한다. */
	float CalculateCurrentWindStrengthAlpha() const;

	/** 이 캐릭터가 현재 강풍 영향을 받을 실제 플레이어인지 확인한다. */
	bool ShouldAffectCharacter(const ASnowRumbleCharacter* Character) const;

	/** 상태 변경에 맞춰 Blueprint 이벤트를 호출한다. */
	void NotifyWindStateChanged(
		EIceGlacierWindGustState PreviousState,
		EIceGlacierWindGustState NewState);

	UPROPERTY(Replicated)
	FVector CurrentWindDirection = FVector::ForwardVector;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWindState)
	EIceGlacierWindGustState CurrentWindState =
		EIceGlacierWindGustState::Idle;

	EIceGlacierWindGustState LastNotifiedWindState =
		EIceGlacierWindGustState::Idle;

	float NextWindStartServerTime = 0.0f;
	float CurrentGustStartServerTime = 0.0f;
	float CurrentStateEndServerTime = 0.0f;
	bool bWindScheduleStarted = false;
};

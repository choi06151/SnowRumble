// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SnowRumbleCharacterMovementComponent_C.generated.h"

USTRUCT(BlueprintType)
struct FEnvironmentalDriftState_C
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Movement|Environmental Drift")
	bool bActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Movement|Environmental Drift")
	FVector Direction = FVector::ForwardVector;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Movement|Environmental Drift")
	float Acceleration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Movement|Environmental Drift")
	float GroundMaxDriftSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Movement|Environmental Drift")
	float FallingMaxDriftSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Movement|Environmental Drift")
	float StartServerTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Movement|Environmental Drift")
	float EndServerTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Movement|Environmental Drift")
	float RampUpSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Movement|Environmental Drift")
	float RampDownSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Movement|Environmental Drift")
	int32 SourceGeneration = 0;
};

UCLASS()
class SNOWRUMBLE_API USnowRumbleCharacterMovementComponent_C
	: public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	/** 현재 캐릭터 이동 계산에 반영할 범용 환경 드리프트를 설정한다. */
	void SetEnvironmentalDrift(const FEnvironmentalDriftState_C& NewDriftState);

	/** 현재 활성 환경 드리프트를 해제한다. Generation이 지정되면 같은 Source만 해제한다. */
	void ClearEnvironmentalDrift(int32 SourceGeneration = INDEX_NONE);

	/** 현재 MovementComponent가 보관 중인 환경 드리프트 상태다. */
	const FEnvironmentalDriftState_C& GetEnvironmentalDriftState() const;

	/** 현재 World에서 서버 동기화 시간을 읽는다. */
	float GetServerWorldTimeSeconds() const;

	/** SavedMove 재시뮬레이션 직전에 당시 환경 드리프트 상태와 시간을 복원한다. */
	void RestoreEnvironmentalDriftForSavedMove(
		const FEnvironmentalDriftState_C& SavedDriftState,
		float SavedServerTimeSeconds);

	virtual void CalcVelocity(
		float DeltaTime,
		float Friction,
		bool bFluid,
		float BrakingDeceleration) override;
	virtual void PerformMovement(float DeltaTime) override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

private:
	/** 현재 환경 드리프트 상태가 이동 계산에 사용할 수 있는 값인지 확인한다. */
	bool HasValidEnvironmentalDrift() const;

	/** 현재 이동 모드에서 사용할 환경 드리프트 최대 속도를 반환한다. */
	float GetEnvironmentalDriftMaxSpeedForCurrentMode() const;

	/** 서버 동기화 시간 기준 현재 환경 드리프트 강도를 계산한다. */
	float CalculateEnvironmentalDriftStrengthAlpha() const;

	/** 지정한 서버 시간 기준 환경 드리프트 강도를 계산한다. */
	float CalculateEnvironmentalDriftStrengthAlphaAtTime(float ServerTimeSeconds) const;

	/** Super::CalcVelocity 이후 수평 환경 드리프트 속도를 추가한다. */
	void ApplyEnvironmentalDrift(float DeltaTime);

	FEnvironmentalDriftState_C EnvironmentalDriftState;
	bool bUseSavedMoveServerTimeForEnvironmentalDrift = false;
	float SavedMoveEnvironmentalDriftServerTime = 0.0f;
};

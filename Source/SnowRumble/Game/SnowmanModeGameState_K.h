// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SnowmanModeGameState_K.generated.h"

UCLASS()
class SNOWRUMBLE_API ASnowmanModeGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	/** 서버가 눈사람 모드 시작 카운트다운을 확정한다. */
	void StartSnowmanModeCountdownFromServer(float CountdownSeconds);

	/** 서버가 눈사람 모드 제한시간을 시작한다. */
	void StartSnowmanModeTimerFromServer(float InTimeLimitSeconds);

	/** 눈사람 모드 시작 전 입력 잠금 상태인지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowman")
	bool IsSnowmanModeInputLocked() const;

	/** 눈사람 모드 시작 카운트다운 UI를 표시해야 하는지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowman")
	bool ShouldShowSnowmanModeStartCountdown() const;

	/** HUD에 표시할 눈사람 모드 시작 카운트다운 텍스트를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowman")
	FText GetSnowmanModeStartCountdownText() const;

	/** 눈사람 모드 제한시간이 진행 중인지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowman")
	bool IsSnowmanModeTimerActive() const;

	/** 눈사람 모드 제한시간 전체 길이를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowman")
	float GetSnowmanModeTimeLimitSeconds() const;

	/** 눈사람 모드 남은 시간을 초 단위로 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowman")
	float GetSnowmanModeRemainingSeconds() const;

	/** 눈사람 모드 진행 시간을 초 단위로 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowman")
	float GetSnowmanModeElapsedSeconds() const;

	/** HUD에 표시할 눈사람 모드 남은 시간 텍스트를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowman")
	FText GetSnowmanModeRemainingTimeText() const;

	/** HUD에 표시할 눈사람 모드 진행 시간 텍스트를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowman")
	FText GetSnowmanModeElapsedTimeText() const;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	/** 서버 동기화 시간을 기준으로 시작까지 남은 시간을 반환한다. */
	float GetSecondsUntilSnowmanModeStart() const;

	/** 초 단위 시간을 0:00 형식으로 만든다. */
	FText FormatSecondsAsClock(float Seconds) const;

	UPROPERTY(Replicated)
	bool bSnowmanModeCountdownActive = false;

	UPROPERTY(Replicated)
	float SnowmanModeMatchStartServerTime = 0.0f;

	UPROPERTY(Replicated)
	float SnowmanModeStartCountdownSeconds = 3.0f;

	UPROPERTY(Replicated)
	bool bSnowmanModeTimerActive = false;

	UPROPERTY(Replicated)
	float SnowmanModeStartServerTime = 0.0f;

	UPROPERTY(Replicated)
	float SnowmanModeTimeLimitSeconds = 600.0f;
};

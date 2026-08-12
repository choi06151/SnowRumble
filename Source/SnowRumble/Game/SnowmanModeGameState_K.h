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
	/** 서버가 눈사람 모드 제한시간을 시작한다. */
	void StartSnowmanModeTimerFromServer(float InTimeLimitSeconds);

	/** 눈사람 모드 제한시간이 진행 중인지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowman")
	bool IsSnowmanModeTimerActive() const;

	/** 눈사람 모드 제한시간 전체 길이를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowman")
	float GetSnowmanModeTimeLimitSeconds() const;

	/** 눈사람 모드 남은 시간을 초 단위로 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowman")
	float GetSnowmanModeRemainingSeconds() const;

	/** HUD에 표시할 눈사람 모드 남은 시간 텍스트를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowman")
	FText GetSnowmanModeRemainingTimeText() const;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	/** 초 단위 시간을 0:00 형식으로 만든다. */
	FText FormatSecondsAsClock(float Seconds) const;

	UPROPERTY(Replicated)
	bool bSnowmanModeTimerActive = false;

	UPROPERTY(Replicated)
	float SnowmanModeStartServerTime = 0.0f;

	UPROPERTY(Replicated)
	float SnowmanModeTimeLimitSeconds = 600.0f;
};

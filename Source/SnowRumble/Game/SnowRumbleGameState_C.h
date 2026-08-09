// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SnowRumblePlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "SnowRumbleGameState_C.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSnowRumbleRoundResultChanged);

UCLASS()
class SNOWRUMBLE_API ASnowRumbleGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	/** 서버가 PvP 시작 카운트다운을 확정한다. */
	void StartMatchCountdownFromServer(float CountdownSeconds);

	/** 서버가 현재 라운드 승리 팀을 확정한다. */
	void EndRoundFromServer(ESnowRumbleTeam WinningTeam);

	/** PvP 시작 전 입력 잠금 중인지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	bool IsMatchInputLocked() const;

	/** 현재 라운드가 종료됐는지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	bool IsRoundEnded() const;

	/** 현재 라운드 승리 팀을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	ESnowRumbleTeam GetRoundWinningTeam() const;

	/** 카운트다운 UI를 표시해야 하는지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	bool ShouldShowStartCountdown() const;

	/** HUD에 표시할 카운트다운 텍스트를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	FText GetStartCountdownText() const;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable, Category = "SnowRumble|Match")
	FOnSnowRumbleRoundResultChanged OnRoundResultChanged;

protected:
	/** 복제된 라운드 결과를 UI와 표현에 알린다. */
	UFUNCTION()
	void OnRep_RoundResult();

	UPROPERTY(Replicated)
	bool bStartCountdownActive = false;

	UPROPERTY(Replicated)
	float MatchStartServerTime = 0.0f;

	UPROPERTY(Replicated)
	float MatchStartCountdownSeconds = 3.0f;

	UPROPERTY(ReplicatedUsing = OnRep_RoundResult)
	bool bRoundEnded = false;

	UPROPERTY(ReplicatedUsing = OnRep_RoundResult)
	ESnowRumbleTeam RoundWinningTeam = ESnowRumbleTeam::None;

private:
	/** 서버 동기화 시간을 기준으로 시작까지 남은 시간을 반환한다. */
	float GetSecondsUntilMatchStart() const;
};

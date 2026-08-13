// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SnowRumbleMatchSubsystem_C.h"
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

	/** 서버가 맵 축소 상태를 시작한다. */
	void StartMapShrinkFromServer(float ShrinkDurationSeconds);

	/** 서버가 맵 축소 완료 상태를 확정하고 다음 축소 시간을 잡는다. */
	void CompleteMapShrinkFromServer();

	/** 서버가 현재 라운드 승리 팀과 매치 누적 상태를 확정한다. */
	void EndRoundFromServer(
		ESnowRumbleTeam WinningTeam,
		const class USnowRumbleMatchSubsystem* MatchSubsystem);

	/** 서버가 새 PvP 레벨 진입 시 현재 매치 누적 상태를 복제값에 반영한다. */
	void ApplyMatchStateFromServer(
		const class USnowRumbleMatchSubsystem* MatchSubsystem);

	/** PvP 시작 전 입력 잠금 중인지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	bool IsMatchInputLocked() const;

	/** 현재 라운드가 종료됐는지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	bool IsRoundEnded() const;

	/** 현재 라운드 승리 팀을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	ESnowRumbleTeam GetRoundWinningTeam() const;

	/** 현재 매치에서 해당 팀이 획득한 라운드 승수를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	int32 GetTeamRoundWinCount(ESnowRumbleTeam Team) const;

	/** 현재 라운드 번호를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	int32 GetCurrentRoundNumber() const;

	/** 총 라운드 수를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	int32 GetRoundLimit() const;

	/** 모든 라운드가 종료돼 매치 1등이 확정됐는지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	bool IsMatchEnded() const;

	/** 공동 1등 단판 승부 라운드인지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	bool IsTiebreakerRound() const;

	/** 매치 1등 팀을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	ESnowRumbleTeam GetMatchWinningTeam() const;

	/** 카운트다운 UI를 표시해야 하는지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	bool ShouldShowStartCountdown() const;

	/** HUD에 표시할 카운트다운 텍스트를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	FText GetStartCountdownText() const;

	/** 현재 라운드의 실제 경기 경과 시간을 초 단위로 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	float GetRoundElapsedSeconds() const;

	/** HUD에 표시할 경기 경과 시간 텍스트를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	FText GetRoundElapsedTimeText() const;

	/** 다음 맵 축소까지 남은 시간을 초 단위로 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	float GetSecondsUntilNextMapShrink() const;

	/** HUD에 표시할 맵 축소 안내 텍스트를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	FText GetMapShrinkCountdownText() const;

	/** 현재 맵 축소가 진행 중인지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	bool IsMapShrinkInProgress() const;

	/** 로비에서 선택한 게임 속도를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	ESnowRumbleGameSpeed GetGameSpeed() const;

	/** 현재 게임 속도의 맵 축소 주기를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Match")
	float GetMapShrinkIntervalSeconds() const;

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

	UPROPERTY(Replicated)
	float RoundStartServerTime = 0.0f;

	UPROPERTY(Replicated)
	float NextMapShrinkServerTime = 0.0f;

	UPROPERTY(Replicated)
	float MapShrinkStartedServerTime = 0.0f;

	UPROPERTY(Replicated)
	float MapShrinkDurationSeconds = 5.0f;

	UPROPERTY(Replicated)
	bool bMapShrinkInProgress = false;

	UPROPERTY(Replicated)
	ESnowRumbleGameSpeed GameSpeed = ESnowRumbleGameSpeed::Normal;

	UPROPERTY(ReplicatedUsing = OnRep_RoundResult)
	bool bRoundEnded = false;

	UPROPERTY(ReplicatedUsing = OnRep_RoundResult)
	ESnowRumbleTeam RoundWinningTeam = ESnowRumbleTeam::None;

	UPROPERTY(Replicated)
	int32 RedTeamRoundWins = 0;

	UPROPERTY(Replicated)
	int32 SkyTeamRoundWins = 0;

	UPROPERTY(Replicated)
	int32 GreenTeamRoundWins = 0;

	UPROPERTY(Replicated)
	int32 YellowTeamRoundWins = 0;

	UPROPERTY(Replicated)
	int32 PurpleTeamRoundWins = 0;

	UPROPERTY(Replicated)
	int32 PinkTeamRoundWins = 0;

	UPROPERTY(Replicated)
	int32 BlueTeamRoundWins = 0;

	UPROPERTY(Replicated)
	int32 WhiteTeamRoundWins = 0;

	UPROPERTY(Replicated)
	int32 CurrentRoundNumber = 1;

	UPROPERTY(Replicated)
	int32 RoundLimit = 1;

	UPROPERTY(ReplicatedUsing = OnRep_RoundResult)
	bool bMatchEnded = false;

	UPROPERTY(Replicated)
	bool bTiebreakerRound = false;

	UPROPERTY(ReplicatedUsing = OnRep_RoundResult)
	ESnowRumbleTeam MatchWinningTeam = ESnowRumbleTeam::None;

private:
	/** 서버 동기화 시간을 기준으로 시작까지 남은 시간을 반환한다. */
	float GetSecondsUntilMatchStart() const;

	/** 초 단위 시간을 0:00 형식으로 만든다. */
	FText FormatSecondsAsClock(float Seconds) const;

	/** 서버가 Subsystem의 누적 점수를 복제 변수에 복사한다. */
	void CopyRoundWinsFromMatchSubsystem(
		const class USnowRumbleMatchSubsystem* MatchSubsystem);
};

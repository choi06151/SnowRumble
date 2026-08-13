// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SnowRumblePlayerState.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SnowRumbleMatchSubsystem_C.generated.h"

UENUM(BlueprintType)
enum class ESnowRumbleGameSpeed : uint8
{
	Slow,
	Normal,
	Fast
};

UCLASS()
class SNOWRUMBLE_API USnowRumbleMatchSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 로비에서 PvP 매치를 시작할 때 라운드 수와 후보 레벨을 저장한다. */
	void BeginPvPMatch(
		int32 InRoundLimit,
		ESnowRumbleGameSpeed InGameSpeed,
		const TArray<FString>& InPvPLevelPaths);

	/** 현재 매치 상태를 지우고 다음 로비 설정을 기다린다. */
	void ResetPvPMatch();

	/** 등록된 PvP 후보 레벨 중 다음 라운드에 사용할 레벨을 고른다. */
	FString SelectNextPvPLevelPath(const FString& FallbackLevelPath);

	/** 서버가 라운드 승리 팀을 누적하고 매치 종료 여부를 반환한다. */
	bool RecordRoundWin(ESnowRumbleTeam WinningTeam);

	/** 정규 라운드 종료 후 공동 1등이 있으면 단판 승부 상태로 전환한다. */
	bool StartTiebreakerForLeadingTie();

	/** 다음 라운드 번호로 진행한다. */
	void AdvanceToNextRound();

	/** 현재 활성 PvP 매치가 있는지 반환한다. */
	bool IsPvPMatchActive() const;

	/** 현재 라운드 번호를 반환한다. */
	int32 GetCurrentRoundNumber() const;

	/** 총 라운드 수를 반환한다. */
	int32 GetRoundLimit() const;

	/** 로비에서 선택한 게임 속도를 반환한다. */
	ESnowRumbleGameSpeed GetGameSpeed() const;

	/** 게임 속도에 대응하는 맵 축소 주기를 반환한다. */
	static float GetMapShrinkIntervalSeconds(ESnowRumbleGameSpeed GameSpeed);

	/** 해당 팀의 누적 라운드 승수를 반환한다. */
	int32 GetTeamRoundWinCount(ESnowRumbleTeam Team) const;

	/** 모든 라운드가 끝났는지 반환한다. */
	bool IsMatchComplete() const;

	/** 현재 공동 1등 단판 승부 중인지 반환한다. */
	bool IsTiebreakerActive() const;

	/** 현재 팀이 단판 승부 대상 팀인지 반환한다. */
	bool IsTiebreakerTeam(ESnowRumbleTeam Team) const;

	/** 현재 1등 팀을 반환한다. 동점이면 None을 반환한다. */
	ESnowRumbleTeam GetLeadingTeam() const;

private:
	int32 NormalizeRoundLimit(int32 InRoundLimit) const;
	bool IsValidTeam(ESnowRumbleTeam Team) const;
	void GetLeadingTiedTeams(TArray<ESnowRumbleTeam>& OutTeams) const;

	bool bPvPMatchActive = false;
	bool bMatchComplete = false;
	bool bTiebreakerActive = false;
	int32 CurrentRoundNumber = 1;
	int32 RoundLimit = 1;
	ESnowRumbleGameSpeed GameSpeed = ESnowRumbleGameSpeed::Normal;
	TArray<FString> PvPLevelPaths;
	FString LastSelectedPvPLevelPath;
	TArray<ESnowRumbleTeam> TiebreakerTeams;
	TMap<ESnowRumbleTeam, int32> TeamRoundWins;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SnowRumbleMatchSubsystem_C.h"
#include "GameFramework/GameStateBase.h"
#include "SnowRumbleLobbyGameState.generated.h"

class ASnowRumblePlayerState;
enum class ESnowRumbleTeam : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSnowRumbleLobbyStateChanged);

UENUM(BlueprintType)
enum class ESnowRumbleLobbyMode : uint8
{
	Pvp,
	Snowman
};

UCLASS()
class SNOWRUMBLE_API ASnowRumbleLobbyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	/** 현재 대기방에 참가한 SnowRumble PlayerState 목록을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Lobby")
	TArray<ASnowRumblePlayerState*> GetLobbyPlayers() const;

	/** 서버 기준으로 현재 대기방에서 경기 시작이 가능한지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Lobby")
	bool CanStartLobbyMatch() const;

	/** 현재 경기 시작이 불가능한 사유를 UI 표시용 문구로 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Lobby")
	FText GetStartMatchInvalidReasonText() const;

	/** 현재 선택된 유효 팀 색 수를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Lobby")
	int32 GetAssignedLobbyTeamCount() const;

	/** 특정 팀에 배정된 현재 플레이어 수를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Lobby")
	int32 GetLobbyTeamPlayerCount(ESnowRumbleTeam Team) const;

	/** 현재 준비 완료한 플레이어 수를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Lobby")
	int32 GetReadyPlayerCount() const;

	/** 준비 상태를 눌러야 하는 비호스트 플레이어 수를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Lobby")
	int32 GetReadyRequiredPlayerCount() const;

	/** 현재 선택된 로비 게임 모드를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Lobby")
	ESnowRumbleLobbyMode GetLobbyMode() const;

	/** 로비에서 선택한 PvP 총 라운드 수를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Lobby")
	int32 GetMatchRoundLimit() const;

	/** 로비에서 선택한 게임 속도를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Lobby")
	ESnowRumbleGameSpeed GetGameSpeed() const;

	/** 서버가 현재 로비 게임 모드를 변경한다. */
	void SetLobbyModeFromServer(ESnowRumbleLobbyMode NewLobbyMode);

	/** 서버가 PvP 총 라운드 수를 1, 3, 5 중 하나로 변경한다. */
	void SetMatchRoundLimitFromServer(int32 NewRoundLimit);

	/** 서버가 PvP 게임 속도를 변경한다. */
	void SetGameSpeedFromServer(ESnowRumbleGameSpeed NewGameSpeed);

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 대기방 상태 변경을 UI에 알린다. */
	void NotifyLobbyStateChanged();

	UPROPERTY(BlueprintAssignable, Category = "SnowRumble|Lobby")
	FOnSnowRumbleLobbyStateChanged OnLobbyStateChanged;

protected:
	/** 복제된 로비 게임 모드 변경을 UI에 알린다. */
	UFUNCTION()
	void OnRep_LobbyMode();

	/** 복제된 총 라운드 수 변경을 UI에 알린다. */
	UFUNCTION()
	void OnRep_MatchRoundLimit();

	/** 복제된 게임 속도 변경을 UI에 알린다. */
	UFUNCTION()
	void OnRep_GameSpeed();

private:
	int32 NormalizeRoundLimit(int32 NewRoundLimit) const;

	/** 방 설정 변경을 비호스트 클라이언트의 개인 알림 UI에 전달한다. */
	void BroadcastRoomSettingsChangedAlarmToClients() const;

	UPROPERTY(ReplicatedUsing = OnRep_LobbyMode)
	ESnowRumbleLobbyMode LobbyMode = ESnowRumbleLobbyMode::Pvp;

	UPROPERTY(ReplicatedUsing = OnRep_MatchRoundLimit)
	int32 MatchRoundLimit = 1;

	UPROPERTY(ReplicatedUsing = OnRep_GameSpeed)
	ESnowRumbleGameSpeed GameSpeed = ESnowRumbleGameSpeed::Normal;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleLobbyGameState.h"

#include "Net/UnrealNetwork.h"
#include "SnowRumblePlayerState.h"
#include "../UI/SnowRumblePlayerController.h"

TArray<ASnowRumblePlayerState*> ASnowRumbleLobbyGameState::GetLobbyPlayers()
	const
{
	TArray<ASnowRumblePlayerState*> LobbyPlayers;
	for (APlayerState* PlayerState : PlayerArray)
	{
		if (ASnowRumblePlayerState* SnowRumblePlayerState =
			Cast<ASnowRumblePlayerState>(PlayerState))
		{
			LobbyPlayers.Add(SnowRumblePlayerState);
		}
	}
	return LobbyPlayers;
}

bool ASnowRumbleLobbyGameState::CanStartLobbyMatch() const
{
	const TArray<ASnowRumblePlayerState*> LobbyPlayers = GetLobbyPlayers();
	if (LobbyPlayers.Num() < 2 || LobbyPlayers.Num() > 8)
	{
		return false;
	}

	TMap<ESnowRumbleTeam, int32> TeamPlayerCounts;
	for (const ASnowRumblePlayerState* PlayerState : LobbyPlayers)
	{
		if (!PlayerState)
		{
			return false;
		}
		if (!PlayerState->IsLobbyHost() && !PlayerState->IsLobbyReady())
		{
			return false;
		}

		const ESnowRumbleTeam LobbyTeam = PlayerState->GetLobbyTeam();
		if (LobbyTeam == ESnowRumbleTeam::None)
		{
			return false;
		}

		int32& TeamPlayerCount = TeamPlayerCounts.FindOrAdd(LobbyTeam);
		++TeamPlayerCount;
	}

	constexpr int32 MaxPlayersPerTeam = 4;
	if (TeamPlayerCounts.Num() < 2)
	{
		return false;
	}

	for (const TPair<ESnowRumbleTeam, int32>& TeamPlayerCount : TeamPlayerCounts)
	{
		if (TeamPlayerCount.Value < 1
			|| TeamPlayerCount.Value > MaxPlayersPerTeam)
		{
			return false;
		}
	}

	return true;
}

FText ASnowRumbleLobbyGameState::GetStartMatchInvalidReasonText() const
{
	const TArray<ASnowRumblePlayerState*> LobbyPlayers = GetLobbyPlayers();
	if (LobbyPlayers.Num() < 2)
	{
		return NSLOCTEXT(
			"SnowRumble",
			"LobbyInvalidStartNeedPlayers",
			"게임 시작에는 최소 2명이 필요합니다.");
	}
	if (LobbyPlayers.Num() > 8)
	{
		return NSLOCTEXT(
			"SnowRumble",
			"LobbyInvalidStartTooManyPlayers",
			"게임 시작은 최대 8명까지 가능합니다.");
	}

	TSet<ESnowRumbleTeam> AssignedTeams;
	for (const ASnowRumblePlayerState* PlayerState : LobbyPlayers)
	{
		if (!PlayerState || PlayerState->GetLobbyTeam() == ESnowRumbleTeam::None)
		{
			return NSLOCTEXT(
				"SnowRumble",
				"LobbyInvalidStartNeedTeam",
				"모든 플레이어가 팀 색을 선택해야 합니다.");
		}

		AssignedTeams.Add(PlayerState->GetLobbyTeam());
	}

	if (AssignedTeams.Num() < 2)
	{
		return NSLOCTEXT(
			"SnowRumble",
			"LobbyInvalidStartNeedTwoTeams",
			"두 개 이상의 팀이 있어야 게임을 시작할 수 있습니다.");
	}

	for (const ASnowRumblePlayerState* PlayerState : LobbyPlayers)
	{
		if (PlayerState && !PlayerState->IsLobbyHost()
			&& !PlayerState->IsLobbyReady())
		{
			return NSLOCTEXT(
				"SnowRumble",
				"LobbyInvalidStartNotReady",
				"모든 플레이어가 준비 완료해야 시작할 수 있습니다.");
		}
	}

	return FText::GetEmpty();
}

int32 ASnowRumbleLobbyGameState::GetAssignedLobbyTeamCount() const
{
	TSet<ESnowRumbleTeam> AssignedTeams;
	for (const ASnowRumblePlayerState* PlayerState : GetLobbyPlayers())
	{
		if (PlayerState && PlayerState->GetLobbyTeam() != ESnowRumbleTeam::None)
		{
			AssignedTeams.Add(PlayerState->GetLobbyTeam());
		}
	}
	return AssignedTeams.Num();
}

int32 ASnowRumbleLobbyGameState::GetLobbyTeamPlayerCount(
	ESnowRumbleTeam Team) const
{
	int32 PlayerCount = 0;
	for (const ASnowRumblePlayerState* PlayerState : GetLobbyPlayers())
	{
		if (PlayerState && PlayerState->GetLobbyTeam() == Team)
		{
			++PlayerCount;
		}
	}
	return PlayerCount;
}

int32 ASnowRumbleLobbyGameState::GetReadyPlayerCount() const
{
	int32 ReadyPlayerCount = 0;
	for (const ASnowRumblePlayerState* PlayerState : GetLobbyPlayers())
	{
		if (PlayerState
			&& !PlayerState->IsLobbyHost()
			&& PlayerState->IsLobbyReady())
		{
			++ReadyPlayerCount;
		}
	}
	return ReadyPlayerCount;
}

int32 ASnowRumbleLobbyGameState::GetReadyRequiredPlayerCount() const
{
	int32 ReadyRequiredPlayerCount = 0;
	for (const ASnowRumblePlayerState* PlayerState : GetLobbyPlayers())
	{
		if (PlayerState && !PlayerState->IsLobbyHost())
		{
			++ReadyRequiredPlayerCount;
		}
	}
	return ReadyRequiredPlayerCount;
}

ESnowRumbleLobbyMode ASnowRumbleLobbyGameState::GetLobbyMode() const
{
	return LobbyMode;
}

int32 ASnowRumbleLobbyGameState::GetMatchRoundLimit() const
{
	return MatchRoundLimit;
}

ESnowRumbleGameSpeed ASnowRumbleLobbyGameState::GetGameSpeed() const
{
	return GameSpeed;
}

void ASnowRumbleLobbyGameState::SetLobbyModeFromServer(
	ESnowRumbleLobbyMode NewLobbyMode)
{
	if (!HasAuthority() || LobbyMode == NewLobbyMode)
	{
		return;
	}

	LobbyMode = NewLobbyMode;
	BroadcastRoomSettingsChangedAlarmToClients();
	NotifyLobbyStateChanged();
}

void ASnowRumbleLobbyGameState::SetMatchRoundLimitFromServer(
	int32 NewRoundLimit)
{
	const int32 NormalizedRoundLimit = NormalizeRoundLimit(NewRoundLimit);
	if (!HasAuthority() || MatchRoundLimit == NormalizedRoundLimit)
	{
		return;
	}

	MatchRoundLimit = NormalizedRoundLimit;
	BroadcastRoomSettingsChangedAlarmToClients();
	NotifyLobbyStateChanged();
}

void ASnowRumbleLobbyGameState::SetGameSpeedFromServer(
	ESnowRumbleGameSpeed NewGameSpeed)
{
	if (!HasAuthority() || GameSpeed == NewGameSpeed)
	{
		return;
	}

	GameSpeed = NewGameSpeed;
	BroadcastRoomSettingsChangedAlarmToClients();
	NotifyLobbyStateChanged();
}

void ASnowRumbleLobbyGameState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASnowRumbleLobbyGameState, LobbyMode);
	DOREPLIFETIME(ASnowRumbleLobbyGameState, MatchRoundLimit);
	DOREPLIFETIME(ASnowRumbleLobbyGameState, GameSpeed);
}

void ASnowRumbleLobbyGameState::NotifyLobbyStateChanged()
{
	OnLobbyStateChanged.Broadcast();
}

void ASnowRumbleLobbyGameState::OnRep_LobbyMode()
{
	NotifyLobbyStateChanged();
}

void ASnowRumbleLobbyGameState::OnRep_MatchRoundLimit()
{
	NotifyLobbyStateChanged();
}

void ASnowRumbleLobbyGameState::OnRep_GameSpeed()
{
	NotifyLobbyStateChanged();
}

int32 ASnowRumbleLobbyGameState::NormalizeRoundLimit(
	int32 NewRoundLimit) const
{
	if (NewRoundLimit <= 1)
	{
		return 1;
	}
	if (NewRoundLimit <= 3)
	{
		return 3;
	}
	return 5;
}

void ASnowRumbleLobbyGameState::BroadcastRoomSettingsChangedAlarmToClients()
	const
{
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FText AlarmText = NSLOCTEXT(
		"SnowRumble",
		"LobbyRoomSettingsChangedAlarm",
		"방장이 방설정을 변경하였습니다");
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator();
		It;
		++It)
	{
		ASnowRumblePlayerController* PlayerController =
			Cast<ASnowRumblePlayerController>(It->Get());
		const ASnowRumblePlayerState* PlayerState = PlayerController
			? PlayerController->GetPlayerState<ASnowRumblePlayerState>()
			: nullptr;
		if (PlayerController && PlayerState && !PlayerState->IsLobbyHost())
		{
			PlayerController->ClientShowPersonalTextAlarm(AlarmText);
		}
	}
}

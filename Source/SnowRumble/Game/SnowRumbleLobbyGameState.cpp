// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleLobbyGameState.h"

#include "Net/UnrealNetwork.h"
#include "SnowRumblePlayerState.h"

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

void ASnowRumbleLobbyGameState::SetLobbyModeFromServer(
	ESnowRumbleLobbyMode NewLobbyMode)
{
	if (!HasAuthority() || LobbyMode == NewLobbyMode)
	{
		return;
	}

	LobbyMode = NewLobbyMode;
	NotifyLobbyStateChanged();
}

void ASnowRumbleLobbyGameState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASnowRumbleLobbyGameState, LobbyMode);
}

void ASnowRumbleLobbyGameState::NotifyLobbyStateChanged()
{
	OnLobbyStateChanged.Broadcast();
}

void ASnowRumbleLobbyGameState::OnRep_LobbyMode()
{
	NotifyLobbyStateChanged();
}

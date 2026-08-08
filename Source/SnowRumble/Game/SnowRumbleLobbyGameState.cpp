// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleLobbyGameState.h"

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

	int32 RedPlayers = 0;
	int32 BluePlayers = 0;
	for (const ASnowRumblePlayerState* PlayerState : LobbyPlayers)
	{
		if (!PlayerState || !PlayerState->IsLobbyReady())
		{
			return false;
		}

		switch (PlayerState->GetLobbyTeam())
		{
		case ESnowRumbleTeam::Red:
			++RedPlayers;
			break;
		case ESnowRumbleTeam::Blue:
			++BluePlayers;
			break;
		default:
			return false;
		}
	}

	constexpr int32 MaxPlayersPerTeam = 4;
	return RedPlayers == BluePlayers
		&& RedPlayers >= 1
		&& RedPlayers <= MaxPlayersPerTeam
		&& BluePlayers <= MaxPlayersPerTeam;
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

void ASnowRumbleLobbyGameState::NotifyLobbyStateChanged()
{
	OnLobbyStateChanged.Broadcast();
}

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
	if (LobbyPlayers.Num() < 2)
	{
		return false;
	}

	bool bHasRedPlayer = false;
	bool bHasBluePlayer = false;
	for (const ASnowRumblePlayerState* PlayerState : LobbyPlayers)
	{
		if (!PlayerState || !PlayerState->IsLobbyReady())
		{
			return false;
		}

		bHasRedPlayer |= PlayerState->GetLobbyTeam() == ESnowRumbleTeam::Red;
		bHasBluePlayer |= PlayerState->GetLobbyTeam() == ESnowRumbleTeam::Blue;
	}

	return bHasRedPlayer && bHasBluePlayer;
}

void ASnowRumbleLobbyGameState::NotifyLobbyStateChanged()
{
	OnLobbyStateChanged.Broadcast();
}

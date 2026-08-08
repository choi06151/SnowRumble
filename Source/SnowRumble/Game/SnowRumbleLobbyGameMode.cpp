// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleLobbyGameMode.h"

#include "GameFramework/PlayerController.h"
#include "../Player/SnowRumbleCharacter.h"
#include "../UI/LobbyPlayerController.h"
#include "SnowRumbleLobbyGameState.h"
#include "SnowRumblePlayerState.h"

ASnowRumbleLobbyGameMode::ASnowRumbleLobbyGameMode()
{
	GameStateClass = ASnowRumbleLobbyGameState::StaticClass();
	PlayerStateClass = ASnowRumblePlayerState::StaticClass();
	PlayerControllerClass = ALobbyPlayerController::StaticClass();
	DefaultPawnClass = ASnowRumbleCharacter::StaticClass();
}

void ASnowRumbleLobbyGameMode::RequestStartMatch(
	APlayerController* RequestingController)
{
	if (!RequestingController || !RequestingController->IsLocalController())
	{
		return;
	}

	ASnowRumbleLobbyGameState* LobbyGameState =
		GetGameState<ASnowRumbleLobbyGameState>();
	if (!LobbyGameState || !LobbyGameState->CanStartLobbyMatch())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World && !MatchTravelUrl.IsEmpty())
	{
		World->ServerTravel(MatchTravelUrl);
	}
}

void ASnowRumbleLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AssignLobbyTeam(NewPlayer);

	if (ALobbyPlayerController* LobbyPlayerController =
		Cast<ALobbyPlayerController>(NewPlayer))
	{
		LobbyPlayerController->ClientRequestApplySavedLobbyPlayerName();
	}

	if (ASnowRumbleLobbyGameState* LobbyGameState =
		GetGameState<ASnowRumbleLobbyGameState>())
	{
		LobbyGameState->NotifyLobbyStateChanged();
	}
}

void ASnowRumbleLobbyGameMode::AssignLobbyTeam(APlayerController* NewPlayer)
{
	if (!NewPlayer)
	{
		return;
	}

	ASnowRumblePlayerState* SnowRumblePlayerState =
		NewPlayer->GetPlayerState<ASnowRumblePlayerState>();
	if (!SnowRumblePlayerState)
	{
		return;
	}

	const int32 RedPlayers = CountLobbyTeamPlayers(ESnowRumbleTeam::Red);
	const int32 BluePlayers = CountLobbyTeamPlayers(ESnowRumbleTeam::Blue);
	constexpr int32 MaxPlayersPerTeam = 4;

	ESnowRumbleTeam AssignedTeam = ESnowRumbleTeam::None;
	if (RedPlayers >= MaxPlayersPerTeam && BluePlayers >= MaxPlayersPerTeam)
	{
		AssignedTeam = ESnowRumbleTeam::None;
	}
	else if (RedPlayers >= MaxPlayersPerTeam)
	{
		AssignedTeam = ESnowRumbleTeam::Blue;
	}
	else if (BluePlayers >= MaxPlayersPerTeam)
	{
		AssignedTeam = ESnowRumbleTeam::Red;
	}
	else if (RedPlayers < BluePlayers)
	{
		AssignedTeam = ESnowRumbleTeam::Red;
	}
	else if (BluePlayers < RedPlayers)
	{
		AssignedTeam = ESnowRumbleTeam::Blue;
	}
	else
	{
		AssignedTeam = FMath::RandBool()
			? ESnowRumbleTeam::Red
			: ESnowRumbleTeam::Blue;
	}

	SnowRumblePlayerState->AssignLobbyTeamFromServer(AssignedTeam);
}

int32 ASnowRumbleLobbyGameMode::CountLobbyTeamPlayers(
	ESnowRumbleTeam Team) const
{
	const ASnowRumbleLobbyGameState* LobbyGameState =
		GetGameState<ASnowRumbleLobbyGameState>();
	return LobbyGameState
		? LobbyGameState->GetLobbyTeamPlayerCount(Team)
		: 0;
}

void ASnowRumbleLobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (ASnowRumbleLobbyGameState* LobbyGameState =
		GetGameState<ASnowRumbleLobbyGameState>())
	{
		LobbyGameState->NotifyLobbyStateChanged();
	}
}

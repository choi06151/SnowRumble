// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleLobbyGameMode.h"

#include "GameFramework/PlayerController.h"
#include "../Player/SnowRumbleCharacter.h"
#include "../UI/LobbyPlayerController.h"
#include "../UI/SnowRumblePlayerController.h"
#include "HAL/IConsoleManager.h"
#include "SnowRumbleLobbyGameState.h"
#include "SnowRumblePlayerState.h"

namespace
{
const TArray<ESnowRumbleTeam>& GetLobbyTeamChoices()
{
	static const TArray<ESnowRumbleTeam> TeamChoices = {
		ESnowRumbleTeam::Red,
		ESnowRumbleTeam::Sky,
		ESnowRumbleTeam::Green,
		ESnowRumbleTeam::Yellow,
		ESnowRumbleTeam::Purple,
		ESnowRumbleTeam::Pink,
		ESnowRumbleTeam::Blue,
		ESnowRumbleTeam::White
	};
	return TeamChoices;
}
}

ASnowRumbleLobbyGameMode::ASnowRumbleLobbyGameMode()
{
	GameStateClass = ASnowRumbleLobbyGameState::StaticClass();
	PlayerStateClass = ASnowRumblePlayerState::StaticClass();
	PlayerControllerClass = ALobbyPlayerController::StaticClass();
	DefaultPawnClass = ASnowRumbleCharacter::StaticClass();
	bUseSeamlessTravel = true;

	if (IConsoleVariable* AllowPieSeamlessTravel =
		IConsoleManager::Get().FindConsoleVariable(TEXT("net.AllowPIESeamlessTravel")))
	{
		AllowPieSeamlessTravel->Set(1);
	}
}

void ASnowRumbleLobbyGameMode::RequestStartMatch(
	APlayerController* RequestingController)
{
	if (bMatchTravelPending
		|| !RequestingController
		|| !RequestingController->IsLocalController())
	{
		return;
	}

	ASnowRumbleLobbyGameState* LobbyGameState =
		GetGameState<ASnowRumbleLobbyGameState>();
	if (!LobbyGameState
		|| LobbyGameState->GetLobbyMode() != ESnowRumbleLobbyMode::Pvp
		|| !LobbyGameState->CanStartLobbyMatch())
	{
		return;
	}

	PendingMatchTravelUrl = BuildMatchTravelUrl(LobbyGameState->GetLobbyPlayers().Num());
	if (PendingMatchTravelUrl.IsEmpty())
	{
		return;
	}

	bMatchTravelPending = true;
	ShowMatchLoadingScreens();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(
			this,
			&ASnowRumbleLobbyGameMode::StartPendingMatchTravel);
	}
}

void ASnowRumbleLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (ASnowRumblePlayerState* SnowRumblePlayerState =
		NewPlayer
			? NewPlayer->GetPlayerState<ASnowRumblePlayerState>()
			: nullptr)
	{
		SnowRumblePlayerState->AssignLobbyHostFromServer(
			NewPlayer->HasAuthority() && NewPlayer->IsLocalController());
	}

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

	constexpr int32 MaxPlayersPerTeam = 4;

	ESnowRumbleTeam AssignedTeam = ESnowRumbleTeam::None;
	int32 LowestPlayerCount = MaxPlayersPerTeam + 1;
	for (const ESnowRumbleTeam Team : GetLobbyTeamChoices())
	{
		const int32 TeamPlayers = CountLobbyTeamPlayers(Team);
		if (TeamPlayers < LowestPlayerCount)
		{
			LowestPlayerCount = TeamPlayers;
			AssignedTeam = Team;
		}
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

FString ASnowRumbleLobbyGameMode::BuildMatchTravelUrl(
	int32 ExpectedPlayerCount) const
{
	if (MatchTravelUrl.IsEmpty())
	{
		return FString();
	}

	FString TravelUrl = MatchTravelUrl;
	if (ExpectedPlayerCount > 0)
	{
		TravelUrl += FString::Printf(
			TEXT("?ExpectedPlayers=%d"),
			ExpectedPlayerCount);
	}
	return TravelUrl;
}

void ASnowRumbleLobbyGameMode::ShowMatchLoadingScreens()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const ASnowRumbleLobbyGameState* LobbyGameState =
		GetGameState<ASnowRumbleLobbyGameState>();
	const int32 ExpectedPlayerCount = LobbyGameState
		? LobbyGameState->GetLobbyPlayers().Num()
		: 0;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator();
		It;
		++It)
	{
		if (ASnowRumblePlayerController* PlayerController =
			Cast<ASnowRumblePlayerController>(It->Get()))
		{
			PlayerController->ClientShowLoadingScreen();
			PlayerController->ClientUpdateLoadingProgress(
				0,
				ExpectedPlayerCount);
		}
	}
}

void ASnowRumbleLobbyGameMode::StartPendingMatchTravel()
{
	if (!bMatchTravelPending || PendingMatchTravelUrl.IsEmpty())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (!World->ServerTravel(PendingMatchTravelUrl))
		{
			bMatchTravelPending = false;
			PendingMatchTravelUrl.Empty();
			for (FConstPlayerControllerIterator It =
					World->GetPlayerControllerIterator();
				It;
				++It)
			{
				if (ASnowRumblePlayerController* PlayerController =
					Cast<ASnowRumblePlayerController>(It->Get()))
				{
					PlayerController->ClientHideLoadingScreen();
				}
			}
		}
	}
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

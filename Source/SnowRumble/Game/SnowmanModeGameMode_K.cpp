// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowmanModeGameMode_K.h"

#include "../Player/SnowRumbleCharacter.h"
#include "../UI/SnowRumblePlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "SnowmanModeGameState_K.h"
#include "SnowRumblePlayerState.h"

ASnowmanModeGameMode::ASnowmanModeGameMode()
{
	PlayerControllerClass = ASnowRumblePlayerController::StaticClass();
	GameStateClass = ASnowmanModeGameState::StaticClass();
	PlayerStateClass = ASnowRumblePlayerState::StaticClass();
	DefaultPawnClass = ASnowRumbleCharacter::StaticClass();
	bUseSeamlessTravel = true;

	if (IConsoleVariable* AllowPieSeamlessTravel =
		IConsoleManager::Get().FindConsoleVariable(TEXT("net.AllowPIESeamlessTravel")))
	{
		AllowPieSeamlessTravel->Set(1);
	}
}

void ASnowmanModeGameMode::InitGame(
	const FString& MapName,
	const FString& Options,
	FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	const FString ExpectedPlayersOption =
		UGameplayStatics::ParseOption(Options, TEXT("ExpectedPlayers"));
	ExpectedPlayerCount = ExpectedPlayersOption.IsEmpty()
		? 0
		: FMath::Max(0, FCString::Atoi(*ExpectedPlayersOption));
	bLoadingScreensDismissed = false;
	bSnowmanTimerStarted = false;
}

void ASnowmanModeGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	BroadcastLoadingProgress();
	TryDismissLoadingScreens();
}

void ASnowmanModeGameMode::HandleStartingNewPlayer_Implementation(
	APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	BroadcastLoadingProgress();
	TryDismissLoadingScreens();
}

void ASnowmanModeGameMode::TryDismissLoadingScreens()
{
	if (bLoadingScreensDismissed)
	{
		return;
	}

	const int32 RequiredPlayerCount =
		ExpectedPlayerCount > 0 ? ExpectedPlayerCount : GetNumPlayers();
	if (RequiredPlayerCount <= 0 || GetNumPlayers() < RequiredPlayerCount)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	bLoadingScreensDismissed = true;
	BroadcastLoadingProgress();
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator();
		It;
		++It)
	{
		if (ASnowRumblePlayerController* PlayerController =
			Cast<ASnowRumblePlayerController>(It->Get()))
		{
			PlayerController->ClientHideLoadingScreen();
		}
	}

	if (!bSnowmanTimerStarted)
	{
		bSnowmanTimerStarted = true;
		World->GetTimerManager().SetTimerForNextTick(
			this,
			&ASnowmanModeGameMode::StartSnowmanModeAfterLoading);
	}
}

void ASnowmanModeGameMode::StartSnowmanModeAfterLoading()
{
	if (ASnowmanModeGameState* SnowmanGameState =
		GetGameState<ASnowmanModeGameState>())
	{
		SnowmanGameState->StartSnowmanModeTimerFromServer(
			SnowmanModeTimeLimitSeconds);
	}
}

void ASnowmanModeGameMode::BroadcastLoadingProgress()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const int32 RequiredPlayerCount =
		ExpectedPlayerCount > 0 ? ExpectedPlayerCount : GetNumPlayers();
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator();
		It;
		++It)
	{
		if (ASnowRumblePlayerController* PlayerController =
			Cast<ASnowRumblePlayerController>(It->Get()))
		{
			PlayerController->ClientUpdateLoadingProgress(
				GetNumPlayers(),
				RequiredPlayerCount);
		}
	}
}

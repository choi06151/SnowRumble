// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleLobbyGameMode.h"

#include "../Audio/SnowRumbleAudioHelpers.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Misc/PackageName.h"
#include "../Player/SnowRumbleCharacter.h"
#include "../UI/LobbyPlayerController.h"
#include "../UI/SnowRumblePlayerController.h"
#include "HAL/IConsoleManager.h"
#include "GameFramework/GameModeBase.h"
#include "SnowmanModeGameMode_K.h"
#include "SnowRumbleLobbyGameState.h"
#include "SnowRumbleMatchSubsystem_C.h"
#include "SnowRumblePlayerState.h"
#include "Sound/SoundBase.h"

namespace
{
constexpr const TCHAR* LobbyPvpGameModeTravelPath =
	TEXT("/Game/Game/BP_SnowRumblePVPGameMode.BP_SnowRumblePVPGameMode_C");

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

void EnsureLobbyTravelOption(FString& TravelUrl, const TCHAR* Option)
{
	if (!TravelUrl.Contains(Option, ESearchCase::IgnoreCase))
	{
		TravelUrl += Option;
	}
}

void EnsureLobbyTravelOptionValue(
	FString& TravelUrl,
	const TCHAR* OptionName,
	const FString& OptionValue)
{
	if (OptionValue.IsEmpty())
	{
		return;
	}

	const FString OptionPrefix = FString::Printf(TEXT("%s="), OptionName);
	if (!TravelUrl.Contains(OptionPrefix, ESearchCase::IgnoreCase))
	{
		TravelUrl += FString::Printf(
			TEXT("?%s=%s"),
			OptionName,
			*OptionValue);
	}
}

FString GetMapPackageNameFromTravelUrl(const FString& TravelUrl)
{
	FString MapPackageName = TravelUrl;
	int32 OptionStartIndex = INDEX_NONE;
	if (MapPackageName.FindChar(TEXT('?'), OptionStartIndex))
	{
		MapPackageName.LeftInline(OptionStartIndex);
	}
	return MapPackageName;
}
}

ASnowRumbleLobbyGameMode::ASnowRumbleLobbyGameMode()
{
	GameStateClass = ASnowRumbleLobbyGameState::StaticClass();
	PlayerStateClass = ASnowRumblePlayerState::StaticClass();
	PlayerControllerClass = ALobbyPlayerController::StaticClass();
	DefaultPawnClass = ASnowRumbleCharacter::StaticClass();
	SnowmanModeGameModeClass = ASnowmanModeGameMode::StaticClass();
	bUseSeamlessTravel = true;

	if (IConsoleVariable* AllowPieSeamlessTravel =
		IConsoleManager::Get().FindConsoleVariable(TEXT("net.AllowPIESeamlessTravel")))
	{
		AllowPieSeamlessTravel->Set(1);
	}
}

void ASnowRumbleLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
	BroadcastBackgroundMusic();
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
		|| !LobbyGameState->CanStartLobbyMatch())
	{
		return;
	}

	PendingMatchTravelUrl =
		LobbyGameState->GetLobbyMode() == ESnowRumbleLobbyMode::Snowman
			? BuildSnowmanModeTravelUrl(LobbyGameState->GetLobbyPlayers().Num())
			: BuildMatchTravelUrl(LobbyGameState->GetLobbyPlayers().Num());
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

	BroadcastBackgroundMusic();
}

void ASnowRumbleLobbyGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);
	BroadcastBackgroundMusic();
}

void ASnowRumbleLobbyGameMode::HandleStartingNewPlayer_Implementation(
	APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	BroadcastBackgroundMusic();
}

void ASnowRumbleLobbyGameMode::ShuffleLobbyTeamsFromServer(int32 TeamCount)
{
	if (!HasAuthority())
	{
		return;
	}

	TeamCount = FMath::Clamp(TeamCount, 2, 4);
	TArray<ASnowRumblePlayerState*> LobbyPlayers = GetShuffleableLobbyPlayers();
	if (LobbyPlayers.Num() < TeamCount)
	{
		return;
	}

	const TArray<ESnowRumbleTeam>& TeamChoices = GetLobbyTeamChoices();
	TArray<int32> PlayerOrder;
	TArray<int32> TeamOrder;
	for (int32 PlayerIndex = 0; PlayerIndex < LobbyPlayers.Num(); ++PlayerIndex)
	{
		PlayerOrder.Add(PlayerIndex);
	}
	for (int32 TeamIndex = 0; TeamIndex < TeamChoices.Num(); ++TeamIndex)
	{
		TeamOrder.Add(TeamIndex);
	}

	for (int32 Index = PlayerOrder.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = FMath::RandRange(0, Index);
		PlayerOrder.Swap(Index, SwapIndex);
	}
	for (int32 Index = TeamOrder.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = FMath::RandRange(0, Index);
		TeamOrder.Swap(Index, SwapIndex);
	}

	for (int32 AssignmentIndex = 0; AssignmentIndex < PlayerOrder.Num();
		++AssignmentIndex)
	{
		ASnowRumblePlayerState* PlayerState =
			LobbyPlayers[PlayerOrder[AssignmentIndex]];
		const ESnowRumbleTeam AssignedTeam =
			TeamChoices[TeamOrder[AssignmentIndex % TeamCount]];
		if (PlayerState)
		{
			PlayerState->AssignLobbyTeamFromServer(AssignedTeam);
		}
	}

	if (ASnowRumbleLobbyGameState* LobbyGameState =
		GetGameState<ASnowRumbleLobbyGameState>())
	{
		LobbyGameState->NotifyLobbyStateChanged();
	}
}

void ASnowRumbleLobbyGameMode::ShuffleLobbyPlayersIndividuallyFromServer()
{
	if (!HasAuthority())
	{
		return;
	}

	TArray<ASnowRumblePlayerState*> LobbyPlayers = GetShuffleableLobbyPlayers();
	const TArray<ESnowRumbleTeam>& TeamChoices = GetLobbyTeamChoices();
	if (LobbyPlayers.IsEmpty() || LobbyPlayers.Num() > TeamChoices.Num())
	{
		return;
	}

	TArray<int32> PlayerOrder;
	TArray<int32> TeamOrder;
	for (int32 PlayerIndex = 0; PlayerIndex < LobbyPlayers.Num(); ++PlayerIndex)
	{
		PlayerOrder.Add(PlayerIndex);
	}
	for (int32 TeamIndex = 0; TeamIndex < TeamChoices.Num(); ++TeamIndex)
	{
		TeamOrder.Add(TeamIndex);
	}

	for (int32 Index = PlayerOrder.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = FMath::RandRange(0, Index);
		PlayerOrder.Swap(Index, SwapIndex);
	}
	for (int32 Index = TeamOrder.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = FMath::RandRange(0, Index);
		TeamOrder.Swap(Index, SwapIndex);
	}

	for (int32 AssignmentIndex = 0; AssignmentIndex < PlayerOrder.Num();
		++AssignmentIndex)
	{
		ASnowRumblePlayerState* PlayerState =
			LobbyPlayers[PlayerOrder[AssignmentIndex]];
		const ESnowRumbleTeam AssignedTeam =
			TeamChoices[TeamOrder[AssignmentIndex]];
		if (PlayerState)
		{
			PlayerState->AssignLobbyTeamFromServer(AssignedTeam);
		}
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

TArray<ASnowRumblePlayerState*> ASnowRumbleLobbyGameMode::GetShuffleableLobbyPlayers()
	const
{
	TArray<ASnowRumblePlayerState*> LobbyPlayers;
	if (const ASnowRumbleLobbyGameState* LobbyGameState =
		GetGameState<ASnowRumbleLobbyGameState>())
	{
		LobbyPlayers = LobbyGameState->GetLobbyPlayers();
	}
	return LobbyPlayers;
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
	int32 ExpectedPlayerCount)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USnowRumbleMatchSubsystem* MatchSubsystem =
			GameInstance->GetSubsystem<USnowRumbleMatchSubsystem>())
		{
			const ASnowRumbleLobbyGameState* LobbyGameState =
				GetGameState<ASnowRumbleLobbyGameState>();
			TArray<FString> PvPLevelPaths = GetPvPLevelCandidatePaths();
			if (PvPLevelPaths.IsEmpty() && !MatchTravelUrl.IsEmpty())
			{
				PvPLevelPaths.Add(MatchTravelUrl);
			}
			MatchSubsystem->BeginPvPMatch(
				LobbyGameState ? LobbyGameState->GetMatchRoundLimit() : 1,
				LobbyGameState
					? LobbyGameState->GetGameSpeed()
					: ESnowRumbleGameSpeed::Normal,
				PvPLevelPaths);
		}
	}

	FString TravelUrl = SelectPvPLevelPath();
	if (TravelUrl.IsEmpty())
	{
		TravelUrl = MatchTravelUrl;
	}
	if (TravelUrl.IsEmpty())
	{
		return FString();
	}

	PendingMatchMapPackageName = GetMapPackageNameFromTravelUrl(TravelUrl);
	EnsureLobbyTravelOption(TravelUrl, TEXT("?listen"));
	EnsureLobbyTravelOptionValue(
		TravelUrl,
		TEXT("game"),
		LobbyPvpGameModeTravelPath);

	if (ExpectedPlayerCount > 0)
	{
		EnsureLobbyTravelOptionValue(
			TravelUrl,
			TEXT("ExpectedPlayers"),
			FString::FromInt(ExpectedPlayerCount));
	}
	return TravelUrl;
}

FString ASnowRumbleLobbyGameMode::BuildSnowmanModeTravelUrl(
	int32 ExpectedPlayerCount)
{
	const TArray<FString> CandidateLevelPaths = GetPvPLevelCandidatePaths();
	FString TravelUrl = CandidateLevelPaths.IsEmpty()
		? FString()
		: CandidateLevelPaths[
			FMath::RandRange(0, CandidateLevelPaths.Num() - 1)];
	if (TravelUrl.IsEmpty())
	{
		TravelUrl = MatchTravelUrl;
	}
	if (TravelUrl.IsEmpty() || !SnowmanModeGameModeClass)
	{
		return FString();
	}

	PendingMatchMapPackageName = GetMapPackageNameFromTravelUrl(TravelUrl);
	EnsureLobbyTravelOption(TravelUrl, TEXT("?listen"));

	const FString GameModePath = SnowmanModeGameModeClass->GetPathName();
	EnsureLobbyTravelOptionValue(TravelUrl, TEXT("game"), GameModePath);

	if (ExpectedPlayerCount > 0)
	{
		EnsureLobbyTravelOptionValue(
			TravelUrl,
			TEXT("ExpectedPlayers"),
			FString::FromInt(ExpectedPlayerCount));
	}
	return TravelUrl;
}

FString ASnowRumbleLobbyGameMode::SelectPvPLevelPath() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USnowRumbleMatchSubsystem* MatchSubsystem =
			GameInstance->GetSubsystem<USnowRumbleMatchSubsystem>())
		{
			return MatchSubsystem->SelectNextPvPLevelPath(MatchTravelUrl);
		}
	}

	const TArray<FString> CandidateLevelPaths = GetPvPLevelCandidatePaths();
	if (CandidateLevelPaths.IsEmpty())
	{
		return FString();
	}

	return CandidateLevelPaths[
		FMath::RandRange(0, CandidateLevelPaths.Num() - 1)];
}

TArray<FString> ASnowRumbleLobbyGameMode::GetPvPLevelCandidatePaths() const
{
	TArray<FString> CandidateLevelPaths;
	for (const TSoftObjectPtr<UWorld>& PvPLevelCandidate : PvPLevelCandidates)
	{
		const FSoftObjectPath LevelPath = PvPLevelCandidate.ToSoftObjectPath();
		if (!LevelPath.IsValid())
		{
			continue;
		}
		const FString LongPackageName = LevelPath.GetLongPackageName();
		if (!LongPackageName.IsEmpty())
		{
			CandidateLevelPaths.AddUnique(LongPackageName);
		}
	}

	return CandidateLevelPaths;
}

FSnowRumbleLoadingMapPresentation
ASnowRumbleLobbyGameMode::GetLoadingMapPresentation(
	const FString& MapPackageName) const
{
	FSnowRumbleLoadingMapPresentation FallbackPresentation;
	if (!MapPackageName.IsEmpty())
	{
		FallbackPresentation.DisplayName =
			FText::FromString(FPackageName::GetShortName(MapPackageName));
	}

	for (const FSnowRumbleLoadingMapPresentation& Presentation
		: PvPLevelLoadingPresentations)
	{
		const FSoftObjectPath LevelPath =
			Presentation.Level.ToSoftObjectPath();
		if (!LevelPath.IsValid())
		{
			continue;
		}

		if (LevelPath.GetLongPackageName().Equals(
			MapPackageName,
			ESearchCase::IgnoreCase))
		{
			FSnowRumbleLoadingMapPresentation Result = Presentation;
			if (Result.DisplayName.IsEmpty())
			{
				Result.DisplayName = FallbackPresentation.DisplayName;
			}
			return Result;
		}
	}

	return FallbackPresentation;
}

TArray<FString> ASnowRumbleLobbyGameMode::GetTeamPlayerNamesFor(
	const ASnowRumblePlayerState* LocalPlayerState) const
{
	TArray<FString> TeamPlayerNames;
	const ASnowRumbleLobbyGameState* LobbyGameState =
		GetGameState<ASnowRumbleLobbyGameState>();
	if (!LobbyGameState || !LocalPlayerState)
	{
		return TeamPlayerNames;
	}

	const ESnowRumbleTeam LocalTeam = LocalPlayerState->GetLobbyTeam();
	for (const ASnowRumblePlayerState* CandidatePlayerState
		: LobbyGameState->GetLobbyPlayers())
	{
		if (!CandidatePlayerState)
		{
			continue;
		}

		if (LocalTeam != ESnowRumbleTeam::None
			&& CandidatePlayerState->GetLobbyTeam() != LocalTeam)
		{
			continue;
		}
		if (LocalTeam == ESnowRumbleTeam::None
			&& CandidatePlayerState != LocalPlayerState)
		{
			continue;
		}

		FString PlayerName = CandidatePlayerState->GetLobbyPlayerName();
		if (PlayerName.IsEmpty())
		{
			PlayerName = CandidatePlayerState->GetPlayerName();
		}
		if (!PlayerName.IsEmpty())
		{
			TeamPlayerNames.Add(PlayerName);
		}
	}

	return TeamPlayerNames;
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
	const FSnowRumbleLoadingMapPresentation LoadingMapPresentation =
		GetLoadingMapPresentation(PendingMatchMapPackageName);

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator();
		It;
		++It)
	{
		if (ASnowRumblePlayerController* PlayerController =
			Cast<ASnowRumblePlayerController>(It->Get()))
		{
			const ASnowRumblePlayerState* LocalPlayerState =
				PlayerController->GetPlayerState<ASnowRumblePlayerState>();
			PlayerController->ClientSetLoadingPresentation(
				PendingMatchMapPackageName,
				LoadingMapPresentation.DisplayName,
				LoadingMapPresentation.LoadingImage,
				GetTeamPlayerNamesFor(LocalPlayerState));
			PlayerController->ClientShowLoadingScreen();
			PlayerController->ClientUpdateLoadingProgress(
				0,
				ExpectedPlayerCount);
		}
	}
}

void ASnowRumbleLobbyGameMode::BroadcastBackgroundMusic() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator();
		It;
		++It)
	{
		if (ASnowRumblePlayerController* PlayerController =
			Cast<ASnowRumblePlayerController>(It->Get()))
		{
			PlayerController->ClientPlayBackgroundMusic(BackgroundMusicSound);
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
			PendingMatchMapPackageName.Empty();
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

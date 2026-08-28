// Copyright Epic Games, Inc. All Rights Reserved.

#include "PodiumGameMode.h"

#include "../Audio/SnowRumbleAudioHelpers.h"
#include "PodiumPlayerController.h"
#include "SnowRumbleMatchSubsystem_C.h"
#include "SnowRumblePlayerState.h"
#include "../Player/SnowRumbleCharacter.h"
#include "Camera/CameraActor.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

namespace
{
constexpr const TCHAR* PodiumLobbyGameModeTravelPath =
	TEXT("/Game/Game/BP_LobbyGameMode.BP_LobbyGameMode_C");

FText BuildPodiumReturnSubtitle(int32 SecondsRemaining)
{
	const int32 SafeSecondsRemaining = FMath::Max(0, SecondsRemaining);
	return FText::Format(
		NSLOCTEXT(
			"SnowRumble",
			"PodiumReturnSubtitleCountdown",
			"{0}초 후 로비로 돌아갑니다."),
		FText::AsNumber(SafeSecondsRemaining));
}

void EnsurePodiumTravelOption(FString& TravelUrl, const TCHAR* Option)
{
	if (!TravelUrl.Contains(Option, ESearchCase::IgnoreCase))
	{
		TravelUrl += Option;
	}
}

void EnsurePodiumTravelOptionValue(
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
}

APodiumGameMode::APodiumGameMode()
{
	PlayerControllerClass = APodiumPlayerController::StaticClass();
	DefaultPawnClass = ASnowRumbleCharacter::StaticClass();
	PlayerStateClass = ASnowRumblePlayerState::StaticClass();
	bStartPlayersAsSpectators = true;
}

void APodiumGameMode::BeginPlay()
{
	Super::BeginPlay();
	SchedulePodiumSetup();
	BroadcastBackgroundMusic();
}

void APodiumGameMode::InitGame(
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
	bPodiumSetupComplete = false;
	GetWorldTimerManager().ClearTimer(PodiumSetupTimerHandle);
	GetWorldTimerManager().ClearTimer(PodiumReturnTimerHandle);
}

void APodiumGameMode::PostLogin(APlayerController* NewPlayer)
{
	AGameModeBase::PostLogin(NewPlayer);
	SchedulePodiumSetup();
	BroadcastBackgroundMusic();
}

void APodiumGameMode::HandleStartingNewPlayer_Implementation(
	APlayerController* NewPlayer)
{
	AGameModeBase::HandleStartingNewPlayer_Implementation(NewPlayer);
	SchedulePodiumSetup();
	BroadcastBackgroundMusic();
}

void APodiumGameMode::BroadcastBackgroundMusic() const
{
	UWorld* World = GetWorld();
	if (!World || !BackgroundMusicSound)
	{
		return;
	}

	const FSoftObjectPath BackgroundMusicPath(BackgroundMusicSound);
	if (!BackgroundMusicPath.IsValid())
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator();
		It;
		++It)
	{
		if (APodiumPlayerController* PodiumController =
			Cast<APodiumPlayerController>(It->Get()))
		{
			PodiumController->ClientPlayBackgroundMusic(BackgroundMusicPath);
		}
	}
}

void APodiumGameMode::SchedulePodiumSetup()
{
	if (bPodiumSetupComplete || !HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(PodiumSetupTimerHandle);
	GetWorldTimerManager().SetTimer(
		PodiumSetupTimerHandle,
		this,
		&APodiumGameMode::SetupPodiumFromServer,
		PodiumSetupRetryDelaySeconds,
		false);
}

void APodiumGameMode::SetupPodiumFromServer()
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = GetGameInstance();
	USnowRumbleMatchSubsystem* MatchSubsystem = GameInstance
		? GameInstance->GetSubsystem<USnowRumbleMatchSubsystem>()
		: nullptr;
	if (!HasAuthority() || !World || !MatchSubsystem)
	{
		return;
	}

	const int32 RequiredPlayerCount =
		ExpectedPlayerCount > 0 ? ExpectedPlayerCount : GetNumPlayers();
	if (RequiredPlayerCount > 0 && GetNumPlayers() < RequiredPlayerCount)
	{
		SchedulePodiumSetup();
		return;
	}

	TArray<FSnowRumblePodiumTeamResult> TeamResults;
	BuildParticipatingTeamResults(MatchSubsystem, TeamResults);
	if (TeamResults.IsEmpty())
	{
		SchedulePodiumSetup();
		return;
	}

	PodiumWinningTeam = TeamResults[0].Team;
	TArray<APlayerStart*> WinningTeamStarts;
	CollectPodiumPlayerStarts(WinningTeamStarts);

	ACameraActor* PodiumCamera = FindPodiumCamera();
	TArray<APlayerController*> WinningTeamControllers;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator();
		It;
		++It)
	{
		APlayerController* PlayerController = It->Get();
		if (!PlayerController)
		{
			continue;
		}

		ASnowRumblePlayerState* SnowPlayerState =
			PlayerController->GetPlayerState<ASnowRumblePlayerState>();
		if (!SnowPlayerState)
		{
			continue;
		}

		const ESnowRumbleTeam PlayerTeam = SnowPlayerState->GetLobbyTeam();
		if (PlayerTeam == PodiumWinningTeam)
		{
			WinningTeamControllers.Add(PlayerController);
		}

		if (PodiumCamera)
		{
			PlayerController->SetViewTargetWithBlend(PodiumCamera, 0.5f);
		}

	}

	if (!WinningTeamStarts.IsEmpty())
	{
		if (WinningTeamControllers.Num() >= 4
			&& WinningTeamStarts.Num() >= 4)
		{
			for (int32 Index = WinningTeamStarts.Num() - 1; Index > 0; --Index)
			{
				const int32 SwapIndex = FMath::RandRange(0, Index);
				WinningTeamStarts.Swap(Index, SwapIndex);
			}
		}

		for (int32 Index = 0; Index < WinningTeamControllers.Num(); ++Index)
		{
			APlayerController* PlayerController = WinningTeamControllers[Index];
			APlayerStart* SelectedStart =
				WinningTeamStarts[Index % WinningTeamStarts.Num()];
			RestartPlayerAtPlayerStart(PlayerController, SelectedStart);
			if (ASnowRumbleCharacter* PodiumCharacter =
				Cast<ASnowRumbleCharacter>(PlayerController->GetPawn()))
			{
				PodiumCharacter->PlayRandomServerDirectedEmote();
			}
			if (PodiumCamera)
			{
				PlayerController->SetViewTargetWithBlend(PodiumCamera, 0.5f);
			}
		}
	}

	StartPodiumReturnCountdown(PodiumWinningTeam);

	bPodiumSetupComplete = true;
}

void APodiumGameMode::ReturnToLobbyAfterPodium()
{
	if (!HasAuthority() || PodiumLobbyReturnTravelUrl.IsEmpty())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(PodiumReturnCountdownTimerHandle);
	GetWorldTimerManager().ClearTimer(PodiumReturnTimerHandle);

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USnowRumbleMatchSubsystem* MatchSubsystem =
			GameInstance->GetSubsystem<USnowRumbleMatchSubsystem>())
		{
			MatchSubsystem->ResetPvPMatch();
		}
	}

	FString TravelUrl = PodiumLobbyReturnTravelUrl;
	EnsurePodiumTravelOption(TravelUrl, TEXT("?listen"));
	EnsurePodiumTravelOptionValue(
		TravelUrl,
		TEXT("game"),
		PodiumLobbyGameModeTravelPath);

	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(TravelUrl);
	}
}

void APodiumGameMode::StartPodiumReturnCountdown(ESnowRumbleTeam WinningTeam)
{
	PodiumWinningTeam = WinningTeam;
	PodiumReturnCountdownRemainingSeconds =
		FMath::Max(0, FMath::CeilToInt(PodiumReturnDelaySeconds));
	BroadcastPodiumReturnCountdown();

	if (PodiumReturnCountdownRemainingSeconds <= 0)
	{
		ReturnToLobbyAfterPodium();
		return;
	}

	GetWorldTimerManager().ClearTimer(PodiumReturnCountdownTimerHandle);
	GetWorldTimerManager().SetTimer(
		PodiumReturnCountdownTimerHandle,
		this,
		&APodiumGameMode::TickPodiumReturnCountdown,
		1.0f,
		true);
}

void APodiumGameMode::TickPodiumReturnCountdown()
{
	if (!HasAuthority())
	{
		return;
	}

	if (PodiumReturnCountdownRemainingSeconds <= 0)
	{
		GetWorldTimerManager().ClearTimer(PodiumReturnCountdownTimerHandle);
		ReturnToLobbyAfterPodium();
		return;
	}

	--PodiumReturnCountdownRemainingSeconds;
	if (PodiumReturnCountdownRemainingSeconds <= 0)
	{
		GetWorldTimerManager().ClearTimer(PodiumReturnCountdownTimerHandle);
		ReturnToLobbyAfterPodium();
		return;
	}

	BroadcastPodiumReturnCountdown();
}

void APodiumGameMode::BroadcastPodiumReturnCountdown()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FText SubtitleText =
		BuildPodiumReturnSubtitle(PodiumReturnCountdownRemainingSeconds);
	for (FConstPlayerControllerIterator It =
		World->GetPlayerControllerIterator();
		It;
		++It)
	{
		if (APodiumPlayerController* PodiumController =
			Cast<APodiumPlayerController>(It->Get()))
		{
			if (PodiumReturnCountdownRemainingSeconds
				== FMath::CeilToInt(PodiumReturnDelaySeconds))
			{
				PodiumController->ClientSetPodiumWinner(
					PodiumWinningTeam,
					SubtitleText);
			}
			else
			{
				PodiumController->ClientUpdatePodiumReturnSubtitle(
					SubtitleText);
			}
		}
	}
}

void APodiumGameMode::BuildParticipatingTeamResults(
	USnowRumbleMatchSubsystem* MatchSubsystem,
	TArray<FSnowRumblePodiumTeamResult>& OutResults) const
{
	OutResults.Reset();
	if (!MatchSubsystem)
	{
		return;
	}

	if (const UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It =
			World->GetPlayerControllerIterator();
			It;
			++It)
		{
			const APlayerController* PlayerController = It->Get();
			const ASnowRumblePlayerState* SnowPlayerState =
				PlayerController
					? PlayerController->GetPlayerState<ASnowRumblePlayerState>()
					: nullptr;
			if (!SnowPlayerState)
			{
				continue;
			}

			const ESnowRumbleTeam Team = SnowPlayerState->GetLobbyTeam();
			if (Team == ESnowRumbleTeam::None
				|| OutResults.ContainsByPredicate(
					[Team](const FSnowRumblePodiumTeamResult& Result)
					{
						return Result.Team == Team;
					}))
			{
				continue;
			}

			FSnowRumblePodiumTeamResult Result;
			Result.Team = Team;
			Result.RoundWins = MatchSubsystem->GetTeamRoundWinCount(Team);
			OutResults.Add(Result);
		}
	}

	OutResults.Sort(
		[](const FSnowRumblePodiumTeamResult& A,
			const FSnowRumblePodiumTeamResult& B)
		{
			if (A.RoundWins != B.RoundWins)
			{
				return A.RoundWins > B.RoundWins;
			}
			return static_cast<uint8>(A.Team) < static_cast<uint8>(B.Team);
		});
}

void APodiumGameMode::CollectPodiumPlayerStarts(
	TArray<APlayerStart*>& OutWinningTeamStarts) const
{
	OutWinningTeamStarts.Reset();
	OutWinningTeamStarts.SetNumZeroed(4);

	if (const UWorld* World = GetWorld())
	{
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			APlayerStart* PlayerStart = *It;
			if (!PlayerStart)
			{
				continue;
			}

			if (PlayerStart->ActorHasTag(TEXT("Podium_Team1")))
			{
				OutWinningTeamStarts[0] = PlayerStart;
			}
			else if (PlayerStart->ActorHasTag(TEXT("Podium_Team2")))
			{
				OutWinningTeamStarts[1] = PlayerStart;
			}
			else if (PlayerStart->ActorHasTag(TEXT("Podium_Team3")))
			{
				OutWinningTeamStarts[2] = PlayerStart;
			}
			else if (PlayerStart->ActorHasTag(TEXT("Podium_Team4")))
			{
				OutWinningTeamStarts[3] = PlayerStart;
			}
		}
	}

	OutWinningTeamStarts.RemoveAll(
		[](const APlayerStart* PlayerStart)
		{
			return PlayerStart == nullptr;
		});
}

ACameraActor* APodiumGameMode::FindPodiumCamera() const
{
	if (const UWorld* World = GetWorld())
	{
		for (TActorIterator<ACameraActor> It(World); It; ++It)
		{
			ACameraActor* Camera = *It;
			if (Camera && Camera->ActorHasTag(TEXT("Podium_Camera")))
			{
				return Camera;
			}
		}
	}

	return nullptr;
}

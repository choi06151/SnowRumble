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
	if (!World)
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
			PodiumController->ClientPlayBackgroundMusic(BackgroundMusicSound);
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

	TArray<APlayerStart*> FirstPlaceStarts;
	TArray<APlayerStart*> SecondPlaceStarts;
	TArray<APlayerStart*> ThirdPlaceStarts;
	CollectPodiumPlayerStarts(
		FirstPlaceStarts,
		SecondPlaceStarts,
		ThirdPlaceStarts);

	ACameraActor* PodiumCamera = FindPodiumCamera();
	TMap<ESnowRumbleTeam, int32> UsedStartCounts;

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
		const int32 RankIndex = TeamResults.IndexOfByPredicate(
			[PlayerTeam](const FSnowRumblePodiumTeamResult& Result)
			{
				return Result.Team == PlayerTeam;
			});

		TArray<APlayerStart*>* StartsForRank = nullptr;
		if (RankIndex == 0)
		{
			StartsForRank = &FirstPlaceStarts;
		}
		else if (RankIndex == 1)
		{
			StartsForRank = &SecondPlaceStarts;
		}
		else if (RankIndex == 2)
		{
			StartsForRank = &ThirdPlaceStarts;
		}

		if (StartsForRank && !StartsForRank->IsEmpty())
		{
			const int32 UsedCount = UsedStartCounts.FindRef(PlayerTeam);
			APlayerStart* SelectedStart =
				(*StartsForRank)[UsedCount % StartsForRank->Num()];
			RestartPlayerAtPlayerStart(PlayerController, SelectedStart);
			UsedStartCounts.Add(PlayerTeam, UsedCount + 1);
		}

		if (PodiumCamera)
		{
			PlayerController->SetViewTargetWithBlend(PodiumCamera, 0.5f);
		}

		if (ASnowRumbleCharacter* Character =
			Cast<ASnowRumbleCharacter>(PlayerController->GetPawn()))
		{
			Character->PlayServerDirectedEmote(0);
		}
	}

	const FSnowRumblePodiumTeamResult* FirstResult =
		TeamResults.IsValidIndex(0) ? &TeamResults[0] : nullptr;
	const FSnowRumblePodiumTeamResult* SecondResult =
		TeamResults.IsValidIndex(1) ? &TeamResults[1] : nullptr;
	const FSnowRumblePodiumTeamResult* ThirdResult =
		TeamResults.IsValidIndex(2) ? &TeamResults[2] : nullptr;
	const FText FirstText = BuildResultText(FirstResult);
	const FText SecondText = BuildResultText(SecondResult);
	const FText ThirdText = BuildResultText(ThirdResult);
	const FText SubtitleText = NSLOCTEXT(
		"SnowRumble",
		"PodiumReturnSubtitle",
		"10초 후 로비로 돌아갑니다.");

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator();
		It;
		++It)
	{
		if (APodiumPlayerController* PodiumController =
			Cast<APodiumPlayerController>(It->Get()))
		{
			PodiumController->ClientSetPodiumResults(
				FirstText,
				SecondText,
				ThirdText,
				SubtitleText);
		}
	}

	bPodiumSetupComplete = true;
	GetWorldTimerManager().SetTimer(
		PodiumReturnTimerHandle,
		this,
		&APodiumGameMode::ReturnToLobbyAfterPodium,
		PodiumReturnDelaySeconds,
		false);
}

void APodiumGameMode::ReturnToLobbyAfterPodium()
{
	if (!HasAuthority() || PodiumLobbyReturnTravelUrl.IsEmpty())
	{
		return;
	}

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
	TArray<APlayerStart*>& OutFirstPlaceStarts,
	TArray<APlayerStart*>& OutSecondPlaceStarts,
	TArray<APlayerStart*>& OutThirdPlaceStarts) const
{
	OutFirstPlaceStarts.Reset();
	OutSecondPlaceStarts.Reset();
	OutThirdPlaceStarts.Reset();

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
				OutFirstPlaceStarts.Add(PlayerStart);
			}
			else if (PlayerStart->ActorHasTag(TEXT("Podium_Team2")))
			{
				OutSecondPlaceStarts.Add(PlayerStart);
			}
			else if (PlayerStart->ActorHasTag(TEXT("Podium_Team3")))
			{
				OutThirdPlaceStarts.Add(PlayerStart);
			}
		}
	}
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

FText APodiumGameMode::BuildResultText(
	const FSnowRumblePodiumTeamResult* Result) const
{
	if (!Result)
	{
		return FText::GetEmpty();
	}

	return FText::Format(
		NSLOCTEXT("SnowRumble", "PodiumTeamResultFormat", "{0} - {1}승"),
		GetTeamDisplayName(Result->Team),
		FText::AsNumber(Result->RoundWins));
}

FText APodiumGameMode::GetTeamDisplayName(ESnowRumbleTeam Team) const
{
	switch (Team)
	{
	case ESnowRumbleTeam::Red:
		return NSLOCTEXT("SnowRumble", "PodiumRedTeam", "빨강 팀");
	case ESnowRumbleTeam::Sky:
		return NSLOCTEXT("SnowRumble", "PodiumSkyTeam", "하늘 팀");
	case ESnowRumbleTeam::Green:
		return NSLOCTEXT("SnowRumble", "PodiumGreenTeam", "초록 팀");
	case ESnowRumbleTeam::Yellow:
		return NSLOCTEXT("SnowRumble", "PodiumYellowTeam", "노랑 팀");
	case ESnowRumbleTeam::Purple:
		return NSLOCTEXT("SnowRumble", "PodiumPurpleTeam", "보라 팀");
	case ESnowRumbleTeam::Pink:
		return NSLOCTEXT("SnowRumble", "PodiumPinkTeam", "분홍 팀");
	case ESnowRumbleTeam::Blue:
		return NSLOCTEXT("SnowRumble", "PodiumBlueTeam", "파랑 팀");
	case ESnowRumbleTeam::White:
		return NSLOCTEXT("SnowRumble", "PodiumWhiteTeam", "하양 팀");
	case ESnowRumbleTeam::None:
	default:
		return NSLOCTEXT("SnowRumble", "PodiumNoTeam", "팀 없음");
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleGameMode.h"

#include "Engine/GameInstance.h"
#include "../Item/GiftBox_C.h"
#include "../Map/SnowIslandWaterPressureActor_J.h"
#include "../Player/SnowRumbleCharacter.h"
#include "../Player/SnowRumbleHealthComponent.h"
#include "../UI/SnowRumblePlayerController.h"
#include "Components/CapsuleComponent.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/Character.h"
#include "Engine/TargetPoint.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "SnowRumbleGameState_C.h"
#include "SnowRumbleMatchSubsystem_C.h"
#include "SnowRumblePlayerState.h"

namespace
{
constexpr const TCHAR* PvpGameModeTravelPath =
	TEXT("/Game/Game/BP_SnowRumblePVPGameMode.BP_SnowRumblePVPGameMode_C");
constexpr const TCHAR* PodiumGameModeTravelPath =
	TEXT("/Game/Game/BP_SnowRumblePodiumGameMode.BP_SnowRumblePodiumGameMode_C");
constexpr const TCHAR* LobbyGameModeTravelPath =
	TEXT("/Game/Game/BP_LobbyGameMode.BP_LobbyGameMode_C");

FVector MakeRandomHorizontalOffset(float Radius)
{
	if (Radius <= 0.0f)
	{
		return FVector::ZeroVector;
	}

	const float AngleRadians =
		FMath::FRandRange(0.0f, UE_TWO_PI);
	const float Distance =
		FMath::Sqrt(FMath::FRand()) * Radius;
	return FVector(
		FMath::Cos(AngleRadians) * Distance,
		FMath::Sin(AngleRadians) * Distance,
		0.0f);
}

void EnsureTravelOption(FString& TravelUrl, const TCHAR* Option)
{
	if (!TravelUrl.Contains(Option, ESearchCase::IgnoreCase))
	{
		TravelUrl += Option;
	}
}

void EnsureTravelOptionValue(
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

ASnowRumbleGameMode::ASnowRumbleGameMode()
{
	PlayerControllerClass = ASnowRumblePlayerController::StaticClass();
	GameStateClass = ASnowRumbleGameState::StaticClass();
	PlayerStateClass = ASnowRumblePlayerState::StaticClass();
	DefaultPawnClass = ASnowRumbleCharacter::StaticClass();
	bUseSeamlessTravel = true;

	if (IConsoleVariable* AllowPieSeamlessTravel =
		IConsoleManager::Get().FindConsoleVariable(TEXT("net.AllowPIESeamlessTravel")))
	{
		AllowPieSeamlessTravel->Set(1);
	}
}

void ASnowRumbleGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (ASnowRumbleGameState* SnowRumbleGameState =
		GetGameState<ASnowRumbleGameState>())
	{
		SnowRumbleGameState->ApplyMatchStateFromServer(GetMatchSubsystem());
	}
}

void ASnowRumbleGameMode::InitGame(
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
	bLoadingScreensHidden = false;
	bStartCountdownStarted = false;
	bMatchIntroStarted = false;
	MatchIntroTeamIndex = 0;
	MatchIntroTeams.Reset();
	MapShrinkStage = 0;
	GetWorldTimerManager().ClearTimer(MapShrinkTimerHandle);
	GetWorldTimerManager().ClearTimer(MapShrinkCompletionTimerHandle);
	GetWorldTimerManager().ClearTimer(GiftBoxSpawnTimerHandle);
	GetWorldTimerManager().ClearTimer(MatchIntroTimerHandle);
}

void ASnowRumbleGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	BroadcastLoadingProgress();
	TryDismissLoadingScreens();
}

void ASnowRumbleGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);

	BroadcastLoadingProgress();
	TryDismissLoadingScreens();
}

void ASnowRumbleGameMode::HandleStartingNewPlayer_Implementation(
	APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	BroadcastLoadingProgress();
	TryDismissLoadingScreens();
}

void ASnowRumbleGameMode::TryDismissLoadingScreens()
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

	if (!bStartCountdownStarted)
	{
		bStartCountdownStarted = true;
		World->GetTimerManager().SetTimerForNextTick(
			this,
			&ASnowRumbleGameMode::StartMatchCountdownAfterLoading);
	}
}

void ASnowRumbleGameMode::StartMatchCountdownAfterLoading()
{
	if (MatchStartCountdownDelaySeconds > 0.0f)
	{
		FTimerHandle CountdownDelayTimerHandle;
		GetWorldTimerManager().SetTimer(
			CountdownDelayTimerHandle,
			[this]()
			{
				StartMatchIntroAfterLoading();
			},
			MatchStartCountdownDelaySeconds,
			false);
		return;
	}

	StartMatchIntroAfterLoading();
}

void ASnowRumbleGameMode::StartMatchIntroAfterLoading()
{
	if (bMatchIntroStarted)
	{
		return;
	}

	bMatchIntroStarted = true;
	if (!ShouldPlayMatchIntroSequence())
	{
		PrepareConfirmedMatchCountdown();
		return;
	}

	MatchIntroTeamIndex = 0;
	GetActiveRoundTeams(MatchIntroTeams);
	if (MatchIntroTeams.IsEmpty() || MatchIntroTeamShotSeconds <= 0.0f)
	{
		PrepareConfirmedMatchCountdown();
		return;
	}

	AdvanceMatchIntroSequence();
}

bool ASnowRumbleGameMode::ShouldPlayMatchIntroSequence() const
{
	const USnowRumbleMatchSubsystem* MatchSubsystem = GetMatchSubsystem();
	if (!MatchSubsystem || !MatchSubsystem->IsPvPMatchActive())
	{
		return true;
	}

	if (MatchSubsystem->IsTiebreakerActive())
	{
		return true;
	}

	return MatchSubsystem->GetCurrentRoundNumber() <= 1;
}

void ASnowRumbleGameMode::AdvanceMatchIntroSequence()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		FinishMatchIntroSequence();
		return;
	}

	if (!MatchIntroTeams.IsValidIndex(MatchIntroTeamIndex))
	{
		FinishMatchIntroSequence();
		return;
	}

	const ESnowRumbleTeam IntroTeam = MatchIntroTeams[MatchIntroTeamIndex];
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator();
		It;
		++It)
	{
		if (ASnowRumblePlayerController* PlayerController =
			Cast<ASnowRumblePlayerController>(It->Get()))
		{
			PlayerController->ClientPlayPvpTeamIntroShot(
				IntroTeam,
				MatchIntroTeamShotSeconds);
		}
	}

	++MatchIntroTeamIndex;
	GetWorldTimerManager().SetTimer(
		MatchIntroTimerHandle,
		this,
		&ASnowRumbleGameMode::AdvanceMatchIntroSequence,
		MatchIntroTeamShotSeconds,
		false);
}

void ASnowRumbleGameMode::FinishMatchIntroSequence()
{
	GetWorldTimerManager().ClearTimer(MatchIntroTimerHandle);

	if (UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It =
				World->GetPlayerControllerIterator();
			It;
			++It)
		{
			if (ASnowRumblePlayerController* PlayerController =
				Cast<ASnowRumblePlayerController>(It->Get()))
			{
				PlayerController->ClientFinishPvpTeamIntro();
			}
		}
	}

	PrepareConfirmedMatchCountdown();
}

void ASnowRumbleGameMode::PrepareConfirmedMatchCountdown()
{
	HideLoadingScreensBeforeCountdown();

	if (LoadingScreenHideLeadTimeSeconds > 0.0f)
	{
		FTimerHandle CountdownLeadTimerHandle;
		GetWorldTimerManager().SetTimer(
			CountdownLeadTimerHandle,
			this,
			&ASnowRumbleGameMode::StartConfirmedMatchCountdown,
			LoadingScreenHideLeadTimeSeconds,
			false);
		return;
	}

	StartConfirmedMatchCountdown();
}

void ASnowRumbleGameMode::HideLoadingScreensBeforeCountdown()
{
	if (bLoadingScreensHidden)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	bLoadingScreensHidden = true;
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
}

void ASnowRumbleGameMode::StartConfirmedMatchCountdown()
{
	if (ASnowRumbleGameState* SnowRumbleGameState =
		GetGameState<ASnowRumbleGameState>())
	{
		SnowRumbleGameState->StartMatchCountdownFromServer(
			MatchStartCountdownSeconds);
		if (!SnowRumbleGameState->IsTiebreakerRound())
		{
			ScheduleNextMapShrink();
		}
		ScheduleNextGiftBoxSpawn(
			MatchStartCountdownSeconds
			+ FirstGiftBoxSpawnDelaySeconds);
	}
}

AActor* ASnowRumbleGameMode::ChoosePlayerStart_Implementation(
	AController* Player)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	TArray<AActor*> AvailablePlayerStarts;
	TArray<AActor*> AllPlayerStarts;
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* PlayerStart = *It;
		if (!PlayerStart)
		{
			continue;
		}

		AllPlayerStarts.Add(PlayerStart);
		if (!UsedPlayerStarts.Contains(PlayerStart))
		{
			AvailablePlayerStarts.Add(PlayerStart);
		}
	}

	TArray<AActor*>& CandidatePlayerStarts =
		AvailablePlayerStarts.IsEmpty()
			? AllPlayerStarts
			: AvailablePlayerStarts;
	if (CandidatePlayerStarts.IsEmpty())
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	AActor* SelectedPlayerStart = CandidatePlayerStarts[
		FMath::RandRange(0, CandidatePlayerStarts.Num() - 1)];
	UsedPlayerStarts.Add(SelectedPlayerStart);
	return SelectedPlayerStart;
}

void ASnowRumbleGameMode::RestartPlayerAtPlayerStart(
	AController* NewPlayer,
	AActor* StartSpot)
{
	if (!NewPlayer || !StartSpot)
	{
		Super::RestartPlayerAtPlayerStart(NewPlayer, StartSpot);
		return;
	}

	const FTransform SpawnTransform =
		BuildScatteredPlayerStartTransform(StartSpot);
	UsedSpawnLocations.Add(SpawnTransform.GetLocation());
	RestartPlayerAtTransform(NewPlayer, SpawnTransform);
	BindPawnLifeState(NewPlayer->GetPawn());
	if (ASnowRumbleCharacter* Character =
		Cast<ASnowRumbleCharacter>(NewPlayer->GetPawn()))
	{
		const USnowRumbleMatchSubsystem* MatchSubsystem = GetMatchSubsystem();
		const ASnowRumblePlayerState* SnowRumblePlayerState =
			NewPlayer->GetPlayerState<ASnowRumblePlayerState>();
		const bool bTiebreakerSpectator =
			MatchSubsystem
			&& MatchSubsystem->IsTiebreakerActive()
			&& (!SnowRumblePlayerState
				|| !MatchSubsystem->IsTiebreakerTeam(
					SnowRumblePlayerState->GetLobbyTeam()));
		Character->SetTiebreakerSpectatorFromServer(bTiebreakerSpectator);
	}
	EvaluateRoundEndCondition();
}

void ASnowRumbleGameMode::EvaluateRoundEndCondition()
{
	ASnowRumbleGameState* SnowRumbleGameState =
		GetGameState<ASnowRumbleGameState>();
	if (!HasAuthority()
		|| !SnowRumbleGameState
		|| SnowRumbleGameState->IsRoundEnded()
		|| SnowRumbleGameState->IsMatchInputLocked())
	{
		return;
	}

	TSet<ESnowRumbleTeam> ParticipatingTeams;
	TSet<ESnowRumbleTeam> AliveTeams;
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		const ASnowRumblePlayerState* SnowRumblePlayerState =
			Cast<ASnowRumblePlayerState>(PlayerState);
		if (!SnowRumblePlayerState
			|| !IsValidRoundTeam(SnowRumblePlayerState->GetLobbyTeam()))
		{
			continue;
		}

		const ESnowRumbleTeam Team = SnowRumblePlayerState->GetLobbyTeam();
		ParticipatingTeams.Add(Team);

		const APawn* Pawn = SnowRumblePlayerState->GetPawn();
		const ASnowRumbleCharacter* Character =
			Cast<ASnowRumbleCharacter>(Pawn);
		if (Character && !Character->IsFrozen() && !Character->IsDead())
		{
			AliveTeams.Add(Team);
		}
	}

	if (ParticipatingTeams.Num() < 2 || AliveTeams.Num() != 1)
	{
		return;
	}

	for (const ESnowRumbleTeam WinningTeam : AliveTeams)
	{
		USnowRumbleMatchSubsystem* MatchSubsystem = GetMatchSubsystem();
		if (MatchSubsystem)
		{
			MatchSubsystem->RecordRoundWin(WinningTeam);
		}

		const bool bStartedTiebreaker =
			MatchSubsystem
			&& MatchSubsystem->StartTiebreakerForLeadingTie();

		SnowRumbleGameState->EndRoundFromServer(
			WinningTeam,
			MatchSubsystem);

		if (bStartedTiebreaker)
		{
			FTimerHandle TiebreakerTravelTimerHandle;
			GetWorldTimerManager().SetTimer(
				TiebreakerTravelTimerHandle,
				this,
				&ASnowRumbleGameMode::TravelToTiebreakerRound,
				NextRoundTravelDelaySeconds,
				false);
		}
		else if (MatchSubsystem && !MatchSubsystem->IsMatchComplete())
		{
			FTimerHandle NextRoundTravelTimerHandle;
			GetWorldTimerManager().SetTimer(
				NextRoundTravelTimerHandle,
				this,
				&ASnowRumbleGameMode::TravelToNextRoundIfNeeded,
				NextRoundTravelDelaySeconds,
				false);
		}
		else if (MatchSubsystem && MatchSubsystem->IsMatchComplete())
		{
			FTimerHandle LobbyReturnTimerHandle;
			GetWorldTimerManager().SetTimer(
				LobbyReturnTimerHandle,
				this,
				&ASnowRumbleGameMode::ReturnToLobbyAfterMatchEnd,
				MatchEndLobbyReturnDelaySeconds,
				false);
		}
		return;
	}
}

void ASnowRumbleGameMode::TravelToNextRoundIfNeeded()
{
	USnowRumbleMatchSubsystem* MatchSubsystem = GetMatchSubsystem();
	if (!HasAuthority()
		|| !MatchSubsystem
		|| MatchSubsystem->IsMatchComplete())
	{
		return;
	}

	MatchSubsystem->AdvanceToNextRound();
	FString NextLevelPath = MatchSubsystem->SelectNextPvPLevelPath(FString());
	if (NextLevelPath.IsEmpty())
	{
		return;
	}

	NextLevelPath = BuildPvPTravelUrl(NextLevelPath);

	if (UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It =
				World->GetPlayerControllerIterator();
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

		World->ServerTravel(NextLevelPath);
	}
}

void ASnowRumbleGameMode::TravelToTiebreakerRound()
{
	USnowRumbleMatchSubsystem* MatchSubsystem = GetMatchSubsystem();
	if (!HasAuthority()
		|| !MatchSubsystem
		|| !MatchSubsystem->IsTiebreakerActive()
		|| TiebreakerTravelUrl.IsEmpty())
	{
		return;
	}

	const FString TravelUrl = BuildPvPTravelUrl(TiebreakerTravelUrl);

	if (UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It =
				World->GetPlayerControllerIterator();
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

		World->ServerTravel(TravelUrl);
	}
}

void ASnowRumbleGameMode::ScheduleNextMapShrink()
{
	ASnowRumbleGameState* SnowRumbleGameState =
		GetGameState<ASnowRumbleGameState>();
	if (!HasAuthority()
		|| !SnowRumbleGameState
		|| SnowRumbleGameState->IsRoundEnded())
	{
		return;
	}
	if (SnowRumbleGameState->IsTiebreakerRound())
	{
		return;
	}

	const float DelaySeconds =
		MatchStartCountdownSeconds
		+ SnowRumbleGameState->GetMapShrinkIntervalSeconds();
	GetWorldTimerManager().SetTimer(
		MapShrinkTimerHandle,
		this,
		&ASnowRumbleGameMode::TriggerMapShrink,
		DelaySeconds,
		false);
}

void ASnowRumbleGameMode::ScheduleNextGiftBoxSpawn(float DelaySeconds)
{
	ASnowRumbleGameState* SnowRumbleGameState =
		GetGameState<ASnowRumbleGameState>();
	if (!HasAuthority()
		|| !SnowRumbleGameState
		|| SnowRumbleGameState->IsRoundEnded())
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		GiftBoxSpawnTimerHandle,
		this,
		&ASnowRumbleGameMode::SpawnGiftBox,
		FMath::Max(0.0f, DelaySeconds),
		false);
}

void ASnowRumbleGameMode::TriggerMapShrink()
{
	ASnowRumbleGameState* SnowRumbleGameState =
		GetGameState<ASnowRumbleGameState>();
	if (!HasAuthority()
		|| !SnowRumbleGameState
		|| SnowRumbleGameState->IsRoundEnded()
		|| SnowRumbleGameState->IsTiebreakerRound()
		|| SnowRumbleGameState->IsMapShrinkInProgress())
	{
		return;
	}

	++MapShrinkStage;
	SnowRumbleGameState->StartMapShrinkFromServer(
		TemporaryMapShrinkDurationSeconds);
	OnMapShrinkRequested(
		MapShrinkStage,
		SnowRumbleGameState->GetRoundElapsedSeconds(),
		TemporaryMapShrinkDurationSeconds);
	for (TActorIterator<ASnowIslandWaterPressureActor> It(GetWorld()); It; ++It)
	{
		if (ASnowIslandWaterPressureActor* WaterPressureActor = *It)
		{
			WaterPressureActor->StartWaterPressureFromMapShrink(
				MapShrinkStage,
				SnowRumbleGameState->GetRoundElapsedSeconds(),
				TemporaryMapShrinkDurationSeconds);
		}
	}

	GetWorldTimerManager().SetTimer(
		MapShrinkCompletionTimerHandle,
		this,
		&ASnowRumbleGameMode::CompleteMapShrinkFromServer,
		TemporaryMapShrinkDurationSeconds,
		false);
}

void ASnowRumbleGameMode::SpawnGiftBox()
{
	ASnowRumbleGameState* SnowRumbleGameState =
		GetGameState<ASnowRumbleGameState>();
	UWorld* World = GetWorld();
	const TSubclassOf<AGiftBox> SpawnGiftBoxClass = ResolveGiftBoxClass();
	if (!HasAuthority()
		|| !World
		|| !SnowRumbleGameState
		|| SnowRumbleGameState->IsRoundEnded()
		|| !SpawnGiftBoxClass)
	{
		if (!SpawnGiftBoxClass)
		{
			BroadcastPersonalTextAlarm(NSLOCTEXT(
				"SnowRumble",
				"GiftBoxClassMissing",
				"선물상자 클래스가 설정되지 않았습니다"));
		}
		return;
	}

	TArray<AActor*> SpawnPointCandidates;
	GetGiftBoxSpawnPointCandidates(SpawnPointCandidates);
	if (SpawnPointCandidates.IsEmpty())
	{
		BroadcastPersonalTextAlarm(NSLOCTEXT(
			"SnowRumble",
			"GiftBoxTargetPointMissing",
			"선물상자 TargetPoint가 없습니다"));
		return;
	}

	AActor* SelectedSpawnPoint =
		SpawnPointCandidates[
			FMath::RandRange(0, SpawnPointCandidates.Num() - 1)];
	if (!SelectedSpawnPoint)
	{
		return;
	}

	const FVector SpawnLocation =
		SelectedSpawnPoint->GetActorLocation()
		+ FVector::UpVector * GiftBoxSpawnHeightOffset;
	const FRotator SpawnRotation = SelectedSpawnPoint->GetActorRotation();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AGiftBox* GiftBox = World->SpawnActor<AGiftBox>(
		SpawnGiftBoxClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParameters);
	if (GiftBox)
	{
		GiftBox->InitializeGiftBoxFromServer(ChooseGiftBoxGrade());
		BroadcastPersonalTextAlarm(NSLOCTEXT(
			"SnowRumble",
			"GiftBoxSantaDroppedGift",
			"산타가 선물을 흘렸다네"));
		BroadcastEventLogMessage(NSLOCTEXT(
			"SnowRumble",
			"GiftBoxSantaDroppedGiftLog",
			"산타가 선물을 흘렸다네"));
	}

	ScheduleNextGiftBoxSpawn(GiftBoxSpawnIntervalSeconds);
}

void ASnowRumbleGameMode::CompleteMapShrinkFromBlueprint()
{
	CompleteMapShrinkFromServer();
}

void ASnowRumbleGameMode::CompleteMapShrinkFromServer()
{
	ASnowRumbleGameState* SnowRumbleGameState =
		GetGameState<ASnowRumbleGameState>();
	if (!HasAuthority()
		|| !SnowRumbleGameState
		|| SnowRumbleGameState->IsRoundEnded()
		|| !SnowRumbleGameState->IsMapShrinkInProgress())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(MapShrinkCompletionTimerHandle);
	SnowRumbleGameState->CompleteMapShrinkFromServer();

	GetWorldTimerManager().SetTimer(
		MapShrinkTimerHandle,
		this,
		&ASnowRumbleGameMode::TriggerMapShrink,
		SnowRumbleGameState->GetMapShrinkIntervalSeconds(),
		false);
}

void ASnowRumbleGameMode::ReturnToLobbyAfterMatchEnd()
{
	USnowRumbleMatchSubsystem* MatchSubsystem = GetMatchSubsystem();
	if (!HasAuthority()
		|| !MatchSubsystem
		|| !MatchSubsystem->IsMatchComplete()
		|| (LobbyReturnTravelUrl.IsEmpty() && PodiumTravelUrl.IsEmpty()))
	{
		return;
	}

	// If a PodiumTravelUrl is set, prefer traveling to podium first.
	if (!PodiumTravelUrl.IsEmpty())
	{
		FTimerHandle PodiumDelayHandle;
		GetWorldTimerManager().SetTimer(
			PodiumDelayHandle,
			[this]() { TravelToPodiumAfterMatchEnd(); },
			PodiumTravelDelaySeconds,
			false);
		return;
	}

	FString TravelUrl = LobbyReturnTravelUrl;
	EnsureTravelOption(TravelUrl, TEXT("?listen"));
	EnsureTravelOptionValue(TravelUrl, TEXT("game"), LobbyGameModeTravelPath);

	MatchSubsystem->ResetPvPMatch();
	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(TravelUrl);
	}
}

void ASnowRumbleGameMode::TravelToPodiumAfterMatchEnd()
{
	USnowRumbleMatchSubsystem* MatchSubsystem = GetMatchSubsystem();
	if (!HasAuthority()
		|| !MatchSubsystem
		|| !MatchSubsystem->IsMatchComplete()
		|| PodiumTravelUrl.IsEmpty())
	{
		if (!LobbyReturnTravelUrl.IsEmpty())
		{
			FString TravelUrl = LobbyReturnTravelUrl;
			EnsureTravelOption(TravelUrl, TEXT("?listen"));
			EnsureTravelOptionValue(
				TravelUrl,
				TEXT("game"),
				LobbyGameModeTravelPath);
			MatchSubsystem->ResetPvPMatch();
			if (UWorld* World = GetWorld())
			{
				World->ServerTravel(TravelUrl);
			}
		}
		return;
	}

	FString TravelUrl = PodiumTravelUrl;
	EnsureTravelOption(TravelUrl, TEXT("?listen"));
	EnsureTravelOptionValue(TravelUrl, TEXT("game"), PodiumGameModeTravelPath);
	if (!TravelUrl.Contains(TEXT("ExpectedPlayers="), ESearchCase::IgnoreCase))
	{
		EnsureTravelOptionValue(
			TravelUrl,
			TEXT("ExpectedPlayers"),
			FString::FromInt(ExpectedPlayerCount));
	}

	// Keep match state until podium placement completes.
	if (UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (ASnowRumblePlayerController* PlayerController = Cast<ASnowRumblePlayerController>(It->Get()))
			{
				PlayerController->ClientShowLoadingScreen();
				PlayerController->ClientUpdateLoadingProgress(0, ExpectedPlayerCount);
			}
		}

		World->ServerTravel(TravelUrl);
	}
}

void ASnowRumbleGameMode::HandlePlayerLifeStateChanged(bool bUnused)
{
	RefreshPlayerLifeEventLogStates();
	EvaluateRoundEndCondition();
}

void ASnowRumbleGameMode::BindPawnLifeState(APawn* Pawn)
{
	ASnowRumbleCharacter* Character = Cast<ASnowRumbleCharacter>(Pawn);
	if (!Character)
	{
		return;
	}

	if (USnowRumbleHealthComponent* HealthComponent =
		Character->FindComponentByClass<USnowRumbleHealthComponent>())
	{
		HealthComponent->OnFrozenChanged.AddUniqueDynamic(
			this,
			&ASnowRumbleGameMode::HandlePlayerLifeStateChanged);
		HealthComponent->OnDeathChanged.AddUniqueDynamic(
			this,
			&ASnowRumbleGameMode::HandlePlayerLifeStateChanged);

		FTrackedLifeState& TrackedLifeState =
			TrackedLifeStates.FindOrAdd(Character);
		TrackedLifeState.bFrozen = HealthComponent->IsFrozen();
		TrackedLifeState.bDead = HealthComponent->IsDead();
	}
}

void ASnowRumbleGameMode::RefreshPlayerLifeEventLogStates()
{
	UWorld* World = GetWorld();
	if (!HasAuthority() || !World)
	{
		return;
	}

	TSet<TWeakObjectPtr<ASnowRumbleCharacter>> ValidCharacters;
	for (TActorIterator<ASnowRumbleCharacter> It(World); It; ++It)
	{
		ASnowRumbleCharacter* Character = *It;
		if (!Character)
		{
			continue;
		}

		ValidCharacters.Add(Character);
		FTrackedLifeState& TrackedLifeState =
			TrackedLifeStates.FindOrAdd(Character);
		const bool bWasFrozen = TrackedLifeState.bFrozen;
		const bool bWasDead = TrackedLifeState.bDead;
		const bool bIsFrozen = Character->IsFrozen();
		const bool bIsDead = Character->IsDead();

		if (!bWasFrozen && bIsFrozen)
		{
			BroadcastEventLogMessage(FText::Format(
				NSLOCTEXT(
					"SnowRumble",
					"EventLogPlayerFrozen",
					"{0}님이 얼었습니다"),
				FText::FromString(GetEventLogPlayerName(Character))));
		}
		if (!bWasDead && bIsDead)
		{
			BroadcastEventLogMessage(FText::Format(
				NSLOCTEXT(
					"SnowRumble",
					"EventLogPlayerDead",
					"{0}님이 죽었습니다"),
				FText::FromString(GetEventLogPlayerName(Character))));
		}

		TrackedLifeState.bFrozen = bIsFrozen;
		TrackedLifeState.bDead = bIsDead;
	}

	for (auto It = TrackedLifeStates.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid() || !ValidCharacters.Contains(It.Key()))
		{
			It.RemoveCurrent();
		}
	}
}

void ASnowRumbleGameMode::BroadcastEventLogMessage(const FText& Message) const
{
	UWorld* World = GetWorld();
	if (!World || Message.IsEmpty())
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
			PlayerController->ClientReceiveEventLogMessage(Message);
		}
	}
}

void ASnowRumbleGameMode::BroadcastPersonalTextAlarm(
	const FText& Message) const
{
	UWorld* World = GetWorld();
	if (!World || Message.IsEmpty())
	{
		return;
	}

	for (FConstPlayerControllerIterator It =
			World->GetPlayerControllerIterator();
		It;
		++It)
	{
		if (ASnowRumblePlayerController* PlayerController =
			Cast<ASnowRumblePlayerController>(It->Get()))
		{
			PlayerController->ClientShowPersonalTextAlarm(Message);
		}
	}
}

FString ASnowRumbleGameMode::GetEventLogPlayerName(
	const ASnowRumbleCharacter* Character) const
{
	const ASnowRumblePlayerState* SnowRumblePlayerState = Character
		? Character->GetPlayerState<ASnowRumblePlayerState>()
		: nullptr;
	return SnowRumblePlayerState
		? SnowRumblePlayerState->GetLobbyPlayerName()
		: TEXT("Player");
}

void ASnowRumbleGameMode::GetGiftBoxSpawnPointCandidates(
	TArray<AActor*>& OutCandidates) const
{
	OutCandidates.Reset();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<AActor*> FallbackCandidates;
	for (TActorIterator<ATargetPoint> It(World); It; ++It)
	{
		ATargetPoint* TargetPoint = *It;
		if (!TargetPoint)
		{
			continue;
		}

		if (!GiftBoxSpawnPointTag.IsNone()
			&& TargetPoint->ActorHasTag(GiftBoxSpawnPointTag))
		{
			OutCandidates.Add(TargetPoint);
			continue;
		}

		FallbackCandidates.Add(TargetPoint);
	}

	if (OutCandidates.IsEmpty())
	{
		OutCandidates = MoveTemp(FallbackCandidates);
	}
}

TSubclassOf<AGiftBox> ASnowRumbleGameMode::ResolveGiftBoxClass() const
{
	if (GiftBoxClass)
	{
		return GiftBoxClass;
	}

	if (!DefaultGiftBoxClassPath.IsValid())
	{
		return nullptr;
	}

	UClass* LoadedClass = DefaultGiftBoxClassPath.TryLoadClass<AGiftBox>();
	return LoadedClass ? TSubclassOf<AGiftBox>(LoadedClass) : nullptr;
}

ESnowRumbleGiftBoxGrade ASnowRumbleGameMode::ChooseGiftBoxGrade() const
{
	return FMath::FRand() <= GoldGiftBoxSpawnChance
		? ESnowRumbleGiftBoxGrade::Gold
		: ESnowRumbleGiftBoxGrade::Red;
}

bool ASnowRumbleGameMode::IsValidRoundTeam(ESnowRumbleTeam Team) const
{
	if (const USnowRumbleMatchSubsystem* MatchSubsystem = GetMatchSubsystem())
	{
		if (MatchSubsystem->IsTiebreakerActive())
		{
			return MatchSubsystem->IsTiebreakerTeam(Team);
		}
	}

	switch (Team)
	{
	case ESnowRumbleTeam::Red:
	case ESnowRumbleTeam::Sky:
	case ESnowRumbleTeam::Green:
	case ESnowRumbleTeam::Yellow:
	case ESnowRumbleTeam::Purple:
	case ESnowRumbleTeam::Pink:
	case ESnowRumbleTeam::Blue:
	case ESnowRumbleTeam::White:
		return true;
	default:
		return false;
	}
}

void ASnowRumbleGameMode::GetActiveRoundTeams(
	TArray<ESnowRumbleTeam>& OutTeams) const
{
	OutTeams.Reset();
	if (!GameState)
	{
		return;
	}

	TSet<ESnowRumbleTeam> PresentTeams;
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		const ASnowRumblePlayerState* SnowRumblePlayerState =
			Cast<ASnowRumblePlayerState>(PlayerState);
		if (!SnowRumblePlayerState)
		{
			continue;
		}

		const ESnowRumbleTeam Team = SnowRumblePlayerState->GetLobbyTeam();
		if (IsValidRoundTeam(Team))
		{
			PresentTeams.Add(Team);
		}
	}

	const ESnowRumbleTeam TeamOrder[] =
	{
		ESnowRumbleTeam::Red,
		ESnowRumbleTeam::Blue,
		ESnowRumbleTeam::Sky,
		ESnowRumbleTeam::Green,
		ESnowRumbleTeam::Yellow,
		ESnowRumbleTeam::Purple,
		ESnowRumbleTeam::Pink,
		ESnowRumbleTeam::White
	};
	for (const ESnowRumbleTeam Team : TeamOrder)
	{
		if (PresentTeams.Contains(Team))
		{
			OutTeams.Add(Team);
		}
	}
}

FString ASnowRumbleGameMode::BuildPvPTravelUrl(
	const FString& BaseTravelUrl) const
{
	FString TravelUrl = BaseTravelUrl;
	EnsureTravelOption(TravelUrl, TEXT("?listen"));
	EnsureTravelOptionValue(TravelUrl, TEXT("game"), PvpGameModeTravelPath);
	if (ExpectedPlayerCount > 0)
	{
		EnsureTravelOptionValue(
			TravelUrl,
			TEXT("ExpectedPlayers"),
			FString::FromInt(ExpectedPlayerCount));
	}
	return TravelUrl;
}

void ASnowRumbleGameMode::BroadcastLoadingProgress()
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

USnowRumbleMatchSubsystem* ASnowRumbleGameMode::GetMatchSubsystem() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance
		? GameInstance->GetSubsystem<USnowRumbleMatchSubsystem>()
		: nullptr;
}

FTransform ASnowRumbleGameMode::BuildScatteredPlayerStartTransform(
	const AActor* StartSpot) const
{
	const FVector StartLocation = StartSpot->GetActorLocation();
	const FRotator StartRotation = StartSpot->GetActorRotation();

	FVector ResolvedStartLocation = StartLocation;
	if (!TryResolveSpawnLocationOnGround(StartLocation, ResolvedStartLocation)
		|| !IsSpawnCapsuleClear(ResolvedStartLocation))
	{
		ResolvedStartLocation = StartLocation;
	}

	if (PlayerStartSpawnScatterRadius <= 0.0f)
	{
		return FTransform(StartRotation, ResolvedStartLocation);
	}

	const int32 Attempts = FMath::Max(1, PlayerStartSpawnScatterAttempts);
	for (int32 AttemptIndex = 0; AttemptIndex < Attempts; ++AttemptIndex)
	{
		const FVector RawCandidateLocation =
			StartLocation
			+ MakeRandomHorizontalOffset(PlayerStartSpawnScatterRadius);
		FVector CandidateLocation = RawCandidateLocation;
		if (TryResolveSpawnLocationOnGround(
				RawCandidateLocation,
				CandidateLocation)
			&& IsSpawnLocationFarEnough(CandidateLocation)
			&& IsSpawnCapsuleClear(CandidateLocation))
		{
			return FTransform(StartRotation, CandidateLocation);
		}
	}

	return FTransform(StartRotation, ResolvedStartLocation);
}

bool ASnowRumbleGameMode::TryResolveSpawnLocationOnGround(
	const FVector& CandidateLocation,
	FVector& OutSpawnLocation) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	float CapsuleRadius = 42.0f;
	float CapsuleHalfHeight = 96.0f;
	GetDefaultPawnCapsuleSize(CapsuleRadius, CapsuleHalfHeight);

	const FVector TraceStart =
		CandidateLocation + FVector::UpVector * 1000.0f;
	const FVector TraceEnd =
		CandidateLocation - FVector::UpVector * 3000.0f;

	FHitResult GroundHit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SnowRumbleSpawnGround), false);
	if (!World->LineTraceSingleByChannel(
			GroundHit,
			TraceStart,
			TraceEnd,
			ECC_WorldStatic,
			QueryParams)
		|| !GroundHit.bBlockingHit)
	{
		return false;
	}

	OutSpawnLocation =
		GroundHit.ImpactPoint
		+ FVector::UpVector * (CapsuleHalfHeight + 3.0f);
	return true;
}

bool ASnowRumbleGameMode::IsSpawnCapsuleClear(
	const FVector& SpawnLocation) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	float CapsuleRadius = 42.0f;
	float CapsuleHalfHeight = 96.0f;
	GetDefaultPawnCapsuleSize(CapsuleRadius, CapsuleHalfHeight);

	const FCollisionShape CapsuleShape =
		FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SnowRumbleSpawnCapsule), false);
	return !World->OverlapBlockingTestByChannel(
		SpawnLocation,
		FQuat::Identity,
		ECC_Pawn,
		CapsuleShape,
		QueryParams);
}

void ASnowRumbleGameMode::GetDefaultPawnCapsuleSize(
	float& OutCapsuleRadius,
	float& OutCapsuleHalfHeight) const
{
	OutCapsuleRadius = 42.0f;
	OutCapsuleHalfHeight = 96.0f;

	const ACharacter* DefaultCharacter =
		DefaultPawnClass
			? Cast<ACharacter>(DefaultPawnClass->GetDefaultObject())
			: nullptr;
	const UCapsuleComponent* Capsule =
		DefaultCharacter ? DefaultCharacter->GetCapsuleComponent() : nullptr;
	if (!Capsule)
	{
		return;
	}

	OutCapsuleRadius = Capsule->GetScaledCapsuleRadius();
	OutCapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
}

bool ASnowRumbleGameMode::IsSpawnLocationFarEnough(
	const FVector& CandidateLocation) const
{
	const float MinimumSpacingSquared =
		FMath::Square(PlayerStartSpawnMinimumSpacing);
	for (const FVector& UsedSpawnLocation : UsedSpawnLocations)
	{
		if (FVector::DistSquared2D(CandidateLocation, UsedSpawnLocation)
			< MinimumSpacingSquared)
		{
			return false;
		}
	}

	return true;
}

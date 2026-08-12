// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleGameMode.h"

#include "Engine/GameInstance.h"
#include "../Item/GiftBox_C.h"
#include "../Player/SnowRumbleCharacter.h"
#include "../Player/SnowRumbleHealthComponent.h"
#include "../UI/SnowRumblePlayerController.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/TargetPoint.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "SnowRumbleGameState_C.h"
#include "SnowRumbleMatchSubsystem_C.h"
#include "SnowRumblePlayerState.h"

namespace
{
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
	bStartCountdownStarted = false;
	MapShrinkStage = 0;
	GetWorldTimerManager().ClearTimer(MapShrinkTimerHandle);
	GetWorldTimerManager().ClearTimer(MapShrinkCompletionTimerHandle);
	GetWorldTimerManager().ClearTimer(GiftBoxSpawnTimerHandle);
}

void ASnowRumbleGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

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
				if (ASnowRumbleGameState* SnowRumbleGameState =
					GetGameState<ASnowRumbleGameState>())
				{
					SnowRumbleGameState->StartMatchCountdownFromServer(
						MatchStartCountdownSeconds);
					ScheduleNextMapShrink();
					ScheduleNextGiftBoxSpawn(
						MatchStartCountdownSeconds
						+ FirstGiftBoxSpawnDelaySeconds);
				}
			},
			MatchStartCountdownDelaySeconds,
			false);
		return;
	}

	if (ASnowRumbleGameState* SnowRumbleGameState =
		GetGameState<ASnowRumbleGameState>())
	{
		SnowRumbleGameState->StartMatchCountdownFromServer(
			MatchStartCountdownSeconds);
		ScheduleNextMapShrink();
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

		SnowRumbleGameState->EndRoundFromServer(
			WinningTeam,
			MatchSubsystem);

		if (MatchSubsystem && !MatchSubsystem->IsMatchComplete())
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

	if (!NextLevelPath.Contains(TEXT("?listen"), ESearchCase::IgnoreCase))
	{
		NextLevelPath += TEXT("?listen");
	}
	if (ExpectedPlayerCount > 0)
	{
		NextLevelPath += FString::Printf(
			TEXT("?ExpectedPlayers=%d"),
			ExpectedPlayerCount);
	}

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
		|| LobbyReturnTravelUrl.IsEmpty())
	{
		return;
	}

	FString TravelUrl = LobbyReturnTravelUrl;
	if (!TravelUrl.Contains(TEXT("?listen"), ESearchCase::IgnoreCase))
	{
		TravelUrl += TEXT("?listen");
	}

	MatchSubsystem->ResetPvPMatch();
	if (UWorld* World = GetWorld())
	{
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

	FVector BestCandidateLocation = StartLocation;
	float BestCandidateDistanceSquared = -1.0f;

	const int32 Attempts = FMath::Max(1, PlayerStartSpawnScatterAttempts);
	for (int32 AttemptIndex = 0; AttemptIndex < Attempts; ++AttemptIndex)
	{
		const FVector CandidateLocation =
			StartLocation
			+ MakeRandomHorizontalOffset(PlayerStartSpawnScatterRadius);
		if (IsSpawnLocationFarEnough(CandidateLocation))
		{
			return FTransform(StartRotation, CandidateLocation);
		}

		float ClosestUsedDistanceSquared = TNumericLimits<float>::Max();
		for (const FVector& UsedSpawnLocation : UsedSpawnLocations)
		{
			const float DistanceSquared =
				FVector::DistSquared2D(CandidateLocation, UsedSpawnLocation);
			ClosestUsedDistanceSquared =
				FMath::Min(ClosestUsedDistanceSquared, DistanceSquared);
		}

		if (ClosestUsedDistanceSquared > BestCandidateDistanceSquared)
		{
			BestCandidateDistanceSquared = ClosestUsedDistanceSquared;
			BestCandidateLocation = CandidateLocation;
		}
	}

	return FTransform(StartRotation, BestCandidateLocation);
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

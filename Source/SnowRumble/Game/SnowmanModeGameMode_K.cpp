// Copyright Epic Games, Inc. All Rights Reserved.

#include "TimerManager.h"
#include "Engine/World.h"
#include "SnowmanModeGameMode_K.h"

#include "../Audio/SnowRumbleAudioHelpers.h"
#include "../Player/SnowRumbleCharacter.h"
#include "../Player/SnowmanModeSnowmanCharacter_K.h"
#include "../UI/SnowRumblePlayerController.h"
#include "EngineUtils.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "SnowmanModeGameState_K.h"
#include "SnowRumbleLobbyGameMode.h"
#include "SnowRumblePlayerState.h"
#include "Sound/SoundBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogSnowmanMode, Log, All);

namespace
{
constexpr const TCHAR* SnowmanModePodiumGameModeTravelPath =
	TEXT("/Game/Game/BP_SnowRumblePodium_Snowman_GameMode_K.BP_SnowRumblePodium_Snowman_GameMode_K_C");

FVector MakeSnowmanModeRandomHorizontalOffset(float Radius)
{
	if (Radius <= 0.0f)
	{
		return FVector::ZeroVector;
	}

	const float AngleRadians = FMath::FRandRange(0.0f, UE_TWO_PI);
	const float Distance = FMath::Sqrt(FMath::FRand()) * Radius;
	return FVector(
		FMath::Cos(AngleRadians) * Distance,
		FMath::Sin(AngleRadians) * Distance,
		0.0f);
}

const TCHAR* LexToString(ESnowmanModePlayerRole Role)
{
	switch (Role)
	{
	case ESnowmanModePlayerRole::Normal:
		return TEXT("Normal");
	case ESnowmanModePlayerRole::InfectionPending:
		return TEXT("InfectionPending");
	case ESnowmanModePlayerRole::Snowman:
		return TEXT("Snowman");
	default:
		return TEXT("Unknown");
	}
}

const TCHAR* LexToString(ESnowmanModeResult Result)
{
	switch (Result)
	{
	case ESnowmanModeResult::SnowmanVictory:
		return TEXT("SnowmanVictory");
	case ESnowmanModeResult::SurvivorVictory:
		return TEXT("SurvivorVictory");
	case ESnowmanModeResult::None:
	default:
		return TEXT("None");
	}
}

void EnsureSnowmanModeTravelOption(FString& TravelUrl, const TCHAR* Option)
{
	if (!TravelUrl.Contains(Option, ESearchCase::IgnoreCase))
	{
		TravelUrl += Option;
	}
}

void EnsureSnowmanModeTravelOptionValue(
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

FString DescribeSnowmanPlayerState(const ASnowRumblePlayerState* PlayerState)
{
	if (!PlayerState)
	{
		return TEXT("None");
	}

	return FString::Printf(
		TEXT("%s Id=%d"),
		*PlayerState->GetPlayerName(),
		PlayerState->GetPlayerId());
}

bool DoesSnowmanPlayerStateMatch(
	const APlayerState* CandidatePlayerState,
	const ASnowRumblePlayerState* TargetPlayerState)
{
	if (!CandidatePlayerState || !TargetPlayerState)
	{
		return false;
	}

	if (CandidatePlayerState == TargetPlayerState)
	{
		return true;
	}

	const int32 TargetPlayerId = TargetPlayerState->GetPlayerId();
	if (TargetPlayerId != INDEX_NONE
		&& CandidatePlayerState->GetPlayerId() == TargetPlayerId)
	{
		return true;
	}

	const FUniqueNetIdRepl& TargetUniqueId = TargetPlayerState->GetUniqueId();
	return TargetUniqueId.IsValid()
		&& CandidatePlayerState->GetUniqueId().IsValid()
		&& CandidatePlayerState->GetUniqueId() == TargetUniqueId;
}

APlayerController* ResolveSnowmanPlayerController(
	UWorld* World,
	const ASnowRumblePlayerState* PlayerState)
{
	if (!World || !PlayerState)
	{
		return nullptr;
	}

	if (APlayerController* OwnerController =
		Cast<APlayerController>(PlayerState->GetOwner()))
	{
		if (DoesSnowmanPlayerStateMatch(
				OwnerController->PlayerState,
				PlayerState))
		{
			return OwnerController;
		}
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator();
		It;
		++It)
	{
		APlayerController* CandidateController = It->Get();
		if (!CandidateController)
		{
			continue;
		}

		if (DoesSnowmanPlayerStateMatch(
				CandidateController->PlayerState,
				PlayerState))
		{
			return CandidateController;
		}

		const APawn* CandidatePawn = CandidateController->GetPawn();
		if (DoesSnowmanPlayerStateMatch(
				CandidatePawn
					? CandidatePawn->GetPlayerState<APlayerState>()
					: nullptr,
				PlayerState))
		{
			return CandidateController;
		}
	}

	for (TActorIterator<ASnowRumbleCharacter> It(World); It; ++It)
	{
		const ASnowRumbleCharacter* Character = *It;
		if (!Character)
		{
			continue;
		}

		if (!DoesSnowmanPlayerStateMatch(
				Character->GetPlayerState<APlayerState>(),
				PlayerState))
		{
			continue;
		}

		if (APlayerController* CharacterController =
			Cast<APlayerController>(Character->GetController()))
		{
			return CharacterController;
		}
	}

	return nullptr;
}

}

ASnowmanModeGameMode::ASnowmanModeGameMode()
{
	PlayerControllerClass = ASnowRumblePlayerController::StaticClass();
	GameStateClass = ASnowmanModeGameState::StaticClass();
	PlayerStateClass = ASnowRumblePlayerState::StaticClass();
	DefaultPawnClass = ASnowRumbleCharacter::StaticClass();
	SnowmanCharacterClass = ASnowmanModeSnowmanCharacter::StaticClass();
	LobbyReturnGameModeClass = ASnowRumbleLobbyGameMode::StaticClass();
	bUseSeamlessTravel = true;

#if WITH_EDITOR
	if (IConsoleVariable* AllowPieSeamlessTravel =
		IConsoleManager::Get().FindConsoleVariable(TEXT("net.AllowPIESeamlessTravel")))
	{
		AllowPieSeamlessTravel->Set(1);
	}
#endif
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
	bSnowmanRolesInitialized = false;
	UsedPlayerStarts.Reset();
	UsedSpawnLocations.Reset();
	SpawnInfectionGraceEndTimes.Reset();
	ConvertingSnowmanPlayerStates.Reset();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(
			SnowmanRoleInitializationRetryTimerHandle);
		World->GetTimerManager().ClearTimer(InfectionScanTimerHandle);
		World->GetTimerManager().ClearTimer(SnowmanModeTimeLimitTimerHandle);
		World->GetTimerManager().ClearTimer(SnowmanModeLobbyReturnTimerHandle);
	}

	BroadcastBackgroundMusic();
}

void ASnowmanModeGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	BroadcastLoadingProgress();
	TryDismissLoadingScreens();
	BroadcastBackgroundMusic();
}

void ASnowmanModeGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);

	BroadcastLoadingProgress();
	TryDismissLoadingScreens();
	BroadcastBackgroundMusic();
}

void ASnowmanModeGameMode::HandleStartingNewPlayer_Implementation(
	APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	BroadcastLoadingProgress();
	TryDismissLoadingScreens();
	BroadcastBackgroundMusic();
}

void ASnowmanModeGameMode::BroadcastBackgroundMusic() const
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

AActor* ASnowmanModeGameMode::ChoosePlayerStart_Implementation(
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

void ASnowmanModeGameMode::RestartPlayerAtPlayerStart(
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
	if (ASnowRumblePlayerState* PlayerState =
		NewPlayer->GetPlayerState<ASnowRumblePlayerState>())
	{
		GrantSpawnInfectionGrace(PlayerState);
	}
}

UClass* ASnowmanModeGameMode::GetDefaultPawnClassForController_Implementation(
	AController* InController)
{
	const ASnowmanModeGameState* SnowmanGameState =
		GetGameState<ASnowmanModeGameState>();
	const APlayerState* PlayerState =
		InController ? InController->PlayerState : nullptr;
	if (SnowmanGameState
		&& PlayerState
		&& SnowmanGameState->IsSnowmanModePlayerSnowman(PlayerState))
	{
		if (SnowmanCharacterClass)
		{
			return SnowmanCharacterClass;
		}

		return ASnowmanModeSnowmanCharacter::StaticClass();
	}

	return Super::GetDefaultPawnClassForController_Implementation(
		InController);
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
			&ASnowmanModeGameMode::StartSnowmanModeCountdownAfterLoading);
	}
}

void ASnowmanModeGameMode::StartSnowmanModeCountdownAfterLoading()
{
	if (ASnowmanModeGameState* SnowmanGameState =
		GetGameState<ASnowmanModeGameState>())
	{
		SnowmanGameState->StartSnowmanModeCountdownFromServer(
			SnowmanModeStartCountdownSeconds);
	}
	InitializeSnowmanRoles();
	ApplySnowmanModeStartInputLock(SnowmanModeStartCountdownSeconds > 0.0f);

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (SnowmanModeStartCountdownSeconds > 0.0f)
	{
		FTimerHandle StartDelayTimerHandle;
		World->GetTimerManager().SetTimer(
			StartDelayTimerHandle,
			this,
			&ASnowmanModeGameMode::StartSnowmanModeAfterCountdown,
			SnowmanModeStartCountdownSeconds,
			false);
		return;
	}

	StartSnowmanModeAfterCountdown();
}

void ASnowmanModeGameMode::StartSnowmanModeAfterCountdown()
{
	ApplySnowmanModeStartInputLock(false);
	GrantSpawnInfectionGraceToAllPlayers();

	if (ASnowmanModeGameState* SnowmanGameState =
		GetGameState<ASnowmanModeGameState>())
	{
		SnowmanGameState->StartSnowmanModeTimerFromServer(
			SnowmanModeTimeLimitSeconds);
	}

	UWorld* World = GetWorld();
	if (World && InfectionScanIntervalSeconds > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			InfectionScanTimerHandle,
			this,
			&ASnowmanModeGameMode::UpdateSnowmanInfectionFlow,
			InfectionScanIntervalSeconds,
			true);
	}
	if (World && SnowmanModeTimeLimitSeconds > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			SnowmanModeTimeLimitTimerHandle,
			this,
			&ASnowmanModeGameMode::HandleSnowmanModeTimeLimitExpired,
			SnowmanModeTimeLimitSeconds,
			false);
	}
}

void ASnowmanModeGameMode::InitializeSnowmanRoles()
{
	if (bSnowmanRolesInitialized)
	{
		ApplySnowmanMovementSpeeds();
		return;
	}

	ASnowmanModeGameState* SnowmanGameState =
		GetGameState<ASnowmanModeGameState>();
	if (!SnowmanGameState)
	{
		ScheduleSnowmanRoleInitializationRetry();
		return;
	}

	TArray<ASnowRumblePlayerState*> PlayerStates =
		CollectSnowmanPlayerStates();
	const int32 RequiredPlayerCount =
		ExpectedPlayerCount > 0 ? ExpectedPlayerCount : GetNumPlayers();
	if (PlayerStates.Num() < FMath::Max(1, RequiredPlayerCount))
	{
		ScheduleSnowmanRoleInitializationRetry();
		return;
	}

	SnowmanGameState->ResetSnowmanModePlayersFromServer(PlayerStates);

	const int32 InitialSnowmanIndex =
		FMath::RandRange(0, PlayerStates.Num() - 1);
	ASnowRumblePlayerState* InitialSnowmanPlayerState =
		PlayerStates[InitialSnowmanIndex];
	SnowmanGameState->SetSnowmanPlayerFromServer(InitialSnowmanPlayerState);
	if (!ConvertPlayerToSnowmanPawn(InitialSnowmanPlayerState))
	{
		ScheduleSnowmanRoleInitializationRetry();
		return;
	}

	bSnowmanRolesInitialized = true;
	ApplySnowmanMovementSpeeds();
}

void ASnowmanModeGameMode::ScheduleSnowmanRoleInitializationRetry()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		SnowmanRoleInitializationRetryTimerHandle,
		this,
		&ASnowmanModeGameMode::InitializeSnowmanRoles,
		0.1f,
		false);
}

void ASnowmanModeGameMode::UpdateSnowmanInfectionFlow()
{
    if (!HasAuthority()) return;

    ASnowmanModeGameState* SnowmanGameState = GetGameState<ASnowmanModeGameState>();
    if (!SnowmanGameState || !SnowmanGameState->IsSnowmanModeTimerActive()) return;

    UWorld* World = GetWorld();
    if (!World) return;

    TArray<ASnowRumbleCharacter*> SnowmanCharacters;
    TArray<ASnowRumbleCharacter*> NormalCharacters;

    // ★ 투명인간 버그 해결: 문제가 되던 람다를 삭제하고, 직관적으로 역할만 가져옵니다.
    for (TActorIterator<ASnowRumbleCharacter> It(World); It; ++It)
    {
        ASnowRumbleCharacter* Character = *It;
        if (!Character || Character->IsPendingKillPending()) continue;

        ASnowRumblePlayerState* PlayerState = ResolvePlayerStateForCharacter(Character);
        if (!PlayerState) continue;

        // 늦게 로딩된 플레이어도 누락 없이 Normal 또는 Snowman 배열에 정상 등록됩니다.
        if (SnowmanGameState->IsSnowmanModePlayerSnowman(PlayerState))
        {
            if (Character->GetCapsuleComponent()) SnowmanCharacters.Add(Character);
        }
        else if (SnowmanGameState->GetSnowmanModePlayerRole(PlayerState) == ESnowmanModePlayerRole::Normal)
        {
            if (Character->GetCapsuleComponent()) NormalCharacters.Add(Character);
        }
    }

    const float ContactRadiusSquared = FMath::Square(InfectionContactRadius);
    if (ContactRadiusSquared > 0.0f)
    {
        TSet<TWeakObjectPtr<ASnowRumblePlayerState>> ConvertedTargetsThisScan;
        for (ASnowRumbleCharacter* SnowmanCharacter : SnowmanCharacters)
        {
            if (!SnowmanCharacter || SnowmanCharacter->IsPendingKillPending() || !SnowmanCharacter->GetCapsuleComponent()) continue;

            ASnowRumblePlayerState* SnowmanPlayerState = ResolvePlayerStateForCharacter(SnowmanCharacter);
            if (!SnowmanPlayerState || IsSpawnInfectionGraceActive(SnowmanPlayerState)) continue;

            for (ASnowRumbleCharacter* NormalCharacter : NormalCharacters)
            {
                if (!NormalCharacter || NormalCharacter->IsPendingKillPending() || !NormalCharacter->GetCapsuleComponent()) continue;

                ASnowRumblePlayerState* InfectionTargetPlayerState = ResolvePlayerStateForCharacter(NormalCharacter);
                if (!InfectionTargetPlayerState) continue;

                const TWeakObjectPtr<ASnowRumblePlayerState> TargetKey(InfectionTargetPlayerState);
                if (ConvertedTargetsThisScan.Contains(TargetKey) || ConvertingSnowmanPlayerStates.Contains(TargetKey)) continue;

                if (IsSpawnInfectionGraceActive(InfectionTargetPlayerState)) continue;

                const float SnowmanCapsuleRadius = SnowmanCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
                const float NormalCapsuleRadius = NormalCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
                const float EffectiveContactRadius = InfectionContactRadius + SnowmanCapsuleRadius + NormalCapsuleRadius;
                
                const float DistanceSquared = FVector::DistSquared2D(SnowmanCharacter->GetActorLocation(), NormalCharacter->GetActorLocation());
                
                if (DistanceSquared <= FMath::Square(EffectiveContactRadius))
                {
                    const bool bConverted = ConvertPlayerToSnowmanPawn(InfectionTargetPlayerState);
                    if (bConverted)
                    {
                        SnowmanGameState->SetSnowmanPlayerFromServer(InfectionTargetPlayerState);
                        ConvertedTargetsThisScan.Add(TargetKey);
                    }
                }
            }
        }
    }

    ApplySnowmanMovementSpeeds();
    EvaluateSnowmanModeEndCondition();
}

void ASnowmanModeGameMode::ApplySnowmanMovementSpeeds()
{
	const ASnowmanModeGameState* SnowmanGameState =
		GetGameState<ASnowmanModeGameState>();
	if (!SnowmanGameState)
	{
		return;
	}

	for (ASnowRumblePlayerState* PlayerState : CollectSnowmanPlayerStates())
	{
		ASnowRumbleCharacter* Character =
			FindCharacterForPlayerState(PlayerState);
		if (!Character)
		{
			continue;
		}

		if (SnowmanGameState->IsSnowmanModePlayerSnowman(PlayerState))
		{
			ApplySnowmanMovementSpeed(Character);
		}
	}
}

void ASnowmanModeGameMode::EvaluateSnowmanModeEndCondition()
{
	ASnowmanModeGameState* SnowmanGameState = GetGameState<ASnowmanModeGameState>();
	if (!HasAuthority() || !SnowmanGameState || SnowmanGameState->IsSnowmanModeEnded() || !SnowmanGameState->IsSnowmanModeTimerActive())
	{
		return;
	}

	// ★ 핵심 수정: 죽은 데이터가 남아있을 수 있는 Entry 배열을 믿지 않고, 
	// 현재 방에 실제로 접속해 있는 플레이어 목록을 실시간으로 가져옵니다.
	TArray<ASnowRumblePlayerState*> CurrentPlayers = CollectSnowmanPlayerStates();
	if (CurrentPlayers.IsEmpty())
	{
		return;
	}

	int32 NormalPlayerCount = 0;

	// 접속 중인 플레이어들의 상태만 검사합니다 (나간 사람은 자동 제외됨)
	for (ASnowRumblePlayerState* PlayerState : CurrentPlayers)
	{
		if (!PlayerState) continue;

		// 눈사람이 아닌 사람(생존자)이 몇 명인지 셉니다.
		if (!SnowmanGameState->IsSnowmanModePlayerSnowman(PlayerState))
		{
			NormalPlayerCount++;
		}
	}

	// 생존자가 1명이라도 남아있다면 아직 게임이 끝나지 않음
	if (NormalPlayerCount > 0)
	{
		return;
	}

	// 모든 생존자가 감염되어 NormalPlayerCount가 0이 되었다면, 눈사람 승리로 즉시 종료!
	EndSnowmanMode(ESnowmanModeResult::SnowmanVictory);
}

void ASnowmanModeGameMode::HandleSnowmanModeTimeLimitExpired()
{
	ASnowmanModeGameState* SnowmanGameState =
		GetGameState<ASnowmanModeGameState>();
	if (!HasAuthority()
		|| !SnowmanGameState
		|| SnowmanGameState->IsSnowmanModeEnded())
	{
		return;
	}

	EndSnowmanMode(ESnowmanModeResult::SurvivorVictory);
}

void ASnowmanModeGameMode::EndSnowmanMode(ESnowmanModeResult Result)
{
	ASnowmanModeGameState* SnowmanGameState =
		GetGameState<ASnowmanModeGameState>();
	UWorld* World = GetWorld();
	if (!HasAuthority()
		|| !SnowmanGameState
		|| !World
		|| SnowmanGameState->IsSnowmanModeEnded()
		|| Result == ESnowmanModeResult::None)
	{
		return;
	}

	SnowmanGameState->EndSnowmanModeFromServer(Result);
	ApplySnowmanModeStartInputLock(true);
	World->GetTimerManager().ClearTimer(InfectionScanTimerHandle);
	World->GetTimerManager().ClearTimer(SnowmanModeTimeLimitTimerHandle);
	if (SnowmanModeResultLobbyReturnDelaySeconds <= 0.0f)
	{
		TravelToPodiumAfterSnowmanModeEnd();
		return;
	}

	World->GetTimerManager().SetTimer(
		SnowmanModeLobbyReturnTimerHandle,
		this,
		&ASnowmanModeGameMode::TravelToPodiumAfterSnowmanModeEnd,
		SnowmanModeResultLobbyReturnDelaySeconds,
		false);
}

void ASnowmanModeGameMode::TravelToPodiumAfterSnowmanModeEnd()
{
	const ASnowmanModeGameState* SnowmanGameState =
		GetGameState<ASnowmanModeGameState>();
	if (!HasAuthority()
		|| !SnowmanGameState
		|| !SnowmanGameState->IsSnowmanModeEnded())
	{
		return;
	}

	if (PodiumTravelUrl.IsEmpty())
	{
		ReturnToLobbyAfterSnowmanModeEnd();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(
			BuildPodiumTravelUrl(SnowmanGameState->GetSnowmanModeResult()),
			true);
	}
}

void ASnowmanModeGameMode::ReturnToLobbyAfterSnowmanModeEnd()
{
	const ASnowmanModeGameState* SnowmanGameState =
		GetGameState<ASnowmanModeGameState>();
	if (!HasAuthority()
		|| !SnowmanGameState
		|| !SnowmanGameState->IsSnowmanModeEnded()
		|| LobbyReturnTravelUrl.IsEmpty())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(BuildLobbyReturnTravelUrl(), true);
	}
}

FString ASnowmanModeGameMode::BuildLobbyReturnTravelUrl() const
{
	FString TravelUrl = LobbyReturnTravelUrl;
	EnsureSnowmanModeTravelOption(TravelUrl, TEXT("?listen"));
	if (LobbyReturnGameModeClass)
	{
		const FString GameModePath = LobbyReturnGameModeClass->GetPathName();
		EnsureSnowmanModeTravelOptionValue(
			TravelUrl,
			TEXT("game"),
			GameModePath);
	}

	return TravelUrl;
}

FString ASnowmanModeGameMode::BuildPodiumTravelUrl(
	ESnowmanModeResult Result)
{
	FString TravelUrl = PodiumTravelUrl;
	EnsureSnowmanModeTravelOption(TravelUrl, TEXT("?listen"));
	EnsureSnowmanModeTravelOptionValue(
		TravelUrl,
		TEXT("game"),
		SnowmanModePodiumGameModeTravelPath);
	EnsureSnowmanModeTravelOptionValue(
		TravelUrl,
		TEXT("SnowmanResult"),
		LexToString(Result));
	EnsureSnowmanModeTravelOptionValue(
		TravelUrl,
		TEXT("WinnerPlayerIds"),
		BuildWinnerPlayerIdsOption(Result));
	if (!TravelUrl.Contains(TEXT("ExpectedPlayers="), ESearchCase::IgnoreCase))
	{
		EnsureSnowmanModeTravelOptionValue(
			TravelUrl,
			TEXT("ExpectedPlayers"),
			FString::FromInt(
				ExpectedPlayerCount > 0 ? ExpectedPlayerCount : GetNumPlayers()));
	}

	return TravelUrl;
}

FString ASnowmanModeGameMode::BuildWinnerPlayerIdsOption(
	ESnowmanModeResult Result) const
{
	const ASnowmanModeGameState* SnowmanGameState =
		GetGameState<ASnowmanModeGameState>();
	if (!SnowmanGameState)
	{
		return FString();
	}

	TArray<FString> WinnerPlayerIds;
	for (ASnowRumblePlayerState* PlayerState : CollectSnowmanPlayerStates())
	{
		if (!PlayerState)
		{
			continue;
		}

		const bool bIsSnowman =
			SnowmanGameState->IsSnowmanModePlayerSnowman(PlayerState);
		const bool bIsWinner =
			Result == ESnowmanModeResult::SnowmanVictory
			|| (Result == ESnowmanModeResult::SurvivorVictory && !bIsSnowman);
		if (bIsWinner && PlayerState->GetPlayerId() != INDEX_NONE)
		{
			WinnerPlayerIds.Add(FString::FromInt(PlayerState->GetPlayerId()));
		}
	}

	return FString::Join(WinnerPlayerIds, TEXT(","));
}

bool ASnowmanModeGameMode::ConvertPlayerToSnowmanPawn(ASnowRumblePlayerState* PlayerState)
{
    if (!HasAuthority() || !PlayerState) return false;

    const TWeakObjectPtr<ASnowRumblePlayerState> PlayerStateKey(PlayerState);
    if (ConvertingSnowmanPlayerStates.Contains(PlayerStateKey)) return false;

    UWorld* World = GetWorld();
    if (!World) return false;

    APlayerController* PlayerController = ResolveSnowmanPlayerController(World, PlayerState);
    if (!PlayerController) return false;

    ASnowRumbleCharacter* ExistingCharacter = Cast<ASnowRumbleCharacter>(PlayerController->GetPawn());

    // 이미 눈사람 폰이면 속도만 다시 맞추고 종료
    if (ExistingCharacter && ExistingCharacter->IsA<ASnowmanModeSnowmanCharacter>())
    {
        ApplySnowmanMovementSpeed(ExistingCharacter);
        return true;
    }

    ConvertingSnowmanPlayerStates.Add(PlayerStateKey);
    auto FinishPawnConversion = [this, PlayerStateKey](bool bResult)
    {
        ConvertingSnowmanPlayerStates.Remove(PlayerStateKey);
        return bResult;
    };

    TSubclassOf<ASnowmanModeSnowmanCharacter> SpawnClass = SnowmanCharacterClass;
    if (!SpawnClass)
    {
        SpawnClass = ASnowmanModeSnowmanCharacter::StaticClass();
    }
    
    FTransform SpawnTransform = FTransform::Identity;
    if (ExistingCharacter)
    {
        SpawnTransform = ExistingCharacter->GetActorTransform();
        // ★ 바닥 충돌 버그를 막기 위해 아주 살짝 띄워서 스폰
        FVector Loc = SpawnTransform.GetLocation();
        Loc.Z += 80.0f; 
        SpawnTransform.SetLocation(Loc);

        // ★ 기존 캐릭터의 충돌을 끄고 완벽하게 조종 해제 (버그 원천 차단)
        ExistingCharacter->SetActorEnableCollision(false);
        ExistingCharacter->SetActorHiddenInGame(true);
        PlayerController->UnPossess();
    }
    else if (!PlayerController->GetSpawnLocation().IsNearlyZero())
    {
        SpawnTransform = FTransform(PlayerController->GetControlRotation(), PlayerController->GetSpawnLocation());
    }
    else if (AActor* StartSpot = ChoosePlayerStart(PlayerController))
    {
        SpawnTransform = BuildScatteredPlayerStartTransform(StartSpot);
    }

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.Owner = PlayerController;
    SpawnParameters.Instigator = ExistingCharacter;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; 

	if (ExistingCharacter)
	{
		ExistingCharacter->SetActorEnableCollision(false);
		ExistingCharacter->SetActorHiddenInGame(true);
       
		// ★ 기존 폰을 바닥 아래로 순간이동 시켜서 새 눈사람과 절대 겹치지 않게 만듭니다.
		ExistingCharacter->SetActorLocation(SpawnTransform.GetLocation() + FVector(0.0f, 0.0f, -5000.0f));
       
		PlayerController->UnPossess();
	}
	
    // 1. 눈사람 스폰
    ASnowmanModeSnowmanCharacter* SnowmanCharacter = World->SpawnActor<ASnowmanModeSnowmanCharacter>(SpawnClass, SpawnTransform, SpawnParameters);

    if (!SnowmanCharacter)
    {
        UE_LOG(LogSnowmanMode, Error, TEXT("ConvertFail: SpawnActor returned nullptr!"));
        if (ExistingCharacter)
        {
            ExistingCharacter->SetActorEnableCollision(true);
            ExistingCharacter->SetActorHiddenInGame(false);
            PlayerController->Possess(ExistingCharacter);
        }
        return FinishPawnConversion(false);
    }

    SnowmanCharacter->SetReplicates(true);
    SnowmanCharacter->SetReplicateMovement(true);

    // 2. 엔진 기본 흐름에 따라 완벽하게 빙의
    PlayerController->Possess(SnowmanCharacter);

    // 3. 빙의가 끝난 후 불필요한 기존 캐릭터 삭제
    if (ExistingCharacter)
    {
        ExistingCharacter->Destroy();
    }

    // 4. 후처리
    ApplySnowmanMovementSpeed(SnowmanCharacter);
    GrantSpawnInfectionGrace(PlayerState);

    return FinishPawnConversion(true);
}

void ASnowmanModeGameMode::ApplySnowmanMovementSpeed(
	ASnowRumbleCharacter* Character)
{
	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent =
		Character->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	const float SnowmanWalkSpeed =
		NormalPlayerReferenceWalkSpeed * SnowmanMovementSpeedMultiplier;
	if (ASnowmanModeSnowmanCharacter* SnowmanCharacter =
		Cast<ASnowmanModeSnowmanCharacter>(Character))
	{
		SnowmanCharacter->SetSnowmanWalkSpeedFromMode(SnowmanWalkSpeed);
		return;
	}

	MovementComponent->MaxWalkSpeed = SnowmanWalkSpeed;
}

TArray<ASnowRumblePlayerState*>
ASnowmanModeGameMode::CollectSnowmanPlayerStates() const
{
	TArray<ASnowRumblePlayerState*> PlayerStates;
	const AGameStateBase* CurrentGameState = GameState;
	if (!CurrentGameState)
	{
		return PlayerStates;
	}

	for (APlayerState* PlayerState : CurrentGameState->PlayerArray)
	{
		if (ASnowRumblePlayerState* SnowRumblePlayerState =
			Cast<ASnowRumblePlayerState>(PlayerState))
		{
			PlayerStates.Add(SnowRumblePlayerState);
		}
	}

	return PlayerStates;
}

void ASnowmanModeGameMode::GrantSpawnInfectionGrace(
	ASnowRumblePlayerState* PlayerState)
{
	if (!HasAuthority() || !PlayerState || SpawnInfectionGraceSeconds <= 0.0f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const TWeakObjectPtr<ASnowRumblePlayerState> PlayerStateKey(PlayerState);
	SpawnInfectionGraceEndTimes.FindOrAdd(PlayerStateKey) =
		World->GetTimeSeconds() + SpawnInfectionGraceSeconds;
}

bool ASnowmanModeGameMode::IsSpawnInfectionGraceActive(
	const ASnowRumblePlayerState* PlayerState) const
{
	if (!PlayerState)
	{
		return false;
	}

	const TWeakObjectPtr<ASnowRumblePlayerState> PlayerStateKey(
		const_cast<ASnowRumblePlayerState*>(PlayerState));
	const double* GraceEndTime =
		SpawnInfectionGraceEndTimes.Find(PlayerStateKey);
	if (!GraceEndTime)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	return World && World->GetTimeSeconds() < *GraceEndTime;
}

void ASnowmanModeGameMode::GrantSpawnInfectionGraceToAllPlayers()
{
	for (ASnowRumblePlayerState* PlayerState : CollectSnowmanPlayerStates())
	{
		GrantSpawnInfectionGrace(PlayerState);
	}
}

ASnowRumbleCharacter* ASnowmanModeGameMode::FindCharacterForPlayerState(
	const ASnowRumblePlayerState* PlayerState) const
{
	UWorld* World = GetWorld();
	if (!World || !PlayerState)
	{
		return nullptr;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator();
		It;
		++It)
	{
		const APlayerController* PlayerController = It->Get();
		if (!PlayerController || PlayerController->PlayerState != PlayerState)
		{
			continue;
		}

		return Cast<ASnowRumbleCharacter>(PlayerController->GetPawn());
	}

	return nullptr;
}

ASnowRumblePlayerState* ASnowmanModeGameMode::ResolvePlayerStateForCharacter(
	const ASnowRumbleCharacter* Character) const
{
	if (!Character)
	{
		return nullptr;
	}

	if (const AController* Controller = Character->GetController())
	{
		if (ASnowRumblePlayerState* ControllerPlayerState =
			Controller->GetPlayerState<ASnowRumblePlayerState>())
		{
			return ControllerPlayerState;
		}
	}

	return Character->GetPlayerState<ASnowRumblePlayerState>();
}

void ASnowmanModeGameMode::ApplySnowmanModeStartInputLock(
	bool bShouldLockInput)
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
		APlayerController* PlayerController = It->Get();
		if (!PlayerController)
		{
			continue;
		}

		if (bShouldLockInput)
		{
			PlayerController->SetIgnoreMoveInput(true);
			PlayerController->SetIgnoreLookInput(true);
		}
		else
		{
			PlayerController->ResetIgnoreMoveInput();
			PlayerController->ResetIgnoreLookInput();
		}

		if (APawn* Pawn = PlayerController->GetPawn())
		{
			if (UCharacterMovementComponent* MovementComponent =
				Pawn->FindComponentByClass<UCharacterMovementComponent>())
			{
				MovementComponent->StopMovementImmediately();
			}
		}
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

FTransform ASnowmanModeGameMode::BuildScatteredPlayerStartTransform(
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
			+ MakeSnowmanModeRandomHorizontalOffset(
				PlayerStartSpawnScatterRadius);
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

bool ASnowmanModeGameMode::TryResolveSpawnLocationOnGround(
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
	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(SnowmanModeSpawnGround),
		false);
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

bool ASnowmanModeGameMode::IsSpawnCapsuleClear(
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
	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(SnowmanModeSpawnCapsule),
		false);
	return !World->OverlapBlockingTestByChannel(
		SpawnLocation,
		FQuat::Identity,
		ECC_Pawn,
		CapsuleShape,
		QueryParams);
}

void ASnowmanModeGameMode::GetDefaultPawnCapsuleSize(
	float& OutCapsuleRadius,
	float& OutCapsuleHalfHeight) const
{
	OutCapsuleRadius = 42.0f;
	OutCapsuleHalfHeight = 96.0f;

	const ASnowRumbleCharacter* DefaultCharacter =
		DefaultPawnClass
			? Cast<ASnowRumbleCharacter>(DefaultPawnClass->GetDefaultObject())
			: nullptr;
	const UCapsuleComponent* DefaultCapsule =
		DefaultCharacter ? DefaultCharacter->GetCapsuleComponent() : nullptr;
	if (DefaultCapsule)
	{
		OutCapsuleRadius = DefaultCapsule->GetScaledCapsuleRadius();
		OutCapsuleHalfHeight = DefaultCapsule->GetScaledCapsuleHalfHeight();
	}

	TSubclassOf<ASnowmanModeSnowmanCharacter> SpawnClass =
		SnowmanCharacterClass;
	if (!SpawnClass)
	{
		SpawnClass = ASnowmanModeSnowmanCharacter::StaticClass();
	}
	const ASnowmanModeSnowmanCharacter* SnowmanDefaultObject =
		SpawnClass ? SpawnClass.GetDefaultObject() : nullptr;
	const UCapsuleComponent* SnowmanCapsule =
		SnowmanDefaultObject
			? SnowmanDefaultObject->GetCapsuleComponent()
			: nullptr;
	if (!SnowmanCapsule)
	{
		return;
	}

	OutCapsuleRadius = FMath::Max(
		OutCapsuleRadius,
		SnowmanCapsule->GetScaledCapsuleRadius());
	OutCapsuleHalfHeight = FMath::Max(
		OutCapsuleHalfHeight,
		SnowmanCapsule->GetScaledCapsuleHalfHeight());
}

bool ASnowmanModeGameMode::IsSpawnLocationFarEnough(
	const FVector& CandidateLocation) const
{
	if (PlayerStartSpawnMinimumSpacing <= 0.0f)
	{
		return true;
	}

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

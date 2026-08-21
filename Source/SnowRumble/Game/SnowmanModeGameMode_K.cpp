// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowmanModeGameMode_K.h"

#include "../Player/SnowRumbleCharacter.h"
#include "../Player/SnowmanModeSnowmanCharacter_K.h"
#include "../UI/SnowRumblePlayerController.h"
#include "EngineUtils.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "SnowmanModeGameState_K.h"
#include "SnowRumbleLobbyGameMode.h"
#include "SnowRumblePlayerState.h"

DEFINE_LOG_CATEGORY_STATIC(LogSnowmanMode, Log, All);

namespace
{
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
	bSnowmanRolesInitialized = false;
	UsedPlayerStarts.Reset();
	UsedSpawnLocations.Reset();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(
			SnowmanRoleInitializationRetryTimerHandle);
		World->GetTimerManager().ClearTimer(InfectionScanTimerHandle);
		World->GetTimerManager().ClearTimer(SnowmanModeTimeLimitTimerHandle);
		World->GetTimerManager().ClearTimer(SnowmanModeLobbyReturnTimerHandle);
	}
}

void ASnowmanModeGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	BroadcastLoadingProgress();
	TryDismissLoadingScreens();
}

void ASnowmanModeGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);

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
	ASnowmanModeGameState* SnowmanGameState =
		GetGameState<ASnowmanModeGameState>();
	if (!SnowmanGameState || !SnowmanGameState->IsSnowmanModeTimerActive())
	{
		if (bLogSnowmanInfectionDebug)
		{
			UE_LOG(
				LogSnowmanMode,
				Verbose,
				TEXT("Infection scan skipped: GameState=%s TimerActive=%s"),
				SnowmanGameState ? TEXT("Valid") : TEXT("None"),
				SnowmanGameState && SnowmanGameState->IsSnowmanModeTimerActive()
					? TEXT("true")
					: TEXT("false"));
		}
		return;
	}

	const TArray<FSnowmanModePlayerEntry>& PlayerEntries =
		SnowmanGameState->GetSnowmanModePlayerEntries();
	TArray<ASnowRumbleCharacter*> SnowmanCharacters;
	TArray<ASnowRumbleCharacter*> NormalCharacters;
	TArray<ASnowRumblePlayerState*> ControllerlessPendingCancels;
	static TSet<TWeakObjectPtr<ASnowRumblePlayerState>>
		CompletingInfectionPlayerStates;
	static TSet<TWeakObjectPtr<ASnowRumblePlayerState>>
		ControllerLostPendingPlayerStates;
	auto ResolveSnowmanModeEntryPlayerState =
		[&PlayerEntries](const ASnowRumblePlayerState* PlayerState)
		-> ASnowRumblePlayerState*
	{
		if (!PlayerState)
		{
			return nullptr;
		}

		for (const FSnowmanModePlayerEntry& Entry : PlayerEntries)
		{
			if (Entry.PlayerState == PlayerState)
			{
				return Entry.PlayerState;
			}
		}

		const int32 PlayerId = PlayerState->GetPlayerId();
		if (PlayerId != INDEX_NONE)
		{
			for (const FSnowmanModePlayerEntry& Entry : PlayerEntries)
			{
				if (Entry.PlayerState
					&& Entry.PlayerState->GetPlayerId() == PlayerId)
				{
					return Entry.PlayerState;
				}
			}
		}

		const FUniqueNetIdRepl& UniqueId = PlayerState->GetUniqueId();
		if (UniqueId.IsValid())
		{
			for (const FSnowmanModePlayerEntry& Entry : PlayerEntries)
			{
				if (Entry.PlayerState
					&& Entry.PlayerState->GetUniqueId().IsValid()
					&& Entry.PlayerState->GetUniqueId() == UniqueId)
				{
					return Entry.PlayerState;
				}
			}
		}

		return nullptr;
	};
	auto HasActiveControllerForPlayerState =
		[](const UWorld* ScanWorld, const ASnowRumblePlayerState* PlayerState)
	{
		if (!ScanWorld || !PlayerState)
		{
			return false;
		}

		for (FConstPlayerControllerIterator It =
				ScanWorld->GetPlayerControllerIterator();
			It;
			++It)
		{
			const APlayerController* CandidateController = It->Get();
			const APlayerState* CandidatePlayerState =
				CandidateController ? CandidateController->PlayerState : nullptr;
			if (!CandidatePlayerState)
			{
				continue;
			}

			if (CandidatePlayerState == PlayerState)
			{
				return true;
			}

			const int32 PlayerId = PlayerState->GetPlayerId();
			if (PlayerId != INDEX_NONE
				&& CandidatePlayerState->GetPlayerId() == PlayerId)
			{
				return true;
			}

			const FUniqueNetIdRepl& UniqueId = PlayerState->GetUniqueId();
			if (UniqueId.IsValid()
				&& CandidatePlayerState->GetUniqueId().IsValid()
				&& CandidatePlayerState->GetUniqueId() == UniqueId)
			{
				return true;
			}
		}

		return false;
	};

	for (const FSnowmanModePlayerEntry& Entry : PlayerEntries)
	{
		ASnowRumblePlayerState* PlayerState = Entry.PlayerState;
		if (!PlayerState)
		{
			continue;
		}

		const TWeakObjectPtr<ASnowRumblePlayerState> PlayerStateKey(PlayerState);
		if (Entry.Role != ESnowmanModePlayerRole::InfectionPending)
		{
			CompletingInfectionPlayerStates.Remove(PlayerStateKey);
			ControllerLostPendingPlayerStates.Remove(PlayerStateKey);
		}

		if (bLogSnowmanInfectionDebug)
		{
			UE_LOG(
				LogSnowmanMode,
				Verbose,
				TEXT("Infection entry: %s Role=%s Remaining=%.2f Completing=%s"),
				*DescribeSnowmanPlayerState(PlayerState),
				LexToString(Entry.Role),
				SnowmanGameState->GetSnowmanModeInfectionRemainingSeconds(
					PlayerState),
				CompletingInfectionPlayerStates.Contains(PlayerStateKey)
					? TEXT("true")
					: TEXT("false"));
		}

		if (Entry.Role == ESnowmanModePlayerRole::InfectionPending
			&& SnowmanGameState->GetSnowmanModeInfectionRemainingSeconds(PlayerState)
				<= 0.0f)
		{
			const bool bHasActiveController =
				HasActiveControllerForPlayerState(GetWorld(), PlayerState);
			if (!bHasActiveController)
			{
				if (!ControllerLostPendingPlayerStates.Contains(PlayerStateKey))
				{
					UE_LOG(
						LogSnowmanMode,
						Warning,
						TEXT("Infection pending cleanup: controller is missing, pending state canceled safely. Player=%s"),
						*DescribeSnowmanPlayerState(PlayerState));
					ControllerLostPendingPlayerStates.Add(PlayerStateKey);
				}
				ControllerlessPendingCancels.Add(PlayerState);
				CompletingInfectionPlayerStates.Remove(PlayerStateKey);
				continue;
			}
			ControllerLostPendingPlayerStates.Remove(PlayerStateKey);

			if (CompletingInfectionPlayerStates.Contains(PlayerStateKey))
			{
				continue;
			}

			CompletingInfectionPlayerStates.Add(PlayerStateKey);
			if (bLogSnowmanInfectionDebug)
			{
				UE_LOG(
					LogSnowmanMode,
					Log,
					TEXT("Infection pending complete: %s"),
					*DescribeSnowmanPlayerState(PlayerState));
			}

			if (ConvertPlayerToSnowmanPawn(PlayerState))
			{
				SnowmanGameState->CompleteInfectionFromServer(PlayerState);
				CompletingInfectionPlayerStates.Remove(PlayerStateKey);
			}
			else
			{
				UE_LOG(
					LogSnowmanMode,
					Warning,
					TEXT("Infection pending complete conversion failed once and remains Pending: %s Role=%s"),
					*DescribeSnowmanPlayerState(PlayerState),
					LexToString(
						SnowmanGameState->GetSnowmanModePlayerRole(PlayerState)));
			}
		}
	}
	for (ASnowRumblePlayerState* PlayerState : ControllerlessPendingCancels)
	{
		SnowmanGameState->CancelControllerlessPendingFromServer(
			PlayerState);
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ASnowRumbleCharacter> It(World); It; ++It)
	{
		ASnowRumbleCharacter* Character = *It;
		if (!Character || Character->IsPendingKillPending())
		{
			continue;
		}

		ASnowRumblePlayerState* PlayerState =
			ResolvePlayerStateForCharacter(Character);
		if (!PlayerState)
		{
			if (bLogSnowmanInfectionDebug)
			{
				UE_LOG(
					LogSnowmanMode,
					Verbose,
					TEXT("Infection candidate skipped: Character=%s PlayerState=None Controller=%s"),
					*GetNameSafe(Character),
					*GetNameSafe(Character->GetController()));
			}
			continue;
		}

		ASnowRumblePlayerState* EntryPlayerState =
			ResolveSnowmanModeEntryPlayerState(PlayerState);
		if (!EntryPlayerState)
		{
			continue;
		}

		if (SnowmanGameState->IsSnowmanModePlayerSnowman(EntryPlayerState))
		{
			SnowmanCharacters.Add(Character);
		}
		else if (SnowmanGameState->GetSnowmanModePlayerRole(EntryPlayerState)
			== ESnowmanModePlayerRole::Normal)
		{
			NormalCharacters.Add(Character);
		}
	}

	if (bLogSnowmanInfectionDebug)
	{
		const double CurrentTime = World->GetTimeSeconds();
		if (LastInfectionDebugSummaryTime < 0.0
			|| CurrentTime - LastInfectionDebugSummaryTime >= 1.0)
		{
			LastInfectionDebugSummaryTime = CurrentTime;
			int32 PendingCount = 0;
			for (const FSnowmanModePlayerEntry& Entry : PlayerEntries)
			{
				if (Entry.Role == ESnowmanModePlayerRole::InfectionPending)
				{
					++PendingCount;
				}
			}
			UE_LOG(
				LogSnowmanMode,
				Log,
				TEXT("Infection scan summary: Entries=%d Snowmen=%d Pending=%d Normals=%d ContactRadius=%.1f"),
				PlayerEntries.Num(),
				SnowmanCharacters.Num(),
				PendingCount,
				NormalCharacters.Num(),
				InfectionContactRadius);
		}
	}

	const float ContactRadiusSquared = FMath::Square(InfectionContactRadius);
	if (ContactRadiusSquared > 0.0f)
	{
		for (const ASnowRumbleCharacter* SnowmanCharacter : SnowmanCharacters)
		{
			if (!SnowmanCharacter)
			{
				continue;
			}

			for (ASnowRumbleCharacter* NormalCharacter : NormalCharacters)
			{
				if (!NormalCharacter)
				{
					continue;
				}

				ASnowRumblePlayerState* NormalPlayerState =
					ResolvePlayerStateForCharacter(NormalCharacter);
				if (!NormalPlayerState)
				{
					continue;
				}
				ASnowRumblePlayerState* InfectionTargetPlayerState =
					ResolveSnowmanModeEntryPlayerState(NormalPlayerState);
				if (!InfectionTargetPlayerState)
				{
					continue;
				}
				const ESnowmanModePlayerRole NormalPlayerRole =
					SnowmanGameState->GetSnowmanModePlayerRole(
						InfectionTargetPlayerState);
				if (NormalPlayerRole != ESnowmanModePlayerRole::Normal)
				{
					continue;
				}

				const float SnowmanCapsuleRadius =
					SnowmanCharacter->GetCapsuleComponent()
						? SnowmanCharacter->GetCapsuleComponent()
							->GetScaledCapsuleRadius()
						: 0.0f;
				const float NormalCapsuleRadius =
					NormalCharacter->GetCapsuleComponent()
						? NormalCharacter->GetCapsuleComponent()
							->GetScaledCapsuleRadius()
						: 0.0f;
				const float EffectiveContactRadius =
					InfectionContactRadius
					+ SnowmanCapsuleRadius
					+ NormalCapsuleRadius;
				const float DistanceSquared = FVector::DistSquared2D(
					SnowmanCharacter->GetActorLocation(),
					NormalCharacter->GetActorLocation());
				const float EffectiveContactRadiusSquared =
					FMath::Square(EffectiveContactRadius);
				if (DistanceSquared <= EffectiveContactRadiusSquared)
				{
					const bool bStartedInfection =
						SnowmanGameState->StartInfectionPendingFromServer(
							InfectionTargetPlayerState,
							InfectionPendingSeconds);
					if (bLogSnowmanInfectionDebug)
					{
						UE_LOG(
							LogSnowmanMode,
							Log,
							TEXT("Infection contact: Snowman=%s Target=%s Distance=%.1f EffectiveRadius=%.1f Started=%s RoleAfter=%s"),
							*DescribeSnowmanPlayerState(
								ResolvePlayerStateForCharacter(
									SnowmanCharacter)),
							*DescribeSnowmanPlayerState(
								InfectionTargetPlayerState),
							FMath::Sqrt(DistanceSquared),
							EffectiveContactRadius,
							bStartedInfection ? TEXT("true") : TEXT("false"),
							LexToString(
								SnowmanGameState->GetSnowmanModePlayerRole(
									InfectionTargetPlayerState)));
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
	ASnowmanModeGameState* SnowmanGameState =
		GetGameState<ASnowmanModeGameState>();
	if (!HasAuthority()
		|| !SnowmanGameState
		|| SnowmanGameState->IsSnowmanModeEnded()
		|| !SnowmanGameState->IsSnowmanModeTimerActive())
	{
		return;
	}

	const TArray<FSnowmanModePlayerEntry>& PlayerEntries =
		SnowmanGameState->GetSnowmanModePlayerEntries();
	if (PlayerEntries.IsEmpty())
	{
		return;
	}

	for (const FSnowmanModePlayerEntry& Entry : PlayerEntries)
	{
		if (!Entry.PlayerState || Entry.Role != ESnowmanModePlayerRole::Snowman)
		{
			return;
		}
	}

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
		ReturnToLobbyAfterSnowmanModeEnd();
		return;
	}

	World->GetTimerManager().SetTimer(
		SnowmanModeLobbyReturnTimerHandle,
		this,
		&ASnowmanModeGameMode::ReturnToLobbyAfterSnowmanModeEnd,
		SnowmanModeResultLobbyReturnDelaySeconds,
		false);
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
	if (!TravelUrl.Contains(TEXT("?listen"), ESearchCase::IgnoreCase))
	{
		TravelUrl += TEXT("?listen");
	}

	if (LobbyReturnGameModeClass
		&& !TravelUrl.Contains(TEXT("?game="), ESearchCase::IgnoreCase))
	{
		const FString GameModePath = LobbyReturnGameModeClass->GetPathName();
		if (!GameModePath.IsEmpty())
		{
			TravelUrl += FString::Printf(TEXT("?game=%s"), *GameModePath);
		}
	}

	return TravelUrl;
}

bool ASnowmanModeGameMode::ConvertPlayerToSnowmanPawn(
	ASnowRumblePlayerState* PlayerState)
{
	if (!HasAuthority() || !PlayerState)
	{
		UE_LOG(
			LogSnowmanMode,
			Warning,
			TEXT("ConvertFail: Authority or PlayerState is invalid! HasAuthority=%s Player=%s"),
			HasAuthority() ? TEXT("true") : TEXT("false"),
			*DescribeSnowmanPlayerState(PlayerState));
		return false;
	}
	static TSet<TWeakObjectPtr<ASnowRumblePlayerState>>
		ConvertingSnowmanPlayerStates;
	const TWeakObjectPtr<ASnowRumblePlayerState> PlayerStateKey(PlayerState);
	if (ConvertingSnowmanPlayerStates.Contains(PlayerStateKey))
	{
		if (bLogSnowmanInfectionDebug)
		{
			UE_LOG(
				LogSnowmanMode,
				Verbose,
				TEXT("Snowman pawn conversion skipped because conversion is already in progress: %s"),
				*DescribeSnowmanPlayerState(PlayerState));
		}
		UE_LOG(
			LogSnowmanMode,
			Warning,
			TEXT("ConvertFail: Pawn conversion is already in progress! Player=%s"),
			*DescribeSnowmanPlayerState(PlayerState));
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(
			LogSnowmanMode,
			Error,
			TEXT("ConvertFail: World is invalid! Player=%s"),
			*DescribeSnowmanPlayerState(PlayerState));
		return false;
	}

	APlayerController* PlayerController = nullptr;
	ASnowRumbleCharacter* ExistingCharacter = nullptr;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator();
		It;
		++It)
	{
		APlayerController* CandidateController = It->Get();
		if (!CandidateController
			|| CandidateController->PlayerState != PlayerState)
		{
			continue;
		}

		PlayerController = CandidateController;
		ExistingCharacter =
			Cast<ASnowRumbleCharacter>(CandidateController->GetPawn());
		break;
	}

	if (!PlayerController)
	{
		UE_LOG(
			LogSnowmanMode,
			Warning,
			TEXT("ConvertFail: PlayerController is invalid! Player=%s"),
			*DescribeSnowmanPlayerState(PlayerState));
		return false;
	}
	if (!ExistingCharacter)
	{
		UE_LOG(
			LogSnowmanMode,
			Warning,
			TEXT("ConvertWarn: OldPawn is invalid; using PlayerStart fallback spawn. Player=%s Controller=%s CurrentPawn=%s"),
			*DescribeSnowmanPlayerState(PlayerState),
			*GetNameSafe(PlayerController),
			*GetNameSafe(PlayerController->GetPawn()));
	}
	else if (ExistingCharacter->IsA<ASnowmanModeSnowmanCharacter>())
	{
		ApplySnowmanMovementSpeed(ExistingCharacter);
		return true;
	}

	ConvertingSnowmanPlayerStates.Add(PlayerStateKey);
	auto FinishPawnConversion = [PlayerStateKey](bool bResult)
	{
		ConvertingSnowmanPlayerStates.Remove(PlayerStateKey);
		return bResult;
	};

	TSubclassOf<ASnowmanModeSnowmanCharacter> SpawnClass =
		SnowmanCharacterClass;
	if (!SpawnClass)
	{
		if (bLogSnowmanInfectionDebug)
		{
			UE_LOG(
				LogSnowmanMode,
				Error,
				TEXT("SnowmanCharacterClass is empty on %s. GameModeClass=%s ConfiguredClass=None NativeClass=%s Player=%s"),
				*GetNameSafe(this),
				*GetPathNameSafe(GetClass()),
				*GetPathNameSafe(ASnowmanModeSnowmanCharacter::StaticClass()),
				*DescribeSnowmanPlayerState(PlayerState));
		}
		SpawnClass = ASnowmanModeSnowmanCharacter::StaticClass();
	}
	if (!SpawnClass)
	{
		UE_LOG(
			LogSnowmanMode,
			Error,
			TEXT("ConvertFail: SnowmanCharacterClass is nullptr! GameModeClass=%s Player=%s"),
			*GetPathNameSafe(GetClass()),
			*DescribeSnowmanPlayerState(PlayerState));
		UE_LOG(
			LogSnowmanMode,
			Error,
			TEXT("Snowman pawn conversion failed because no spawn class is available. GameModeClass=%s Player=%s"),
			*GetPathNameSafe(GetClass()),
			*DescribeSnowmanPlayerState(PlayerState));
		return FinishPawnConversion(false);
	}
	if (bLogSnowmanInfectionDebug)
	{
		UE_LOG(
			LogSnowmanMode,
			Log,
			TEXT("Snowman pawn conversion spawn class resolved. GameModeClass=%s SpawnClass=%s Player=%s"),
			*GetPathNameSafe(GetClass()),
			*GetPathNameSafe(SpawnClass.Get()),
			*DescribeSnowmanPlayerState(PlayerState));
	}

	const ASnowmanModeSnowmanCharacter* SnowmanDefaultObject =
		SpawnClass.GetDefaultObject();
	if (!SnowmanDefaultObject)
	{
		if (bLogSnowmanInfectionDebug)
		{
			UE_LOG(
				LogSnowmanMode,
				Warning,
				TEXT("Snowman pawn class default object is invalid for %s"),
				*DescribeSnowmanPlayerState(PlayerState));
		}
		UE_LOG(
			LogSnowmanMode,
			Error,
			TEXT("ConvertFail: SnowmanCharacterClass default object is invalid! SpawnClass=%s Player=%s"),
			*GetPathNameSafe(SpawnClass.Get()),
			*DescribeSnowmanPlayerState(PlayerState));
		return FinishPawnConversion(false);
	}

	if (ExistingCharacter && PlayerController->GetPawn() != ExistingCharacter)
	{
		UE_LOG(
			LogSnowmanMode,
			Warning,
			TEXT("ConvertFail: OldPawn is invalid! Controller pawn changed before spawn. Player=%s CurrentPawn=%s ExpectedPawn=%s"),
			*DescribeSnowmanPlayerState(PlayerState),
			*GetNameSafe(PlayerController->GetPawn()),
			*GetNameSafe(ExistingCharacter));
		UE_LOG(
			LogSnowmanMode,
			Warning,
			TEXT("Snowman pawn conversion aborted because controller pawn changed before spawn. Player=%s CurrentPawn=%s ExpectedPawn=%s SpawnClass=%s"),
			*DescribeSnowmanPlayerState(PlayerState),
			*GetNameSafe(PlayerController->GetPawn()),
			*GetNameSafe(ExistingCharacter),
			*GetPathNameSafe(SpawnClass.Get()));
		return FinishPawnConversion(false);
	}

	FTransform BaseTransform = FTransform::Identity;
	if (ExistingCharacter)
	{
		BaseTransform = ExistingCharacter->GetActorTransform();
	}
	else if (!PlayerController->GetSpawnLocation().IsNearlyZero())
	{
		BaseTransform = FTransform(
			PlayerController->GetControlRotation(),
			PlayerController->GetSpawnLocation());
	}
	if (!ExistingCharacter)
	{
		if (AActor* StartSpot = ChoosePlayerStart(PlayerController))
		{
			BaseTransform = BuildScatteredPlayerStartTransform(StartSpot);
		}
	}
	FVector SpawnLocation = BaseTransform.GetLocation();
	SpawnLocation.Z += 75.0f;
	const FTransform SpawnTransform(
		BaseTransform.GetRotation(),
		SpawnLocation);
	const FRotator ControlRotation = PlayerController->GetControlRotation();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = PlayerController;
	SpawnParameters.Instigator = ExistingCharacter;
	SpawnParameters.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (ExistingCharacter)
	{
		PlayerController->UnPossess();
		ExistingCharacter->SetActorEnableCollision(false);
		ExistingCharacter->ForceNetUpdate();
	}
	PlayerController->ForceNetUpdate();

	ASnowmanModeSnowmanCharacter* SnowmanCharacter =
		World->SpawnActor<ASnowmanModeSnowmanCharacter>(
			SpawnClass,
			SpawnTransform,
			SpawnParameters);
	if (!SnowmanCharacter)
	{
		UE_LOG(
			LogSnowmanMode,
			Error,
			TEXT("ConvertFail: SpawnActor returned nullptr! (Check Collision or Transform) Player=%s SpawnClass=%s SpawnLocation=%s"),
			*DescribeSnowmanPlayerState(PlayerState),
			*GetPathNameSafe(SpawnClass.Get()),
			*SpawnTransform.GetLocation().ToCompactString());
		UE_LOG(
			LogSnowmanMode,
			Warning,
			TEXT("Snowman pawn spawn failed for %s at %s. SpawnClass=%s ExistingPawn=%s"),
			*DescribeSnowmanPlayerState(PlayerState),
			*SpawnTransform.GetLocation().ToCompactString(),
			*GetPathNameSafe(SpawnClass.Get()),
			*GetNameSafe(ExistingCharacter));
		if (ExistingCharacter)
		{
			ExistingCharacter->SetActorEnableCollision(true);
			PlayerController->Possess(ExistingCharacter);
			ExistingCharacter->ForceNetUpdate();
		}
		PlayerController->ForceNetUpdate();
		return FinishPawnConversion(false);
	}

	SnowmanCharacter->SetReplicates(true);
	SnowmanCharacter->SetReplicateMovement(true);
	SnowmanCharacter->SetOwner(PlayerController);
	SnowmanCharacter->ForceNetUpdate();
	PlayerController->ForceNetUpdate();
	PlayerController->Possess(SnowmanCharacter);
	SnowmanCharacter->ForceNetUpdate();
	PlayerController->ForceNetUpdate();
	if (PlayerController->GetPawn() != SnowmanCharacter)
	{
		SnowmanCharacter->SetActorLocation(
			SpawnLocation + FVector(0.0f, 0.0f, 50.0f),
			false,
			nullptr,
			ETeleportType::TeleportPhysics);
		SnowmanCharacter->ForceNetUpdate();
		PlayerController->UnPossess();
		PlayerController->Possess(SnowmanCharacter);
		SnowmanCharacter->ForceNetUpdate();
		PlayerController->ForceNetUpdate();
	}
	if (PlayerController->GetPawn() != SnowmanCharacter)
	{
		UE_LOG(
			LogSnowmanMode,
			Error,
			TEXT("ConvertFail: Controller Possess failed! Player=%s CurrentPawn=%s NewPawn=%s SpawnClass=%s"),
			*DescribeSnowmanPlayerState(PlayerState),
			*GetNameSafe(PlayerController->GetPawn()),
			*GetNameSafe(SnowmanCharacter),
			*GetPathNameSafe(SpawnClass.Get()));
		UE_LOG(
			LogSnowmanMode,
			Warning,
			TEXT("Snowman pawn possess failed for %s. SpawnClass=%s SnowmanPawn=%s ExistingPawn=%s"),
			*DescribeSnowmanPlayerState(PlayerState),
			*GetPathNameSafe(SpawnClass.Get()),
			*GetNameSafe(SnowmanCharacter),
			*GetNameSafe(ExistingCharacter));
		SnowmanCharacter->Destroy();
		if (ExistingCharacter)
		{
			ExistingCharacter->SetActorEnableCollision(true);
			PlayerController->Possess(ExistingCharacter);
			ExistingCharacter->ForceNetUpdate();
		}
		PlayerController->ForceNetUpdate();
		return FinishPawnConversion(false);
	}

	PlayerController->SetControlRotation(ControlRotation);
	if (ExistingCharacter)
	{
		ExistingCharacter->Destroy();
	}
	ApplySnowmanMovementSpeed(SnowmanCharacter);
	SnowmanCharacter->ForceNetUpdate();
	PlayerController->ForceNetUpdate();
	PlayerState->ForceNetUpdate();
	if (bLogSnowmanInfectionDebug)
	{
		UE_LOG(
			LogSnowmanMode,
			Log,
			TEXT("Snowman pawn conversion succeeded. Player=%s SnowmanPawn=%s SpawnClass=%s"),
			*DescribeSnowmanPlayerState(PlayerState),
			*GetNameSafe(SnowmanCharacter),
			*GetPathNameSafe(SpawnClass.Get()));
	}
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
	if (PlayerStartSpawnScatterRadius <= 0.0f)
	{
		return FTransform(StartRotation, StartLocation);
	}

	const int32 Attempts = FMath::Max(1, PlayerStartSpawnScatterAttempts);
	for (int32 AttemptIndex = 0; AttemptIndex < Attempts; ++AttemptIndex)
	{
		const FVector CandidateLocation =
			StartLocation
			+ MakeSnowmanModeRandomHorizontalOffset(
				PlayerStartSpawnScatterRadius);
		if (IsSpawnLocationFarEnough(CandidateLocation))
		{
			return FTransform(StartRotation, CandidateLocation);
		}
	}

	return FTransform(
		StartRotation,
		StartLocation
			+ MakeSnowmanModeRandomHorizontalOffset(
				PlayerStartSpawnScatterRadius));
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

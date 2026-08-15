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
	}
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

	for (const FSnowmanModePlayerEntry& Entry : PlayerEntries)
	{
		ASnowRumblePlayerState* PlayerState = Entry.PlayerState;
		if (!PlayerState)
		{
			continue;
		}

		if (bLogSnowmanInfectionDebug)
		{
			UE_LOG(
				LogSnowmanMode,
				Verbose,
				TEXT("Infection entry: %s Role=%s Remaining=%.2f"),
				*DescribeSnowmanPlayerState(PlayerState),
				LexToString(Entry.Role),
				SnowmanGameState->GetSnowmanModeInfectionRemainingSeconds(
					PlayerState));
		}

		if (Entry.Role == ESnowmanModePlayerRole::InfectionPending
			&& SnowmanGameState->GetSnowmanModeInfectionRemainingSeconds(PlayerState)
				<= 0.0f)
		{
			if (bLogSnowmanInfectionDebug)
			{
				UE_LOG(
					LogSnowmanMode,
					Log,
					TEXT("Infection complete: %s"),
					*DescribeSnowmanPlayerState(PlayerState));
			}
			SnowmanGameState->CompleteInfectionFromServer(PlayerState);
		}

		if (SnowmanGameState->IsSnowmanModePlayerSnowman(PlayerState))
		{
			ConvertPlayerToSnowmanPawn(PlayerState);
		}
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

		if (SnowmanGameState->IsSnowmanModePlayerSnowman(PlayerState))
		{
			SnowmanCharacters.Add(Character);
		}
		else if (SnowmanGameState->GetSnowmanModePlayerRole(PlayerState)
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
			UE_LOG(
				LogSnowmanMode,
				Log,
				TEXT("Infection scan summary: Entries=%d Snowmen=%d Normals=%d ContactRadius=%.1f PendingSeconds=%.1f"),
				PlayerEntries.Num(),
				SnowmanCharacters.Num(),
				NormalCharacters.Num(),
				InfectionContactRadius,
				InfectionPendingSeconds);
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
						NormalPlayerState,
						InfectionPendingSeconds);
					if (bLogSnowmanInfectionDebug)
					{
						UE_LOG(
							LogSnowmanMode,
							Log,
							TEXT("Infection contact: Snowman=%s Normal=%s Distance=%.1f EffectiveRadius=%.1f Started=%s NormalRoleAfter=%s"),
							*DescribeSnowmanPlayerState(
								ResolvePlayerStateForCharacter(
									SnowmanCharacter)),
							*DescribeSnowmanPlayerState(NormalPlayerState),
							FMath::Sqrt(DistanceSquared),
							EffectiveContactRadius,
							bStartedInfection ? TEXT("true") : TEXT("false"),
							LexToString(
								SnowmanGameState->GetSnowmanModePlayerRole(
									NormalPlayerState)));
					}
				}
			}
		}
	}

	ApplySnowmanMovementSpeeds();
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

bool ASnowmanModeGameMode::ConvertPlayerToSnowmanPawn(
	ASnowRumblePlayerState* PlayerState)
{
	if (!PlayerState)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
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

	if (!PlayerController
		|| !ExistingCharacter)
	{
		return false;
	}
	if (ExistingCharacter->IsA<ASnowmanModeSnowmanCharacter>())
	{
		ApplySnowmanMovementSpeed(ExistingCharacter);
		return true;
	}

	TSubclassOf<ASnowmanModeSnowmanCharacter> SpawnClass =
		SnowmanCharacterClass;
	if (!SpawnClass)
	{
		SpawnClass = ASnowmanModeSnowmanCharacter::StaticClass();
	}
	const UCapsuleComponent* ExistingCapsule =
		ExistingCharacter->GetCapsuleComponent();
	const ASnowmanModeSnowmanCharacter* SnowmanDefaultObject =
		SpawnClass.GetDefaultObject();
	const UCapsuleComponent* SnowmanCapsule =
		SnowmanDefaultObject ? SnowmanDefaultObject->GetCapsuleComponent() : nullptr;

	const float ExistingHalfHeight =
		ExistingCapsule ? ExistingCapsule->GetScaledCapsuleHalfHeight() : 96.0f;
	const float SnowmanHalfHeight =
		SnowmanCapsule ? SnowmanCapsule->GetScaledCapsuleHalfHeight() : ExistingHalfHeight;

	FVector SpawnLocation = ExistingCharacter->GetActorLocation();
	const FVector TraceStart = SpawnLocation + FVector(0.0f, 0.0f, 100.0f);
	const FVector TraceEnd =
		SpawnLocation - FVector(0.0f, 0.0f, ExistingHalfHeight + 500.0f);
	FHitResult GroundHit;
	FCollisionQueryParams GroundTraceParams(
		SCENE_QUERY_STAT(SnowmanPawnGroundTrace),
		false,
		ExistingCharacter);
	if (World->LineTraceSingleByChannel(
			GroundHit,
			TraceStart,
			TraceEnd,
			ECC_WorldStatic,
			GroundTraceParams))
	{
		SpawnLocation.Z = GroundHit.ImpactPoint.Z + SnowmanHalfHeight + 3.0f;
	}
	else
	{
		const float ExistingFootZ =
			ExistingCharacter->GetActorLocation().Z - ExistingHalfHeight;
		SpawnLocation.Z = ExistingFootZ + SnowmanHalfHeight + 3.0f;
	}

	const FTransform SpawnTransform(
		ExistingCharacter->GetActorRotation(),
		SpawnLocation);
	const FRotator ControlRotation = PlayerController->GetControlRotation();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = PlayerController;
	SpawnParameters.Instigator = ExistingCharacter;
	SpawnParameters.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ASnowmanModeSnowmanCharacter* SnowmanCharacter =
		World->SpawnActor<ASnowmanModeSnowmanCharacter>(
			SpawnClass,
			SpawnTransform,
			SpawnParameters);
	if (!SnowmanCharacter)
	{
		return false;
	}

	PlayerController->UnPossess();
	PlayerController->Possess(SnowmanCharacter);
	PlayerController->SetControlRotation(ControlRotation);
	ExistingCharacter->Destroy();
	ApplySnowmanMovementSpeed(SnowmanCharacter);
	return true;
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

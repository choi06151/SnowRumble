// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowmanModeGameMode_K.h"

#include "../Player/SnowRumbleCharacter.h"
#include "../UI/SnowRumblePlayerController.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "SnowmanModeGameState_K.h"
#include "SnowRumblePlayerState.h"

namespace
{
FVector MakeRandomHorizontalOffset(float Radius)
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
}

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
	UsedPlayerStarts.Reset();
	UsedSpawnLocations.Reset();
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
			+ MakeRandomHorizontalOffset(PlayerStartSpawnScatterRadius);
		if (IsSpawnLocationFarEnough(CandidateLocation))
		{
			return FTransform(StartRotation, CandidateLocation);
		}
	}

	return FTransform(
		StartRotation,
		StartLocation
			+ MakeRandomHorizontalOffset(PlayerStartSpawnScatterRadius));
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

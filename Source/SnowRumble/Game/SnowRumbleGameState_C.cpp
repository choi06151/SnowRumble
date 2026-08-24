// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleGameState_C.h"

#include "Net/UnrealNetwork.h"
#include "SnowRumbleMatchSubsystem_C.h"

void ASnowRumbleGameState::StartMatchCountdownFromServer(
	float CountdownSeconds)
{
	if (!HasAuthority())
	{
		return;
	}

	MatchStartCountdownSeconds = FMath::Max(0.0f, CountdownSeconds);
	MatchStartServerTime =
		GetServerWorldTimeSeconds() + MatchStartCountdownSeconds;
	RoundStartServerTime = MatchStartServerTime;
	NextMapShrinkServerTime = RoundStartServerTime
		+ MapShrinkWaitDurationSeconds;
	bStartCountdownActive = true;
	ForceNetUpdate();
}

void ASnowRumbleGameState::StartMapShrinkFromServer(
	float ShrinkDurationSeconds)
{
	if (!HasAuthority() || bRoundEnded || bMapShrinkInProgress)
	{
		return;
	}

	bMapShrinkInProgress = true;
	MapShrinkStartedServerTime = GetServerWorldTimeSeconds();
	MapShrinkDurationSeconds = FMath::Max(0.0f, ShrinkDurationSeconds);
	NextMapShrinkServerTime = MapShrinkStartedServerTime;
	ForceNetUpdate();
}

void ASnowRumbleGameState::CompleteMapShrinkFromServer()
{
	if (!HasAuthority() || bRoundEnded || !bMapShrinkInProgress)
	{
		return;
	}

	bMapShrinkInProgress = false;
	NextMapShrinkServerTime =
		GetServerWorldTimeSeconds() + MapShrinkWaitDurationSeconds;
	ForceNetUpdate();
}

void ASnowRumbleGameState::EndRoundFromServer(
	ESnowRumbleTeam WinningTeam,
	const USnowRumbleMatchSubsystem* MatchSubsystem)
{
	if (!HasAuthority() || bRoundEnded || WinningTeam == ESnowRumbleTeam::None)
	{
		return;
	}

	bRoundEnded = true;
	RoundWinningTeam = WinningTeam;
	ApplyMatchStateFromServer(MatchSubsystem);
	if (bMatchEnded && MatchWinningTeam == ESnowRumbleTeam::None)
	{
		MatchWinningTeam = WinningTeam;
	}
	OnRep_RoundResult();
	ForceNetUpdate();
}

void ASnowRumbleGameState::ApplyMatchStateFromServer(
	const USnowRumbleMatchSubsystem* MatchSubsystem)
{
	if (!HasAuthority() || !MatchSubsystem)
	{
		return;
	}

	CurrentRoundNumber = MatchSubsystem->GetCurrentRoundNumber();
	RoundLimit = MatchSubsystem->GetRoundLimit();
	GameSpeed = MatchSubsystem->GetGameSpeed();
	bMatchEnded = MatchSubsystem->IsMatchComplete();
	bTiebreakerRound = MatchSubsystem->IsTiebreakerActive();
	MatchWinningTeam = bMatchEnded
		? MatchSubsystem->GetLeadingTeam()
		: ESnowRumbleTeam::None;
	CopyRoundWinsFromMatchSubsystem(MatchSubsystem);
	ForceNetUpdate();
}

bool ASnowRumbleGameState::IsMatchInputLocked() const
{
	return bRoundEnded
		|| !bStartCountdownActive
		|| GetSecondsUntilMatchStart() > 0.0f;
}

bool ASnowRumbleGameState::IsRoundEnded() const
{
	return bRoundEnded;
}

ESnowRumbleTeam ASnowRumbleGameState::GetRoundWinningTeam() const
{
	return RoundWinningTeam;
}

int32 ASnowRumbleGameState::GetTeamRoundWinCount(
	ESnowRumbleTeam Team) const
{
	switch (Team)
	{
	case ESnowRumbleTeam::Red:
		return RedTeamRoundWins;
	case ESnowRumbleTeam::Sky:
		return SkyTeamRoundWins;
	case ESnowRumbleTeam::Green:
		return GreenTeamRoundWins;
	case ESnowRumbleTeam::Yellow:
		return YellowTeamRoundWins;
	case ESnowRumbleTeam::Purple:
		return PurpleTeamRoundWins;
	case ESnowRumbleTeam::Pink:
		return PinkTeamRoundWins;
	case ESnowRumbleTeam::Blue:
		return BlueTeamRoundWins;
	case ESnowRumbleTeam::Orange:
		return WhiteTeamRoundWins;
	default:
		return 0;
	}
}

int32 ASnowRumbleGameState::GetCurrentRoundNumber() const
{
	return CurrentRoundNumber;
}

int32 ASnowRumbleGameState::GetRoundLimit() const
{
	return RoundLimit;
}

bool ASnowRumbleGameState::IsMatchEnded() const
{
	return bMatchEnded;
}

bool ASnowRumbleGameState::IsTiebreakerRound() const
{
	return bTiebreakerRound;
}

ESnowRumbleTeam ASnowRumbleGameState::GetMatchWinningTeam() const
{
	return MatchWinningTeam;
}

bool ASnowRumbleGameState::ShouldShowStartCountdown() const
{
	if (!bStartCountdownActive)
	{
		return false;
	}

	const float SecondsUntilStart = GetSecondsUntilMatchStart();
	return SecondsUntilStart > -1.0f;
}

FText ASnowRumbleGameState::GetStartCountdownText() const
{
	const float SecondsUntilStart = GetSecondsUntilMatchStart();
	if (SecondsUntilStart <= 0.0f)
	{
		return NSLOCTEXT("SnowRumble", "MatchStartCountdownGo", "시작!");
	}

	const int32 DisplaySeconds =
		FMath::Clamp(FMath::CeilToInt(SecondsUntilStart), 1, 3);
	return FText::AsNumber(DisplaySeconds);
}

float ASnowRumbleGameState::GetRoundElapsedSeconds() const
{
	if (!bStartCountdownActive || RoundStartServerTime <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Max(
		0.0f,
		GetServerWorldTimeSeconds() - RoundStartServerTime);
}

FText ASnowRumbleGameState::GetRoundElapsedTimeText() const
{
	return FText::Format(
		NSLOCTEXT("SnowRumble", "RoundElapsedTimeFormat", "경기 시간 {0}"),
		FormatSecondsAsClock(GetRoundElapsedSeconds()));
}

float ASnowRumbleGameState::GetSecondsUntilNextMapShrink() const
{
	if (bMapShrinkInProgress)
	{
		return 0.0f;
	}
	if (!bStartCountdownActive || NextMapShrinkServerTime <= 0.0f)
	{
		return MapShrinkWaitDurationSeconds;
	}

	return FMath::Max(
		0.0f,
		NextMapShrinkServerTime - GetServerWorldTimeSeconds());
}

FText ASnowRumbleGameState::GetMapShrinkCountdownText() const
{
	if (bMapShrinkInProgress)
	{
		return NSLOCTEXT(
			"SnowRumble",
			"MapShrinkInProgressText",
			"맵이 축소됩니다!");
	}

	const int32 DisplaySeconds =
		FMath::Max(0, FMath::CeilToInt(GetSecondsUntilNextMapShrink()));
	return FText::Format(
		NSLOCTEXT(
			"SnowRumble",
			"MapShrinkCountdownFormat",
			"{0}초 후 맵이 축소됩니다"),
		FText::AsNumber(DisplaySeconds));
}

bool ASnowRumbleGameState::IsMapShrinkInProgress() const
{
	return bMapShrinkInProgress;
}

ESnowRumbleGameSpeed ASnowRumbleGameState::GetGameSpeed() const
{
	return GameSpeed;
}

float ASnowRumbleGameState::GetMapShrinkIntervalSeconds() const
{
	return FMath::Max(1.0f, MapShrinkIntervalSeconds);
}

void ASnowRumbleGameState::SetMapShrinkIntervalSecondsFromServer(
	float IntervalSeconds)
{
	if (!HasAuthority())
	{
		return;
	}

	MapShrinkIntervalSeconds = FMath::Max(1.0f, IntervalSeconds);
	ForceNetUpdate();
}

void ASnowRumbleGameState::SetMapShrinkWaitDurationSecondsFromServer(
	float WaitDurationSeconds)
{
	if (!HasAuthority())
	{
		return;
	}

	MapShrinkWaitDurationSeconds = FMath::Max(1.0f, WaitDurationSeconds);
	ForceNetUpdate();
}

void ASnowRumbleGameState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASnowRumbleGameState, bStartCountdownActive);
	DOREPLIFETIME(ASnowRumbleGameState, MatchStartServerTime);
	DOREPLIFETIME(ASnowRumbleGameState, MatchStartCountdownSeconds);
	DOREPLIFETIME(ASnowRumbleGameState, RoundStartServerTime);
	DOREPLIFETIME(ASnowRumbleGameState, NextMapShrinkServerTime);
	DOREPLIFETIME(ASnowRumbleGameState, MapShrinkStartedServerTime);
	DOREPLIFETIME(ASnowRumbleGameState, MapShrinkDurationSeconds);
	DOREPLIFETIME(ASnowRumbleGameState, bMapShrinkInProgress);
	DOREPLIFETIME(ASnowRumbleGameState, GameSpeed);
	DOREPLIFETIME(ASnowRumbleGameState, MapShrinkIntervalSeconds);
	DOREPLIFETIME(ASnowRumbleGameState, MapShrinkWaitDurationSeconds);
	DOREPLIFETIME(ASnowRumbleGameState, bRoundEnded);
	DOREPLIFETIME(ASnowRumbleGameState, RoundWinningTeam);
	DOREPLIFETIME(ASnowRumbleGameState, RedTeamRoundWins);
	DOREPLIFETIME(ASnowRumbleGameState, SkyTeamRoundWins);
	DOREPLIFETIME(ASnowRumbleGameState, GreenTeamRoundWins);
	DOREPLIFETIME(ASnowRumbleGameState, YellowTeamRoundWins);
	DOREPLIFETIME(ASnowRumbleGameState, PurpleTeamRoundWins);
	DOREPLIFETIME(ASnowRumbleGameState, PinkTeamRoundWins);
	DOREPLIFETIME(ASnowRumbleGameState, BlueTeamRoundWins);
	DOREPLIFETIME(ASnowRumbleGameState, WhiteTeamRoundWins);
	DOREPLIFETIME(ASnowRumbleGameState, CurrentRoundNumber);
	DOREPLIFETIME(ASnowRumbleGameState, RoundLimit);
	DOREPLIFETIME(ASnowRumbleGameState, bMatchEnded);
	DOREPLIFETIME(ASnowRumbleGameState, bTiebreakerRound);
	DOREPLIFETIME(ASnowRumbleGameState, MatchWinningTeam);
}

float ASnowRumbleGameState::GetSecondsUntilMatchStart() const
{
	return MatchStartServerTime - GetServerWorldTimeSeconds();
}

FText ASnowRumbleGameState::FormatSecondsAsClock(float Seconds) const
{
	const int32 TotalSeconds = FMath::Max(0, FMath::FloorToInt(Seconds));
	const int32 Minutes = TotalSeconds / 60;
	const int32 RemainingSeconds = TotalSeconds % 60;
	return FText::FromString(FString::Printf(
		TEXT("%d:%02d"),
		Minutes,
		RemainingSeconds));
}

void ASnowRumbleGameState::OnRep_RoundResult()
{
	OnRoundResultChanged.Broadcast();
}

void ASnowRumbleGameState::CopyRoundWinsFromMatchSubsystem(
	const USnowRumbleMatchSubsystem* MatchSubsystem)
{
	if (!HasAuthority() || !MatchSubsystem)
	{
		return;
	}

	RedTeamRoundWins =
		MatchSubsystem->GetTeamRoundWinCount(ESnowRumbleTeam::Red);
	SkyTeamRoundWins =
		MatchSubsystem->GetTeamRoundWinCount(ESnowRumbleTeam::Sky);
	GreenTeamRoundWins =
		MatchSubsystem->GetTeamRoundWinCount(ESnowRumbleTeam::Green);
	YellowTeamRoundWins =
		MatchSubsystem->GetTeamRoundWinCount(ESnowRumbleTeam::Yellow);
	PurpleTeamRoundWins =
		MatchSubsystem->GetTeamRoundWinCount(ESnowRumbleTeam::Purple);
	PinkTeamRoundWins =
		MatchSubsystem->GetTeamRoundWinCount(ESnowRumbleTeam::Pink);
	BlueTeamRoundWins =
		MatchSubsystem->GetTeamRoundWinCount(ESnowRumbleTeam::Blue);
	WhiteTeamRoundWins =
		MatchSubsystem->GetTeamRoundWinCount(ESnowRumbleTeam::Orange);
}

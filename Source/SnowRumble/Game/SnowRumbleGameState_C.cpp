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
	bStartCountdownActive = true;
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
	bMatchEnded = MatchSubsystem->IsMatchComplete();
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
	case ESnowRumbleTeam::White:
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

void ASnowRumbleGameState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASnowRumbleGameState, bStartCountdownActive);
	DOREPLIFETIME(ASnowRumbleGameState, MatchStartServerTime);
	DOREPLIFETIME(ASnowRumbleGameState, MatchStartCountdownSeconds);
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
	DOREPLIFETIME(ASnowRumbleGameState, MatchWinningTeam);
}

float ASnowRumbleGameState::GetSecondsUntilMatchStart() const
{
	return MatchStartServerTime - GetServerWorldTimeSeconds();
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
		MatchSubsystem->GetTeamRoundWinCount(ESnowRumbleTeam::White);
}

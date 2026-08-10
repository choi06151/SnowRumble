// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleMatchSubsystem_C.h"

void USnowRumbleMatchSubsystem::BeginPvPMatch(
	int32 InRoundLimit,
	ESnowRumbleGameSpeed InGameSpeed,
	const TArray<FString>& InPvPLevelPaths)
{
	bPvPMatchActive = true;
	bMatchComplete = false;
	CurrentRoundNumber = 1;
	RoundLimit = NormalizeRoundLimit(InRoundLimit);
	GameSpeed = InGameSpeed;
	TeamRoundWins.Empty();
	PvPLevelPaths.Empty();
	LastSelectedPvPLevelPath.Empty();

	for (const FString& LevelPath : InPvPLevelPaths)
	{
		if (!LevelPath.IsEmpty())
		{
			PvPLevelPaths.AddUnique(LevelPath);
		}
	}
}

void USnowRumbleMatchSubsystem::ResetPvPMatch()
{
	bPvPMatchActive = false;
	bMatchComplete = false;
	CurrentRoundNumber = 1;
	RoundLimit = 1;
	GameSpeed = ESnowRumbleGameSpeed::Normal;
	PvPLevelPaths.Empty();
	LastSelectedPvPLevelPath.Empty();
	TeamRoundWins.Empty();
}

FString USnowRumbleMatchSubsystem::SelectNextPvPLevelPath(
	const FString& FallbackLevelPath)
{
	TArray<FString> CandidateLevelPaths = PvPLevelPaths;
	if (CandidateLevelPaths.IsEmpty() && !FallbackLevelPath.IsEmpty())
	{
		CandidateLevelPaths.Add(FallbackLevelPath);
	}
	if (CandidateLevelPaths.IsEmpty())
	{
		return FString();
	}

	if (CandidateLevelPaths.Num() > 1)
	{
		CandidateLevelPaths.Remove(LastSelectedPvPLevelPath);
	}

	const FString SelectedLevelPath = CandidateLevelPaths[
		FMath::RandRange(0, CandidateLevelPaths.Num() - 1)];
	LastSelectedPvPLevelPath = SelectedLevelPath;
	return SelectedLevelPath;
}

bool USnowRumbleMatchSubsystem::RecordRoundWin(ESnowRumbleTeam WinningTeam)
{
	if (!bPvPMatchActive || bMatchComplete || !IsValidTeam(WinningTeam))
	{
		return bMatchComplete;
	}

	int32& RoundWins = TeamRoundWins.FindOrAdd(WinningTeam);
	++RoundWins;

	if (CurrentRoundNumber >= RoundLimit)
	{
		bMatchComplete = true;
	}

	return bMatchComplete;
}

void USnowRumbleMatchSubsystem::AdvanceToNextRound()
{
	if (!bPvPMatchActive || bMatchComplete)
	{
		return;
	}

	CurrentRoundNumber = FMath::Clamp(
		CurrentRoundNumber + 1,
		1,
		RoundLimit);
}

bool USnowRumbleMatchSubsystem::IsPvPMatchActive() const
{
	return bPvPMatchActive;
}

int32 USnowRumbleMatchSubsystem::GetCurrentRoundNumber() const
{
	return CurrentRoundNumber;
}

int32 USnowRumbleMatchSubsystem::GetRoundLimit() const
{
	return RoundLimit;
}

ESnowRumbleGameSpeed USnowRumbleMatchSubsystem::GetGameSpeed() const
{
	return GameSpeed;
}

float USnowRumbleMatchSubsystem::GetMapShrinkIntervalSeconds(
	ESnowRumbleGameSpeed InGameSpeed)
{
	switch (InGameSpeed)
	{
	case ESnowRumbleGameSpeed::Slow:
		return 90.0f;
	case ESnowRumbleGameSpeed::Fast:
		return 30.0f;
	case ESnowRumbleGameSpeed::Normal:
	default:
		return 60.0f;
	}
}

int32 USnowRumbleMatchSubsystem::GetTeamRoundWinCount(
	ESnowRumbleTeam Team) const
{
	if (const int32* RoundWins = TeamRoundWins.Find(Team))
	{
		return *RoundWins;
	}

	return 0;
}

bool USnowRumbleMatchSubsystem::IsMatchComplete() const
{
	return bMatchComplete;
}

ESnowRumbleTeam USnowRumbleMatchSubsystem::GetLeadingTeam() const
{
	ESnowRumbleTeam LeadingTeam = ESnowRumbleTeam::None;
	int32 HighestRoundWins = 0;
	bool bHasTie = false;

	for (const TPair<ESnowRumbleTeam, int32>& TeamRoundWin : TeamRoundWins)
	{
		if (TeamRoundWin.Value > HighestRoundWins)
		{
			HighestRoundWins = TeamRoundWin.Value;
			LeadingTeam = TeamRoundWin.Key;
			bHasTie = false;
		}
		else if (TeamRoundWin.Value == HighestRoundWins
			&& HighestRoundWins > 0)
		{
			bHasTie = true;
		}
	}

	return bHasTie ? ESnowRumbleTeam::None : LeadingTeam;
}

int32 USnowRumbleMatchSubsystem::NormalizeRoundLimit(int32 InRoundLimit) const
{
	if (InRoundLimit <= 1)
	{
		return 1;
	}
	if (InRoundLimit <= 3)
	{
		return 3;
	}
	return 5;
}

bool USnowRumbleMatchSubsystem::IsValidTeam(ESnowRumbleTeam Team) const
{
	return Team != ESnowRumbleTeam::None;
}

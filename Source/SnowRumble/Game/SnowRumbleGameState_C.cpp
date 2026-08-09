// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleGameState_C.h"

#include "Net/UnrealNetwork.h"

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

void ASnowRumbleGameState::EndRoundFromServer(ESnowRumbleTeam WinningTeam)
{
	if (!HasAuthority() || bRoundEnded || WinningTeam == ESnowRumbleTeam::None)
	{
		return;
	}

	bRoundEnded = true;
	RoundWinningTeam = WinningTeam;
	OnRep_RoundResult();
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
}

float ASnowRumbleGameState::GetSecondsUntilMatchStart() const
{
	return MatchStartServerTime - GetServerWorldTimeSeconds();
}

void ASnowRumbleGameState::OnRep_RoundResult()
{
	OnRoundResultChanged.Broadcast();
}

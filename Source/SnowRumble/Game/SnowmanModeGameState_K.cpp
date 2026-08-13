// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowmanModeGameState_K.h"

#include "Net/UnrealNetwork.h"

void ASnowmanModeGameState::StartSnowmanModeCountdownFromServer(
	float CountdownSeconds)
{
	if (!HasAuthority())
	{
		return;
	}

	SnowmanModeStartCountdownSeconds = FMath::Max(0.0f, CountdownSeconds);
	SnowmanModeMatchStartServerTime =
		GetServerWorldTimeSeconds() + SnowmanModeStartCountdownSeconds;
	bSnowmanModeCountdownActive = true;
	bSnowmanModeTimerActive = false;
	ForceNetUpdate();
}

void ASnowmanModeGameState::StartSnowmanModeTimerFromServer(
	float InTimeLimitSeconds)
{
	if (!HasAuthority())
	{
		return;
	}

	SnowmanModeTimeLimitSeconds = FMath::Max(0.0f, InTimeLimitSeconds);
	SnowmanModeStartServerTime = GetServerWorldTimeSeconds();
	bSnowmanModeTimerActive = true;
	ForceNetUpdate();
}

bool ASnowmanModeGameState::IsSnowmanModeInputLocked() const
{
	return !bSnowmanModeTimerActive
		&& (!bSnowmanModeCountdownActive
			|| GetSecondsUntilSnowmanModeStart() > 0.0f);
}

bool ASnowmanModeGameState::ShouldShowSnowmanModeStartCountdown() const
{
	if (!bSnowmanModeCountdownActive)
	{
		return false;
	}

	const float SecondsUntilStart = GetSecondsUntilSnowmanModeStart();
	return SecondsUntilStart > -1.0f;
}

FText ASnowmanModeGameState::GetSnowmanModeStartCountdownText() const
{
	const float SecondsUntilStart = GetSecondsUntilSnowmanModeStart();
	if (SecondsUntilStart <= 0.0f)
	{
		return NSLOCTEXT("SnowRumble", "SnowmanModeStartCountdownGo", "시작!");
	}

	const int32 DisplaySeconds =
		FMath::Clamp(FMath::CeilToInt(SecondsUntilStart), 1, 3);
	return FText::AsNumber(DisplaySeconds);
}

bool ASnowmanModeGameState::IsSnowmanModeTimerActive() const
{
	return bSnowmanModeTimerActive;
}

float ASnowmanModeGameState::GetSnowmanModeTimeLimitSeconds() const
{
	return SnowmanModeTimeLimitSeconds;
}

float ASnowmanModeGameState::GetSnowmanModeRemainingSeconds() const
{
	if (!bSnowmanModeTimerActive)
	{
		return SnowmanModeTimeLimitSeconds;
	}

	const float ElapsedSeconds =
		GetServerWorldTimeSeconds() - SnowmanModeStartServerTime;
	return FMath::Max(0.0f, SnowmanModeTimeLimitSeconds - ElapsedSeconds);
}

float ASnowmanModeGameState::GetSnowmanModeElapsedSeconds() const
{
	if (!bSnowmanModeTimerActive)
	{
		return 0.0f;
	}

	return FMath::Max(
		0.0f,
		GetServerWorldTimeSeconds() - SnowmanModeStartServerTime);
}

FText ASnowmanModeGameState::GetSnowmanModeRemainingTimeText() const
{
	return FormatSecondsAsClock(GetSnowmanModeRemainingSeconds());
}

FText ASnowmanModeGameState::GetSnowmanModeElapsedTimeText() const
{
	return FText::Format(
		NSLOCTEXT("SnowRumble", "SnowmanModeElapsedTimeFormat", "경기 시간 {0}"),
		FormatSecondsAsClock(GetSnowmanModeElapsedSeconds()));
}

void ASnowmanModeGameState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASnowmanModeGameState, bSnowmanModeTimerActive);
	DOREPLIFETIME(ASnowmanModeGameState, bSnowmanModeCountdownActive);
	DOREPLIFETIME(ASnowmanModeGameState, SnowmanModeMatchStartServerTime);
	DOREPLIFETIME(ASnowmanModeGameState, SnowmanModeStartCountdownSeconds);
	DOREPLIFETIME(ASnowmanModeGameState, SnowmanModeStartServerTime);
	DOREPLIFETIME(ASnowmanModeGameState, SnowmanModeTimeLimitSeconds);
}

float ASnowmanModeGameState::GetSecondsUntilSnowmanModeStart() const
{
	return SnowmanModeMatchStartServerTime - GetServerWorldTimeSeconds();
}

FText ASnowmanModeGameState::FormatSecondsAsClock(float Seconds) const
{
	const int32 TotalSeconds = FMath::Max(0, FMath::CeilToInt(Seconds));
	const int32 Minutes = TotalSeconds / 60;
	const int32 RemainingSeconds = TotalSeconds % 60;
	return FText::FromString(FString::Printf(
		TEXT("%d:%02d"),
		Minutes,
		RemainingSeconds));
}

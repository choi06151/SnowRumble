// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowmanModeGameState_K.h"

#include "Net/UnrealNetwork.h"

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

FText ASnowmanModeGameState::GetSnowmanModeRemainingTimeText() const
{
	return FormatSecondsAsClock(GetSnowmanModeRemainingSeconds());
}

void ASnowmanModeGameState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASnowmanModeGameState, bSnowmanModeTimerActive);
	DOREPLIFETIME(ASnowmanModeGameState, SnowmanModeStartServerTime);
	DOREPLIFETIME(ASnowmanModeGameState, SnowmanModeTimeLimitSeconds);
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

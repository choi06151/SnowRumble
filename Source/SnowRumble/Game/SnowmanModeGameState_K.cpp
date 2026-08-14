// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowmanModeGameState_K.h"

#include "SnowRumblePlayerState.h"
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

void ASnowmanModeGameState::ResetSnowmanModePlayersFromServer(
	const TArray<ASnowRumblePlayerState*>& PlayerStates)
{
	if (!HasAuthority())
	{
		return;
	}

	SnowmanModePlayerEntries.Reset();
	for (ASnowRumblePlayerState* PlayerState : PlayerStates)
	{
		if (!PlayerState)
		{
			continue;
		}

		FSnowmanModePlayerEntry NewEntry;
		NewEntry.PlayerState = PlayerState;
		SnowmanModePlayerEntries.Add(NewEntry);
	}

	ForceNetUpdate();
}

void ASnowmanModeGameState::SetSnowmanPlayerFromServer(
	ASnowRumblePlayerState* PlayerState)
{
	if (!HasAuthority() || !PlayerState)
	{
		return;
	}

	const int32 EntryIndex = FindSnowmanModePlayerEntryIndex(PlayerState);
	if (EntryIndex == INDEX_NONE)
	{
		FSnowmanModePlayerEntry NewEntry;
		NewEntry.PlayerState = PlayerState;
		NewEntry.Role = ESnowmanModePlayerRole::Snowman;
		SnowmanModePlayerEntries.Add(NewEntry);
	}
	else
	{
		SnowmanModePlayerEntries[EntryIndex].Role =
			ESnowmanModePlayerRole::Snowman;
		SnowmanModePlayerEntries[EntryIndex].InfectionCompleteServerTime = 0.0f;
	}

	ForceNetUpdate();
}

bool ASnowmanModeGameState::StartInfectionPendingFromServer(
	ASnowRumblePlayerState* PlayerState,
	float InfectionDelaySeconds)
{
	if (!HasAuthority() || !PlayerState)
	{
		return false;
	}

	const int32 EntryIndex = FindSnowmanModePlayerEntryIndex(PlayerState);
	if (EntryIndex == INDEX_NONE)
	{
		return false;
	}

	FSnowmanModePlayerEntry& Entry = SnowmanModePlayerEntries[EntryIndex];
	if (Entry.Role != ESnowmanModePlayerRole::Normal)
	{
		return false;
	}

	Entry.Role = ESnowmanModePlayerRole::InfectionPending;
	Entry.InfectionCompleteServerTime =
		GetServerWorldTimeSeconds() + FMath::Max(0.0f, InfectionDelaySeconds);
	ForceNetUpdate();
	return true;
}

void ASnowmanModeGameState::CompleteInfectionFromServer(
	ASnowRumblePlayerState* PlayerState)
{
	SetSnowmanPlayerFromServer(PlayerState);
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

ESnowmanModePlayerRole ASnowmanModeGameState::GetSnowmanModePlayerRole(
	const APlayerState* PlayerState) const
{
	const int32 EntryIndex = FindSnowmanModePlayerEntryIndex(PlayerState);
	return EntryIndex == INDEX_NONE
		? ESnowmanModePlayerRole::Normal
		: SnowmanModePlayerEntries[EntryIndex].Role;
}

bool ASnowmanModeGameState::IsSnowmanModePlayerSnowman(
	const APlayerState* PlayerState) const
{
	return GetSnowmanModePlayerRole(PlayerState)
		== ESnowmanModePlayerRole::Snowman;
}

bool ASnowmanModeGameState::IsSnowmanModePlayerInfectionPending(
	const APlayerState* PlayerState) const
{
	return GetSnowmanModePlayerRole(PlayerState)
		== ESnowmanModePlayerRole::InfectionPending;
}

float ASnowmanModeGameState::GetSnowmanModeInfectionRemainingSeconds(
	const APlayerState* PlayerState) const
{
	const int32 EntryIndex = FindSnowmanModePlayerEntryIndex(PlayerState);
	if (EntryIndex == INDEX_NONE
		|| SnowmanModePlayerEntries[EntryIndex].Role
			!= ESnowmanModePlayerRole::InfectionPending)
	{
		return 0.0f;
	}

	return FMath::Max(
		0.0f,
		SnowmanModePlayerEntries[EntryIndex].InfectionCompleteServerTime
			- GetServerWorldTimeSeconds());
}

const TArray<FSnowmanModePlayerEntry>&
ASnowmanModeGameState::GetSnowmanModePlayerEntries() const
{
	return SnowmanModePlayerEntries;
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
	DOREPLIFETIME(ASnowmanModeGameState, SnowmanModePlayerEntries);
}

int32 ASnowmanModeGameState::FindSnowmanModePlayerEntryIndex(
	const APlayerState* PlayerState) const
{
	if (!PlayerState)
	{
		return INDEX_NONE;
	}

	for (int32 EntryIndex = 0;
		EntryIndex < SnowmanModePlayerEntries.Num();
		++EntryIndex)
	{
		if (SnowmanModePlayerEntries[EntryIndex].PlayerState == PlayerState)
		{
			return EntryIndex;
		}
	}

	const int32 PlayerId = PlayerState->GetPlayerId();
	if (PlayerId != INDEX_NONE)
	{
		for (int32 EntryIndex = 0;
			EntryIndex < SnowmanModePlayerEntries.Num();
			++EntryIndex)
		{
			const APlayerState* EntryPlayerState =
				SnowmanModePlayerEntries[EntryIndex].PlayerState;
			if (EntryPlayerState
				&& EntryPlayerState->GetPlayerId() == PlayerId)
			{
				return EntryIndex;
			}
		}
	}

	const FUniqueNetIdRepl& UniqueId = PlayerState->GetUniqueId();
	if (UniqueId.IsValid())
	{
		for (int32 EntryIndex = 0;
			EntryIndex < SnowmanModePlayerEntries.Num();
			++EntryIndex)
		{
			const APlayerState* EntryPlayerState =
				SnowmanModePlayerEntries[EntryIndex].PlayerState;
			if (EntryPlayerState
				&& EntryPlayerState->GetUniqueId().IsValid()
				&& EntryPlayerState->GetUniqueId() == UniqueId)
			{
				return EntryIndex;
			}
		}
	}

	return INDEX_NONE;
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

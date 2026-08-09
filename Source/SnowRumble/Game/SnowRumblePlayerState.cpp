// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumblePlayerState.h"

#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "SnowRumbleLobbyGameMode.h"
#include "SnowRumbleLobbyGameState.h"

namespace
{
bool IsValidLobbyTeam(ESnowRumbleTeam Team)
{
	switch (Team)
	{
	case ESnowRumbleTeam::Red:
	case ESnowRumbleTeam::Sky:
	case ESnowRumbleTeam::Green:
	case ESnowRumbleTeam::Yellow:
	case ESnowRumbleTeam::Purple:
	case ESnowRumbleTeam::Pink:
	case ESnowRumbleTeam::Blue:
	case ESnowRumbleTeam::White:
		return true;
	default:
		return false;
	}
}
}

FString ASnowRumblePlayerState::GetLobbyPlayerName() const
{
	return LobbyPlayerName.IsEmpty()
		? GetPlayerName()
		: LobbyPlayerName;
}

ESnowRumbleTeam ASnowRumblePlayerState::GetLobbyTeam() const
{
	return LobbyTeam;
}

FLinearColor ASnowRumblePlayerState::GetLobbyTeamColor() const
{
	switch (LobbyTeam)
	{
	case ESnowRumbleTeam::Red:
		return FLinearColor(0.95f, 0.08f, 0.08f, 1.0f);
	case ESnowRumbleTeam::Sky:
		return FLinearColor(0.2f, 0.75f, 1.0f, 1.0f);
	case ESnowRumbleTeam::Green:
		return FLinearColor(0.1f, 0.8f, 0.25f, 1.0f);
	case ESnowRumbleTeam::Yellow:
		return FLinearColor(1.0f, 0.82f, 0.05f, 1.0f);
	case ESnowRumbleTeam::Purple:
		return FLinearColor(0.55f, 0.25f, 1.0f, 1.0f);
	case ESnowRumbleTeam::Pink:
		return FLinearColor(1.0f, 0.25f, 0.65f, 1.0f);
	case ESnowRumbleTeam::Blue:
		return FLinearColor(0.05f, 0.25f, 1.0f, 1.0f);
	case ESnowRumbleTeam::White:
		return FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
	default:
		return FLinearColor::White;
	}
}

bool ASnowRumblePlayerState::IsLobbyReady() const
{
	return bLobbyReady;
}

bool ASnowRumblePlayerState::IsLobbyHost() const
{
	return bLobbyHost;
}

void ASnowRumblePlayerState::AssignLobbyTeamFromServer(ESnowRumbleTeam NewTeam)
{
	if (!HasAuthority())
	{
		return;
	}

	LobbyTeam = IsValidLobbyTeam(NewTeam)
		? NewTeam
		: ESnowRumbleTeam::None;
	bLobbyReady = false;
	BroadcastLobbyPlayerChanged();
}

void ASnowRumblePlayerState::AssignLobbyHostFromServer(bool bNewLobbyHost)
{
	if (!HasAuthority() || bLobbyHost == bNewLobbyHost)
	{
		return;
	}

	bLobbyHost = bNewLobbyHost;
	BroadcastLobbyPlayerChanged();
}

void ASnowRumblePlayerState::RequestSetLobbyPlayerName(
	const FString& NewName)
{
	if (HasAuthority())
	{
		ServerSetLobbyPlayerName_Implementation(NewName);
	}
	else
	{
		ServerSetLobbyPlayerName(NewName);
	}
}

void ASnowRumblePlayerState::RequestSetLobbyTeam(ESnowRumbleTeam NewTeam)
{
	if (HasAuthority())
	{
		ServerSetLobbyTeam_Implementation(NewTeam);
	}
	else
	{
		ServerSetLobbyTeam(NewTeam);
	}
}

void ASnowRumblePlayerState::RequestSetLobbyReady(bool bNewReady)
{
	if (HasAuthority())
	{
		ServerSetLobbyReady_Implementation(bNewReady);
	}
	else
	{
		ServerSetLobbyReady(bNewReady);
	}
}

void ASnowRumblePlayerState::RequestStartLobbyMatch()
{
	if (HasAuthority())
	{
		ServerRequestStartLobbyMatch_Implementation();
	}
	else
	{
		ServerRequestStartLobbyMatch();
	}
}

void ASnowRumblePlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	ASnowRumblePlayerState* TargetPlayerState =
		Cast<ASnowRumblePlayerState>(PlayerState);
	if (!TargetPlayerState)
	{
		return;
	}

	TargetPlayerState->LobbyPlayerName = LobbyPlayerName;
	TargetPlayerState->LobbyTeam = LobbyTeam;
	TargetPlayerState->bLobbyReady = bLobbyReady;
	TargetPlayerState->bLobbyHost = bLobbyHost;
	TargetPlayerState->SetPlayerName(GetLobbyPlayerName());
}

void ASnowRumblePlayerState::OverrideWith(APlayerState* PlayerState)
{
	Super::OverrideWith(PlayerState);

	const ASnowRumblePlayerState* SourcePlayerState =
		Cast<ASnowRumblePlayerState>(PlayerState);
	if (!SourcePlayerState)
	{
		return;
	}

	LobbyPlayerName = SourcePlayerState->LobbyPlayerName;
	LobbyTeam = SourcePlayerState->LobbyTeam;
	bLobbyReady = SourcePlayerState->bLobbyReady;
	bLobbyHost = SourcePlayerState->bLobbyHost;
	SetPlayerName(GetLobbyPlayerName());
	BroadcastLobbyPlayerChanged();
}

void ASnowRumblePlayerState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASnowRumblePlayerState, LobbyPlayerName);
	DOREPLIFETIME(ASnowRumblePlayerState, LobbyTeam);
	DOREPLIFETIME(ASnowRumblePlayerState, bLobbyReady);
	DOREPLIFETIME(ASnowRumblePlayerState, bLobbyHost);
}

void ASnowRumblePlayerState::OnRep_LobbyPlayerName()
{
	BroadcastLobbyPlayerChanged();
}

void ASnowRumblePlayerState::OnRep_LobbyTeam()
{
	BroadcastLobbyPlayerChanged();
}

void ASnowRumblePlayerState::OnRep_LobbyReady()
{
	BroadcastLobbyPlayerChanged();
}

void ASnowRumblePlayerState::OnRep_LobbyHost()
{
	BroadcastLobbyPlayerChanged();
}

void ASnowRumblePlayerState::ServerSetLobbyPlayerName_Implementation(
	const FString& NewName)
{
	LobbyPlayerName = SanitizeLobbyPlayerName(NewName);
	SetPlayerName(LobbyPlayerName);
	BroadcastLobbyPlayerChanged();
}

void ASnowRumblePlayerState::ServerSetLobbyTeam_Implementation(
	ESnowRumbleTeam NewTeam)
{
	if (!IsValidLobbyTeam(NewTeam))
	{
		return;
	}

	LobbyTeam = NewTeam;
	bLobbyReady = false;
	BroadcastLobbyPlayerChanged();
}

void ASnowRumblePlayerState::ServerSetLobbyReady_Implementation(
	bool bNewReady)
{
	bLobbyReady = bNewReady && LobbyTeam != ESnowRumbleTeam::None;
	BroadcastLobbyPlayerChanged();
}

void ASnowRumblePlayerState::ServerRequestStartLobbyMatch_Implementation()
{
	UWorld* World = GetWorld();
	ASnowRumbleLobbyGameMode* LobbyGameMode =
		World ? World->GetAuthGameMode<ASnowRumbleLobbyGameMode>() : nullptr;
	APlayerController* RequestingController =
		Cast<APlayerController>(GetOwner());
	if (LobbyGameMode)
	{
		LobbyGameMode->RequestStartMatch(RequestingController);
	}
}

void ASnowRumblePlayerState::BroadcastLobbyPlayerChanged()
{
	OnLobbyPlayerChanged.Broadcast();

	if (UWorld* World = GetWorld())
	{
		if (ASnowRumbleLobbyGameState* LobbyGameState =
			World->GetGameState<ASnowRumbleLobbyGameState>())
		{
			LobbyGameState->NotifyLobbyStateChanged();
		}
	}
}

FString ASnowRumblePlayerState::SanitizeLobbyPlayerName(
	const FString& NewName) const
{
	FString SanitizedName = NewName.TrimStartAndEnd();
	constexpr int32 MaximumNameLength = 16;
	if (SanitizedName.Len() > MaximumNameLength)
	{
		SanitizedName.LeftInline(MaximumNameLength);
	}

	return SanitizedName.IsEmpty()
		? GetPlayerName()
		: SanitizedName;
}

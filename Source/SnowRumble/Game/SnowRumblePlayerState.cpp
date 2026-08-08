// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumblePlayerState.h"

#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "SnowRumbleLobbyGameMode.h"
#include "SnowRumbleLobbyGameState.h"

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

bool ASnowRumblePlayerState::IsLobbyReady() const
{
	return bLobbyReady;
}

void ASnowRumblePlayerState::AssignLobbyTeamFromServer(ESnowRumbleTeam NewTeam)
{
	if (!HasAuthority())
	{
		return;
	}

	LobbyTeam =
		NewTeam == ESnowRumbleTeam::Red || NewTeam == ESnowRumbleTeam::Blue
			? NewTeam
			: ESnowRumbleTeam::None;
	bLobbyReady = false;
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

void ASnowRumblePlayerState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASnowRumblePlayerState, LobbyPlayerName);
	DOREPLIFETIME(ASnowRumblePlayerState, LobbyTeam);
	DOREPLIFETIME(ASnowRumblePlayerState, bLobbyReady);
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
	// MVP에서는 직접 팀 선택을 제공하지 않고 서버의 랜덤 배정만 사용한다.
	(void)NewTeam;
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

// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumblePlayerState.h"

#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "SnowRumbleLobbyGameMode.h"
#include "SnowRumbleLobbyGameState.h"
#include "../Player/SnowRumbleCustomizationSubsystem_C.h"
#include "../Player/LocalPlayerIdentitySubsystem_C.h"
#include "../UI/SnowRumblePlayerController.h"

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
	case ESnowRumbleTeam::Orange:
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
	case ESnowRumbleTeam::Orange:
		return FLinearColor(1.0f, 0.35f, 0.05f, 1.0f);
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

bool ASnowRumblePlayerState::IsVoiceSpeaking() const
{
	return bVoiceSpeaking;
}

ESnowRumbleVoiceChannel ASnowRumblePlayerState::GetVoiceChannel() const
{
	return VoiceChannel;
}

FSnowRumbleCustomizationData
ASnowRumblePlayerState::GetCustomizationData() const
{
	return CustomizationData;
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

void ASnowRumblePlayerState::SetVoiceSpeakingFromServer(
	bool bNewVoiceSpeaking)
{
	if (!HasAuthority() || bVoiceSpeaking == bNewVoiceSpeaking)
	{
		return;
	}

	bVoiceSpeaking = bNewVoiceSpeaking;
}

void ASnowRumblePlayerState::SetVoiceChannelFromServer(
	ESnowRumbleVoiceChannel NewVoiceChannel)
{
	if (!HasAuthority() || VoiceChannel == NewVoiceChannel)
	{
		return;
	}

	VoiceChannel = NewVoiceChannel;
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

void ASnowRumblePlayerState::RequestSetCustomizationData(
	const FSnowRumbleCustomizationData& NewData)
{
	if (HasAuthority())
	{
		ServerSetCustomizationData_Implementation(NewData);
	}
	else
	{
		ServerSetCustomizationData(NewData);
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
	TargetPlayerState->bVoiceSpeaking = false;
	TargetPlayerState->VoiceChannel = VoiceChannel;
	TargetPlayerState->CustomizationData = CustomizationData;
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
	bVoiceSpeaking = false;
	VoiceChannel = SourcePlayerState->VoiceChannel;
	CustomizationData = SourcePlayerState->CustomizationData;
	SetPlayerName(GetLobbyPlayerName());
	BroadcastLobbyPlayerChanged();
	OnCustomizationChanged.Broadcast();
}

void ASnowRumblePlayerState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASnowRumblePlayerState, LobbyPlayerName);
	DOREPLIFETIME(ASnowRumblePlayerState, LobbyTeam);
	DOREPLIFETIME(ASnowRumblePlayerState, bLobbyReady);
	DOREPLIFETIME(ASnowRumblePlayerState, bLobbyHost);
	DOREPLIFETIME(ASnowRumblePlayerState, bVoiceSpeaking);
	DOREPLIFETIME(ASnowRumblePlayerState, VoiceChannel);
	DOREPLIFETIME(ASnowRumblePlayerState, CustomizationData);
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

void ASnowRumblePlayerState::OnRep_VoiceSpeaking()
{
}

void ASnowRumblePlayerState::OnRep_VoiceChannel()
{
}

void ASnowRumblePlayerState::OnRep_CustomizationData()
{
	OnCustomizationChanged.Broadcast();
}

void ASnowRumblePlayerState::ServerSetLobbyPlayerName_Implementation(
	const FString& NewName)
{
	if (!ULocalPlayerIdentitySubsystem::IsPlayerNameAllowed(NewName))
	{
		return;
	}

	LobbyPlayerName = SanitizeLobbyPlayerName(NewName);
	SetPlayerName(LobbyPlayerName);
	BroadcastLobbyPlayerChanged();

	if (!bLobbyJoinAnnouncementSent)
	{
		bLobbyJoinAnnouncementSent = true;
		BroadcastLobbyEventLogMessage(FText::Format(
			NSLOCTEXT(
				"SnowRumble",
				"LobbyEventPlayerJoined",
				"{0}님이 접속했습니다"),
			FText::FromString(GetLobbyPlayerName())));
	}
}

void ASnowRumblePlayerState::ServerSetLobbyTeam_Implementation(
	ESnowRumbleTeam NewTeam)
{
	if (!IsValidLobbyTeam(NewTeam))
	{
		return;
	}
	if (bLobbyReady && LobbyTeam != NewTeam)
	{
		return;
	}

	LobbyTeam = NewTeam;
	BroadcastLobbyPlayerChanged();
}

void ASnowRumblePlayerState::ServerSetLobbyReady_Implementation(
	bool bNewReady)
{
	const bool bWasReady = bLobbyReady;
	bLobbyReady = bNewReady && LobbyTeam != ESnowRumbleTeam::None;
	BroadcastLobbyPlayerChanged();

	if (!bWasReady && bLobbyReady)
	{
		BroadcastLobbyEventLogMessage(FText::Format(
			NSLOCTEXT(
				"SnowRumble",
				"LobbyEventPlayerReady",
				"{0}님이 준비 완료했습니다"),
			FText::FromString(GetLobbyPlayerName())));
	}
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

void ASnowRumblePlayerState::ServerSetCustomizationData_Implementation(
	const FSnowRumbleCustomizationData& NewData)
{
	const FSnowRumbleCustomizationData SanitizedData =
		SanitizeCustomizationData(NewData);
	if (CustomizationData == SanitizedData)
	{
		return;
	}

	CustomizationData = SanitizedData;
	OnCustomizationChanged.Broadcast();
	ForceNetUpdate();
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

void ASnowRumblePlayerState::BroadcastLobbyEventLogMessage(
	const FText& Message) const
{
	UWorld* World = GetWorld();
	if (!World || Message.IsEmpty())
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator();
		It;
		++It)
	{
		if (ASnowRumblePlayerController* PlayerController =
			Cast<ASnowRumblePlayerController>(It->Get()))
		{
			PlayerController->ClientReceiveEventLogMessage(Message);
		}
	}
}

FString ASnowRumblePlayerState::SanitizeLobbyPlayerName(
	const FString& NewName) const
{
	FString SanitizedName =
		ULocalPlayerIdentitySubsystem::SanitizePlayerName(NewName);
	return SanitizedName.IsEmpty()
		? GetPlayerName()
		: SanitizedName;
}

FSnowRumbleCustomizationData
ASnowRumblePlayerState::SanitizeCustomizationData(
	const FSnowRumbleCustomizationData& NewData) const
{
	return USnowRumbleCustomizationSubsystem::SanitizeCustomizationData(NewData);
}

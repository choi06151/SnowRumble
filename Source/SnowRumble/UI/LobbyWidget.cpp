// Copyright Epic Games, Inc. All Rights Reserved.

#include "LobbyWidget.h"

#include "../Game/SnowRumbleLobbyGameState.h"
#include "../Online/SnowRumbleSessionSubsystem.h"
#include "../Player/LocalPlayerIdentitySubsystem_C.h"
#include "LobbyPlayerController.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshLobbyBindings();
	ApplyLocalPlayerIdentity();
	RefreshRoomCodeText();
	OnLobbyStateChanged();
}

void ULobbyWidget::NativeDestruct()
{
	UnbindLobbyBindings();

	Super::NativeDestruct();
}

void ULobbyWidget::NativeTick(
	const FGeometry& MyGeometry,
	float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshLobbyBindings();
	ApplyLocalPlayerIdentity();
	RefreshRoomCodeText();
}

TArray<ASnowRumblePlayerState*> ULobbyWidget::GetLobbyPlayers() const
{
	if (ASnowRumbleLobbyGameState* LobbyGameState = GetLobbyGameState())
	{
		return LobbyGameState->GetLobbyPlayers();
	}
	return {};
}

void ULobbyWidget::RequestSetLocalPlayerName(const FString& NewName)
{
	if (ASnowRumblePlayerState* PlayerState =
		GetLocalSnowRumblePlayerState())
	{
		PlayerState->RequestSetLobbyPlayerName(NewName);
	}
}

void ULobbyWidget::RequestSetLocalPlayerTeam(ESnowRumbleTeam NewTeam)
{
	if (ASnowRumblePlayerState* PlayerState =
		GetLocalSnowRumblePlayerState())
	{
		PlayerState->RequestSetLobbyTeam(NewTeam);
	}
}

void ULobbyWidget::RequestSetLocalPlayerReady(bool bNewReady)
{
	if (ASnowRumblePlayerState* PlayerState =
		GetLocalSnowRumblePlayerState())
	{
		PlayerState->RequestSetLobbyReady(bNewReady);
	}
}

void ULobbyWidget::RequestStartMatch()
{
	if (ASnowRumblePlayerState* PlayerState =
		GetLocalSnowRumblePlayerState())
	{
		PlayerState->RequestStartLobbyMatch();
	}
}

bool ULobbyWidget::IsLocalPlayerHost() const
{
	const APlayerController* PlayerController = GetOwningPlayer();
	return PlayerController && PlayerController->IsLocalController()
		&& PlayerController->HasAuthority();
}

bool ULobbyWidget::CanStartMatch() const
{
	const ASnowRumbleLobbyGameState* LobbyGameState = GetLobbyGameState();
	return LobbyGameState && LobbyGameState->CanStartLobbyMatch();
}

FString ULobbyWidget::GetCurrentRoomCode() const
{
	UGameInstance* GameInstance = GetGameInstance();
	const USnowRumbleSessionSubsystem* SessionSubsystem = GameInstance
		? GameInstance->GetSubsystem<USnowRumbleSessionSubsystem>()
		: nullptr;
	return SessionSubsystem ? SessionSubsystem->GetCurrentRoomCode() : FString();
}

ASnowRumblePlayerState* ULobbyWidget::GetLocalSnowRumblePlayerState() const
{
	const APlayerController* PlayerController = GetOwningPlayer();
	return PlayerController
		? PlayerController->GetPlayerState<ASnowRumblePlayerState>()
		: nullptr;
}

ASnowRumbleLobbyGameState* ULobbyWidget::GetLobbyGameState() const
{
	UWorld* World = GetWorld();
	return World ? World->GetGameState<ASnowRumbleLobbyGameState>() : nullptr;
}

void ULobbyWidget::ApplyLocalPlayerIdentity()
{
	ASnowRumblePlayerState* PlayerState = GetLocalSnowRumblePlayerState();
	if (!PlayerState)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	const ULocalPlayerIdentitySubsystem* IdentitySubsystem = GameInstance
		? GameInstance->GetSubsystem<ULocalPlayerIdentitySubsystem>()
		: nullptr;
	if (!IdentitySubsystem || !IdentitySubsystem->HasDesiredPlayerName())
	{
		return;
	}

	const FString DesiredPlayerName = IdentitySubsystem->GetDesiredPlayerName();
	if (PlayerState->GetLobbyPlayerName() == DesiredPlayerName)
	{
		IdentityAppliedPlayerState = PlayerState;
		return;
	}

	const UWorld* World = GetWorld();
	const double CurrentTime = World ? World->GetTimeSeconds() : 0.0;
	constexpr double RetryIntervalSeconds = 0.5;
	if (IdentityAppliedPlayerState == PlayerState
		&& CurrentTime - LastIdentityApplyRequestTime < RetryIntervalSeconds)
	{
		return;
	}

	if (ALobbyPlayerController* LobbyPlayerController =
		Cast<ALobbyPlayerController>(GetOwningPlayer()))
	{
		LobbyPlayerController->RequestApplyLobbyPlayerName(DesiredPlayerName);
	}
	else
	{
		PlayerState->RequestSetLobbyPlayerName(DesiredPlayerName);
	}
	IdentityAppliedPlayerState = PlayerState;
	LastIdentityApplyRequestTime = CurrentTime;
}

void ULobbyWidget::RefreshLobbyBindings()
{
	ASnowRumbleLobbyGameState* CurrentGameState = GetLobbyGameState();
	if (BoundLobbyGameState == CurrentGameState)
	{
		return;
	}

	UnbindLobbyBindings();
	BoundLobbyGameState = CurrentGameState;
	if (BoundLobbyGameState)
	{
		BoundLobbyGameState->OnLobbyStateChanged.AddUniqueDynamic(
			this,
			&ULobbyWidget::HandleLobbyStateChanged);
		OnLobbyStateChanged();
	}
}

void ULobbyWidget::RefreshRoomCodeText()
{
	if (RoomCodeTextBlock)
	{
		RoomCodeTextBlock->SetText(FText::FromString(GetCurrentRoomCode()));
	}
}

void ULobbyWidget::UnbindLobbyBindings()
{
	if (BoundLobbyGameState)
	{
		BoundLobbyGameState->OnLobbyStateChanged.RemoveAll(this);
		BoundLobbyGameState = nullptr;
	}
}

void ULobbyWidget::HandleLobbyStateChanged()
{
	OnLobbyStateChanged();
}

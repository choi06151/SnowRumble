// Copyright Epic Games, Inc. All Rights Reserved.

#include "LobbyWidget.h"

#include "../Game/SnowRumbleLobbyGameState.h"
#include "GameFramework/PlayerController.h"

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshLobbyBindings();
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

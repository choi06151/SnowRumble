// Copyright Epic Games, Inc. All Rights Reserved.

#include "LobbyEscapeMenuWidget.h"

#include "../Audio/SnowRumbleAudioHelpers.h"
#include "Components/Button.h"
#include "InputCoreTypes.h"
#include "LobbyPlayerController.h"

void ULobbyEscapeMenuWidget::SetLobbyPlayerController(
	ALobbyPlayerController* NewPlayerController)
{
	LobbyPlayerController = NewPlayerController;
}

void ULobbyEscapeMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	BindMenuButtons();
}

void ULobbyEscapeMenuWidget::NativeDestruct()
{
	UnbindMenuButtons();

	Super::NativeDestruct();
}

FReply ULobbyEscapeMenuWidget::NativeOnKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent)
{
	const FKey PressedKey = InKeyEvent.GetKey();
	if (PressedKey == EKeys::Escape || PressedKey == EKeys::F10)
	{
		CloseMenu();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void ULobbyEscapeMenuWidget::BindMenuButtons()
{
	if (ReturnToMainMenuButton)
	{
		ReturnToMainMenuButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyEscapeMenuWidget::HandleReturnToMainMenuButtonClicked);
	}
	if (ReturnToLobbyButton)
	{
		ReturnToLobbyButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyEscapeMenuWidget::HandleReturnToMainMenuButtonClicked);
	}
	if (SettingsButton)
	{
		SettingsButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyEscapeMenuWidget::HandleSettingsButtonClicked);
	}
	if (InviteFriendsButton)
	{
		InviteFriendsButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyEscapeMenuWidget::HandleInviteFriendsButtonClicked);
	}
	if (BackButton)
	{
		BackButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyEscapeMenuWidget::HandleBackButtonClicked);
	}
}

void ULobbyEscapeMenuWidget::UnbindMenuButtons()
{
	if (ReturnToMainMenuButton)
	{
		ReturnToMainMenuButton->OnClicked.RemoveAll(this);
	}
	if (ReturnToLobbyButton)
	{
		ReturnToLobbyButton->OnClicked.RemoveAll(this);
	}
	if (SettingsButton)
	{
		SettingsButton->OnClicked.RemoveAll(this);
	}
	if (InviteFriendsButton)
	{
		InviteFriendsButton->OnClicked.RemoveAll(this);
	}
	if (BackButton)
	{
		BackButton->OnClicked.RemoveAll(this);
	}
}

void ULobbyEscapeMenuWidget::HandleReturnToMainMenuButtonClicked()
{
	OnReturnToMainMenuRequested();
	if (LobbyPlayerController)
	{
		LobbyPlayerController->RequestReturnToMainMenu();
	}
	else
	{
		CloseMenu();
	}
}

void ULobbyEscapeMenuWidget::HandleSettingsButtonClicked()
{
	OnSettingsRequested();
	if (LobbyPlayerController)
	{
		LobbyPlayerController->ShowOptionsMenu();
	}
}

void ULobbyEscapeMenuWidget::HandleInviteFriendsButtonClicked()
{
	OnInviteFriendsRequested();
	if (LobbyPlayerController)
	{
		LobbyPlayerController->OpenSteamSessionInviteUI();
	}
}

void ULobbyEscapeMenuWidget::HandleBackButtonClicked()
{
	CloseMenu();
}

void ULobbyEscapeMenuWidget::CloseMenu()
{
	if (LobbyPlayerController)
	{
		LobbyPlayerController->HideLobbyEscapeMenu();
	}
}

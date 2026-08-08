// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainMenuWidget.h"

#include "../Player/LocalPlayerIdentitySubsystem_C.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Engine/GameInstance.h"

const TArray<FSnowRumbleSessionInfo> UMainMenuWidget::EmptyResults;

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindMenuButtons();
	SetRoomCodeJoinPanelVisible(false);
	InitializePlayerNameInput();
	SetStatusMessage(TEXT("Ready"));
	RefreshJoinButtonEnabled();

	if (USnowRumbleSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->OnSessionStateChanged.AddUniqueDynamic(
			this,
			&UMainMenuWidget::HandleSessionStateChanged);
		SessionSubsystem->OnSessionSearchCompleted.AddUniqueDynamic(
			this,
			&UMainMenuWidget::HandleSearchCompleted);
	}
}

void UMainMenuWidget::NativeDestruct()
{
	UnbindMenuButtons();

	if (USnowRumbleSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->OnSessionStateChanged.RemoveAll(this);
		SessionSubsystem->OnSessionSearchCompleted.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UMainMenuWidget::HostLanGame(int32 MaxPlayers, const FString& RoomName)
{
	SavePlayerNameInput();
	if (USnowRumbleSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->HostLanSession(MaxPlayers, RoomName);
	}
}

void UMainMenuWidget::FindLanGames()
{
	if (USnowRumbleSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->FindLanSessions();
	}
}

void UMainMenuWidget::QuickJoinLanGame()
{
	SavePlayerNameInput();
	if (USnowRumbleSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->QuickJoinLanSession();
	}
}

void UMainMenuWidget::JoinLanGame(int32 ResultIndex)
{
	SavePlayerNameInput();
	if (USnowRumbleSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->JoinLanSession(ResultIndex);
	}
}

void UMainMenuWidget::JoinLanGameByRoomCode(const FString& RoomCode)
{
	SavePlayerNameInput();
	if (USnowRumbleSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->JoinLanSessionByRoomCode(RoomCode);
	}
}

const TArray<FSnowRumbleSessionInfo>& UMainMenuWidget::GetLanSearchResults()
	const
{
	const USnowRumbleSessionSubsystem* SessionSubsystem =
		GetSessionSubsystem();
	return SessionSubsystem
		? SessionSubsystem->GetSearchResults()
		: EmptyResults;
}

ESnowRumbleSessionState UMainMenuWidget::GetSessionState() const
{
	const USnowRumbleSessionSubsystem* SessionSubsystem =
		GetSessionSubsystem();
	return SessionSubsystem
		? SessionSubsystem->GetCurrentState()
		: ESnowRumbleSessionState::Idle;
}

USnowRumbleSessionSubsystem* UMainMenuWidget::GetSessionSubsystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance
		? GameInstance->GetSubsystem<USnowRumbleSessionSubsystem>()
		: nullptr;
}

void UMainMenuWidget::HandleSessionStateChanged(
	ESnowRumbleSessionOperation Operation,
	ESnowRumbleSessionState State,
	const FString& Message)
{
	SetStatusMessage(Message.IsEmpty() ? TEXT("Session state changed") : Message);
	if (Operation == ESnowRumbleSessionOperation::JoinByCode
		&& State == ESnowRumbleSessionState::Succeeded)
	{
		SetRoomCodeJoinPanelVisible(false);
	}
	RefreshJoinButtonEnabled();
	OnMainMenuSessionStateChanged(Operation, State, Message);
}

void UMainMenuWidget::HandleSearchCompleted(
	const TArray<FSnowRumbleSessionInfo>& Results)
{
	SetStatusMessage(FString::Printf(TEXT("Found %d LAN session(s)"), Results.Num()));
	RefreshJoinButtonEnabled();
	OnMainMenuSearchCompleted(Results);
}

void UMainMenuWidget::HandleHostButtonClicked()
{
	HostLanGame(8);
}

void UMainMenuWidget::HandleQuickJoinButtonClicked()
{
	QuickJoinLanGame();
}

void UMainMenuWidget::HandleFindButtonClicked()
{
	SetRoomCodeJoinPanelVisible(true);
	OnRoomCodeJoinPromptRequested();
}

void UMainMenuWidget::HandleConfirmRoomCodeJoinClicked()
{
	const FString RoomCode = RoomCodeTextBox
		? RoomCodeTextBox->GetText().ToString()
		: FString();
	JoinLanGameByRoomCode(RoomCode);
}

void UMainMenuWidget::HandleCancelRoomCodeJoinClicked()
{
	SetRoomCodeJoinPanelVisible(false);
}

void UMainMenuWidget::BindMenuButtons()
{
	if (HostButton)
	{
		HostButton->OnClicked.AddUniqueDynamic(
			this,
			&UMainMenuWidget::HandleHostButtonClicked);
	}

	if (QuickJoinButton)
	{
		QuickJoinButton->OnClicked.AddUniqueDynamic(
			this,
			&UMainMenuWidget::HandleQuickJoinButtonClicked);
	}

	if (FindButton)
	{
		FindButton->OnClicked.AddUniqueDynamic(
			this,
			&UMainMenuWidget::HandleFindButtonClicked);
	}

	if (ConfirmRoomCodeJoinButton)
	{
		ConfirmRoomCodeJoinButton->OnClicked.AddUniqueDynamic(
			this,
			&UMainMenuWidget::HandleConfirmRoomCodeJoinClicked);
	}

	if (CancelRoomCodeJoinButton)
	{
		CancelRoomCodeJoinButton->OnClicked.AddUniqueDynamic(
			this,
			&UMainMenuWidget::HandleCancelRoomCodeJoinClicked);
	}
}

void UMainMenuWidget::UnbindMenuButtons()
{
	if (HostButton)
	{
		HostButton->OnClicked.RemoveAll(this);
	}

	if (QuickJoinButton)
	{
		QuickJoinButton->OnClicked.RemoveAll(this);
	}

	if (FindButton)
	{
		FindButton->OnClicked.RemoveAll(this);
	}

	if (ConfirmRoomCodeJoinButton)
	{
		ConfirmRoomCodeJoinButton->OnClicked.RemoveAll(this);
	}

	if (CancelRoomCodeJoinButton)
	{
		CancelRoomCodeJoinButton->OnClicked.RemoveAll(this);
	}
}

void UMainMenuWidget::SetRoomCodeJoinPanelVisible(bool bVisible)
{
	if (RoomCodeJoinPanel)
	{
		RoomCodeJoinPanel->SetVisibility(
			bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (bVisible && RoomCodeTextBox)
	{
		RoomCodeTextBox->SetText(FText::GetEmpty());
		RoomCodeTextBox->SetKeyboardFocus();
	}
}

void UMainMenuWidget::SavePlayerNameInput()
{
	if (!PlayerNameTextBox)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	ULocalPlayerIdentitySubsystem* IdentitySubsystem = GameInstance
		? GameInstance->GetSubsystem<ULocalPlayerIdentitySubsystem>()
		: nullptr;
	if (IdentitySubsystem)
	{
		IdentitySubsystem->SetDesiredPlayerName(
			PlayerNameTextBox->GetText().ToString());
	}
}

void UMainMenuWidget::InitializePlayerNameInput()
{
	if (!PlayerNameTextBox)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	const ULocalPlayerIdentitySubsystem* IdentitySubsystem = GameInstance
		? GameInstance->GetSubsystem<ULocalPlayerIdentitySubsystem>()
		: nullptr;
	if (IdentitySubsystem && IdentitySubsystem->HasDesiredPlayerName())
	{
		PlayerNameTextBox->SetText(
			FText::FromString(IdentitySubsystem->GetDesiredPlayerName()));
	}
}

void UMainMenuWidget::SetStatusMessage(const FString& Message)
{
	if (StatusTextBlock)
	{
		StatusTextBlock->SetText(FText::FromString(Message));
	}
}

void UMainMenuWidget::RefreshJoinButtonEnabled()
{
	const bool bCanRequestSession =
		GetSessionState() != ESnowRumbleSessionState::InProgress;
	if (QuickJoinButton)
	{
		QuickJoinButton->SetIsEnabled(bCanRequestSession);
	}

	if (FindButton)
	{
		FindButton->SetIsEnabled(bCanRequestSession);
	}
}

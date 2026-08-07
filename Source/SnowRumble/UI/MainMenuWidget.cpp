// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainMenuWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"

const TArray<FSnowRumbleSessionInfo> UMainMenuWidget::EmptyResults;

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindMenuButtons();
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

void UMainMenuWidget::HostLanGame(int32 MaxPlayers)
{
	if (USnowRumbleSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->HostLanSession(MaxPlayers);
	}
}

void UMainMenuWidget::FindLanGames()
{
	if (USnowRumbleSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->FindLanSessions();
	}
}

void UMainMenuWidget::JoinLanGame(int32 ResultIndex)
{
	if (USnowRumbleSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->JoinLanSession(ResultIndex);
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
	HostLanGame(16);
}

void UMainMenuWidget::HandleFindButtonClicked()
{
	FindLanGames();
}

void UMainMenuWidget::HandleJoinFirstButtonClicked()
{
	JoinLanGame(0);
}

void UMainMenuWidget::BindMenuButtons()
{
	if (HostButton)
	{
		HostButton->OnClicked.AddUniqueDynamic(
			this,
			&UMainMenuWidget::HandleHostButtonClicked);
	}

	if (FindButton)
	{
		FindButton->OnClicked.AddUniqueDynamic(
			this,
			&UMainMenuWidget::HandleFindButtonClicked);
	}

	if (JoinFirstButton)
	{
		JoinFirstButton->OnClicked.AddUniqueDynamic(
			this,
			&UMainMenuWidget::HandleJoinFirstButtonClicked);
	}
}

void UMainMenuWidget::UnbindMenuButtons()
{
	if (HostButton)
	{
		HostButton->OnClicked.RemoveAll(this);
	}

	if (FindButton)
	{
		FindButton->OnClicked.RemoveAll(this);
	}

	if (JoinFirstButton)
	{
		JoinFirstButton->OnClicked.RemoveAll(this);
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
	if (JoinFirstButton)
	{
		JoinFirstButton->SetIsEnabled(GetLanSearchResults().Num() > 0);
	}
}

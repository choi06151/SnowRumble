// Copyright Epic Games, Inc. All Rights Reserved.

#include "LobbyEscapeMenuWidget.h"

#include "../Audio/SnowRumbleAudioHelpers.h"
#include "Components/Button.h"
#include "Components/ContentWidget.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "InputCoreTypes.h"
#include "LobbyPlayerController.h"

namespace
{
	void CollectButtonTextBlocks(
		UWidget* Widget,
		TArray<UTextBlock*>& OutTextBlocks)
	{
		if (!Widget)
		{
			return;
		}

		if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
		{
			OutTextBlocks.AddUnique(TextBlock);
		}

		if (UPanelWidget* PanelWidget = Cast<UPanelWidget>(Widget))
		{
			for (int32 ChildIndex = 0;
				ChildIndex < PanelWidget->GetChildrenCount();
				++ChildIndex)
			{
				CollectButtonTextBlocks(
					PanelWidget->GetChildAt(ChildIndex),
					OutTextBlocks);
			}
		}
		else if (UContentWidget* ContentWidget = Cast<UContentWidget>(Widget))
		{
			CollectButtonTextBlocks(ContentWidget->GetContent(), OutTextBlocks);
		}
	}
}

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

void ULobbyEscapeMenuWidget::NativeTick(
	const FGeometry& MyGeometry,
	float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshTargetButtonTextColors();
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
	BindTargetButtonTextColor(ReturnToMainMenuButton);
	BindTargetButtonTextColor(ReturnToLobbyButton);
	BindTargetButtonTextColor(SettingsButton);
	BindTargetButtonTextColor(InviteFriendsButton);
	BindTargetButtonTextColor(BackButton);
	RefreshTargetButtonTextColors();

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
	UnbindTargetButtonTextColor(ReturnToMainMenuButton);
	UnbindTargetButtonTextColor(ReturnToLobbyButton);
	UnbindTargetButtonTextColor(SettingsButton);
	UnbindTargetButtonTextColor(InviteFriendsButton);
	UnbindTargetButtonTextColor(BackButton);

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

void ULobbyEscapeMenuWidget::BindTargetButtonTextColor(UButton* Button)
{
	if (!Button)
	{
		return;
	}

	Button->OnHovered.AddUniqueDynamic(
		this,
		&ULobbyEscapeMenuWidget::RefreshTargetButtonTextColors);
	Button->OnUnhovered.AddUniqueDynamic(
		this,
		&ULobbyEscapeMenuWidget::RefreshTargetButtonTextColors);
	Button->OnPressed.AddUniqueDynamic(
		this,
		&ULobbyEscapeMenuWidget::RefreshTargetButtonTextColors);
	Button->OnReleased.AddUniqueDynamic(
		this,
		&ULobbyEscapeMenuWidget::RefreshTargetButtonTextColors);

	TArray<UTextBlock*> TextBlocks;
	CollectButtonTextBlocks(Button->GetContent(), TextBlocks);
	for (UTextBlock* TextBlock : TextBlocks)
	{
		if (TextBlock)
		{
			TargetButtonTextDefaultColors.FindOrAdd(
				TextBlock,
				TextBlock->GetColorAndOpacity());
		}
	}
}

void ULobbyEscapeMenuWidget::UnbindTargetButtonTextColor(UButton* Button)
{
	if (Button)
	{
		Button->OnHovered.RemoveAll(this);
		Button->OnUnhovered.RemoveAll(this);
		Button->OnPressed.RemoveAll(this);
		Button->OnReleased.RemoveAll(this);
	}
}

void ULobbyEscapeMenuWidget::RefreshTargetButtonTextColors()
{
	const FSlateColor ActiveTextColor(FLinearColor::White);
	const TArray<UButton*> TargetButtons =
	{
		ReturnToMainMenuButton,
		ReturnToLobbyButton,
		SettingsButton,
		InviteFriendsButton,
		BackButton
	};

	for (UButton* Button : TargetButtons)
	{
		if (!Button)
		{
			continue;
		}

		TArray<UTextBlock*> TextBlocks;
		CollectButtonTextBlocks(Button->GetContent(), TextBlocks);
		const bool bUseActiveColor = Button->IsHovered() || Button->IsPressed();
		for (UTextBlock* TextBlock : TextBlocks)
		{
			if (!TextBlock)
			{
				continue;
			}

			if (bUseActiveColor)
			{
				TextBlock->SetColorAndOpacity(ActiveTextColor);
			}
			else if (const FSlateColor* DefaultColor =
				TargetButtonTextDefaultColors.Find(TextBlock))
			{
				TextBlock->SetColorAndOpacity(*DefaultColor);
			}
		}
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

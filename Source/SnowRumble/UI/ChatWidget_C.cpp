// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChatWidget_C.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "InputCoreTypes.h"
#include "SnowRumblePlayerController.h"

namespace
{
FText GetChatChannelText(ESnowRumbleChatChannel Channel)
{
	switch (Channel)
	{
	case ESnowRumbleChatChannel::Team:
		return NSLOCTEXT("SnowRumble", "ChatChannelTeam", "팀");
	case ESnowRumbleChatChannel::All:
	default:
		return NSLOCTEXT("SnowRumble", "ChatChannelAll", "전체");
	}
}
}

void UChatWidget::SetChatPlayerController(
	ASnowRumblePlayerController* NewPlayerController)
{
	ChatPlayerController = NewPlayerController;
}

void UChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	if (ChatInputTextBox)
	{
		ChatInputTextBox->OnTextCommitted.AddUniqueDynamic(
			this,
			&UChatWidget::HandleChatTextCommitted);
		ChatInputTextBox->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (ChatLogBorder)
	{
		OriginalChatLogBorderBrushColor = ChatLogBorder->GetBrushColor();
	}

	ApplyConfiguredFonts();
	RefreshChatLogChrome();
	RefreshChannelText();
}

void UChatWidget::NativeDestruct()
{
	if (ChatInputTextBox)
	{
		ChatInputTextBox->OnTextCommitted.RemoveAll(this);
	}

	Super::NativeDestruct();
}

FReply UChatWidget::NativeOnKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent)
{
	if (bChatInputOpen && InKeyEvent.GetKey() == EKeys::Tab)
	{
		ToggleChatChannel();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UChatWidget::NativeOnMouseWheel(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (bChatInputOpen && ChatLogScrollBox)
	{
		const float NewScrollOffset =
			ChatLogScrollBox->GetScrollOffset()
			- InMouseEvent.GetWheelDelta() * 48.0f;
		ChatLogScrollBox->SetScrollOffset(FMath::Max(0.0f, NewScrollOffset));
		return FReply::Handled();
	}

	return Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
}

void UChatWidget::OpenChatInput(ESnowRumbleChatChannel InitialChannel)
{
	SetActiveChatChannel(InitialChannel);
	bChatInputOpen = true;

	if (ChatInputTextBox)
	{
		ChatInputTextBox->SetText(FText::GetEmpty());
		ChatInputTextBox->SetVisibility(ESlateVisibility::Visible);
		if (ChatPlayerController)
		{
			ChatInputTextBox->SetUserFocus(ChatPlayerController);
		}
		ChatInputTextBox->SetKeyboardFocus();
	}
	if (ChatChannelText)
	{
		ChatChannelText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	RefreshChatLogChrome();

	OnChatInputOpenChanged(true);
}

void UChatWidget::CloseChatInput()
{
	bChatInputOpen = false;

	if (ChatInputTextBox)
	{
		ChatInputTextBox->SetText(FText::GetEmpty());
		ChatInputTextBox->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (ChatChannelText)
	{
		ChatChannelText->SetVisibility(ESlateVisibility::Collapsed);
	}
	RefreshChatLogChrome();

	OnChatInputOpenChanged(false);
}

bool UChatWidget::IsChatInputOpen() const
{
	return bChatInputOpen;
}

void UChatWidget::AddChatMessage(
	ESnowRumbleChatChannel Channel,
	const FString& SenderName,
	const FString& Message)
{
	const FText DisplayText = FText::Format(
		NSLOCTEXT("SnowRumble", "ChatMessageFormat", "[{0}] {1}: {2}"),
		GetChatChannelText(Channel),
		FText::FromString(SenderName),
		FText::FromString(Message));

	if (!ChatLog.IsEmpty())
	{
		ChatLog += LINE_TERMINATOR;
	}
	ChatLog += DisplayText.ToString();

	if (ChatLogText)
	{
		ChatLogText->SetText(FText::FromString(ChatLog));
	}
	AddMessageTextRow(DisplayText);

	OnChatMessageAdded(Channel, SenderName, Message, DisplayText);
}

void UChatWidget::SetActiveChatChannel(ESnowRumbleChatChannel NewChannel)
{
	ActiveChatChannel =
		NewChannel == ESnowRumbleChatChannel::Team
		&& ChatPlayerController
		&& !ChatPlayerController->IsTeamChatAvailable()
			? ESnowRumbleChatChannel::All
			: NewChannel;
	RefreshChannelText();
}

ESnowRumbleChatChannel UChatWidget::GetActiveChatChannel() const
{
	return ActiveChatChannel;
}

void UChatWidget::ToggleChatChannel()
{
	if (!ChatPlayerController || !ChatPlayerController->IsTeamChatAvailable())
	{
		SetActiveChatChannel(ESnowRumbleChatChannel::All);
		return;
	}

	SetActiveChatChannel(
		ActiveChatChannel == ESnowRumbleChatChannel::All
			? ESnowRumbleChatChannel::Team
			: ESnowRumbleChatChannel::All);
}

void UChatWidget::HandleChatTextCommitted(
	const FText& Text,
	ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnCleared)
	{
		if (ChatPlayerController)
		{
			ChatPlayerController->CloseChatInput();
		}
		else
		{
			CloseChatInput();
		}
		return;
	}
	if (CommitMethod != ETextCommit::OnEnter)
	{
		return;
	}

	const FString Message = Text.ToString().TrimStartAndEnd();
	if (ChatPlayerController && !Message.IsEmpty())
	{
		ChatPlayerController->SubmitChatMessage(Message, ActiveChatChannel);
	}
	if (ChatPlayerController)
	{
		ChatPlayerController->CloseChatInput();
	}
	else
	{
		CloseChatInput();
	}
}

void UChatWidget::RefreshChannelText()
{
	if (ChatChannelText)
	{
		ChatChannelText->SetText(GetChatChannelText(ActiveChatChannel));
	}
}

void UChatWidget::ApplyConfiguredFonts()
{
	if (ChatLogText)
	{
		ChatLogText->SetFont(ChatMessageFont);
	}
	if (ChatChannelText)
	{
		ChatChannelText->SetFont(ChatChannelFont);
		ChatChannelText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UChatWidget::RefreshChatLogChrome()
{
	if (ChatLogScrollBox)
	{
		ChatLogScrollBox->SetScrollBarVisibility(
			bChatInputOpen
				? ESlateVisibility::Visible
				: ESlateVisibility::Collapsed);
	}
	if (ChatLogBorder)
	{
		FLinearColor BorderColor = OriginalChatLogBorderBrushColor;
		if (!bChatInputOpen)
		{
			BorderColor.A = 0.0f;
		}
		ChatLogBorder->SetBrushColor(BorderColor);
	}
}

void UChatWidget::AddMessageTextRow(const FText& DisplayText)
{
	if (!ChatLogScrollBox || !WidgetTree)
	{
		return;
	}

	UTextBlock* MessageTextBlock =
		WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(),
			NAME_None);
	if (!MessageTextBlock)
	{
		return;
	}

	MessageTextBlock->SetText(DisplayText);
	MessageTextBlock->SetFont(ChatMessageFont);
	MessageTextBlock->SetAutoWrapText(true);
	ChatLogScrollBox->AddChild(MessageTextBlock);
	if (!bChatInputOpen)
	{
		ChatLogScrollBox->ScrollToEnd();
	}
}

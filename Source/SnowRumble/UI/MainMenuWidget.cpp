// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainMenuWidget.h"

#include "../Audio/SnowRumbleAudioHelpers.h"
#include "../Player/LocalPlayerIdentitySubsystem_C.h"
#include "Components/Button.h"
#include "Components/ContentWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Engine/GameInstance.h"
#include "MainMenuPlayerController.h"

const TArray<FSnowRumbleSessionInfo> UMainMenuWidget::EmptyResults;

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

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindMenuButtons();
	SetRoomCodeJoinPanelVisible(false);
	InitializePlayerNameInput();
	SetStatusMessage(TEXT("Ready"));
	if (MainMenuAlarmText)
	{
		MainMenuAlarmText->SetText(FText::GetEmpty());
		MainMenuAlarmText->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (AlarmText)
	{
		AlarmText->SetText(FText::GetEmpty());
		AlarmText->SetVisibility(ESlateVisibility::Collapsed);
	}
	ConsumePendingMainMenuAlarm();
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

void UMainMenuWidget::NativeTick(
	const FGeometry& MyGeometry,
	float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	ConsumePendingMainMenuAlarm();
}

void UMainMenuWidget::HostLanGame(int32 MaxPlayers, const FString& RoomName)
{
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
	if (USnowRumbleSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->QuickJoinLanSession();
	}
}

void UMainMenuWidget::JoinLanGame(int32 ResultIndex)
{
	if (USnowRumbleSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->JoinLanSession(ResultIndex);
	}
}

void UMainMenuWidget::JoinLanGameByRoomCode(const FString& RoomCode)
{
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
	if (State == ESnowRumbleSessionState::InProgress)
	{
		const FText ProgressText = GetSessionProgressAlarmText(Operation);
		if (!ProgressText.IsEmpty())
		{
			ShowMainMenuAlarm(ProgressText);
		}
	}
	if (State == ESnowRumbleSessionState::Failed)
	{
		switch (Operation)
		{
		case ESnowRumbleSessionOperation::QuickJoin:
		case ESnowRumbleSessionOperation::Join:
		case ESnowRumbleSessionOperation::JoinByCode:
			ShowMainMenuAlarm(FText::FromString(
				Message.IsEmpty()
					? FString(TEXT("방이 존재하지 않습니다."))
					: Message));
			if (USnowRumbleSessionSubsystem* SessionSubsystem =
				GetSessionSubsystem())
			{
				SessionSubsystem->SetPendingMainMenuAlarmMessage(
					Message.IsEmpty()
						? FString(TEXT("방이 존재하지 않습니다."))
						: Message);
			}
			break;
		default:
			break;
		}
	}
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
	if (Results.IsEmpty())
	{
		if (USnowRumbleSessionSubsystem* SessionSubsystem =
			GetSessionSubsystem())
		{
			SessionSubsystem->SetPendingMainMenuAlarmMessage(
				TEXT("방이 존재하지 않습니다."));
		}
		ShowMainMenuAlarm(
			NSLOCTEXT("SnowRumble", "MainMenuNoSessionFound", "방이 존재하지 않습니다."));
	}
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

void UMainMenuWidget::HandleSettingsButtonClicked()
{
	if (AMainMenuPlayerController* MainMenuPlayerController =
		Cast<AMainMenuPlayerController>(GetOwningPlayer()))
	{
		MainMenuPlayerController->ShowOptionsMenu();
	}
}

void UMainMenuWidget::HandleCustomizationButtonClicked()
{
	if (AMainMenuPlayerController* MainMenuPlayerController =
		Cast<AMainMenuPlayerController>(GetOwningPlayer()))
	{
		MainMenuPlayerController->TravelToCustomizationLevel();
	}
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
	// 메인메뉴의 주요 4개 버튼만 텍스트 호버 색상 처리를 적용한다.
	BindTargetButtonTextColor(HostButton);
	BindTargetButtonTextColor(QuickJoinButton);
	BindTargetButtonTextColor(FindButton);
	BindTargetButtonTextColor(CustomizationButton);

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

	if (SettingsButton)
	{
		SettingsButton->OnClicked.AddUniqueDynamic(
			this,
			&UMainMenuWidget::HandleSettingsButtonClicked);
	}

	if (CustomizationButton)
	{
		CustomizationButton->OnClicked.AddUniqueDynamic(
			this,
			&UMainMenuWidget::HandleCustomizationButtonClicked);
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

	if (PlayerNameTextBox)
	{
		PlayerNameTextBox->OnTextCommitted.AddUniqueDynamic(
			this,
			&UMainMenuWidget::HandlePlayerNameTextCommitted);
	}
}

void UMainMenuWidget::UnbindMenuButtons()
{
	UnbindTargetButtonTextColor(HostButton);
	UnbindTargetButtonTextColor(QuickJoinButton);
	UnbindTargetButtonTextColor(FindButton);
	UnbindTargetButtonTextColor(CustomizationButton);

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

	if (SettingsButton)
	{
		SettingsButton->OnClicked.RemoveAll(this);
	}

	if (CustomizationButton)
	{
		CustomizationButton->OnClicked.RemoveAll(this);
	}

	if (ConfirmRoomCodeJoinButton)
	{
		ConfirmRoomCodeJoinButton->OnClicked.RemoveAll(this);
	}

	if (CancelRoomCodeJoinButton)
	{
		CancelRoomCodeJoinButton->OnClicked.RemoveAll(this);
	}

	if (PlayerNameTextBox)
	{
		PlayerNameTextBox->OnTextCommitted.RemoveAll(this);
	}
}

void UMainMenuWidget::BindTargetButtonTextColor(UButton* Button)
{
	if (!Button)
	{
		return;
	}

	Button->OnHovered.AddUniqueDynamic(
		this,
		&UMainMenuWidget::RefreshTargetButtonTextColors);
	Button->OnUnhovered.AddUniqueDynamic(
		this,
		&UMainMenuWidget::RefreshTargetButtonTextColors);
	Button->OnPressed.AddUniqueDynamic(
		this,
		&UMainMenuWidget::RefreshTargetButtonTextColors);
	Button->OnReleased.AddUniqueDynamic(
		this,
		&UMainMenuWidget::RefreshTargetButtonTextColors);

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

void UMainMenuWidget::UnbindTargetButtonTextColor(UButton* Button)
{
	if (Button)
	{
		Button->OnHovered.RemoveAll(this);
		Button->OnUnhovered.RemoveAll(this);
		Button->OnPressed.RemoveAll(this);
		Button->OnReleased.RemoveAll(this);
	}
}

void UMainMenuWidget::RefreshTargetButtonTextColors()
{
	const FSlateColor ActiveTextColor(MainMenuButtonActiveTextColor);
	const TArray<UButton*> TargetButtons =
	{
		HostButton,
		QuickJoinButton,
		FindButton,
		CustomizationButton
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

void UMainMenuWidget::HandlePlayerNameTextCommitted(
	const FText& Text,
	ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnCleared)
	{
		RestorePlayerNameInput();
		return;
	}

	ValidateAndSavePlayerNameInput();
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

bool UMainMenuWidget::ValidateAndSavePlayerNameInput()
{
	if (!PlayerNameTextBox)
	{
		return true;
	}

	UGameInstance* GameInstance = GetGameInstance();
	ULocalPlayerIdentitySubsystem* IdentitySubsystem = GameInstance
		? GameInstance->GetSubsystem<ULocalPlayerIdentitySubsystem>()
		: nullptr;
	if (!IdentitySubsystem)
	{
		return true;
	}

	if (!IdentitySubsystem->TrySetDesiredPlayerName(
		PlayerNameTextBox->GetText().ToString()))
	{
		ShowMainMenuAlarm(NSLOCTEXT(
			"SnowRumble",
			"MainMenuInvalidPlayerName",
			"적합하지 않은 이름입니다."));
		RestorePlayerNameInput();
		return false;
	}

	PlayerNameTextBox->SetText(
		FText::FromString(IdentitySubsystem->GetDesiredPlayerName()));
	return true;
}

void UMainMenuWidget::RestorePlayerNameInput()
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

void UMainMenuWidget::ConsumePendingMainMenuAlarm()
{
	if (USnowRumbleSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		const FString AlarmMessage =
			SessionSubsystem->ConsumePendingMainMenuAlarmMessage();
		if (!AlarmMessage.IsEmpty())
		{
			ShowMainMenuAlarm(FText::FromString(AlarmMessage));
			return;
		}
	}
}

void UMainMenuWidget::ShowMainMenuAlarm(const FText& Message)
{
	OnMainMenuAlarmRequested(Message);

	if (MainMenuAlarmText)
	{
		MainMenuAlarmText->SetText(Message);
		MainMenuAlarmText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	if (AlarmText)
	{
		AlarmText->SetText(Message);
		AlarmText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (MainMenuAlarmAnimation)
	{
		PlayAnimation(MainMenuAlarmAnimation);
	}
	else if (AlarmAnimation)
	{
		PlayAnimation(AlarmAnimation);
	}
}

FText UMainMenuWidget::GetSessionProgressAlarmText(
	ESnowRumbleSessionOperation Operation) const
{
	switch (Operation)
	{
	case ESnowRumbleSessionOperation::Host:
		return NSLOCTEXT("SnowRumble", "MainMenuHostInProgress", "방 생성중...");
	case ESnowRumbleSessionOperation::Search:
		return NSLOCTEXT("SnowRumble", "MainMenuSearchInProgress", "방 찾는중...");
	case ESnowRumbleSessionOperation::QuickJoin:
	case ESnowRumbleSessionOperation::Join:
	case ESnowRumbleSessionOperation::JoinByCode:
		return NSLOCTEXT("SnowRumble", "MainMenuJoinInProgress", "방 참가중...");
	case ESnowRumbleSessionOperation::None:
	default:
		return FText::GetEmpty();
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

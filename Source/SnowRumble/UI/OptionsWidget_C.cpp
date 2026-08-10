// Copyright Epic Games, Inc. All Rights Reserved.

#include "OptionsWidget_C.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"
#include "../Player/SnowRumbleCharacter.h"
#include "../Player/SnowRumbleUserSettingsSubsystem_C.h"
#include "Engine/GameInstance.h"
#include "OptionsKeyBindingRowWidget_C.h"

namespace
{
FSnowRumbleKeyBindingViewData MakeKeyBindingRow(
	const FName BindingId,
	const FText& DisplayName,
	const FKey& DefaultKey)
{
	FSnowRumbleKeyBindingViewData Row;
	Row.BindingId = BindingId;
	Row.DisplayName = DisplayName;
	Row.CurrentKey = DefaultKey;
	Row.DefaultKey = DefaultKey;
	return Row;
}
}

UOptionsWidget::UOptionsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UOptionsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	InitializeDefaultKeyBindingRows();
	BindOptionButtons();
	RefreshKeyBindingPanel();
	SetOptionsCategory(CurrentOptionsCategory);
}

void UOptionsWidget::NativeDestruct()
{
	UnbindOptionButtons();

	Super::NativeDestruct();
}

void UOptionsWidget::SetOptionsCategory(
	ESnowRumbleOptionsCategory NewCategory)
{
	CurrentOptionsCategory = NewCategory;

	if (OptionsContentSwitcher)
	{
		const int32 SwitcherIndex = GetSwitcherIndexForCategory(NewCategory);
		if (OptionsContentSwitcher->GetNumWidgets() > SwitcherIndex)
		{
			OptionsContentSwitcher->SetActiveWidgetIndex(SwitcherIndex);
		}
	}

	OnOptionsCategoryChanged(NewCategory);
}

ESnowRumbleOptionsCategory UOptionsWidget::GetCurrentOptionsCategory() const
{
	return CurrentOptionsCategory;
}

const TArray<FSnowRumbleKeyBindingViewData>&
UOptionsWidget::GetKeyBindingRows() const
{
	return KeyBindingRows;
}

FReply UOptionsWidget::NativeOnPreviewKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent)
{
	if (!PendingKeyBindingId.IsNone())
	{
		if (InKeyEvent.GetKey() == EKeys::Escape)
		{
			CancelKeyRebind();
			return FReply::Handled();
		}

		ApplyCapturedKey(InKeyEvent.GetKey());
		return FReply::Handled();
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

FReply UOptionsWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (!PendingKeyBindingId.IsNone())
	{
		ApplyCapturedKey(InMouseEvent.GetEffectingButton());
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UOptionsWidget::BindOptionButtons()
{
	if (SensitivityCategoryButton)
	{
		SensitivityCategoryButton->OnClicked.AddUniqueDynamic(
			this,
			&UOptionsWidget::HandleSensitivityCategoryButtonClicked);
	}
	if (AudioCategoryButton)
	{
		AudioCategoryButton->OnClicked.AddUniqueDynamic(
			this,
			&UOptionsWidget::HandleAudioCategoryButtonClicked);
	}
	if (KeyBindingCategoryButton)
	{
		KeyBindingCategoryButton->OnClicked.AddUniqueDynamic(
			this,
			&UOptionsWidget::HandleKeyBindingCategoryButtonClicked);
	}
	if (MicrophoneCategoryButton)
	{
		MicrophoneCategoryButton->OnClicked.AddUniqueDynamic(
			this,
			&UOptionsWidget::HandleMicrophoneCategoryButtonClicked);
	}
	if (CloseButton)
	{
		CloseButton->OnClicked.AddUniqueDynamic(
			this,
			&UOptionsWidget::HandleCloseButtonClicked);
	}
	if (ApplyButton)
	{
		ApplyButton->OnClicked.AddUniqueDynamic(
			this,
			&UOptionsWidget::HandleApplyButtonClicked);
	}
	if (ResetButton)
	{
		ResetButton->OnClicked.AddUniqueDynamic(
			this,
			&UOptionsWidget::HandleResetButtonClicked);
	}
}

void UOptionsWidget::UnbindOptionButtons()
{
	if (SensitivityCategoryButton)
	{
		SensitivityCategoryButton->OnClicked.RemoveAll(this);
	}
	if (AudioCategoryButton)
	{
		AudioCategoryButton->OnClicked.RemoveAll(this);
	}
	if (KeyBindingCategoryButton)
	{
		KeyBindingCategoryButton->OnClicked.RemoveAll(this);
	}
	if (MicrophoneCategoryButton)
	{
		MicrophoneCategoryButton->OnClicked.RemoveAll(this);
	}
	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveAll(this);
	}
	if (ApplyButton)
	{
		ApplyButton->OnClicked.RemoveAll(this);
	}
	if (ResetButton)
	{
		ResetButton->OnClicked.RemoveAll(this);
	}
}

void UOptionsWidget::HandleSensitivityCategoryButtonClicked()
{
	SetOptionsCategory(ESnowRumbleOptionsCategory::Sensitivity);
}

void UOptionsWidget::HandleAudioCategoryButtonClicked()
{
	SetOptionsCategory(ESnowRumbleOptionsCategory::Audio);
}

void UOptionsWidget::HandleKeyBindingCategoryButtonClicked()
{
	SetOptionsCategory(ESnowRumbleOptionsCategory::KeyBinding);
}

void UOptionsWidget::HandleMicrophoneCategoryButtonClicked()
{
	SetOptionsCategory(ESnowRumbleOptionsCategory::Microphone);
}

void UOptionsWidget::HandleCloseButtonClicked()
{
	OnOptionsCloseRequested();
	OnOptionsCloseRequestedNative.Broadcast();
}

void UOptionsWidget::HandleApplyButtonClicked()
{
	OnOptionsApplyRequested();
}

void UOptionsWidget::HandleResetButtonClicked()
{
	OnOptionsResetRequested();
}

void UOptionsWidget::HandleKeyRowRebindRequested(FName BindingId)
{
	BeginKeyRebind(BindingId);
}

void UOptionsWidget::HandleKeyRowResetRequested(FName BindingId)
{
	TArray<FName> ClearedBindingIds;
	for (FSnowRumbleKeyBindingViewData& Row : KeyBindingRows)
	{
		if (Row.BindingId == BindingId)
		{
			for (FSnowRumbleKeyBindingViewData& OtherRow : KeyBindingRows)
			{
				if (OtherRow.BindingId != BindingId
					&& OtherRow.CurrentKey == Row.DefaultKey)
				{
					OtherRow.CurrentKey = EKeys::Invalid;
					ClearedBindingIds.Add(OtherRow.BindingId);
				}
			}

			Row.CurrentKey = Row.DefaultKey;
			break;
		}
	}

	if (PendingKeyBindingId == BindingId)
	{
		PendingKeyBindingId = NAME_None;
	}

	UGameInstance* GameInstance = GetGameInstance();
	USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem = GameInstance
		? GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>()
		: nullptr;
	if (UserSettingsSubsystem)
	{
		UserSettingsSubsystem->ResetKeyBinding(BindingId);
		for (const FName ClearedBindingId : ClearedBindingIds)
		{
			UserSettingsSubsystem->SetKeyBinding(
				ClearedBindingId,
				EKeys::Invalid);
		}
	}

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		if (ASnowRumbleCharacter* SnowRumbleCharacter =
			Cast<ASnowRumbleCharacter>(PlayerController->GetPawn()))
		{
			SnowRumbleCharacter->ApplyInputMappingContext();
		}
	}

	RefreshKeyBindingPanel();
	if (KeyBindingStatusText)
	{
		KeyBindingStatusText->SetText(FText::Format(
			NSLOCTEXT(
				"SnowRumble",
				"KeyBindingResetRequested",
				"{0} 기본값 복원"),
			FText::FromName(BindingId)));
	}

	OnKeyBindingResetRequested(BindingId);
}

int32 UOptionsWidget::GetSwitcherIndexForCategory(
	ESnowRumbleOptionsCategory Category) const
{
	switch (Category)
	{
	case ESnowRumbleOptionsCategory::Audio:
		return 1;
	case ESnowRumbleOptionsCategory::KeyBinding:
		return 2;
	case ESnowRumbleOptionsCategory::Microphone:
		return 3;
	case ESnowRumbleOptionsCategory::Sensitivity:
	default:
		return 0;
	}
}

void UOptionsWidget::BeginKeyRebind(FName BindingId)
{
	if (KeyBindingStatusText)
	{
		KeyBindingStatusText->SetText(FText::Format(
			NSLOCTEXT(
				"SnowRumble",
				"KeyBindingRebindPending",
				"{0} 키 입력 대기 중"),
			FText::FromName(BindingId)));
	}

	PendingKeyBindingId = BindingId;
	SetKeyboardFocus();
	OnKeyRebindRequested(BindingId);
}

void UOptionsWidget::CancelKeyRebind()
{
	const FName CanceledBindingId = PendingKeyBindingId;
	PendingKeyBindingId = NAME_None;

	if (KeyBindingStatusText)
	{
		KeyBindingStatusText->SetText(FText::Format(
			NSLOCTEXT(
				"SnowRumble",
				"KeyBindingRebindCanceled",
				"{0} 키 변경 취소"),
			FText::FromName(CanceledBindingId)));
	}

	OnKeyRebindCanceled(CanceledBindingId);
}

bool UOptionsWidget::ApplyCapturedKey(FKey NewKey)
{
	if (PendingKeyBindingId.IsNone())
	{
		return false;
	}

	const FName BindingId = PendingKeyBindingId;

	if (!IsBindableKey(NewKey))
	{
		if (KeyBindingStatusText)
		{
			KeyBindingStatusText->SetText(
				NSLOCTEXT(
					"SnowRumble",
					"KeyBindingInvalidKey",
					"사용할 수 없는 키입니다"));
		}
		return false;
	}

	bool bApplied = false;
	TArray<FName> ClearedBindingIds;
	for (FSnowRumbleKeyBindingViewData& Row : KeyBindingRows)
	{
		if (Row.BindingId != BindingId && Row.CurrentKey == NewKey)
		{
			Row.CurrentKey = EKeys::Invalid;
			ClearedBindingIds.Add(Row.BindingId);
		}
	}

	for (FSnowRumbleKeyBindingViewData& Row : KeyBindingRows)
	{
		if (Row.BindingId == BindingId)
		{
			Row.CurrentKey = NewKey;
			bApplied = true;
			break;
		}
	}

	if (!bApplied)
	{
		return false;
	}

	PendingKeyBindingId = NAME_None;

	UGameInstance* GameInstance = GetGameInstance();
	USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem = GameInstance
		? GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>()
		: nullptr;
	if (UserSettingsSubsystem)
	{
		UserSettingsSubsystem->SetKeyBinding(BindingId, NewKey);
		for (const FName ClearedBindingId : ClearedBindingIds)
		{
			UserSettingsSubsystem->SetKeyBinding(
				ClearedBindingId,
				EKeys::Invalid);
		}
	}

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		if (ASnowRumbleCharacter* SnowRumbleCharacter =
			Cast<ASnowRumbleCharacter>(PlayerController->GetPawn()))
		{
			SnowRumbleCharacter->ApplyInputMappingContext();
		}
	}

	RefreshKeyBindingPanel();

	if (KeyBindingStatusText)
	{
		KeyBindingStatusText->SetText(FText::Format(
			NSLOCTEXT(
				"SnowRumble",
				"KeyBindingChanged",
				"{0} 키를 {1}(으)로 변경"),
			FText::FromName(BindingId),
			NewKey.GetDisplayName()));
	}

	OnKeyBindingChanged(BindingId, NewKey);
	return true;
}

bool UOptionsWidget::IsBindableKey(FKey Key) const
{
	return Key.IsValid()
		&& Key != EKeys::AnyKey
		&& Key != EKeys::Escape
		&& Key != EKeys::F10
		&& Key != EKeys::Tab;
}

void UOptionsWidget::InitializeDefaultKeyBindingRows()
{
	KeyBindingRows.Reset();

	KeyBindingRows.Add(MakeKeyBindingRow(
		TEXT("MoveForward"),
		NSLOCTEXT("SnowRumble", "KeyBindingMoveForward", "앞으로 이동"),
		EKeys::W));
	KeyBindingRows.Add(MakeKeyBindingRow(
		TEXT("MoveBackward"),
		NSLOCTEXT("SnowRumble", "KeyBindingMoveBackward", "뒤로 이동"),
		EKeys::S));
	KeyBindingRows.Add(MakeKeyBindingRow(
		TEXT("MoveLeft"),
		NSLOCTEXT("SnowRumble", "KeyBindingMoveLeft", "왼쪽 이동"),
		EKeys::A));
	KeyBindingRows.Add(MakeKeyBindingRow(
		TEXT("MoveRight"),
		NSLOCTEXT("SnowRumble", "KeyBindingMoveRight", "오른쪽 이동"),
		EKeys::D));
	KeyBindingRows.Add(MakeKeyBindingRow(
		TEXT("Jump"),
		NSLOCTEXT("SnowRumble", "KeyBindingJump", "점프"),
		EKeys::SpaceBar));
	KeyBindingRows.Add(MakeKeyBindingRow(
		TEXT("Sprint"),
		NSLOCTEXT("SnowRumble", "KeyBindingSprint", "달리기"),
		EKeys::LeftShift));
	KeyBindingRows.Add(MakeKeyBindingRow(
		TEXT("Interact"),
		NSLOCTEXT("SnowRumble", "KeyBindingInteract", "상호작용"),
		EKeys::E));
	KeyBindingRows.Add(MakeKeyBindingRow(
		TEXT("Aim"),
		NSLOCTEXT("SnowRumble", "KeyBindingAim", "조준"),
		EKeys::RightMouseButton));
	KeyBindingRows.Add(MakeKeyBindingRow(
		TEXT("Action"),
		NSLOCTEXT("SnowRumble", "KeyBindingAction", "행동"),
		EKeys::LeftMouseButton));
	KeyBindingRows.Add(MakeKeyBindingRow(
		TEXT("DropEquipment"),
		NSLOCTEXT("SnowRumble", "KeyBindingDropEquipment", "장비 내려놓기"),
		EKeys::Q));
	KeyBindingRows.Add(MakeKeyBindingRow(
		TEXT("Emote"),
		NSLOCTEXT("SnowRumble", "KeyBindingEmote", "이모션"),
		EKeys::B));
	KeyBindingRows.Add(MakeKeyBindingRow(
		TEXT("Chat"),
		NSLOCTEXT("SnowRumble", "KeyBindingChat", "채팅"),
		EKeys::Enter));

	UGameInstance* GameInstance = GetGameInstance();
	const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
		GameInstance
			? GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>()
			: nullptr;
	if (!UserSettingsSubsystem)
	{
		return;
	}

	for (FSnowRumbleKeyBindingViewData& Row : KeyBindingRows)
	{
		Row.CurrentKey = UserSettingsSubsystem->GetKeyBinding(
			Row.BindingId,
			Row.DefaultKey);
	}
}

void UOptionsWidget::RefreshKeyBindingPanel()
{
	if (KeyBindingListBox)
	{
		KeyBindingListBox->ClearChildren();

		if (KeyBindingRowWidgetClass)
		{
			for (const FSnowRumbleKeyBindingViewData& RowData : KeyBindingRows)
			{
				UOptionsKeyBindingRowWidget* RowWidget =
					CreateWidget<UOptionsKeyBindingRowWidget>(
						GetOwningPlayer(),
						KeyBindingRowWidgetClass);
				if (!RowWidget)
				{
					continue;
				}

				RowWidget->SetKeyBindingData(RowData);
				RowWidget->OnRebindRequestedNative.AddUObject(
					this,
					&UOptionsWidget::HandleKeyRowRebindRequested);
				RowWidget->OnResetRequestedNative.AddUObject(
					this,
					&UOptionsWidget::HandleKeyRowResetRequested);
				KeyBindingListBox->AddChild(RowWidget);
			}
		}
	}

	if (KeyBindingStatusText)
	{
		KeyBindingStatusText->SetText(FText::GetEmpty());
	}

	OnKeyBindingRowsRefreshed(KeyBindingRows);
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "OptionsWidget_C.h"

#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "../Player/SnowRumbleCharacter.h"
#include "../Player/SnowRumbleUserSettingsSubsystem_C.h"
#include "Engine/GameInstance.h"
#include "OptionsKeyBindingRowWidget_C.h"
#include "Sound/SoundClass.h"

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
	InitializeSensitivitySetting();
	InitializeAudioSettings();
	InitializeMicrophoneSettings();
	InitializeDefaultKeyBindingRows();
	BindOptionButtons();
	RefreshSensitivityValueText();
	RefreshAudioValueText();
	RefreshMicrophoneValueText();
	RefreshKeyBindingPanel();
	SetOptionsCategory(CurrentOptionsCategory);
	SetHasPendingOptionChanges(false);
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
	RefreshCategoryButtonSelection();
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

void UOptionsWidget::SetHasPendingOptionChanges(
	bool bNewHasPendingChanges)
{
	bHasPendingOptionChanges = bNewHasPendingChanges;
	RefreshApplyButtonEnabled();
}

bool UOptionsWidget::HasPendingOptionChanges() const
{
	return bHasPendingOptionChanges;
}

void UOptionsWidget::DiscardPendingOptionChanges()
{
	PendingKeyBindingId = NAME_None;
	InitializeSensitivitySetting();
	InitializeAudioSettings();
	InitializeMicrophoneSettings();
	InitializeDefaultKeyBindingRows();
	RefreshSensitivityValueText();
	RefreshAudioValueText();
	RefreshMicrophoneValueText();
	RefreshKeyBindingPanel();
	SetHasPendingOptionChanges(false);
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
	if (SensitivitySlider)
	{
		SensitivitySlider->OnValueChanged.AddUniqueDynamic(
			this,
			&UOptionsWidget::HandleSensitivitySliderValueChanged);
	}
	if (BgmVolumeSlider)
	{
		BgmVolumeSlider->OnValueChanged.AddUniqueDynamic(
			this,
			&UOptionsWidget::HandleBgmVolumeSliderValueChanged);
	}
	if (SfxVolumeSlider)
	{
		SfxVolumeSlider->OnValueChanged.AddUniqueDynamic(
			this,
			&UOptionsWidget::HandleSfxVolumeSliderValueChanged);
	}
	if (MicrophoneVolumeSlider)
	{
		MicrophoneVolumeSlider->OnValueChanged.AddUniqueDynamic(
			this,
			&UOptionsWidget::HandleMicrophoneVolumeSliderValueChanged);
	}
	if (MicrophonePushToTalkButton)
	{
		MicrophonePushToTalkButton->OnClicked.AddUniqueDynamic(
			this,
			&UOptionsWidget::HandleMicrophonePushToTalkButtonClicked);
	}
	if (MicrophoneAlwaysOnButton)
	{
		MicrophoneAlwaysOnButton->OnClicked.AddUniqueDynamic(
			this,
			&UOptionsWidget::HandleMicrophoneAlwaysOnButtonClicked);
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
	if (SensitivitySlider)
	{
		SensitivitySlider->OnValueChanged.RemoveAll(this);
	}
	if (BgmVolumeSlider)
	{
		BgmVolumeSlider->OnValueChanged.RemoveAll(this);
	}
	if (SfxVolumeSlider)
	{
		SfxVolumeSlider->OnValueChanged.RemoveAll(this);
	}
	if (MicrophoneVolumeSlider)
	{
		MicrophoneVolumeSlider->OnValueChanged.RemoveAll(this);
	}
	if (MicrophonePushToTalkButton)
	{
		MicrophonePushToTalkButton->OnClicked.RemoveAll(this);
	}
	if (MicrophoneAlwaysOnButton)
	{
		MicrophoneAlwaysOnButton->OnClicked.RemoveAll(this);
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
	DiscardPendingOptionChanges();
	OnOptionsCloseRequested();
	OnOptionsCloseRequestedNative.Broadcast();
}

void UOptionsWidget::HandleApplyButtonClicked()
{
	ApplyPendingOptionChanges();
	OnOptionsApplyRequested();
	SetHasPendingOptionChanges(false);
}

void UOptionsWidget::HandleResetButtonClicked()
{
	ResetCurrentOptionsCategory();
	OnOptionsResetRequested();
	OnOptionsCategoryResetRequested(CurrentOptionsCategory);
}

void UOptionsWidget::HandleSensitivitySliderValueChanged(float NewValue)
{
	if (bIsUpdatingSensitivitySlider)
	{
		return;
	}

	PendingMouseSensitivity = ConvertSliderValueToSensitivity(NewValue);
	RefreshSensitivityValueText();
	SetHasPendingOptionChanges(HasAnyPendingOptionChanges());
}

void UOptionsWidget::HandleBgmVolumeSliderValueChanged(float NewValue)
{
	if (bIsUpdatingAudioSliders)
	{
		return;
	}

	PendingBgmVolume = FMath::Clamp(NewValue, 0.0f, 1.0f);
	RefreshAudioValueText();
	SetHasPendingOptionChanges(HasAnyPendingOptionChanges());
}

void UOptionsWidget::HandleSfxVolumeSliderValueChanged(float NewValue)
{
	if (bIsUpdatingAudioSliders)
	{
		return;
	}

	PendingSfxVolume = FMath::Clamp(NewValue, 0.0f, 1.0f);
	RefreshAudioValueText();
	SetHasPendingOptionChanges(HasAnyPendingOptionChanges());
}

void UOptionsWidget::HandleMicrophoneVolumeSliderValueChanged(float NewValue)
{
	if (bIsUpdatingMicrophoneSlider)
	{
		return;
	}

	PendingMicrophoneVolume = FMath::Clamp(NewValue, 0.0f, 1.0f);
	RefreshMicrophoneValueText();
	SetHasPendingOptionChanges(HasAnyPendingOptionChanges());
}

void UOptionsWidget::HandleMicrophonePushToTalkButtonClicked()
{
	PendingMicrophoneMode = ESnowRumbleMicrophoneMode::PushToTalk;
	OnMicrophoneModeChanged(PendingMicrophoneMode);
	RefreshMicrophoneModeButtonSelection();
	SetHasPendingOptionChanges(HasAnyPendingOptionChanges());
}

void UOptionsWidget::HandleMicrophoneAlwaysOnButtonClicked()
{
	PendingMicrophoneMode = ESnowRumbleMicrophoneMode::AlwaysOn;
	OnMicrophoneModeChanged(PendingMicrophoneMode);
	RefreshMicrophoneModeButtonSelection();
	SetHasPendingOptionChanges(HasAnyPendingOptionChanges());
}

void UOptionsWidget::HandleKeyRowRebindRequested(FName BindingId)
{
	BeginKeyRebind(BindingId);
}

void UOptionsWidget::HandleKeyRowResetRequested(FName BindingId)
{
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

	RefreshKeyBindingPanel();
	SetHasPendingOptionChanges(HasAnyPendingOptionChanges());
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

void UOptionsWidget::ApplyPendingOptionChanges()
{
	UGameInstance* GameInstance = GetGameInstance();
	USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem = GameInstance
		? GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>()
		: nullptr;

	if (UserSettingsSubsystem)
	{
		UserSettingsSubsystem->SetMouseSensitivity(PendingMouseSensitivity);
		UserSettingsSubsystem->SetBgmVolume(PendingBgmVolume);
		UserSettingsSubsystem->SetSfxVolume(PendingSfxVolume);
		UserSettingsSubsystem->SetMicrophoneVolume(PendingMicrophoneVolume);
		UserSettingsSubsystem->SetMicrophoneMode(PendingMicrophoneMode);
		for (const FSnowRumbleKeyBindingViewData& Row : KeyBindingRows)
		{
			if (Row.CurrentKey == Row.DefaultKey)
			{
				UserSettingsSubsystem->ResetKeyBinding(Row.BindingId);
			}
			else
			{
				UserSettingsSubsystem->SetKeyBinding(
					Row.BindingId,
					Row.CurrentKey);
			}
		}
		ApplyAudioVolumeSettings();
	}

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		if (ASnowRumbleCharacter* SnowRumbleCharacter =
			Cast<ASnowRumbleCharacter>(PlayerController->GetPawn()))
		{
			SnowRumbleCharacter->ApplyInputMappingContext();
		}
	}
}

void UOptionsWidget::ResetCurrentOptionsCategory()
{
	switch (CurrentOptionsCategory)
	{
	case ESnowRumbleOptionsCategory::Sensitivity:
		if (const UGameInstance* GameInstance = GetGameInstance())
		{
			if (const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
				GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>())
			{
				PendingMouseSensitivity =
					UserSettingsSubsystem->GetDefaultMouseSensitivity();
			}
		}
		RefreshSensitivityValueText();
		if (SensitivitySlider)
		{
			bIsUpdatingSensitivitySlider = true;
			SensitivitySlider->SetValue(
				ConvertSensitivityToSliderValue(PendingMouseSensitivity));
			bIsUpdatingSensitivitySlider = false;
		}
		SetHasPendingOptionChanges(HasAnyPendingOptionChanges());
		break;
	case ESnowRumbleOptionsCategory::KeyBinding:
		ResetAllKeyBindingsToDefault();
		break;
	case ESnowRumbleOptionsCategory::Audio:
		if (const UGameInstance* GameInstance = GetGameInstance())
		{
			if (const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
				GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>())
			{
				PendingBgmVolume =
					UserSettingsSubsystem->GetDefaultAudioVolume();
				PendingSfxVolume =
					UserSettingsSubsystem->GetDefaultAudioVolume();
			}
		}
		if (BgmVolumeSlider || SfxVolumeSlider)
		{
			bIsUpdatingAudioSliders = true;
			if (BgmVolumeSlider)
			{
				BgmVolumeSlider->SetValue(PendingBgmVolume);
			}
			if (SfxVolumeSlider)
			{
				SfxVolumeSlider->SetValue(PendingSfxVolume);
			}
			bIsUpdatingAudioSliders = false;
		}
		RefreshAudioValueText();
		SetHasPendingOptionChanges(HasAnyPendingOptionChanges());
		break;
	case ESnowRumbleOptionsCategory::Microphone:
		if (const UGameInstance* GameInstance = GetGameInstance())
		{
			if (const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
				GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>())
			{
				PendingMicrophoneVolume =
					UserSettingsSubsystem->GetDefaultMicrophoneVolume();
				PendingMicrophoneMode =
					UserSettingsSubsystem->GetDefaultMicrophoneMode();
			}
		}
		if (MicrophoneVolumeSlider)
		{
			bIsUpdatingMicrophoneSlider = true;
			MicrophoneVolumeSlider->SetValue(PendingMicrophoneVolume);
			bIsUpdatingMicrophoneSlider = false;
		}
		RefreshMicrophoneValueText();
		OnMicrophoneModeChanged(PendingMicrophoneMode);
		RefreshMicrophoneModeButtonSelection();
		SetHasPendingOptionChanges(HasAnyPendingOptionChanges());
		break;
	default:
		break;
	}
}

void UOptionsWidget::ResetAllKeyBindingsToDefault()
{
	PendingKeyBindingId = NAME_None;

	for (FSnowRumbleKeyBindingViewData& Row : KeyBindingRows)
	{
		Row.CurrentKey = Row.DefaultKey;
	}

	RefreshKeyBindingPanel();
	SetHasPendingOptionChanges(HasAnyPendingOptionChanges());
	if (KeyBindingStatusText)
	{
		KeyBindingStatusText->SetText(
			NSLOCTEXT(
				"SnowRumble",
				"KeyBindingAllResetRequested",
				"키 설정 기본값 복원"));
	}
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
	for (FSnowRumbleKeyBindingViewData& Row : KeyBindingRows)
	{
		if (Row.BindingId != BindingId && Row.CurrentKey == NewKey)
		{
			Row.CurrentKey = EKeys::Invalid;
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

	RefreshKeyBindingPanel();
	SetHasPendingOptionChanges(HasAnyPendingOptionChanges());

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
	KeyBindingRows.Add(MakeKeyBindingRow(
		TEXT("MicrophonePushToTalk"),
		NSLOCTEXT("SnowRumble", "KeyBindingMicrophonePushToTalk", "마이크 입력"),
		EKeys::K));
	KeyBindingRows.Add(MakeKeyBindingRow(
		TEXT("MicrophoneChannelToggle"),
		NSLOCTEXT("SnowRumble", "KeyBindingMicrophoneChannelToggle", "마이크 채널 전환"),
		EKeys::N));
	KeyBindingRows.Add(MakeKeyBindingRow(
		TEXT("VoiceTargetMute"),
		NSLOCTEXT("SnowRumble", "KeyBindingVoiceTargetMute", "플레이어 음소거"),
		EKeys::M));

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
	RefreshApplyButtonEnabled();
}

void UOptionsWidget::InitializeSensitivitySetting()
{
	const UGameInstance* GameInstance = GetGameInstance();
	const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
		GameInstance
			? GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>()
			: nullptr;

	PendingMouseSensitivity = UserSettingsSubsystem
		? UserSettingsSubsystem->GetMouseSensitivity()
		: 1.0f;

	if (SensitivitySlider)
	{
		bIsUpdatingSensitivitySlider = true;
		SensitivitySlider->SetValue(
			ConvertSensitivityToSliderValue(PendingMouseSensitivity));
		bIsUpdatingSensitivitySlider = false;
	}
}

void UOptionsWidget::InitializeAudioSettings()
{
	const UGameInstance* GameInstance = GetGameInstance();
	const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
		GameInstance
			? GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>()
			: nullptr;

	PendingBgmVolume = UserSettingsSubsystem
		? UserSettingsSubsystem->GetBgmVolume()
		: 1.0f;
	PendingSfxVolume = UserSettingsSubsystem
		? UserSettingsSubsystem->GetSfxVolume()
		: 1.0f;

	if (BgmVolumeSlider || SfxVolumeSlider)
	{
		bIsUpdatingAudioSliders = true;
		if (BgmVolumeSlider)
		{
			BgmVolumeSlider->SetValue(PendingBgmVolume);
		}
		if (SfxVolumeSlider)
		{
			SfxVolumeSlider->SetValue(PendingSfxVolume);
		}
		bIsUpdatingAudioSliders = false;
	}

	ApplyAudioVolumeSettings();
}

void UOptionsWidget::InitializeMicrophoneSettings()
{
	const UGameInstance* GameInstance = GetGameInstance();
	const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
		GameInstance
			? GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>()
			: nullptr;

	PendingMicrophoneVolume = UserSettingsSubsystem
		? UserSettingsSubsystem->GetMicrophoneVolume()
		: 1.0f;
	PendingMicrophoneMode = UserSettingsSubsystem
		? UserSettingsSubsystem->GetMicrophoneMode()
		: ESnowRumbleMicrophoneMode::PushToTalk;

	if (MicrophoneVolumeSlider)
	{
		bIsUpdatingMicrophoneSlider = true;
		MicrophoneVolumeSlider->SetValue(PendingMicrophoneVolume);
		bIsUpdatingMicrophoneSlider = false;
	}

	OnMicrophoneModeChanged(PendingMicrophoneMode);
	RefreshMicrophoneModeButtonSelection();
}

void UOptionsWidget::ApplyAudioVolumeSettings() const
{
	if (BgmSoundClass)
	{
		BgmSoundClass->Properties.Volume = PendingBgmVolume;
	}
	if (SfxSoundClass)
	{
		SfxSoundClass->Properties.Volume = PendingSfxVolume;
	}
}

void UOptionsWidget::RefreshApplyButtonEnabled()
{
	if (ApplyButton)
	{
		ApplyButton->SetIsEnabled(bHasPendingOptionChanges);
	}
}

bool UOptionsWidget::HasPendingKeyBindingChanges() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
		GameInstance
			? GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>()
			: nullptr;

	for (const FSnowRumbleKeyBindingViewData& Row : KeyBindingRows)
	{
		const FKey SavedKey = UserSettingsSubsystem
			? UserSettingsSubsystem->GetKeyBinding(
				Row.BindingId,
				Row.DefaultKey)
			: Row.DefaultKey;
		if (Row.CurrentKey != SavedKey)
		{
			return true;
		}
	}

	return false;
}

bool UOptionsWidget::HasAnyPendingOptionChanges() const
{
	return HasPendingSensitivityChanges()
		|| HasPendingAudioChanges()
		|| HasPendingMicrophoneChanges()
		|| HasPendingKeyBindingChanges();
}

bool UOptionsWidget::HasPendingSensitivityChanges() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
		GameInstance
			? GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>()
			: nullptr;
	const float SavedSensitivity = UserSettingsSubsystem
		? UserSettingsSubsystem->GetMouseSensitivity()
		: 1.0f;

	return !FMath::IsNearlyEqual(
		PendingMouseSensitivity,
		SavedSensitivity,
		0.001f);
}

bool UOptionsWidget::HasPendingAudioChanges() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
		GameInstance
			? GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>()
			: nullptr;
	const float SavedBgmVolume = UserSettingsSubsystem
		? UserSettingsSubsystem->GetBgmVolume()
		: 1.0f;
	const float SavedSfxVolume = UserSettingsSubsystem
		? UserSettingsSubsystem->GetSfxVolume()
		: 1.0f;

	return !FMath::IsNearlyEqual(PendingBgmVolume, SavedBgmVolume, 0.001f)
		|| !FMath::IsNearlyEqual(PendingSfxVolume, SavedSfxVolume, 0.001f);
}

bool UOptionsWidget::HasPendingMicrophoneChanges() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
		GameInstance
			? GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>()
			: nullptr;
	const float SavedMicrophoneVolume = UserSettingsSubsystem
		? UserSettingsSubsystem->GetMicrophoneVolume()
		: 1.0f;
	const ESnowRumbleMicrophoneMode SavedMicrophoneMode =
		UserSettingsSubsystem
			? UserSettingsSubsystem->GetMicrophoneMode()
			: ESnowRumbleMicrophoneMode::PushToTalk;

	return !FMath::IsNearlyEqual(
			PendingMicrophoneVolume,
			SavedMicrophoneVolume,
			0.001f)
		|| PendingMicrophoneMode != SavedMicrophoneMode;
}

float UOptionsWidget::ConvertSliderValueToSensitivity(
	float SliderValue) const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
		GameInstance
			? GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>()
			: nullptr;
	const float MinSensitivity = UserSettingsSubsystem
		? UserSettingsSubsystem->GetMinMouseSensitivity()
		: 0.2f;
	const float MaxSensitivity = UserSettingsSubsystem
		? UserSettingsSubsystem->GetMaxMouseSensitivity()
		: 3.0f;

	return FMath::Lerp(
		MinSensitivity,
		MaxSensitivity,
		FMath::Clamp(SliderValue, 0.0f, 1.0f));
}

float UOptionsWidget::ConvertSensitivityToSliderValue(float Sensitivity) const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
		GameInstance
			? GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>()
			: nullptr;
	const float MinSensitivity = UserSettingsSubsystem
		? UserSettingsSubsystem->GetMinMouseSensitivity()
		: 0.2f;
	const float MaxSensitivity = UserSettingsSubsystem
		? UserSettingsSubsystem->GetMaxMouseSensitivity()
		: 3.0f;

	return FMath::GetRangePct(
		MinSensitivity,
		MaxSensitivity,
		FMath::Clamp(Sensitivity, MinSensitivity, MaxSensitivity));
}

void UOptionsWidget::RefreshSensitivityValueText()
{
	if (SensitivityValueText)
	{
		SensitivityValueText->SetText(FText::Format(
			NSLOCTEXT("SnowRumble", "SensitivityValuePercent", "{0}%"),
			FText::AsNumber(FMath::RoundToInt(
				PendingMouseSensitivity * 100.0f))));
	}
}

void UOptionsWidget::RefreshAudioValueText()
{
	if (BgmVolumeValueText)
	{
		BgmVolumeValueText->SetText(FText::Format(
			NSLOCTEXT("SnowRumble", "BgmVolumeValuePercent", "{0}%"),
			FText::AsNumber(FMath::RoundToInt(PendingBgmVolume * 100.0f))));
	}
	if (SfxVolumeValueText)
	{
		SfxVolumeValueText->SetText(FText::Format(
			NSLOCTEXT("SnowRumble", "SfxVolumeValuePercent", "{0}%"),
			FText::AsNumber(FMath::RoundToInt(PendingSfxVolume * 100.0f))));
	}
}

void UOptionsWidget::RefreshMicrophoneValueText()
{
	if (MicrophoneVolumeValueText)
	{
		MicrophoneVolumeValueText->SetText(FText::Format(
			NSLOCTEXT("SnowRumble", "MicrophoneVolumeValuePercent", "{0}%"),
			FText::AsNumber(FMath::RoundToInt(
				PendingMicrophoneVolume * 100.0f))));
	}
}

void UOptionsWidget::SetButtonSelectedVisual(UButton* Button, bool bSelected)
{
	if (!Button)
	{
		return;
	}

	FButtonStyle* CachedStyle = DefaultButtonStyles.Find(Button);
	if (!CachedStyle)
	{
		DefaultButtonStyles.Add(Button, Button->GetStyle());
		CachedStyle = DefaultButtonStyles.Find(Button);
	}
	if (!CachedStyle)
	{
		return;
	}

	if (!bSelected)
	{
		Button->SetStyle(*CachedStyle);
		return;
	}

	FButtonStyle SelectedStyle = *CachedStyle;
	SelectedStyle.SetNormal(CachedStyle->Pressed);
	SelectedStyle.SetHovered(CachedStyle->Pressed);
	SelectedStyle.SetPressed(CachedStyle->Pressed);
	Button->SetStyle(SelectedStyle);
}

void UOptionsWidget::RefreshCategoryButtonSelection()
{
	SetButtonSelectedVisual(
		SensitivityCategoryButton,
		CurrentOptionsCategory == ESnowRumbleOptionsCategory::Sensitivity);
	SetButtonSelectedVisual(
		AudioCategoryButton,
		CurrentOptionsCategory == ESnowRumbleOptionsCategory::Audio);
	SetButtonSelectedVisual(
		KeyBindingCategoryButton,
		CurrentOptionsCategory == ESnowRumbleOptionsCategory::KeyBinding);
	SetButtonSelectedVisual(
		MicrophoneCategoryButton,
		CurrentOptionsCategory == ESnowRumbleOptionsCategory::Microphone);
}

void UOptionsWidget::RefreshMicrophoneModeButtonSelection()
{
	SetButtonSelectedVisual(
		MicrophonePushToTalkButton,
		PendingMicrophoneMode == ESnowRumbleMicrophoneMode::PushToTalk);
	SetButtonSelectedVisual(
		MicrophoneAlwaysOnButton,
		PendingMicrophoneMode == ESnowRumbleMicrophoneMode::AlwaysOn);
}

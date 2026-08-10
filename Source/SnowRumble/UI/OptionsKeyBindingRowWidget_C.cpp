// Copyright Epic Games, Inc. All Rights Reserved.

#include "OptionsKeyBindingRowWidget_C.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UOptionsKeyBindingRowWidget::SetKeyBindingData(
	const FSnowRumbleKeyBindingViewData& NewData)
{
	Data = NewData;
	RefreshDisplayedText();
	OnKeyBindingDataChanged(Data);
}

FName UOptionsKeyBindingRowWidget::GetBindingId() const
{
	return Data.BindingId;
}

void UOptionsKeyBindingRowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindRowButtons();
	RefreshDisplayedText();
}

void UOptionsKeyBindingRowWidget::NativeDestruct()
{
	UnbindRowButtons();

	Super::NativeDestruct();
}

void UOptionsKeyBindingRowWidget::HandleRebindButtonClicked()
{
	OnRebindRequestedNative.Broadcast(Data.BindingId);
}

void UOptionsKeyBindingRowWidget::HandleResetButtonClicked()
{
	OnResetRequestedNative.Broadcast(Data.BindingId);
}

void UOptionsKeyBindingRowWidget::BindRowButtons()
{
	if (RebindButton)
	{
		RebindButton->OnClicked.AddUniqueDynamic(
			this,
			&UOptionsKeyBindingRowWidget::HandleRebindButtonClicked);
	}
	if (ResetButton)
	{
		ResetButton->OnClicked.AddUniqueDynamic(
			this,
			&UOptionsKeyBindingRowWidget::HandleResetButtonClicked);
	}
}

void UOptionsKeyBindingRowWidget::UnbindRowButtons()
{
	if (RebindButton)
	{
		RebindButton->OnClicked.RemoveAll(this);
	}
	if (ResetButton)
	{
		ResetButton->OnClicked.RemoveAll(this);
	}
}

void UOptionsKeyBindingRowWidget::RefreshDisplayedText()
{
	if (ActionNameText)
	{
		ActionNameText->SetText(Data.DisplayName);
	}
	if (CurrentKeyText)
	{
		CurrentKeyText->SetText(
			Data.CurrentKey.IsValid()
				? Data.CurrentKey.GetDisplayName()
				: NSLOCTEXT("SnowRumble", "KeyBindingUnassigned", "미할당"));
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomizationWidget_C.h"

#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "CustomizationPlayerController_C.h"
#include "InputCoreTypes.h"
#include "Input/Reply.h"

void UCustomizationWidget::SetCustomizationPlayerController(
	ACustomizationPlayerController* NewPlayerController)
{
	CustomizationPlayerController = NewPlayerController;
}

void UCustomizationWidget::SetCustomizationPage(
	ESnowRumbleCustomizationPage NewPage)
{
	CurrentCustomizationPage = NewPage;
	if (CustomizationContentSwitcher)
	{
		const int32 SwitcherIndex = GetSwitcherIndexForPage(NewPage);
		if (CustomizationContentSwitcher->GetNumWidgets() > SwitcherIndex)
		{
			CustomizationContentSwitcher->SetActiveWidgetIndex(SwitcherIndex);
		}
	}

	OnCustomizationPageChanged(NewPage);
}

ESnowRumbleCustomizationPage
UCustomizationWidget::GetCurrentCustomizationPage() const
{
	return CurrentCustomizationPage;
}

void UCustomizationWidget::SetPreviewBodyColor(FLinearColor NewBodyColor)
{
	if (CustomizationPlayerController)
	{
		CustomizationPlayerController->SetPreviewBodyColor(NewBodyColor);
	}
}

FSnowRumbleCustomizationData
UCustomizationWidget::GetPreviewCustomizationData() const
{
	return CustomizationPlayerController
		? CustomizationPlayerController->GetPreviewCustomizationData()
		: FSnowRumbleCustomizationData();
}

void UCustomizationWidget::RequestUndoLastPaintStroke()
{
	if (CurrentCustomizationPage == ESnowRumbleCustomizationPage::PaintMode
		&& CustomizationPlayerController)
	{
		CustomizationPlayerController->UndoLastPaintStroke();
	}
}

void UCustomizationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	BindCustomizationButtons();
	SetCustomizationPage(CurrentCustomizationPage);
}

void UCustomizationWidget::NativeDestruct()
{
	UnbindCustomizationButtons();

	Super::NativeDestruct();
}

FReply UCustomizationWidget::NativeOnKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent)
{
	if (CurrentCustomizationPage == ESnowRumbleCustomizationPage::PaintMode
		&& InKeyEvent.GetKey() == EKeys::Z
		&& InKeyEvent.IsControlDown())
	{
		RequestUndoLastPaintStroke();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UCustomizationWidget::HandlePaintModeButtonClicked()
{
	SetCustomizationPage(ESnowRumbleCustomizationPage::PaintMode);
}

void UCustomizationWidget::HandleRotateLeftButtonPressed()
{
	if (CustomizationPlayerController)
	{
		CustomizationPlayerController->StartRotatePreviewLeft();
	}
}

void UCustomizationWidget::HandleRotateRightButtonPressed()
{
	if (CustomizationPlayerController)
	{
		CustomizationPlayerController->StartRotatePreviewRight();
	}
}

void UCustomizationWidget::HandleRotateButtonReleased()
{
	if (CustomizationPlayerController)
	{
		CustomizationPlayerController->StopRotatePreview();
	}
}

void UCustomizationWidget::HandleReturnToLobbyButtonClicked()
{
	if (CustomizationPlayerController)
	{
		CustomizationPlayerController->ReturnToMainMenu();
	}
}

void UCustomizationWidget::HandleBackButtonClicked()
{
	if (CurrentCustomizationPage == ESnowRumbleCustomizationPage::PaintMode)
	{
		RequestUndoLastPaintStroke();
		return;
	}

	SetCustomizationPage(ESnowRumbleCustomizationPage::Main);
}

void UCustomizationWidget::HandleApplyButtonClicked()
{
	if (CustomizationPlayerController)
	{
		CustomizationPlayerController->ApplyPreviewCustomization();
	}

	OnCustomizationApplyRequested(CurrentCustomizationPage);
}

void UCustomizationWidget::HandleResetButtonClicked()
{
	if (CustomizationPlayerController)
	{
		if (CurrentCustomizationPage == ESnowRumbleCustomizationPage::PaintMode)
		{
			CustomizationPlayerController->ResetPaintStrokes();
		}
		else
		{
			CustomizationPlayerController->ResetPreviewCustomization();
		}
	}

	OnCustomizationResetRequested(CurrentCustomizationPage);
}

void UCustomizationWidget::BindCustomizationButtons()
{
	if (PaintModeButton)
	{
		PaintModeButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandlePaintModeButtonClicked);
	}
	if (ReturnToLobbyButton)
	{
		ReturnToLobbyButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleReturnToLobbyButtonClicked);
	}
	if (RotateLeftButton)
	{
		RotateLeftButton->OnPressed.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleRotateLeftButtonPressed);
		RotateLeftButton->OnReleased.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleRotateButtonReleased);
	}
	if (RotateRightButton)
	{
		RotateRightButton->OnPressed.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleRotateRightButtonPressed);
		RotateRightButton->OnReleased.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleRotateButtonReleased);
	}
	if (BackButton)
	{
		BackButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleBackButtonClicked);
	}
	if (ApplyButton)
	{
		ApplyButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleApplyButtonClicked);
	}
	if (ResetButton)
	{
		ResetButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleResetButtonClicked);
	}
}

void UCustomizationWidget::UnbindCustomizationButtons()
{
	if (PaintModeButton)
	{
		PaintModeButton->OnClicked.RemoveAll(this);
	}
	if (ReturnToLobbyButton)
	{
		ReturnToLobbyButton->OnClicked.RemoveAll(this);
	}
	if (RotateLeftButton)
	{
		RotateLeftButton->OnPressed.RemoveAll(this);
		RotateLeftButton->OnReleased.RemoveAll(this);
	}
	if (RotateRightButton)
	{
		RotateRightButton->OnPressed.RemoveAll(this);
		RotateRightButton->OnReleased.RemoveAll(this);
	}
	if (BackButton)
	{
		BackButton->OnClicked.RemoveAll(this);
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

int32 UCustomizationWidget::GetSwitcherIndexForPage(
	ESnowRumbleCustomizationPage Page) const
{
	switch (Page)
	{
	case ESnowRumbleCustomizationPage::ViewMode:
		return 1;
	case ESnowRumbleCustomizationPage::PaintMode:
		return 2;
	case ESnowRumbleCustomizationPage::Main:
	default:
		return 0;
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomizationWidget_C.h"

#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/Image.h"
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
	if (CustomizationPlayerController)
	{
		CustomizationPlayerController->SetPaintCursorActive(
			NewPage == ESnowRumbleCustomizationPage::PaintMode);
	}
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

FLinearColor UCustomizationWidget::GetPaintBrushColor() const
{
	return CustomizationPlayerController
		? CustomizationPlayerController->GetPaintBrushColor()
		: FLinearColor::Black;
}

float UCustomizationWidget::GetPaintBrushSize() const
{
	return CustomizationPlayerController
		? CustomizationPlayerController->GetPaintBrushSize()
		: 0.0f;
}

int32 UCustomizationWidget::GetPreviewHatMeshIndex() const
{
	return CustomizationPlayerController
		? CustomizationPlayerController->GetPreviewHatMeshIndex()
		: INDEX_NONE;
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
	RefreshPaintBrushPreview();
}

void UCustomizationWidget::NativeDestruct()
{
	UnbindCustomizationButtons();

	Super::NativeDestruct();
}

void UCustomizationWidget::NativeTick(
	const FGeometry& MyGeometry,
	float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshPaintBrushPreview();
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

FReply UCustomizationWidget::NativeOnMouseWheel(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (CurrentCustomizationPage == ESnowRumbleCustomizationPage::PaintMode
		&& bIsBrushSizeButtonPressed
		&& CustomizationPlayerController)
	{
		CustomizationPlayerController->AdjustPaintBrushSizeFromWheel(
			InMouseEvent.GetWheelDelta());
		return FReply::Handled();
	}

	return Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
}

void UCustomizationWidget::HandlePaintModeButtonClicked()
{
	SetCustomizationPage(
		CurrentCustomizationPage == ESnowRumbleCustomizationPage::PaintMode
			? ESnowRumbleCustomizationPage::Main
			: ESnowRumbleCustomizationPage::PaintMode);
}

void UCustomizationWidget::HandleHatModeButtonClicked()
{
	SetCustomizationPage(ESnowRumbleCustomizationPage::HatMode);
}

void UCustomizationWidget::HandleBrushColorButtonClicked()
{
	RefreshPaintBrushPreview();
}

void UCustomizationWidget::HandleRedBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		RedBrushColorButton,
		FLinearColor::Red);
}

void UCustomizationWidget::HandleOrangeBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		OrangeBrushColorButton,
		FLinearColor(1.0f, 0.45f, 0.0f, 1.0f));
}

void UCustomizationWidget::HandleYellowBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		YellowBrushColorButton,
		FLinearColor::Yellow);
}

void UCustomizationWidget::HandleGreenBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		GreenBrushColorButton,
		FLinearColor::Green);
}

void UCustomizationWidget::HandleBlueBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		BlueBrushColorButton,
		FLinearColor::Blue);
}

void UCustomizationWidget::HandleIndigoBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		IndigoBrushColorButton,
		FLinearColor(0.25f, 0.0f, 0.55f, 1.0f));
}

void UCustomizationWidget::HandlePurpleBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		PurpleBrushColorButton,
		FLinearColor(0.55f, 0.0f, 1.0f, 1.0f));
}

void UCustomizationWidget::HandleBlackBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		BlackBrushColorButton,
		FLinearColor::Black);
}

void UCustomizationWidget::HandleWhiteBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		WhiteBrushColorButton,
		FLinearColor::White);
}

void UCustomizationWidget::HandleBrushSizeButtonPressed()
{
	bIsBrushSizeButtonPressed = true;
	if (CustomizationPlayerController)
	{
		CustomizationPlayerController->StartAdjustPaintBrushSize();
	}
}

void UCustomizationWidget::HandleBrushSizeButtonReleased()
{
	bIsBrushSizeButtonPressed = false;
	if (CustomizationPlayerController)
	{
		CustomizationPlayerController->StopAdjustPaintBrushSize();
	}
}

void UCustomizationWidget::HandleFillBodyColorButtonClicked()
{
	if (CustomizationPlayerController)
	{
		CustomizationPlayerController->FillPreviewBodyWithBrushColor();
		RefreshPaintBrushPreview();
	}
}

void UCustomizationWidget::HandleHatPreviousButtonClicked()
{
	if (CustomizationPlayerController)
	{
		CustomizationPlayerController->SelectPreviousPreviewHat();
	}
}

void UCustomizationWidget::HandleHatNextButtonClicked()
{
	if (CustomizationPlayerController)
	{
		CustomizationPlayerController->SelectNextPreviewHat();
	}
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
	if (HatModeButton)
	{
		HatModeButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleHatModeButtonClicked);
	}
	if (BrushColorButton)
	{
		BrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleBrushColorButtonClicked);
	}
	if (RedBrushColorButton)
	{
		RedBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleRedBrushColorButtonClicked);
	}
	if (OrangeBrushColorButton)
	{
		OrangeBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleOrangeBrushColorButtonClicked);
	}
	if (YellowBrushColorButton)
	{
		YellowBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleYellowBrushColorButtonClicked);
	}
	if (GreenBrushColorButton)
	{
		GreenBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleGreenBrushColorButtonClicked);
	}
	if (BlueBrushColorButton)
	{
		BlueBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleBlueBrushColorButtonClicked);
	}
	if (IndigoBrushColorButton)
	{
		IndigoBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleIndigoBrushColorButtonClicked);
	}
	if (PurpleBrushColorButton)
	{
		PurpleBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandlePurpleBrushColorButtonClicked);
	}
	if (BlackBrushColorButton)
	{
		BlackBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleBlackBrushColorButtonClicked);
	}
	if (WhiteBrushColorButton)
	{
		WhiteBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleWhiteBrushColorButtonClicked);
	}
	if (BrushSizeButton)
	{
		BrushSizeButton->OnPressed.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleBrushSizeButtonPressed);
		BrushSizeButton->OnReleased.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleBrushSizeButtonReleased);
	}
	if (FillBodyColorButton)
	{
		FillBodyColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleFillBodyColorButtonClicked);
	}
	if (HatPreviousButton)
	{
		HatPreviousButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleHatPreviousButtonClicked);
	}
	if (HatNextButton)
	{
		HatNextButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleHatNextButtonClicked);
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
	if (HatModeButton)
	{
		HatModeButton->OnClicked.RemoveAll(this);
	}
	if (BrushColorButton)
	{
		BrushColorButton->OnClicked.RemoveAll(this);
	}
	UnbindPaletteColorButton(RedBrushColorButton);
	UnbindPaletteColorButton(OrangeBrushColorButton);
	UnbindPaletteColorButton(YellowBrushColorButton);
	UnbindPaletteColorButton(GreenBrushColorButton);
	UnbindPaletteColorButton(BlueBrushColorButton);
	UnbindPaletteColorButton(IndigoBrushColorButton);
	UnbindPaletteColorButton(PurpleBrushColorButton);
	UnbindPaletteColorButton(BlackBrushColorButton);
	UnbindPaletteColorButton(WhiteBrushColorButton);
	if (BrushSizeButton)
	{
		BrushSizeButton->OnPressed.RemoveAll(this);
		BrushSizeButton->OnReleased.RemoveAll(this);
	}
	if (FillBodyColorButton)
	{
		FillBodyColorButton->OnClicked.RemoveAll(this);
	}
	if (HatPreviousButton)
	{
		HatPreviousButton->OnClicked.RemoveAll(this);
	}
	if (HatNextButton)
	{
		HatNextButton->OnClicked.RemoveAll(this);
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

void UCustomizationWidget::RefreshPaintBrushPreview()
{
	const FLinearColor CurrentBrushColor = GetPaintBrushColor();
	if (BrushColorPreviewBorder)
	{
		BrushColorPreviewBorder->SetBrushColor(CurrentBrushColor);
	}
	if (BrushColorPreviewImage)
	{
		BrushColorPreviewImage->SetColorAndOpacity(CurrentBrushColor);
	}
	RefreshPaletteButtonSelection();
}

void UCustomizationWidget::SetPaintBrushColorFromPalette(
	FLinearColor NewBrushColor)
{
	if (CustomizationPlayerController)
	{
		CustomizationPlayerController->SetPaintBrushColor(NewBrushColor);
	}

	RefreshPaintBrushPreview();
}

void UCustomizationWidget::SetPaintBrushColorFromPaletteButton(
	UButton* Button,
	FLinearColor FallbackColor)
{
	SetPaintBrushColorFromPalette(
		GetPaletteButtonBrushColor(Button, FallbackColor));
}

FLinearColor UCustomizationWidget::GetPaletteButtonBrushColor(
	UButton* Button,
	FLinearColor FallbackColor)
{
	if (!Button)
	{
		return FallbackColor;
	}

	FButtonStyle* CachedStyle = DefaultPaletteButtonStyles.Find(Button);
	if (!CachedStyle)
	{
		DefaultPaletteButtonStyles.Add(Button, Button->GetStyle());
		CachedStyle = DefaultPaletteButtonStyles.Find(Button);
	}
	if (!CachedStyle)
	{
		return FallbackColor;
	}

	FLinearColor BrushColor = CachedStyle->Normal.TintColor.GetSpecifiedColor();
	BrushColor *= Button->GetBackgroundColor();
	if (BrushColor.A <= 0.0f)
	{
		BrushColor.A = FallbackColor.A;
	}
	return BrushColor;
}

void UCustomizationWidget::UnbindPaletteColorButton(UButton* Button)
{
	if (Button)
	{
		Button->OnClicked.RemoveAll(this);
	}
}

void UCustomizationWidget::RefreshPaletteButtonSelection()
{
	const FLinearColor CurrentBrushColor = GetPaintBrushColor();

	auto IsSameColor = [&CurrentBrushColor](const FLinearColor& PaletteColor)
	{
		return FMath::IsNearlyEqual(CurrentBrushColor.R, PaletteColor.R, 0.01f)
			&& FMath::IsNearlyEqual(CurrentBrushColor.G, PaletteColor.G, 0.01f)
			&& FMath::IsNearlyEqual(CurrentBrushColor.B, PaletteColor.B, 0.01f)
			&& FMath::IsNearlyEqual(CurrentBrushColor.A, PaletteColor.A, 0.01f);
	};

	SetButtonPressedVisual(
		RedBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(RedBrushColorButton, FLinearColor::Red)));
	SetButtonPressedVisual(
		OrangeBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				OrangeBrushColorButton,
				FLinearColor(1.0f, 0.45f, 0.0f, 1.0f))));
	SetButtonPressedVisual(
		YellowBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				YellowBrushColorButton,
				FLinearColor::Yellow)));
	SetButtonPressedVisual(
		GreenBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				GreenBrushColorButton,
				FLinearColor::Green)));
	SetButtonPressedVisual(
		BlueBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				BlueBrushColorButton,
				FLinearColor::Blue)));
	SetButtonPressedVisual(
		IndigoBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				IndigoBrushColorButton,
				FLinearColor(0.25f, 0.0f, 0.55f, 1.0f))));
	SetButtonPressedVisual(
		PurpleBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				PurpleBrushColorButton,
				FLinearColor(0.55f, 0.0f, 1.0f, 1.0f))));
	SetButtonPressedVisual(
		BlackBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				BlackBrushColorButton,
				FLinearColor::Black)));
	SetButtonPressedVisual(
		WhiteBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				WhiteBrushColorButton,
				FLinearColor::White)));
}

void UCustomizationWidget::SetButtonPressedVisual(
	UButton* Button,
	bool bSelected)
{
	if (!Button)
	{
		return;
	}

	FButtonStyle* CachedStyle = DefaultPaletteButtonStyles.Find(Button);
	if (!CachedStyle)
	{
		DefaultPaletteButtonStyles.Add(Button, Button->GetStyle());
		CachedStyle = DefaultPaletteButtonStyles.Find(Button);
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

int32 UCustomizationWidget::GetSwitcherIndexForPage(
	ESnowRumbleCustomizationPage Page) const
{
	switch (Page)
	{
	case ESnowRumbleCustomizationPage::ViewMode:
		return 0;
	case ESnowRumbleCustomizationPage::PaintMode:
		return 1;
	case ESnowRumbleCustomizationPage::HatMode:
		return 2;
	case ESnowRumbleCustomizationPage::Main:
	default:
		return 0;
	}
}

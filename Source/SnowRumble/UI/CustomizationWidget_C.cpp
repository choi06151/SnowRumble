// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomizationWidget_C.h"

#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/WidgetSwitcher.h"
#include "Blueprint/WidgetTree.h"
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
	if (CustomizationContentSwitcher
		&& NewPage != ESnowRumbleCustomizationPage::PaintMode)
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
	if (BrushSizeSlider)
	{
		// 슬라이더 Hover 커서가 별도 모양으로 바뀌지 않게 기본 커서를 유지한다.
		BrushSizeSlider->SetCursor(EMouseCursor::Default);
	}
	BindCustomizationButtons();
	ApplyPaletteButtonColors();
	SetCustomizationPage(CurrentCustomizationPage);
	RefreshBrushSizeSlider();
	RefreshPaintBrushPreview();
	BindAccessoryItemButtons();
	RefreshAccessoryItemButtonSelection();
}

void UCustomizationWidget::NativeDestruct()
{
	UnbindCustomizationButtons();
	UnbindAccessoryItemButtons();

	Super::NativeDestruct();
}

void UCustomizationWidget::NativeTick(
	const FGeometry& MyGeometry,
	float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshPaintBrushPreview();
	RefreshBrushSizeSlider();
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

void UCustomizationWidget::HandleGlassesModeButtonClicked()
{
	SetCustomizationPage(ESnowRumbleCustomizationPage::GlassesMode);
}

void UCustomizationWidget::HandleNoseModeButtonClicked()
{
	SetCustomizationPage(ESnowRumbleCustomizationPage::NoseMode);
}

void UCustomizationWidget::HandleEarmuffsModeButtonClicked()
{
	SetCustomizationPage(ESnowRumbleCustomizationPage::EarmuffsMode);
}

void UCustomizationWidget::HandleAccessoryItemButtonClicked()
{
	if (!CustomizationPlayerController)
	{
		return;
	}

	for (const TPair<UButton*, FAccessoryButtonBinding>& Pair : AccessoryItemButtons)
	{
		UButton* Button = Pair.Key;
		if (Button && (Button->IsHovered() || Button->HasKeyboardFocus()))
		{
			CustomizationPlayerController->SetPreviewAccessoryMeshIndex(
				Pair.Value.Accessory,
				Pair.Value.MeshIndex);
			RefreshAccessoryItemButtonSelection();
			return;
		}
	}
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
void UCustomizationWidget::HandleLightRedBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		LightRedBrushColorButton,
		FLinearColor(1.0f, 0.52f, 0.46f, 1.0f));
}

void UCustomizationWidget::HandleDarkRedBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		DarkRedBrushColorButton,
		FLinearColor(0.78f, 0.02f, 0.02f, 1.0f));
}

void UCustomizationWidget::HandleSoftRedBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		SoftRedBrushColorButton,
		FLinearColor(1.0f, 0.28f, 0.20f, 1.0f));
}

void UCustomizationWidget::HandleLightOrangeBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		LightOrangeBrushColorButton,
		FLinearColor(1.0f, 0.76f, 0.42f, 1.0f));
}

void UCustomizationWidget::HandleDarkOrangeBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		DarkOrangeBrushColorButton,
		FLinearColor(0.82f, 0.22f, 0.01f, 1.0f));
}

void UCustomizationWidget::HandleSoftOrangeBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		SoftOrangeBrushColorButton,
		FLinearColor(1.0f, 0.62f, 0.20f, 1.0f));
}

void UCustomizationWidget::HandleLightYellowBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		LightYellowBrushColorButton,
		FLinearColor(1.0f, 0.96f, 0.58f, 1.0f));
}

void UCustomizationWidget::HandleDarkYellowBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		DarkYellowBrushColorButton,
		FLinearColor(0.82f, 0.68f, 0.0f, 1.0f));
}

void UCustomizationWidget::HandleSoftYellowBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		SoftYellowBrushColorButton,
		FLinearColor(1.0f, 0.98f, 0.38f, 1.0f));
}

void UCustomizationWidget::HandleLightGreenBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		LightGreenBrushColorButton,
		FLinearColor(0.46f, 1.0f, 0.54f, 1.0f));
}

void UCustomizationWidget::HandleDarkGreenBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		DarkGreenBrushColorButton,
		FLinearColor(0.02f, 0.58f, 0.10f, 1.0f));
}

void UCustomizationWidget::HandleSoftGreenBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		SoftGreenBrushColorButton,
		FLinearColor(0.30f, 1.0f, 0.42f, 1.0f));
}

void UCustomizationWidget::HandleLightBlueBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		LightBlueBrushColorButton,
		FLinearColor(0.52f, 0.88f, 1.0f, 1.0f));
}

void UCustomizationWidget::HandleDarkBlueBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		DarkBlueBrushColorButton,
		FLinearColor(0.0f, 0.38f, 0.68f, 1.0f));
}

void UCustomizationWidget::HandleSoftBlueBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		SoftBlueBrushColorButton,
		FLinearColor(0.22f, 0.82f, 1.0f, 1.0f));
}

void UCustomizationWidget::HandleLightIndigoBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		LightIndigoBrushColorButton,
		FLinearColor(0.78f, 0.62f, 1.0f, 1.0f));
}

void UCustomizationWidget::HandleDarkIndigoBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		DarkIndigoBrushColorButton,
		FLinearColor(0.22f, 0.02f, 0.62f, 1.0f));
}

void UCustomizationWidget::HandleSoftIndigoBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		SoftIndigoBrushColorButton,
		FLinearColor(0.62f, 0.34f, 1.0f, 1.0f));
}

void UCustomizationWidget::HandleLightPurpleBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		LightPurpleBrushColorButton,
		FLinearColor(0.86f, 0.60f, 1.0f, 1.0f));
}

void UCustomizationWidget::HandleDarkPurpleBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		DarkPurpleBrushColorButton,
		FLinearColor(0.38f, 0.12f, 0.92f, 1.0f));
}

void UCustomizationWidget::HandleSoftPurpleBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		SoftPurpleBrushColorButton,
		FLinearColor(0.70f, 0.40f, 1.0f, 1.0f));
}

void UCustomizationWidget::HandlePinkBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		PinkBrushColorButton,
		FLinearColor(1.0f, 0.18f, 0.55f, 1.0f));
}

void UCustomizationWidget::HandleCyanBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		CyanBrushColorButton,
		FLinearColor(0.05f, 0.75f, 0.85f, 1.0f));
}

void UCustomizationWidget::HandleBrownBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		BrownBrushColorButton,
		FLinearColor(0.42f, 0.18f, 0.05f, 1.0f));
}

void UCustomizationWidget::HandleDarkGrayBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		DarkGrayBrushColorButton,
		FLinearColor(0.18f, 0.18f, 0.18f, 1.0f));
}

void UCustomizationWidget::HandleGrayBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		GrayBrushColorButton,
		FLinearColor(0.50f, 0.50f, 0.50f, 1.0f));
}

void UCustomizationWidget::HandleLightGrayBrushColorButtonClicked()
{
	SetPaintBrushColorFromPaletteButton(
		LightGrayBrushColorButton,
		FLinearColor(0.78f, 0.78f, 0.78f, 1.0f));
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

void UCustomizationWidget::HandleBrushSizeSliderValueChanged(float NewValue)
{
	if (CustomizationPlayerController)
	{
		CustomizationPlayerController->SetPaintBrushSizeFromNormalizedValue(
			NewValue);
	}
}

void UCustomizationWidget::HandleBrushSizeSliderMouseCaptureBegin()
{
	if (CustomizationPlayerController)
	{
		CustomizationPlayerController->RefreshCustomizationMouseCursor();
	}
}

void UCustomizationWidget::HandleBrushSizeSliderMouseCaptureEnd()
{
	if (CustomizationPlayerController)
	{
		CustomizationPlayerController->RefreshCustomizationMouseCursor();
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

	RefreshAccessoryItemButtonSelection();
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
	if (GlassesModeButton)
	{
		GlassesModeButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleGlassesModeButtonClicked);
	}
	if (NoseModeButton)
	{
		NoseModeButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleNoseModeButtonClicked);
	}
	if (EarmuffsModeButton)
	{
		EarmuffsModeButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleEarmuffsModeButtonClicked);
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
	if (LightRedBrushColorButton)
	{
		LightRedBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleLightRedBrushColorButtonClicked);
	}
	if (DarkRedBrushColorButton)
	{
		DarkRedBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleDarkRedBrushColorButtonClicked);
	}
	if (SoftRedBrushColorButton)
	{
		SoftRedBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleSoftRedBrushColorButtonClicked);
	}
	if (LightOrangeBrushColorButton)
	{
		LightOrangeBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleLightOrangeBrushColorButtonClicked);
	}
	if (DarkOrangeBrushColorButton)
	{
		DarkOrangeBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleDarkOrangeBrushColorButtonClicked);
	}
	if (SoftOrangeBrushColorButton)
	{
		SoftOrangeBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleSoftOrangeBrushColorButtonClicked);
	}
	if (LightYellowBrushColorButton)
	{
		LightYellowBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleLightYellowBrushColorButtonClicked);
	}
	if (DarkYellowBrushColorButton)
	{
		DarkYellowBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleDarkYellowBrushColorButtonClicked);
	}
	if (SoftYellowBrushColorButton)
	{
		SoftYellowBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleSoftYellowBrushColorButtonClicked);
	}
	if (LightGreenBrushColorButton)
	{
		LightGreenBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleLightGreenBrushColorButtonClicked);
	}
	if (DarkGreenBrushColorButton)
	{
		DarkGreenBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleDarkGreenBrushColorButtonClicked);
	}
	if (SoftGreenBrushColorButton)
	{
		SoftGreenBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleSoftGreenBrushColorButtonClicked);
	}
	if (LightBlueBrushColorButton)
	{
		LightBlueBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleLightBlueBrushColorButtonClicked);
	}
	if (DarkBlueBrushColorButton)
	{
		DarkBlueBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleDarkBlueBrushColorButtonClicked);
	}
	if (SoftBlueBrushColorButton)
	{
		SoftBlueBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleSoftBlueBrushColorButtonClicked);
	}
	if (LightIndigoBrushColorButton)
	{
		LightIndigoBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleLightIndigoBrushColorButtonClicked);
	}
	if (DarkIndigoBrushColorButton)
	{
		DarkIndigoBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleDarkIndigoBrushColorButtonClicked);
	}
	if (SoftIndigoBrushColorButton)
	{
		SoftIndigoBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleSoftIndigoBrushColorButtonClicked);
	}
	if (LightPurpleBrushColorButton)
	{
		LightPurpleBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleLightPurpleBrushColorButtonClicked);
	}
	if (DarkPurpleBrushColorButton)
	{
		DarkPurpleBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleDarkPurpleBrushColorButtonClicked);
	}
	if (SoftPurpleBrushColorButton)
	{
		SoftPurpleBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleSoftPurpleBrushColorButtonClicked);
	}
	if (PinkBrushColorButton)
	{
		PinkBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandlePinkBrushColorButtonClicked);
	}
	if (CyanBrushColorButton)
	{
		CyanBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleCyanBrushColorButtonClicked);
	}
	if (BrownBrushColorButton)
	{
		BrownBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleBrownBrushColorButtonClicked);
	}
	if (DarkGrayBrushColorButton)
	{
		DarkGrayBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleDarkGrayBrushColorButtonClicked);
	}
	if (GrayBrushColorButton)
	{
		GrayBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleGrayBrushColorButtonClicked);
	}
	if (LightGrayBrushColorButton)
	{
		LightGrayBrushColorButton->OnClicked.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleLightGrayBrushColorButtonClicked);
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
	if (BrushSizeSlider)
	{
		BrushSizeSlider->OnValueChanged.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleBrushSizeSliderValueChanged);
		BrushSizeSlider->OnMouseCaptureBegin.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleBrushSizeSliderMouseCaptureBegin);
		BrushSizeSlider->OnMouseCaptureEnd.AddUniqueDynamic(
			this,
			&UCustomizationWidget::HandleBrushSizeSliderMouseCaptureEnd);
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

void UCustomizationWidget::BindAccessoryItemButtons()
{
	if (!WidgetTree)
	{
		return;
	}

	TArray<UWidget*> Widgets;
	WidgetTree->GetAllWidgets(Widgets);
	const TArray<TPair<FString, ESnowRumbleCustomizationAccessory>> Prefixes =
	{
		{TEXT("HatItemButton_"), ESnowRumbleCustomizationAccessory::Hat},
		{TEXT("GlassesItemButton_"), ESnowRumbleCustomizationAccessory::Glasses},
		{TEXT("NoseItemButton_"), ESnowRumbleCustomizationAccessory::Nose},
		{TEXT("EarmuffsItemButton_"), ESnowRumbleCustomizationAccessory::Earmuffs}
	};

	for (UWidget* Widget : Widgets)
	{
		UButton* Button = Cast<UButton>(Widget);
		if (!Button)
		{
			continue;
		}

		const FString ButtonName = Button->GetName();
		for (const TPair<FString, ESnowRumbleCustomizationAccessory>& Prefix : Prefixes)
		{
			if (!ButtonName.StartsWith(Prefix.Key))
			{
				continue;
			}

			const FString IndexText = ButtonName.RightChop(Prefix.Key.Len());
			if (IndexText.IsEmpty())
			{
				break;
			}

			FAccessoryButtonBinding Binding;
			Binding.Accessory = Prefix.Value;
			Binding.MeshIndex = FCString::Atoi(*IndexText) - 1;
			AccessoryItemButtons.Add(Button, Binding);
			DefaultAccessoryItemButtonStyles.Add(Button, Button->GetStyle());
			Button->OnClicked.AddUniqueDynamic(
				this,
				&UCustomizationWidget::HandleAccessoryItemButtonClicked);
			break;
		}
	}
}

void UCustomizationWidget::UnbindAccessoryItemButtons()
{
	for (const TPair<UButton*, FAccessoryButtonBinding>& Pair : AccessoryItemButtons)
	{
		if (Pair.Key)
		{
			Pair.Key->OnClicked.RemoveDynamic(
				this,
				&UCustomizationWidget::HandleAccessoryItemButtonClicked);
		}
	}
	AccessoryItemButtons.Reset();
	DefaultAccessoryItemButtonStyles.Reset();
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
	if (GlassesModeButton)
	{
		GlassesModeButton->OnClicked.RemoveAll(this);
	}
	if (NoseModeButton)
	{
		NoseModeButton->OnClicked.RemoveAll(this);
	}
	if (EarmuffsModeButton)
	{
		EarmuffsModeButton->OnClicked.RemoveAll(this);
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
	UnbindPaletteColorButton(LightRedBrushColorButton);
	UnbindPaletteColorButton(DarkRedBrushColorButton);
	UnbindPaletteColorButton(SoftRedBrushColorButton);
	UnbindPaletteColorButton(LightOrangeBrushColorButton);
	UnbindPaletteColorButton(DarkOrangeBrushColorButton);
	UnbindPaletteColorButton(SoftOrangeBrushColorButton);
	UnbindPaletteColorButton(LightYellowBrushColorButton);
	UnbindPaletteColorButton(DarkYellowBrushColorButton);
	UnbindPaletteColorButton(SoftYellowBrushColorButton);
	UnbindPaletteColorButton(LightGreenBrushColorButton);
	UnbindPaletteColorButton(DarkGreenBrushColorButton);
	UnbindPaletteColorButton(SoftGreenBrushColorButton);
	UnbindPaletteColorButton(LightBlueBrushColorButton);
	UnbindPaletteColorButton(DarkBlueBrushColorButton);
	UnbindPaletteColorButton(SoftBlueBrushColorButton);
	UnbindPaletteColorButton(LightIndigoBrushColorButton);
	UnbindPaletteColorButton(DarkIndigoBrushColorButton);
	UnbindPaletteColorButton(SoftIndigoBrushColorButton);
	UnbindPaletteColorButton(LightPurpleBrushColorButton);
	UnbindPaletteColorButton(DarkPurpleBrushColorButton);
	UnbindPaletteColorButton(SoftPurpleBrushColorButton);
	UnbindPaletteColorButton(PinkBrushColorButton);
	UnbindPaletteColorButton(CyanBrushColorButton);
	UnbindPaletteColorButton(BrownBrushColorButton);
	UnbindPaletteColorButton(DarkGrayBrushColorButton);
	UnbindPaletteColorButton(GrayBrushColorButton);
	UnbindPaletteColorButton(LightGrayBrushColorButton);
	if (BrushSizeButton)
	{
		BrushSizeButton->OnPressed.RemoveAll(this);
		BrushSizeButton->OnReleased.RemoveAll(this);
	}
	if (BrushSizeSlider)
	{
		BrushSizeSlider->OnValueChanged.RemoveAll(this);
		BrushSizeSlider->OnMouseCaptureBegin.RemoveAll(this);
		BrushSizeSlider->OnMouseCaptureEnd.RemoveAll(this);
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

void UCustomizationWidget::ApplyPaletteButtonColors()
{
	ApplyPaletteButtonColor(
		RedBrushColorButton,
		FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("D93939"))));
	ApplyPaletteButtonColor(OrangeBrushColorButton, FLinearColor(1.0f, 0.45f, 0.0f, 1.0f));
	ApplyPaletteButtonColor(
		YellowBrushColorButton,
		FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FFFB3C"))));
	ApplyPaletteButtonColor(
		GreenBrushColorButton,
		FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("6DDD62"))));
	ApplyPaletteButtonColor(
		BlueBrushColorButton,
		FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("00A6D7"))));
	ApplyPaletteButtonColor(
		IndigoBrushColorButton,
		FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("BC7AFF"))));
	ApplyPaletteButtonColor(
		PurpleBrushColorButton,
		FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FD8BFF"))));
	ApplyPaletteButtonColor(BlackBrushColorButton, FLinearColor::Black);
	ApplyPaletteButtonColor(
		WhiteBrushColorButton,
		FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("E664E9"))));
	ApplyPaletteButtonColor(LightRedBrushColorButton, FLinearColor(1.0f, 0.52f, 0.46f, 1.0f));
	ApplyPaletteButtonColor(
		DarkRedBrushColorButton,
		FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FF674C"))));
	ApplyPaletteButtonColor(SoftRedBrushColorButton, FLinearColor(1.0f, 0.28f, 0.20f, 1.0f));
	ApplyPaletteButtonColor(LightOrangeBrushColorButton, FLinearColor(1.0f, 0.76f, 0.42f, 1.0f));
	ApplyPaletteButtonColor(DarkOrangeBrushColorButton, FLinearColor(0.82f, 0.22f, 0.01f, 1.0f));
	ApplyPaletteButtonColor(SoftOrangeBrushColorButton, FLinearColor(1.0f, 0.62f, 0.20f, 1.0f));
	ApplyPaletteButtonColor(LightYellowBrushColorButton, FLinearColor(1.0f, 0.96f, 0.58f, 1.0f));
	ApplyPaletteButtonColor(DarkYellowBrushColorButton, FLinearColor(0.82f, 0.68f, 0.0f, 1.0f));
	ApplyPaletteButtonColor(SoftYellowBrushColorButton, FLinearColor(1.0f, 0.98f, 0.38f, 1.0f));
	ApplyPaletteButtonColor(LightGreenBrushColorButton, FLinearColor(0.46f, 1.0f, 0.54f, 1.0f));
	ApplyPaletteButtonColor(DarkGreenBrushColorButton, FLinearColor(0.02f, 0.58f, 0.10f, 1.0f));
	ApplyPaletteButtonColor(SoftGreenBrushColorButton, FLinearColor(0.30f, 1.0f, 0.42f, 1.0f));
	ApplyPaletteButtonColor(LightBlueBrushColorButton, FLinearColor(0.52f, 0.88f, 1.0f, 1.0f));
	ApplyPaletteButtonColor(
		DarkBlueBrushColorButton,
		FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("2FD0F0"))));
	ApplyPaletteButtonColor(SoftBlueBrushColorButton, FLinearColor(0.22f, 0.82f, 1.0f, 1.0f));
	ApplyPaletteButtonColor(
		LightIndigoBrushColorButton,
		FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("A9D8FF"))));
	ApplyPaletteButtonColor(
		DarkIndigoBrushColorButton,
		FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("1064FF"))));
	ApplyPaletteButtonColor(
		SoftIndigoBrushColorButton,
		FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("7EC5FF"))));
	ApplyPaletteButtonColor(LightPurpleBrushColorButton, FLinearColor(0.86f, 0.60f, 1.0f, 1.0f));
	ApplyPaletteButtonColor(
		DarkPurpleBrushColorButton,
		FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("9E40FF"))));
	ApplyPaletteButtonColor(SoftPurpleBrushColorButton, FLinearColor(0.70f, 0.40f, 1.0f, 1.0f));
	ApplyPaletteButtonColor(
		PinkBrushColorButton,
		FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FEC8FF"))));
	ApplyPaletteButtonColor(
		CyanBrushColorButton,
		FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FEADFF"))));
	ApplyPaletteButtonColor(
		BrownBrushColorButton,
		FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("AAAAAA"))));
	ApplyPaletteButtonColor(DarkGrayBrushColorButton, FLinearColor(0.18f, 0.18f, 0.18f, 1.0f));
	ApplyPaletteButtonColor(
		GrayBrushColorButton,
		FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("E2E2E2"))));
	ApplyPaletteButtonColor(
		LightGrayBrushColorButton,
		FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FD8BFF"))));
}

void UCustomizationWidget::ApplyPaletteButtonColor(
	UButton* Button,
	FLinearColor Color)
{
	if (!Button)
	{
		return;
	}

	FButtonStyle Style = Button->GetStyle();
	FLinearColor HoverColor = Color;
	HoverColor.R = FMath::Min(HoverColor.R * 1.1f, 1.0f);
	HoverColor.G = FMath::Min(HoverColor.G * 1.1f, 1.0f);
	HoverColor.B = FMath::Min(HoverColor.B * 1.1f, 1.0f);
	FLinearColor PressedColor = Color * 0.8f;
	PressedColor.A = Color.A;
	FLinearColor DisabledColor = Color * 0.5f;
	DisabledColor.A = Color.A;
	Style.Normal.TintColor = Color;
	Style.Hovered.TintColor = HoverColor;
	Style.Pressed.TintColor = PressedColor;
	Style.Disabled.TintColor = DisabledColor;
	Button->SetStyle(Style);
	Button->SetBackgroundColor(FLinearColor::White);
	DefaultPaletteButtonStyles.Add(Button, Style);
}

void UCustomizationWidget::RefreshBrushSizeSlider()
{
	if (BrushSizeSlider && CustomizationPlayerController)
	{
		BrushSizeSlider->SetValue(
			CustomizationPlayerController->GetPaintBrushSizeNormalizedValue());
	}
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
	SetButtonPressedVisual(
		LightRedBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				LightRedBrushColorButton,
				FLinearColor(1.0f, 0.52f, 0.46f, 1.0f))));
	SetButtonPressedVisual(
		DarkRedBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				DarkRedBrushColorButton,
				FLinearColor(0.78f, 0.02f, 0.02f, 1.0f))));
	SetButtonPressedVisual(
		SoftRedBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				SoftRedBrushColorButton,
				FLinearColor(1.0f, 0.28f, 0.20f, 1.0f))));
	SetButtonPressedVisual(
		LightOrangeBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				LightOrangeBrushColorButton,
				FLinearColor(1.0f, 0.76f, 0.42f, 1.0f))));
	SetButtonPressedVisual(
		DarkOrangeBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				DarkOrangeBrushColorButton,
				FLinearColor(0.82f, 0.22f, 0.01f, 1.0f))));
	SetButtonPressedVisual(
		SoftOrangeBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				SoftOrangeBrushColorButton,
				FLinearColor(1.0f, 0.62f, 0.20f, 1.0f))));
	SetButtonPressedVisual(
		LightYellowBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				LightYellowBrushColorButton,
				FLinearColor(1.0f, 0.96f, 0.58f, 1.0f))));
	SetButtonPressedVisual(
		DarkYellowBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				DarkYellowBrushColorButton,
				FLinearColor(0.82f, 0.68f, 0.0f, 1.0f))));
	SetButtonPressedVisual(
		SoftYellowBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				SoftYellowBrushColorButton,
				FLinearColor(1.0f, 0.98f, 0.38f, 1.0f))));
	SetButtonPressedVisual(
		LightGreenBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				LightGreenBrushColorButton,
				FLinearColor(0.46f, 1.0f, 0.54f, 1.0f))));
	SetButtonPressedVisual(
		DarkGreenBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				DarkGreenBrushColorButton,
				FLinearColor(0.02f, 0.58f, 0.10f, 1.0f))));
	SetButtonPressedVisual(
		SoftGreenBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				SoftGreenBrushColorButton,
				FLinearColor(0.30f, 1.0f, 0.42f, 1.0f))));
	SetButtonPressedVisual(
		LightBlueBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				LightBlueBrushColorButton,
				FLinearColor(0.52f, 0.88f, 1.0f, 1.0f))));
	SetButtonPressedVisual(
		DarkBlueBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				DarkBlueBrushColorButton,
				FLinearColor(0.0f, 0.38f, 0.68f, 1.0f))));
	SetButtonPressedVisual(
		SoftBlueBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				SoftBlueBrushColorButton,
				FLinearColor(0.22f, 0.82f, 1.0f, 1.0f))));
	SetButtonPressedVisual(
		LightIndigoBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				LightIndigoBrushColorButton,
				FLinearColor(0.78f, 0.62f, 1.0f, 1.0f))));
	SetButtonPressedVisual(
		DarkIndigoBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				DarkIndigoBrushColorButton,
				FLinearColor(0.22f, 0.02f, 0.62f, 1.0f))));
	SetButtonPressedVisual(
		SoftIndigoBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				SoftIndigoBrushColorButton,
				FLinearColor(0.62f, 0.34f, 1.0f, 1.0f))));
	SetButtonPressedVisual(
		LightPurpleBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				LightPurpleBrushColorButton,
				FLinearColor(0.86f, 0.60f, 1.0f, 1.0f))));
	SetButtonPressedVisual(
		DarkPurpleBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				DarkPurpleBrushColorButton,
				FLinearColor(0.38f, 0.12f, 0.92f, 1.0f))));
	SetButtonPressedVisual(
		SoftPurpleBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				SoftPurpleBrushColorButton,
				FLinearColor(0.70f, 0.40f, 1.0f, 1.0f))));
	SetButtonPressedVisual(
		PinkBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				PinkBrushColorButton,
				FLinearColor(1.0f, 0.18f, 0.55f, 1.0f))));
	SetButtonPressedVisual(
		CyanBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				CyanBrushColorButton,
				FLinearColor(0.05f, 0.75f, 0.85f, 1.0f))));
	SetButtonPressedVisual(
		BrownBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				BrownBrushColorButton,
				FLinearColor(0.42f, 0.18f, 0.05f, 1.0f))));
	SetButtonPressedVisual(
		DarkGrayBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				DarkGrayBrushColorButton,
				FLinearColor(0.18f, 0.18f, 0.18f, 1.0f))));
	SetButtonPressedVisual(
		GrayBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				GrayBrushColorButton,
				FLinearColor(0.50f, 0.50f, 0.50f, 1.0f))));
	SetButtonPressedVisual(
		LightGrayBrushColorButton,
		IsSameColor(
			GetPaletteButtonBrushColor(
				LightGrayBrushColorButton,
				FLinearColor(0.78f, 0.78f, 0.78f, 1.0f))));
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

void UCustomizationWidget::RefreshAccessoryItemButtonSelection()
{
	if (!CustomizationPlayerController)
	{
		return;
	}

	for (const TPair<UButton*, FAccessoryButtonBinding>& Pair : AccessoryItemButtons)
	{
		const bool bSelected =
			CustomizationPlayerController->GetPreviewAccessoryMeshIndex(
				Pair.Value.Accessory) == Pair.Value.MeshIndex;
		SetAccessoryButtonPressedVisual(Pair.Key, bSelected);
	}
}

void UCustomizationWidget::SetAccessoryButtonPressedVisual(
	UButton* Button,
	bool bSelected)
{
	if (!Button)
	{
		return;
	}

	FButtonStyle* CachedStyle = DefaultAccessoryItemButtonStyles.Find(Button);
	if (!CachedStyle)
	{
		DefaultAccessoryItemButtonStyles.Add(Button, Button->GetStyle());
		CachedStyle = DefaultAccessoryItemButtonStyles.Find(Button);
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
		// 색칠하기는 액세서리 WidgetSwitcher와 무관한 모드다.
		return 0;
	case ESnowRumbleCustomizationPage::HatMode:
		return 0;
	case ESnowRumbleCustomizationPage::GlassesMode:
		return 1;
	case ESnowRumbleCustomizationPage::NoseMode:
		return 2;
	case ESnowRumbleCustomizationPage::EarmuffsMode:
		return 3;
	case ESnowRumbleCustomizationPage::Main:
	default:
		return 0;
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../Player/SnowRumbleCustomizationData_C.h"
#include "SnowRumbleAudioUserWidget.h"
#include "Styling/SlateTypes.h"
#include "CustomizationWidget_C.generated.h"

class ACustomizationPlayerController;
class UBorder;
class UButton;
class UImage;
class USlider;
class UWidgetSwitcher;

UENUM(BlueprintType)
enum class ESnowRumbleCustomizationPage : uint8
{
	Main,
	ViewMode,
	PaintMode,
	HatMode
};

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API UCustomizationWidget : public USnowRumbleAudioUserWidget
{
	GENERATED_BODY()

public:
	/** 커스터마이징 화면을 소유한 컨트롤러를 설정한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void SetCustomizationPlayerController(
		ACustomizationPlayerController* NewPlayerController);

	/** 커스터마이징 화면을 WidgetSwitcher 인덱스 기준으로 전환한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void SetCustomizationPage(ESnowRumbleCustomizationPage NewPage);

	/** 현재 커스터마이징 화면을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Customization")
	ESnowRumbleCustomizationPage GetCurrentCustomizationPage() const;

	/** WBP 색상 버튼이 프리뷰 몸 색을 바꿀 때 호출한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void SetPreviewBodyColor(FLinearColor NewBodyColor);

	/** 현재 프리뷰 중인 커스터마이징 데이터를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Customization")
	FSnowRumbleCustomizationData GetPreviewCustomizationData() const;

	/** 현재 선택된 페인트 브러시 색을 반환한다. WBP 색상 표시 바인딩에서 사용할 수 있다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Customization")
	FLinearColor GetPaintBrushColor() const;

	/** 현재 선택된 페인트 브러시 크기를 반환한다. WBP 크기 표시 바인딩에서 사용할 수 있다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Customization")
	float GetPaintBrushSize() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|Customization|Hat")
	int32 GetPreviewHatMeshIndex() const;

	/** 페인트 화면의 마지막 완료 Stroke를 하나 제거한다. 버튼이나 WBP 입력에서 직접 호출할 수 있다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void RequestUndoLastPaintStroke();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(
		const FGeometry& MyGeometry,
		float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseWheel(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	/** 화면 전환 시 Blueprint가 선택 표시나 추가 연출을 갱신할 수 있다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Customization")
	void OnCustomizationPageChanged(ESnowRumbleCustomizationPage NewPage);

	/** 적용 버튼을 눌렀을 때 Blueprint가 현재 페이지별 작업을 적용할 수 있다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Customization")
	void OnCustomizationApplyRequested(ESnowRumbleCustomizationPage CurrentPage);

	/** 초기화 버튼을 눌렀을 때 Blueprint가 현재 페이지별 작업을 초기화할 수 있다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Customization")
	void OnCustomizationResetRequested(ESnowRumbleCustomizationPage CurrentPage);

	/** 메인/색칠하기 페이지를 담는 WidgetSwitcher다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UWidgetSwitcher> CustomizationContentSwitcher;

	/** 메인 페이지에서 색칠하기 페이지로 이동하는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> PaintModeButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> HatModeButton;

	/** 이전 WBP 호환용 선택 버튼이다. 새 팔레트 UI에서는 배치하지 않는다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> BrushColorButton;

	/** 이전 WBP 호환용 현재 색 Border다. 새 팔레트 UI에서는 배치하지 않는다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UBorder> BrushColorPreviewBorder;

	/** 이전 WBP 호환용 현재 색 Image다. 새 팔레트 UI에서는 배치하지 않는다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UImage> BrushColorPreviewImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> RedBrushColorButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> OrangeBrushColorButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> YellowBrushColorButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> GreenBrushColorButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> BlueBrushColorButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> IndigoBrushColorButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> PurpleBrushColorButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> BlackBrushColorButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> WhiteBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> LightRedBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> DarkRedBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> SoftRedBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> LightOrangeBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> DarkOrangeBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> SoftOrangeBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> LightYellowBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> DarkYellowBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> SoftYellowBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> LightGreenBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> DarkGreenBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> SoftGreenBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> LightBlueBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> DarkBlueBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> SoftBlueBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> LightIndigoBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> DarkIndigoBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> SoftIndigoBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> LightPurpleBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> DarkPurpleBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> SoftPurpleBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> PinkBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> CyanBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> BrownBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> DarkGrayBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> GrayBrushColorButton;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> LightGrayBrushColorButton;

	/** 누른 상태에서 마우스 휠로 브러시 크기를 조정하는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> BrushSizeButton;

	/** 0~1 값으로 브러시 크기를 조절하는 선택 슬라이더다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<USlider> BrushSizeSlider;

	/** 현재 브러시 색을 BodyColor에 적용하는 전체 칠하기 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> FillBodyColorButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> HatPreviousButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> HatNextButton;

	/** 누르고 있는 동안 프리뷰 캐릭터를 왼쪽으로 회전하는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> RotateLeftButton;

	/** 누르고 있는 동안 프리뷰 캐릭터를 오른쪽으로 회전하는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> RotateRightButton;

	/** 메인메뉴 또는 지정 로비 URL로 돌아가는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> ReturnToLobbyButton;

	/** 하위 페이지에서 메인 페이지로 돌아가는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> BackButton;

	/** 현재 하위 페이지 작업을 적용하는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> ApplyButton;

	/** 현재 하위 페이지 작업을 초기화하는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> ResetButton;

private:
	UFUNCTION()
	void HandlePaintModeButtonClicked();

	UFUNCTION()
	void HandleHatModeButtonClicked();

	UFUNCTION()
	void HandleBrushColorButtonClicked();

	UFUNCTION()
	void HandleRedBrushColorButtonClicked();

	UFUNCTION()
	void HandleOrangeBrushColorButtonClicked();

	UFUNCTION()
	void HandleYellowBrushColorButtonClicked();

	UFUNCTION()
	void HandleGreenBrushColorButtonClicked();

	UFUNCTION()
	void HandleBlueBrushColorButtonClicked();

	UFUNCTION()
	void HandleIndigoBrushColorButtonClicked();

	UFUNCTION()
	void HandlePurpleBrushColorButtonClicked();

	UFUNCTION()
	void HandleBlackBrushColorButtonClicked();

	UFUNCTION()
	void HandleWhiteBrushColorButtonClicked();
	UFUNCTION()
	void HandleLightRedBrushColorButtonClicked();
	UFUNCTION()
	void HandleDarkRedBrushColorButtonClicked();
	UFUNCTION()
	void HandleSoftRedBrushColorButtonClicked();
	UFUNCTION()
	void HandleLightOrangeBrushColorButtonClicked();
	UFUNCTION()
	void HandleDarkOrangeBrushColorButtonClicked();
	UFUNCTION()
	void HandleSoftOrangeBrushColorButtonClicked();
	UFUNCTION()
	void HandleLightYellowBrushColorButtonClicked();
	UFUNCTION()
	void HandleDarkYellowBrushColorButtonClicked();
	UFUNCTION()
	void HandleSoftYellowBrushColorButtonClicked();
	UFUNCTION()
	void HandleLightGreenBrushColorButtonClicked();
	UFUNCTION()
	void HandleDarkGreenBrushColorButtonClicked();
	UFUNCTION()
	void HandleSoftGreenBrushColorButtonClicked();
	UFUNCTION()
	void HandleLightBlueBrushColorButtonClicked();
	UFUNCTION()
	void HandleDarkBlueBrushColorButtonClicked();
	UFUNCTION()
	void HandleSoftBlueBrushColorButtonClicked();
	UFUNCTION()
	void HandleLightIndigoBrushColorButtonClicked();
	UFUNCTION()
	void HandleDarkIndigoBrushColorButtonClicked();
	UFUNCTION()
	void HandleSoftIndigoBrushColorButtonClicked();
	UFUNCTION()
	void HandleLightPurpleBrushColorButtonClicked();
	UFUNCTION()
	void HandleDarkPurpleBrushColorButtonClicked();
	UFUNCTION()
	void HandleSoftPurpleBrushColorButtonClicked();
	UFUNCTION()
	void HandlePinkBrushColorButtonClicked();
	UFUNCTION()
	void HandleCyanBrushColorButtonClicked();
	UFUNCTION()
	void HandleBrownBrushColorButtonClicked();
	UFUNCTION()
	void HandleDarkGrayBrushColorButtonClicked();
	UFUNCTION()
	void HandleGrayBrushColorButtonClicked();
	UFUNCTION()
	void HandleLightGrayBrushColorButtonClicked();

	UFUNCTION()
	void HandleBrushSizeButtonPressed();

	UFUNCTION()
	void HandleBrushSizeButtonReleased();

	UFUNCTION()
	void HandleBrushSizeSliderValueChanged(float NewValue);

	UFUNCTION()
	void HandleFillBodyColorButtonClicked();

	UFUNCTION()
	void HandleHatPreviousButtonClicked();

	UFUNCTION()
	void HandleHatNextButtonClicked();

	UFUNCTION()
	void HandleRotateLeftButtonPressed();

	UFUNCTION()
	void HandleRotateRightButtonPressed();

	UFUNCTION()
	void HandleRotateButtonReleased();

	UFUNCTION()
	void HandleReturnToLobbyButtonClicked();

	UFUNCTION()
	void HandleBackButtonClicked();

	UFUNCTION()
	void HandleApplyButtonClicked();

	UFUNCTION()
	void HandleResetButtonClicked();

	void BindCustomizationButtons();
	void UnbindCustomizationButtons();
	void RefreshPaintBrushPreview();
	void ApplyPaletteButtonColors();
	void ApplyPaletteButtonColor(UButton* Button, FLinearColor Color);
	void RefreshBrushSizeSlider();
	void SetPaintBrushColorFromPalette(FLinearColor NewBrushColor);
	void SetPaintBrushColorFromPaletteButton(
		UButton* Button,
		FLinearColor FallbackColor);
	FLinearColor GetPaletteButtonBrushColor(
		UButton* Button,
		FLinearColor FallbackColor);
	void UnbindPaletteColorButton(UButton* Button);
	void RefreshPaletteButtonSelection();
	void SetButtonPressedVisual(UButton* Button, bool bSelected);
	int32 GetSwitcherIndexForPage(ESnowRumbleCustomizationPage Page) const;

	UPROPERTY(Transient)
	TObjectPtr<ACustomizationPlayerController> CustomizationPlayerController;

	ESnowRumbleCustomizationPage CurrentCustomizationPage =
		ESnowRumbleCustomizationPage::Main;

	bool bIsBrushSizeButtonPressed = false;

	TMap<UButton*, FButtonStyle> DefaultPaletteButtonStyles;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../Player/SnowRumbleCustomizationData_C.h"
#include "Blueprint/UserWidget.h"
#include "CustomizationWidget_C.generated.h"

class ACustomizationPlayerController;
class UBorder;
class UButton;
class UImage;
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
class SNOWRUMBLE_API UCustomizationWidget : public UUserWidget
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

	/** WBP 색상 버튼이나 컬러 피커가 프리뷰 몸 색을 바꿀 때 호출한다. */
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

	/** 색칠하기 페이지에서 언리얼 기본 컬러 피커를 여는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> BrushColorButton;

	/** 현재 브러시 색을 보여주는 Border다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UBorder> BrushColorPreviewBorder;

	/** 현재 브러시 색을 보여주는 Image다. Border 대신 이 이름을 써도 된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UImage> BrushColorPreviewImage;

	/** 누른 상태에서 마우스 휠로 브러시 크기를 조정하는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|Customization")
	TObjectPtr<UButton> BrushSizeButton;

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
	void HandleBrushSizeButtonPressed();

	UFUNCTION()
	void HandleBrushSizeButtonReleased();

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
	int32 GetSwitcherIndexForPage(ESnowRumbleCustomizationPage Page) const;

	UPROPERTY(Transient)
	TObjectPtr<ACustomizationPlayerController> CustomizationPlayerController;

	ESnowRumbleCustomizationPage CurrentCustomizationPage =
		ESnowRumbleCustomizationPage::Main;

	bool bIsBrushSizeButtonPressed = false;
};

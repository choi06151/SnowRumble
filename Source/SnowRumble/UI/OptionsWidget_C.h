// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"
#include "OptionsWidget_C.generated.h"

class UButton;
class UOptionsKeyBindingRowWidget;
class UTextBlock;
class UVerticalBox;
class UWidgetSwitcher;

USTRUCT(BlueprintType)
struct FSnowRumbleKeyBindingViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnowRumble|UI|Options")
	FName BindingId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnowRumble|UI|Options")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnowRumble|UI|Options")
	FKey CurrentKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnowRumble|UI|Options")
	FKey DefaultKey;
};

UENUM(BlueprintType)
enum class ESnowRumbleOptionsCategory : uint8
{
	Sensitivity,
	Audio,
	KeyBinding,
	Microphone
};

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API UOptionsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UOptionsWidget(const FObjectInitializer& ObjectInitializer);

	/** C++ 소유 메뉴가 옵션 닫기 요청을 받을 때 사용하는 델리게이트다. */
	FSimpleMulticastDelegate OnOptionsCloseRequestedNative;

	/** 옵션 카테고리를 선택하고 하단 WidgetSwitcher 페이지를 전환한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Options")
	void SetOptionsCategory(ESnowRumbleOptionsCategory NewCategory);

	/** 현재 선택된 옵션 카테고리를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Options")
	ESnowRumbleOptionsCategory GetCurrentOptionsCategory() const;

	/** 현재 키 설정 패널에 표시할 기본 조작 목록을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Options|Key Binding")
	const TArray<FSnowRumbleKeyBindingViewData>& GetKeyBindingRows() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnPreviewKeyDown(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	/** 카테고리가 바뀔 때 Blueprint가 버튼 상태와 패널 표현을 갱신할 수 있다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options")
	void OnOptionsCategoryChanged(ESnowRumbleOptionsCategory NewCategory);

	/** 닫기 버튼이 눌렸을 때 호출된다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options")
	void OnOptionsCloseRequested();

	/** 적용 버튼이 눌렸을 때 호출된다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options")
	void OnOptionsApplyRequested();

	/** 초기화 버튼이 눌렸을 때 호출된다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options")
	void OnOptionsResetRequested();

	/** 키 설정 행 목록이 갱신될 때 Blueprint 표현을 보강할 수 있다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options|Key Binding")
	void OnKeyBindingRowsRefreshed(
		const TArray<FSnowRumbleKeyBindingViewData>& Rows);

	/** 키 변경 버튼이 눌려 다음 키 입력 대기 상태가 됐을 때 호출된다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options|Key Binding")
	void OnKeyRebindRequested(FName BindingId);

	/** 키가 실제로 변경됐을 때 호출된다. 실제 Enhanced Input 적용은 후속 저장·적용 단계에서 붙인다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options|Key Binding")
	void OnKeyBindingChanged(FName BindingId, FKey NewKey);

	/** 키 변경 대기가 취소됐을 때 호출된다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options|Key Binding")
	void OnKeyRebindCanceled(FName BindingId);

	/** 키 초기화 버튼이 눌렸을 때 호출된다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options|Key Binding")
	void OnKeyBindingResetRequested(FName BindingId);

	/** 상단 감도 카테고리 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options")
	TObjectPtr<UButton> SensitivityCategoryButton;

	/** 상단 사운드 카테고리 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options")
	TObjectPtr<UButton> AudioCategoryButton;

	/** 상단 키 설정 카테고리 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options")
	TObjectPtr<UButton> KeyBindingCategoryButton;

	/** 상단 마이크 카테고리 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options")
	TObjectPtr<UButton> MicrophoneCategoryButton;

	/** 하단 옵션 내용 패널을 전환하는 WidgetSwitcher다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options")
	TObjectPtr<UWidgetSwitcher> OptionsContentSwitcher;

	/** 옵션 메뉴를 닫는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options")
	TObjectPtr<UButton> CloseButton;

	/** 현재 옵션 값을 적용하는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options")
	TObjectPtr<UButton> ApplyButton;

	/** 옵션 값을 기본값으로 되돌리는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options")
	TObjectPtr<UButton> ResetButton;

	/** 키 설정 행을 자동 생성할 세로 박스다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Key Binding")
	TObjectPtr<UVerticalBox> KeyBindingListBox;

	/** 키 변경 대기 상태 같은 짧은 안내를 표시할 선택 텍스트다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Key Binding")
	TObjectPtr<UTextBlock> KeyBindingStatusText;

	/** 있으면 키 설정 행을 이 WBP 클래스로 생성한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Options|Key Binding")
	TSubclassOf<UOptionsKeyBindingRowWidget> KeyBindingRowWidgetClass;

private:
	UFUNCTION()
	void HandleSensitivityCategoryButtonClicked();

	UFUNCTION()
	void HandleAudioCategoryButtonClicked();

	UFUNCTION()
	void HandleKeyBindingCategoryButtonClicked();

	UFUNCTION()
	void HandleMicrophoneCategoryButtonClicked();

	UFUNCTION()
	void HandleCloseButtonClicked();

	UFUNCTION()
	void HandleApplyButtonClicked();

	UFUNCTION()
	void HandleResetButtonClicked();

	void HandleKeyRowRebindRequested(FName BindingId);
	void HandleKeyRowResetRequested(FName BindingId);

	/** 선택 바인딩된 버튼 클릭 이벤트를 연결한다. */
	void BindOptionButtons();

	/** 선택 바인딩된 버튼 클릭 이벤트를 해제한다. */
	void UnbindOptionButtons();

	/** 카테고리에 대응하는 WidgetSwitcher 인덱스를 반환한다. */
	int32 GetSwitcherIndexForCategory(ESnowRumbleOptionsCategory Category) const;

	/** 키 설정 패널 표시 데이터를 구성한다. */
	void InitializeDefaultKeyBindingRows();

	/** KeyBindingListBox가 있으면 키 설정 행을 자동 생성한다. */
	void RefreshKeyBindingPanel();

	/** 키 변경 대기를 시작한다. */
	void BeginKeyRebind(FName BindingId);

	/** 키 변경 대기를 취소한다. */
	void CancelKeyRebind();

	/** 대기 중인 키바인딩에 새 키를 적용한다. */
	bool ApplyCapturedKey(FKey NewKey);

	/** 입력 캡처 대상으로 쓸 수 있는 키인지 확인한다. */
	bool IsBindableKey(FKey Key) const;

	ESnowRumbleOptionsCategory CurrentOptionsCategory =
		ESnowRumbleOptionsCategory::Sensitivity;

	UPROPERTY(Transient)
	TArray<FSnowRumbleKeyBindingViewData> KeyBindingRows;

	FName PendingKeyBindingId = NAME_None;
};

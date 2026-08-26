// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SnowRumbleAudioUserWidget.h"
#include "OptionsWidget_C.h"
#include "OptionsKeyBindingRowWidget_C.generated.h"

class UButton;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE_OneParam(FSnowRumbleKeyBindingRowRequest, FName);

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API UOptionsKeyBindingRowWidget : public USnowRumbleAudioUserWidget
{
	GENERATED_BODY()

public:
	FSnowRumbleKeyBindingRowRequest OnRebindRequestedNative;
	FSnowRumbleKeyBindingRowRequest OnResetRequestedNative;

	/** 행에 표시할 조작 이름과 키 이름을 설정한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Options|Key Binding")
	void SetKeyBindingData(const FSnowRumbleKeyBindingViewData& NewData);

	/** 이 행이 담당하는 바인딩 ID를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Options|Key Binding")
	FName GetBindingId() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options|Key Binding")
	void OnKeyBindingDataChanged(
		const FSnowRumbleKeyBindingViewData& NewData);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Key Binding")
	TObjectPtr<UTextBlock> ActionNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Key Binding")
	TObjectPtr<UTextBlock> CurrentKeyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Key Binding")
	TObjectPtr<UButton> RebindButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Key Binding")
	TObjectPtr<UButton> ResetButton;

private:
	UFUNCTION()
	void HandleRebindButtonClicked();

	UFUNCTION()
	void HandleResetButtonClicked();

	void BindRowButtons();
	void UnbindRowButtons();
	void RefreshDisplayedText();

	UPROPERTY(Transient)
	FSnowRumbleKeyBindingViewData Data;
};

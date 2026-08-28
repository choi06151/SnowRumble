// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "KeyGuideWidget_C.generated.h"

class UTextBlock;

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API UKeyGuideWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 현재 로컬 키 설정을 읽어 키 가이드의 키 TextBlock만 갱신한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|KeyGuide")
	void RefreshKeyGuideTexts();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 키 가이드의 키 표시가 갱신된 뒤 WBP가 추가 연출을 할 수 있다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|KeyGuide")
	void OnKeyGuideTextsRefreshed();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|KeyGuide")
	TObjectPtr<UTextBlock> MoveKeyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|KeyGuide")
	TObjectPtr<UTextBlock> SnowCreateKeyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|KeyGuide")
	TObjectPtr<UTextBlock> SnowPickupKeyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|KeyGuide")
	TObjectPtr<UTextBlock> SnowRollKeyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|KeyGuide")
	TObjectPtr<UTextBlock> AimKeyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|KeyGuide")
	TObjectPtr<UTextBlock> ThrowKeyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|KeyGuide")
	TObjectPtr<UTextBlock> EmoteKeyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|KeyGuide")
	TObjectPtr<UTextBlock> VoiceMuteKeyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|KeyGuide")
	TObjectPtr<UTextBlock> VoiceChannelKeyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|KeyGuide")
	TObjectPtr<UTextBlock> KeyGuideKeyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|KeyGuide")
	TObjectPtr<UTextBlock> ChatKeyText;

private:
	FText GetKeyDisplayText(FName BindingId, FKey DefaultKey) const;
	static FText FormatKeyDisplayText(FKey Key);
	void SetOptionalText(UTextBlock* TextBlock, const FText& Text) const;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "KeyGuideWidget_C.h"
#include "MainMenuKeyGuideWidget_C.generated.h"

class UButton;

/** 메인 메뉴에서만 사용하는 키 가이드 WBP 부모다. */
UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API UMainMenuKeyGuideWidget : public UKeyGuideWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	/** 메인 메뉴 전용 키 가이드에서 위젯을 닫는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|KeyGuide")
	TObjectPtr<UButton> CloseButton;

private:
	UFUNCTION()
	void HandleCloseButtonClicked();
};

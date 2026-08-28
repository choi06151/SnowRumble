// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionPromptWidget_C.generated.h"

class UTextBlock;

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API UInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 현재 상호작용 안내 문구를 표시한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Interaction")
	void SetPromptText(const FText& NewPromptText);

	/** 상호작용 안내 문구를 숨긴다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Interaction")
	void ClearPrompt();

protected:
	virtual void NativeConstruct() override;

	/** WBP에 같은 이름으로 배치하면 `F - 게시판` 같은 안내 문구를 자동 표시한다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Interaction")
	TObjectPtr<UTextBlock> PromptText;
};

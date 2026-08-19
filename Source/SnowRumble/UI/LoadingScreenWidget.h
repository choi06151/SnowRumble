// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingScreenWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS(Blueprintable)
class SNOWRUMBLE_API ULoadingScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Loading")
	float GetLoadingProgress() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Loading")
	FText GetLoadingStatusText() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Loading")
	FText GetLoadingMessageText() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(
		const FGeometry& MyGeometry,
		float InDeltaTime) override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Loading")
	TObjectPtr<UProgressBar> LoadingProgressBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Loading")
	TObjectPtr<UTextBlock> LoadingStatusText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Loading")
	TObjectPtr<UTextBlock> LoadingMessageText;

private:
	float GetTargetLoadingProgress() const;
	void RefreshLoadingPresentation();

	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|UI|Loading", meta = (ClampMin = "0.0"))
	float LoadingProgressInterpSpeed = 6.0f;

	float DisplayedLoadingProgress = 0.0f;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LatestPhotoScreenshotWidget_C.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API ULatestPhotoScreenshotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Saved/Screenshots 아래에서 가장 최근 사진 스크린샷을 찾아 Image에 표시한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Photo")
	bool RefreshLatestScreenshot();

	/** 마지막으로 표시한 스크린샷 파일 경로를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Photo")
	const FString& GetLatestScreenshotPath() const;

	/** 마지막으로 로드한 스크린샷 Texture를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Photo")
	UTexture2D* GetLatestScreenshotTexture() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** WBP에서 같은 이름으로 만든 Image에 가장 최근 스크린샷을 넣는다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Photo")
	TObjectPtr<UImage> LatestScreenshotImage;

	/** WBP에서 같은 이름으로 만들면 마지막 스크린샷 파일명을 표시한다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Photo")
	TObjectPtr<UTextBlock> LatestScreenshotNameText;

	/** true면 위젯 생성 시 자동으로 가장 최근 스크린샷을 표시한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Photo")
	bool bRefreshOnConstruct = true;

	/** true면 Photo_ 접두사로 저장한 사진 스크린샷만 찾는다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Photo")
	bool bOnlyPhotoScreenshots = true;

private:
	bool FindLatestScreenshotFile(FString& OutScreenshotPath) const;
	void ApplyScreenshotTexture(UTexture2D* ScreenshotTexture);
	void UpdateScreenshotNameText();
	void HandleScreenshotRequestProcessed();

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> LatestScreenshotTexture;

	FString LatestScreenshotPath;
	FDelegateHandle ScreenshotProcessedDelegateHandle;
};

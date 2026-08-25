// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SnowRumbleAudioUserWidget.generated.h"

class UTextBlock;
class USoundBase;

/** 공통 UI 버튼 상호작용음을 로컬 플레이어에게 재생하는 UI 부모 클래스다. */
UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API USnowRumbleAudioUserWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 모든 버튼 hover에 사용하는 공통 UI 사운드 자산이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Audio")
	TObjectPtr<USoundBase> ButtonHoverSound;

	/** 모든 버튼 click에 사용하는 공통 UI 사운드 자산이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Audio")
	TObjectPtr<USoundBase> ButtonClickSound;

	/** 영어 표시 시 각 텍스트 블록의 원본 폰트 크기에서 줄일 값이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Localization", meta = (ClampMin = "0"))
	int32 EnglishFontSizeReduction = 10;

private:
	UFUNCTION()
	void HandleButtonClicked();

	UFUNCTION()
	void HandleButtonHovered();

	void ApplyLocalizedFontSize();
	void HandleTextRevisionChanged();
	bool IsEnglishLanguage() const;

	FDelegateHandle TextRevisionChangedHandle;
	TMap<TWeakObjectPtr<UTextBlock>, int32> OriginalTextBlockFontSizes;
};

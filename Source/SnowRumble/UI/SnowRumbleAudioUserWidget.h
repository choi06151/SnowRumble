// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SnowRumbleAudioUserWidget.generated.h"

class USoundBase;

/** 공통 UI 버튼 상호작용음을 로컬 플레이어에게 재생하는 UI 부모 클래스다. */
UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API USnowRumbleAudioUserWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	/** 모든 버튼 hover에 사용하는 공통 UI 사운드 자산이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Audio")
	TObjectPtr<USoundBase> ButtonHoverSound;

	/** 모든 버튼 click에 사용하는 공통 UI 사운드 자산이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Audio")
	TObjectPtr<USoundBase> ButtonClickSound;

private:
	UFUNCTION()
	void HandleButtonClicked();

	UFUNCTION()
	void HandleButtonHovered();
};

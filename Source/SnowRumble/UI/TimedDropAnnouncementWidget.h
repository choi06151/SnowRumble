// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimedDropAnnouncementWidget.generated.h"

class UWidgetAnimation;

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API UTimedDropAnnouncementWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 지정된 Fade In/Out 애니메이션을 재생한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Announcement")
	void StartAnnouncementAnimation();

protected:
	/** WBP에서 Fade In과 Fade Out을 하나로 구성해 연결할 애니메이션이다. */
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> FadeInOutAnimation;
};

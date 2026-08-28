// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamageTextWidget_C.generated.h"

class UTextBlock;

UENUM(BlueprintType)
enum class ESnowRumbleDamageTextType : uint8
{
	Normal,
	Headshot
};

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API UDamageTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Damage Text")
	void InitializeDamageText(
		float AppliedDamage,
		ESnowRumbleDamageTextType DamageTextType);

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Damage Text")
	void OnDamageTextInitialized(
		float AppliedDamage,
		ESnowRumbleDamageTextType DamageTextType);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DamageText;
};

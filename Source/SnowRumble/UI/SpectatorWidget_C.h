// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SpectatorWidget_C.generated.h"

class ASnowRumbleCharacter;
class UTextBlock;

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API USpectatorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Spectator")
	void SetSpectatorViewTarget(ASnowRumbleCharacter* NewViewTarget);

	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Spectator")
	int32 GetCurrentViewTargetId() const;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CurrentViewTargetIdText;

	UPROPERTY(Transient)
	TObjectPtr<ASnowRumbleCharacter> CurrentViewTarget;

	void RefreshCurrentViewTargetIdText();
};

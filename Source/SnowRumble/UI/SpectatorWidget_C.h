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
	/** 기존 WBP 바인딩 이름을 유지하며 현재 관전 대상 닉네임을 표시한다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CurrentViewTargetIdText;

	UPROPERTY(Transient)
	TObjectPtr<ASnowRumbleCharacter> CurrentViewTarget;

	void RefreshCurrentViewTargetIdText();
};

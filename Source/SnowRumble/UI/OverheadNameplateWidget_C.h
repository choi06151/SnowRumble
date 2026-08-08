// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadNameplateWidget_C.generated.h"

class ASnowRumbleCharacter;
class UTextBlock;

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API UOverheadNameplateWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 이름표가 표시할 캐릭터를 지정하고 즉시 표시를 갱신한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Nameplate")
	void SetObservedCharacter(ASnowRumbleCharacter* NewCharacter);

	/** 현재 이름표가 읽는 캐릭터를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Nameplate")
	ASnowRumbleCharacter* GetObservedCharacter() const;

	/** 현재 표시할 닉네임을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Nameplate")
	FString GetDisplayName() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(
		const FGeometry& MyGeometry,
		float InDeltaTime) override;

	/** 있으면 C++ 부모가 닉네임을 자동 표시한다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Nameplate")
	TObjectPtr<UTextBlock> PlayerNameTextBlock;

private:
	void RefreshNameText();

	UPROPERTY(Transient)
	TObjectPtr<ASnowRumbleCharacter> ObservedCharacter;

	FString LastDisplayName;
};

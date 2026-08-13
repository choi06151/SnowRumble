// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PodiumWidget.generated.h"

class UTextBlock;

UCLASS()
class SNOWRUMBLE_API UPodiumWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 1~3등 결과 문구를 포디움 UI에 표시한다. */
	UFUNCTION(BlueprintCallable, Category = "Podium")
	void SetPodiumNames(const FText& FirstPlace, const FText& SecondPlace, const FText& ThirdPlace);

	/** 로비 복귀 안내 같은 보조 문구를 표시한다. */
	UFUNCTION(BlueprintCallable, Category = "Podium")
	void SetSubtitle(const FText& SubtitleText);

protected:
	virtual void NativeConstruct() override;

	/** WBP가 같은 이름의 TextBlock을 두면 C++가 자동으로 결과 문구를 넣는다. */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* FirstPlaceText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* SecondPlaceText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ThirdPlaceText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* SubtitleText;

	/** C++가 텍스트를 갱신한 뒤 Blueprint가 연출을 연결할 수 있게 알린다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Podium")
	void OnPodiumUpdated();

private:
	FText CachedFirst;
	FText CachedSecond;
	FText CachedThird;
	FText CachedSubtitle;
};

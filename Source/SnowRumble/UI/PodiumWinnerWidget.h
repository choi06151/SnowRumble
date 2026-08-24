// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../Game/SnowRumblePlayerState.h"
#include "Blueprint/UserWidget.h"
#include "PodiumWinnerWidget.generated.h"

class UTextBlock;

UCLASS()
class SNOWRUMBLE_API UPodiumWinnerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 포디움 전용 승리 팀 표시와 복귀 안내를 갱신한다. */
	UFUNCTION(BlueprintCallable, Category = "Podium")
	void SetWinnerPresentation(
		ESnowRumbleTeam WinningTeam,
		const FText& SubtitleText);

	/** 현재 표시 중인 승리 팀을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "Podium")
	ESnowRumbleTeam GetWinningTeam() const;

	/** 현재 표시 중인 승리 팀 텍스트를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "Podium")
	FText GetWinningTeamText() const;

	/** 포디움 복귀 안내 문구만 갱신한다. */
	UFUNCTION(BlueprintCallable, Category = "Podium")
	void SetSubtitleText(const FText& NewSubtitleText);

protected:
	virtual void NativeConstruct() override;

	/** WBP가 같은 이름의 텍스트 위젯을 두면 C++가 자동으로 팀명을 넣는다. */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* WinningTeamText;

	/** WBP가 같은 이름의 텍스트 위젯을 두면 C++가 자동으로 보조 문구를 넣는다. */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* SubtitleText;

	/** C++가 갱신한 뒤 Blueprint가 전용 연출을 연결할 수 있게 알린다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Podium")
	void OnWinnerPresentationUpdated();

private:
	FText CachedWinningTeamText;
	FText CachedSubtitleText;
	ESnowRumbleTeam CachedWinningTeam = ESnowRumbleTeam::None;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PodiumPlayerController.h"
#include "SnowmanModePodiumPlayerController_K.generated.h"

class UPodiumWinnerWidget;

UCLASS()
class SNOWRUMBLE_API ASnowmanModePodiumPlayerController
	: public APodiumPlayerController
{
	GENERATED_BODY()

public:
	ASnowmanModePodiumPlayerController();

	/** 서버가 확정한 눈사람 모드 포디움 결과를 로컬 화면에 표시한다. */
	UFUNCTION(Client, Reliable)
	void ClientSetSnowmanPodiumResult(
		const FText& ResultText,
		const FText& SubtitleText);

	/** 눈사람 모드 포디움 로비 복귀 카운트다운 문구만 갱신한다. */
	UFUNCTION(Client, Reliable)
	void ClientUpdateSnowmanPodiumSubtitle(const FText& SubtitleText);

	/** 각 클라이언트가 로컬 포디움 카메라를 활성화한다. */
	UFUNCTION(Client, Reliable)
	void ClientSetSnowmanPodiumCamera();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UPodiumWinnerWidget> SnowmanPodiumResultWidget;

	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Snowman|Podium")
	TSubclassOf<UPodiumWinnerWidget> SnowmanPodiumWinnerWidgetClass;

	void EnsureSnowmanPodiumResultWidget();
};

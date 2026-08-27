// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UObject/SoftObjectPath.h"
#include "PodiumPlayerController.generated.h"

class UPodiumWinnerWidget;
class USoundBase;
enum class ESnowRumbleTeam : uint8; 

UCLASS()
class SNOWRUMBLE_API APodiumPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APodiumPlayerController();

	/** 서버가 확정한 포디움 승리 팀 정보를 로컬 위젯에 전달한다. */
	UFUNCTION(Client, Reliable)
	void ClientSetPodiumWinner(
		ESnowRumbleTeam WinningTeam,
		const FText& Subtitle);

	/** 포디움 복귀 카운트다운 문구만 갱신한다. */
	UFUNCTION(Client, Reliable)
	void ClientUpdatePodiumReturnSubtitle(const FText& Subtitle);

	UFUNCTION(Client, Reliable, Category = "SnowRumble|Audio")
	void ClientPlayBackgroundMusic(const FSoftObjectPath& BackgroundMusicPath);

	UFUNCTION(Client, Reliable, Category = "SnowRumble|Audio")
	void ClientStopBackgroundMusic();

	/** 현재 재생 중인 배경음악의 볼륨 프리뷰를 갱신한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Audio")
	void SetBackgroundMusicPreviewVolume(float MasterVolume, float BgmVolume);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 포디움에서 로컬 플레이어에게 보여줄 UI WBP 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Podium|UI")
	TSubclassOf<UPodiumWinnerWidget> PodiumWinnerWidgetClass;

	/** 포디움에서 재생할 배경음악이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Audio")
	TObjectPtr<USoundBase> BackgroundMusicSound;

	/** 현재 로컬 화면에 띄운 포디움 UI 인스턴스다. */
	UPROPERTY(Transient)
	TObjectPtr<UPodiumWinnerWidget> PodiumWinnerWidget;

	void PlayBackgroundMusic(USoundBase* Music);
	void StopBackgroundMusic();
};

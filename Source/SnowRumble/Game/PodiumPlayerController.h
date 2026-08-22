// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PodiumPlayerController.generated.h"

class UPodiumWidget;
class UAudioComponent;
class USoundBase;

UCLASS()
class SNOWRUMBLE_API APodiumPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APodiumPlayerController();

	/** 서버가 확정한 포디움 결과 문구를 로컬 위젯에 전달한다. */
	UFUNCTION(Client, Reliable)
	void ClientSetPodiumResults(
		const FText& FirstPlace,
		const FText& SecondPlace,
		const FText& ThirdPlace,
		const FText& Subtitle);

	UFUNCTION(Client, Reliable, Category = "SnowRumble|Audio")
	void ClientPlayBackgroundMusic(USoundBase* NewBackgroundMusicSound);

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
	TSubclassOf<UPodiumWidget> PodiumWidgetClass;

	/** 포디움에서 재생할 배경음악이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Audio")
	TObjectPtr<USoundBase> BackgroundMusicSound;

	/** 현재 로컬 화면에 띄운 포디움 UI 인스턴스다. */
	UPROPERTY(Transient)
	TObjectPtr<UPodiumWidget> PodiumWidget;

	void PlayBackgroundMusic(USoundBase* Music);
	void StopBackgroundMusic();

	TWeakObjectPtr<UAudioComponent> BackgroundMusicComponent;
};

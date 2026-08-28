// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SnowRumbleBackgroundMusicSubsystem_C.generated.h"

class UAudioComponent;
class USoundBase;

UCLASS()
class SNOWRUMBLE_API USnowRumbleBackgroundMusicSubsystem
	: public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	void PlayBackgroundMusic(
		USoundBase* BackgroundMusicSound,
		bool bShouldLoop = true);
	void StopBackgroundMusic();
	void SetBackgroundMusicPreviewVolume(float MasterVolume, float BgmVolume);

private:
	UFUNCTION()
	void HandleBackgroundMusicFinished();

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> BackgroundMusicComponent;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> CurrentBackgroundMusicSound;

	bool bCurrentBackgroundMusicShouldLoop = true;

	FTimerHandle NonLoopStopTimerHandle;
};

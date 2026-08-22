// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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

	void PlayBackgroundMusic(USoundBase* BackgroundMusicSound);
	void StopBackgroundMusic();
	void SetBackgroundMusicPreviewVolume(float MasterVolume, float BgmVolume);

private:
	UFUNCTION()
	void HandleBackgroundMusicFinished();

	TWeakObjectPtr<UAudioComponent> BackgroundMusicComponent;
	TWeakObjectPtr<USoundBase> CurrentBackgroundMusicSound;
};

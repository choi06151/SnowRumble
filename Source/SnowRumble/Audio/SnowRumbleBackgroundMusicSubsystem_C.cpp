// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleBackgroundMusicSubsystem_C.h"

#include "Components/AudioComponent.h"
#include "../Audio/SnowRumbleAudioHelpers.h"
#include "Sound/SoundBase.h"

void USnowRumbleBackgroundMusicSubsystem::Deinitialize()
{
	StopBackgroundMusic();
	Super::Deinitialize();
}

void USnowRumbleBackgroundMusicSubsystem::PlayBackgroundMusic(
	USoundBase* BackgroundMusicSound)
{
	if (!BackgroundMusicSound)
	{
		StopBackgroundMusic();
		return;
	}

	if (UAudioComponent* CurrentBackgroundMusicComponent =
		BackgroundMusicComponent.Get())
	{
		if (CurrentBackgroundMusicComponent->Sound == BackgroundMusicSound
			&& CurrentBackgroundMusicComponent->IsPlaying())
		{
			CurrentBackgroundMusicComponent->SetVolumeMultiplier(
				SnowRumbleAudio::GetEffectiveVolume(
					this,
					ESnowRumbleAudioMixChannel::BackgroundMusic));
			return;
		}
	}

	StopBackgroundMusic();
	BackgroundMusicComponent = SnowRumbleAudio::SpawnSound2D(
		this,
		BackgroundMusicSound,
		ESnowRumbleAudioMixChannel::BackgroundMusic,
		1.0f,
		1.0f,
		true);
}

void USnowRumbleBackgroundMusicSubsystem::StopBackgroundMusic()
{
	if (UAudioComponent* CurrentBackgroundMusicComponent =
		BackgroundMusicComponent.Get())
	{
		CurrentBackgroundMusicComponent->Stop();
	}

	BackgroundMusicComponent.Reset();
}

void USnowRumbleBackgroundMusicSubsystem::SetBackgroundMusicPreviewVolume(
	float MasterVolume,
	float BgmVolume)
{
	if (UAudioComponent* CurrentBackgroundMusicComponent =
		BackgroundMusicComponent.Get())
	{
		CurrentBackgroundMusicComponent->SetVolumeMultiplier(
			FMath::Clamp(MasterVolume, 0.0f, 1.0f)
			* FMath::Clamp(BgmVolume, 0.0f, 1.0f));
	}
}

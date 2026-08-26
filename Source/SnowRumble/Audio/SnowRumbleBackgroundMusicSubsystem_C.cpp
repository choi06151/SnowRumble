// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleBackgroundMusicSubsystem_C.h"

#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "../Audio/SnowRumbleAudioHelpers.h"
#include "Sound/SoundBase.h"

void USnowRumbleBackgroundMusicSubsystem::Deinitialize()
{
	StopBackgroundMusic();
	Super::Deinitialize();
}

void USnowRumbleBackgroundMusicSubsystem::PlayBackgroundMusic(
	USoundBase* BackgroundMusicSound,
	bool bShouldLoop)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(NonLoopStopTimerHandle);
	}

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
			bCurrentBackgroundMusicShouldLoop = bShouldLoop;
			if (!bShouldLoop)
			{
				const float MusicDuration = BackgroundMusicSound->GetDuration();
				if (MusicDuration > 0.0f && FMath::IsFinite(MusicDuration))
				{
					if (UWorld* World = GetWorld())
					{
						World->GetTimerManager().SetTimer(
							NonLoopStopTimerHandle,
							this,
							&USnowRumbleBackgroundMusicSubsystem::StopBackgroundMusic,
							MusicDuration,
							false);
					}
				}
			}
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
	if (UAudioComponent* CurrentBackgroundMusicComponent =
		BackgroundMusicComponent.Get())
	{
		CurrentBackgroundMusicSound = BackgroundMusicSound;
		bCurrentBackgroundMusicShouldLoop = bShouldLoop;
		if (!bShouldLoop)
		{
			const float MusicDuration = BackgroundMusicSound->GetDuration();
			if (MusicDuration > 0.0f && FMath::IsFinite(MusicDuration))
			{
				if (UWorld* World = GetWorld())
				{
					World->GetTimerManager().SetTimer(
						NonLoopStopTimerHandle,
						this,
						&USnowRumbleBackgroundMusicSubsystem::StopBackgroundMusic,
						MusicDuration,
						false);
				}
			}
		}
		CurrentBackgroundMusicComponent->OnAudioFinished.AddUniqueDynamic(
			this,
			&USnowRumbleBackgroundMusicSubsystem::HandleBackgroundMusicFinished);
	}
}

void USnowRumbleBackgroundMusicSubsystem::StopBackgroundMusic()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(NonLoopStopTimerHandle);
	}

	if (UAudioComponent* CurrentBackgroundMusicComponent =
		BackgroundMusicComponent.Get())
	{
		CurrentBackgroundMusicComponent->OnAudioFinished.RemoveAll(this);
		CurrentBackgroundMusicComponent->Stop();
	}

	CurrentBackgroundMusicSound = nullptr;
	bCurrentBackgroundMusicShouldLoop = true;
	BackgroundMusicComponent = nullptr;
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

void USnowRumbleBackgroundMusicSubsystem::HandleBackgroundMusicFinished()
{
	USoundBase* MusicToRestart = CurrentBackgroundMusicSound;
	const bool bShouldRestart = bCurrentBackgroundMusicShouldLoop;
	if (!MusicToRestart || !bShouldRestart)
	{
		CurrentBackgroundMusicSound = nullptr;
		bCurrentBackgroundMusicShouldLoop = true;
		BackgroundMusicComponent = nullptr;
		return;
	}

	if (UAudioComponent* CurrentBackgroundMusicComponent =
		BackgroundMusicComponent.Get())
	{
		CurrentBackgroundMusicComponent->OnAudioFinished.RemoveAll(this);
	}

	// 종료 콜백 프레임에서는 기존 컴포넌트가 아직 유효하거나 재생 중으로
	// 보일 수 있으므로, 이전 컴포넌트를 먼저 비우고 같은 음원을 새로 시작한다.
	BackgroundMusicComponent = nullptr;
	CurrentBackgroundMusicSound = nullptr;
	PlayBackgroundMusic(MusicToRestart);
}

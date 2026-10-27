// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleAudioHelpers.h"

#include "../Player/SnowRumbleUserSettingsSubsystem_C.h"
#include "Components/AudioComponent.h"
#include "HAL/IConsoleManager.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"

namespace
{
// 반복 재생되는 게임플레이 효과음이 똑같이 들리지 않도록 재생할 때마다 피치를 조금씩 흔든다.
// 0.06이면 0.94~1.06배 범위다. 실행 중 콘솔에서 snowrumble.Audio.GameplayPitchVariation 0.1 처럼 바꿔 볼 수 있다.
float GGameplayPitchVariation = 0.06f;
FAutoConsoleVariableRef CVarGameplayPitchVariation(
	TEXT("snowrumble.Audio.GameplayPitchVariation"),
	GGameplayPitchVariation,
	TEXT("Random pitch range applied to one-shot Gameplay channel sounds. 0 disables it. Default 0.06 (+/-6%)."),
	ECVF_Default);

float ApplyGameplayPitchVariation(
	ESnowRumbleAudioMixChannel Channel,
	float PitchMultiplier)
{
	// UI, 배경음악, 보이스는 음정이 바뀌면 어색하므로 게임플레이 효과음에만 적용한다.
	if (Channel != ESnowRumbleAudioMixChannel::Gameplay)
	{
		return PitchMultiplier;
	}

	const float Variation = FMath::Clamp(GGameplayPitchVariation, 0.0f, 0.5f);
	if (Variation <= KINDA_SMALL_NUMBER)
	{
		return PitchMultiplier;
	}

	return PitchMultiplier * FMath::FRandRange(1.0f - Variation, 1.0f + Variation);
}

float GetChannelVolume(
	const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem,
	ESnowRumbleAudioMixChannel Channel)
{
	if (!UserSettingsSubsystem)
	{
		return 1.0f;
	}

	switch (Channel)
	{
	case ESnowRumbleAudioMixChannel::BackgroundMusic:
		return UserSettingsSubsystem->GetBgmVolume();
	case ESnowRumbleAudioMixChannel::Voice:
		return UserSettingsSubsystem->GetVoiceVolume();
	case ESnowRumbleAudioMixChannel::UserInterface:
	case ESnowRumbleAudioMixChannel::Gameplay:
	default:
		return UserSettingsSubsystem->GetSfxVolume();
	}
}
}

float SnowRumbleAudio::GetEffectiveVolume(
	const UObject* WorldContextObject,
	ESnowRumbleAudioMixChannel Channel)
{
	if (!WorldContextObject)
	{
		return 1.0f;
	}

	const UGameInstance* GameInstance =
		UGameplayStatics::GetGameInstance(WorldContextObject);
	const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem = GameInstance
		? GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>()
		: nullptr;

	const float MasterVolume = UserSettingsSubsystem
		? UserSettingsSubsystem->GetMasterVolume()
		: 1.0f;
	return FMath::Clamp(MasterVolume, 0.0f, 1.0f)
		* FMath::Clamp(GetChannelVolume(UserSettingsSubsystem, Channel), 0.0f, 1.0f);
}

void SnowRumbleAudio::PlaySound2D(
	const UObject* WorldContextObject,
	USoundBase* Sound,
	ESnowRumbleAudioMixChannel Channel,
	float BaseVolumeMultiplier,
	float PitchMultiplier)
{
	if (!WorldContextObject || !Sound)
	{
		return;
	}

	UGameplayStatics::PlaySound2D(
		WorldContextObject,
		Sound,
		GetEffectiveVolume(WorldContextObject, Channel)
			* FMath::Max(0.0f, BaseVolumeMultiplier),
		FMath::Max(0.0f, ApplyGameplayPitchVariation(Channel, PitchMultiplier)));
}

UAudioComponent* SnowRumbleAudio::SpawnSound2D(
	const UObject* WorldContextObject,
	USoundBase* Sound,
	ESnowRumbleAudioMixChannel Channel,
	float BaseVolumeMultiplier,
	float PitchMultiplier,
	bool bPersistAcrossLevelTransition)
{
	if (!WorldContextObject || !Sound)
	{
		return nullptr;
	}

	return UGameplayStatics::SpawnSound2D(
		WorldContextObject,
		Sound,
		GetEffectiveVolume(WorldContextObject, Channel)
			* FMath::Max(0.0f, BaseVolumeMultiplier),
		FMath::Max(0.0f, PitchMultiplier),
		0.0f,
		nullptr,
		bPersistAcrossLevelTransition,
		true);
}

void SnowRumbleAudio::PlaySoundAtLocation(
	const UObject* WorldContextObject,
	USoundBase* Sound,
	ESnowRumbleAudioMixChannel Channel,
	const FVector& Location,
	float BaseVolumeMultiplier,
	float PitchMultiplier,
	USoundAttenuation* AttenuationSettings)
{
	if (!WorldContextObject || !Sound || Location.ContainsNaN())
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		WorldContextObject,
		Sound,
		Location,
		GetEffectiveVolume(WorldContextObject, Channel)
			* FMath::Max(0.0f, BaseVolumeMultiplier),
		FMath::Max(0.0f, ApplyGameplayPitchVariation(Channel, PitchMultiplier)),
		0.0f,
		AttenuationSettings);
}

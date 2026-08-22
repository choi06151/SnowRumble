// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleAudioHelpers.h"

#include "../Player/SnowRumbleUserSettingsSubsystem_C.h"
#include "Components/AudioComponent.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

namespace
{
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
		FMath::Max(0.0f, PitchMultiplier));
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
	float PitchMultiplier)
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
		FMath::Max(0.0f, PitchMultiplier));
}

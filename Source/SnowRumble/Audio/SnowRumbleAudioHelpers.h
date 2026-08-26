// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SnowRumbleAudioHelpers.generated.h"

class USoundBase;
class USoundAttenuation;
class UAudioComponent;

UENUM(BlueprintType)
enum class ESnowRumbleAudioMixChannel : uint8
{
	UserInterface,
	Gameplay,
	BackgroundMusic,
	Voice
};

namespace SnowRumbleAudio
{
	/** 현재 로컬 사용자 설정을 반영한 실제 볼륨 배율을 반환한다. */
	float GetEffectiveVolume(
		const UObject* WorldContextObject,
		ESnowRumbleAudioMixChannel Channel);

	/** 2D 사운드를 현재 볼륨 설정에 맞춰 재생한다. */
	void PlaySound2D(
		const UObject* WorldContextObject,
		USoundBase* Sound,
		ESnowRumbleAudioMixChannel Channel,
		float BaseVolumeMultiplier = 1.0f,
		float PitchMultiplier = 1.0f);

	/** 2D 사운드를 현재 볼륨 설정에 맞춰 생성하고 컴포넌트를 반환한다. */
	UAudioComponent* SpawnSound2D(
		const UObject* WorldContextObject,
		USoundBase* Sound,
		ESnowRumbleAudioMixChannel Channel,
		float BaseVolumeMultiplier = 1.0f,
		float PitchMultiplier = 1.0f,
		bool bPersistAcrossLevelTransition = true);

	/** 위치 기반 사운드를 현재 볼륨 설정에 맞춰 재생한다. */
	void PlaySoundAtLocation(
		const UObject* WorldContextObject,
		USoundBase* Sound,
		ESnowRumbleAudioMixChannel Channel,
		const FVector& Location,
		float BaseVolumeMultiplier = 1.0f,
		float PitchMultiplier = 1.0f,
		USoundAttenuation* AttenuationSettings = nullptr);
}

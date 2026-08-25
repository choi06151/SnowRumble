// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SnowRumbleUserSettingsSubsystem_C.generated.h"

DECLARE_MULTICAST_DELEGATE(FSnowRumbleUserSettingsChanged);

UENUM(BlueprintType)
enum class ESnowRumbleMicrophoneMode : uint8
{
	PushToTalk,
	AlwaysOn
};

UCLASS(Config = GameUserSettings)
class SNOWRUMBLE_API USnowRumbleUserSettingsSubsystem
	: public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	FSnowRumbleUserSettingsChanged OnKeyBindingsChanged;
	FSnowRumbleUserSettingsChanged OnMicrophoneSettingsChanged;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|User Settings|Input")
	void SetKeyBinding(FName BindingId, FKey NewKey);

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|User Settings|Input")
	void ResetKeyBinding(FName BindingId);

	UFUNCTION(BlueprintPure, Category = "SnowRumble|User Settings|Input")
	FKey GetKeyBinding(FName BindingId, FKey DefaultKey) const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|User Settings|Input")
	bool HasKeyBinding(FName BindingId) const;

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|User Settings|Sensitivity")
	void SetMouseSensitivity(float NewSensitivity);

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|User Settings|Sensitivity")
	void ResetMouseSensitivity();

	UFUNCTION(BlueprintPure, Category = "SnowRumble|User Settings|Sensitivity")
	float GetMouseSensitivity() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|User Settings|Sensitivity")
	float GetDefaultMouseSensitivity() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|User Settings|Sensitivity")
	float GetMinMouseSensitivity() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|User Settings|Sensitivity")
	float GetMaxMouseSensitivity() const;

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|User Settings|Audio")
	void SetMasterVolume(float NewVolume);

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|User Settings|Audio")
	void SetBgmVolume(float NewVolume);

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|User Settings|Audio")
	void SetSfxVolume(float NewVolume);

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|User Settings|Audio")
	void SetVoiceVolume(float NewVolume);

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|User Settings|Audio")
	void ResetMasterVolume();

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|User Settings|Audio")
	void ResetBgmVolume();

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|User Settings|Audio")
	void ResetSfxVolume();

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|User Settings|Audio")
	void ResetVoiceVolume();

	UFUNCTION(BlueprintPure, Category = "SnowRumble|User Settings|Audio")
	float GetMasterVolume() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|User Settings|Audio")
	float GetBgmVolume() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|User Settings|Audio")
	float GetSfxVolume() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|User Settings|Audio")
	float GetVoiceVolume() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|User Settings|Audio")
	float GetDefaultAudioVolume() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|User Settings|Audio")
	float GetDefaultVoiceVolume() const;

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|User Settings|Microphone")
	void SetMicrophoneVolume(float NewVolume);

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|User Settings|Microphone")
	void ResetMicrophoneVolume();

	UFUNCTION(BlueprintPure, Category = "SnowRumble|User Settings|Microphone")
	float GetMicrophoneVolume() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|User Settings|Microphone")
	float GetDefaultMicrophoneVolume() const;

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|User Settings|Microphone")
	void SetMicrophoneMode(ESnowRumbleMicrophoneMode NewMode);

	UFUNCTION(BlueprintPure, Category = "SnowRumble|User Settings|Microphone")
	ESnowRumbleMicrophoneMode GetMicrophoneMode() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|User Settings|Microphone")
	ESnowRumbleMicrophoneMode GetDefaultMicrophoneMode() const;

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|User Settings|Microphone")
	void SetMicrophoneDeviceId(const FString& NewDeviceId);

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|User Settings|Microphone")
	void ResetMicrophoneDeviceId();

	UFUNCTION(BlueprintPure, Category = "SnowRumble|User Settings|Microphone")
	FString GetMicrophoneDeviceId() const;

private:
	UPROPERTY(Config)
	TMap<FName, FKey> KeyBindings;

	UPROPERTY(Config)
	float MouseSensitivity = 1.0f;

	UPROPERTY(Config)
	float MasterVolume = 1.0f;

	UPROPERTY(Config)
	float BgmVolume = 1.0f;

	UPROPERTY(Config)
	float SfxVolume = 1.0f;

	UPROPERTY(Config)
	float VoiceVolume = 1.0f;

	UPROPERTY(Config)
	float MicrophoneVolume = 1.0f;

	UPROPERTY(Config)
	ESnowRumbleMicrophoneMode MicrophoneMode =
		ESnowRumbleMicrophoneMode::PushToTalk;

	UPROPERTY(Config)
	FString MicrophoneDeviceId;

	static constexpr float MinMouseSensitivity = 0.2f;
	static constexpr float MaxMouseSensitivity = 3.0f;
	static constexpr float DefaultMouseSensitivity = 1.0f;
	static constexpr float DefaultAudioVolume = 1.0f;
	static constexpr float DefaultVoiceVolume = 1.0f;
	static constexpr float DefaultMicrophoneVolume = 1.0f;
	static constexpr ESnowRumbleMicrophoneMode DefaultMicrophoneMode =
		ESnowRumbleMicrophoneMode::PushToTalk;
};

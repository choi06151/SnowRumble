// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleUserSettingsSubsystem_C.h"

void USnowRumbleUserSettingsSubsystem::Initialize(
	FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadConfig();
}

void USnowRumbleUserSettingsSubsystem::SetKeyBinding(
	FName BindingId,
	FKey NewKey)
{
	if (BindingId.IsNone())
	{
		return;
	}

	KeyBindings.Add(BindingId, NewKey);
	SaveConfig();
	OnKeyBindingsChanged.Broadcast();
}

void USnowRumbleUserSettingsSubsystem::ResetKeyBinding(FName BindingId)
{
	if (BindingId.IsNone())
	{
		return;
	}

	KeyBindings.Remove(BindingId);
	SaveConfig();
	OnKeyBindingsChanged.Broadcast();
}

FKey USnowRumbleUserSettingsSubsystem::GetKeyBinding(
	FName BindingId,
	FKey DefaultKey) const
{
	if (const FKey* SavedKey = KeyBindings.Find(BindingId))
	{
		return *SavedKey;
	}

	return DefaultKey;
}

bool USnowRumbleUserSettingsSubsystem::HasKeyBinding(FName BindingId) const
{
	return KeyBindings.Contains(BindingId);
}

void USnowRumbleUserSettingsSubsystem::SetMouseSensitivity(
	float NewSensitivity)
{
	MouseSensitivity = FMath::Clamp(
		NewSensitivity,
		MinMouseSensitivity,
		MaxMouseSensitivity);
	SaveConfig();
}

void USnowRumbleUserSettingsSubsystem::ResetMouseSensitivity()
{
	MouseSensitivity = DefaultMouseSensitivity;
	SaveConfig();
}

float USnowRumbleUserSettingsSubsystem::GetMouseSensitivity() const
{
	return FMath::Clamp(
		MouseSensitivity,
		MinMouseSensitivity,
		MaxMouseSensitivity);
}

float USnowRumbleUserSettingsSubsystem::GetDefaultMouseSensitivity() const
{
	return DefaultMouseSensitivity;
}

float USnowRumbleUserSettingsSubsystem::GetMinMouseSensitivity() const
{
	return MinMouseSensitivity;
}

float USnowRumbleUserSettingsSubsystem::GetMaxMouseSensitivity() const
{
	return MaxMouseSensitivity;
}

void USnowRumbleUserSettingsSubsystem::SetBgmVolume(float NewVolume)
{
	BgmVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
	SaveConfig();
}

void USnowRumbleUserSettingsSubsystem::SetSfxVolume(float NewVolume)
{
	SfxVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
	SaveConfig();
}

void USnowRumbleUserSettingsSubsystem::ResetBgmVolume()
{
	BgmVolume = DefaultAudioVolume;
	SaveConfig();
}

void USnowRumbleUserSettingsSubsystem::ResetSfxVolume()
{
	SfxVolume = DefaultAudioVolume;
	SaveConfig();
}

float USnowRumbleUserSettingsSubsystem::GetBgmVolume() const
{
	return FMath::Clamp(BgmVolume, 0.0f, 1.0f);
}

float USnowRumbleUserSettingsSubsystem::GetSfxVolume() const
{
	return FMath::Clamp(SfxVolume, 0.0f, 1.0f);
}

float USnowRumbleUserSettingsSubsystem::GetDefaultAudioVolume() const
{
	return DefaultAudioVolume;
}

void USnowRumbleUserSettingsSubsystem::SetMicrophoneVolume(float NewVolume)
{
	MicrophoneVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
	SaveConfig();
	OnMicrophoneSettingsChanged.Broadcast();
}

void USnowRumbleUserSettingsSubsystem::ResetMicrophoneVolume()
{
	MicrophoneVolume = DefaultMicrophoneVolume;
	SaveConfig();
	OnMicrophoneSettingsChanged.Broadcast();
}

float USnowRumbleUserSettingsSubsystem::GetMicrophoneVolume() const
{
	return FMath::Clamp(MicrophoneVolume, 0.0f, 1.0f);
}

float USnowRumbleUserSettingsSubsystem::GetDefaultMicrophoneVolume() const
{
	return DefaultMicrophoneVolume;
}

void USnowRumbleUserSettingsSubsystem::SetMicrophoneMode(
	ESnowRumbleMicrophoneMode NewMode)
{
	MicrophoneMode = NewMode;
	SaveConfig();
	OnMicrophoneSettingsChanged.Broadcast();
}

ESnowRumbleMicrophoneMode
USnowRumbleUserSettingsSubsystem::GetMicrophoneMode() const
{
	return MicrophoneMode;
}

ESnowRumbleMicrophoneMode
USnowRumbleUserSettingsSubsystem::GetDefaultMicrophoneMode() const
{
	return DefaultMicrophoneMode;
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleUserSettingsSubsystem_C.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/TextLocalizationManager.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Paths.h"

void USnowRumbleUserSettingsSubsystem::Initialize(
	FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FCoreDelegates::GatherAdditionalLocResPathsCallback.AddUObject(
		this,
		&USnowRumbleUserSettingsSubsystem::AddSnowRumbleLocalizationPath);

	LoadConfig();
	const FString Culture = LanguageCulture.IsEmpty() ? TEXT("ko") : LanguageCulture;
	ApplyLanguageCulture(Culture);
}

void USnowRumbleUserSettingsSubsystem::Deinitialize()
{
	FCoreDelegates::GatherAdditionalLocResPathsCallback.RemoveAll(this);

	Super::Deinitialize();
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

void USnowRumbleUserSettingsSubsystem::SetMasterVolume(float NewVolume)
{
	MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
	SaveConfig();
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

void USnowRumbleUserSettingsSubsystem::SetVoiceVolume(float NewVolume)
{
	VoiceVolume = FMath::Clamp(NewVolume, 0.0f, MaxVoiceVolume);
	SaveConfig();
}

void USnowRumbleUserSettingsSubsystem::ResetMasterVolume()
{
	MasterVolume = DefaultAudioVolume;
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

void USnowRumbleUserSettingsSubsystem::ResetVoiceVolume()
{
	VoiceVolume = DefaultVoiceVolume;
	SaveConfig();
}

float USnowRumbleUserSettingsSubsystem::GetMasterVolume() const
{
	return FMath::Clamp(MasterVolume, 0.0f, 1.0f);
}

float USnowRumbleUserSettingsSubsystem::GetBgmVolume() const
{
	return FMath::Clamp(BgmVolume, 0.0f, 1.0f);
}

float USnowRumbleUserSettingsSubsystem::GetSfxVolume() const
{
	return FMath::Clamp(SfxVolume, 0.0f, 1.0f);
}

float USnowRumbleUserSettingsSubsystem::GetVoiceVolume() const
{
	return FMath::Clamp(VoiceVolume, 0.0f, MaxVoiceVolume);
}

float USnowRumbleUserSettingsSubsystem::GetDefaultAudioVolume() const
{
	return DefaultAudioVolume;
}

float USnowRumbleUserSettingsSubsystem::GetDefaultVoiceVolume() const
{
	return DefaultVoiceVolume;
}

void USnowRumbleUserSettingsSubsystem::SetMicrophoneVolume(float NewVolume)
{
	MicrophoneVolume = FMath::Clamp(NewVolume, 0.0f, MaxMicrophoneVolume);
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
	return FMath::Clamp(MicrophoneVolume, 0.0f, MaxMicrophoneVolume);
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

void USnowRumbleUserSettingsSubsystem::SetMicrophoneDeviceId(
	const FString& NewDeviceId)
{
	MicrophoneDeviceId = NewDeviceId;
	SaveConfig();
	OnMicrophoneSettingsChanged.Broadcast();
}

void USnowRumbleUserSettingsSubsystem::ResetMicrophoneDeviceId()
{
	MicrophoneDeviceId.Reset();
	SaveConfig();
	OnMicrophoneSettingsChanged.Broadcast();
}

FString USnowRumbleUserSettingsSubsystem::GetMicrophoneDeviceId() const
{
	return MicrophoneDeviceId;
}

void USnowRumbleUserSettingsSubsystem::SetLanguageCulture(
	const FString& NewCulture)
{
	const FString Culture = NewCulture.Equals(TEXT("en"), ESearchCase::IgnoreCase)
		? TEXT("en")
		: TEXT("ko");

	LanguageCulture = Culture;
	SaveConfig();
	ApplyLanguageCulture(Culture);
}

FString USnowRumbleUserSettingsSubsystem::GetLanguageCulture() const
{
	return LanguageCulture.IsEmpty() ? TEXT("ko") : LanguageCulture;
}

void USnowRumbleUserSettingsSubsystem::ApplyLanguageCulture(
	const FString& Culture)
{
	const bool bCultureApplied =
		FInternationalization::Get().SetCurrentLanguageAndLocale(Culture);

	FTextLocalizationManager::Get().RefreshResources(
		[Culture]()
		{
			LogLanguageProbe(Culture);
		});

	UE_LOG(
		LogTemp,
		Log,
		TEXT("SnowRumble language requested=%s applied=%s currentLanguage=%s currentLocale=%s"),
		*Culture,
		bCultureApplied ? TEXT("true") : TEXT("false"),
		*FInternationalization::Get().GetCurrentLanguage()->GetName(),
		*FInternationalization::Get().GetCurrentLocale()->GetName());
}

void USnowRumbleUserSettingsSubsystem::AddSnowRumbleLocalizationPath(
	TArray<FString>& LocalizationPaths)
{
	LocalizationPaths.AddUnique(
		FPaths::ProjectContentDir() / TEXT("Localization/SnowRumble"));
}

void USnowRumbleUserSettingsSubsystem::LogLanguageProbe(const FString& Culture)
{
	const FText ProbeText = NSLOCTEXT(
		"",
		"497C2BE446D905082A79A1863B8734D5",
		"방 만들기");

	UE_LOG(
		LogTemp,
		Log,
		TEXT("SnowRumble language resources refreshed culture=%s probe='%s'"),
		*Culture,
		*ProbeText.ToString());
}

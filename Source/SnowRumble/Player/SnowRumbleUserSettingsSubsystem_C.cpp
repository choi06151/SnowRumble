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

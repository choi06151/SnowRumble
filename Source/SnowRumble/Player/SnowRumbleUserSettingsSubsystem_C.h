// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SnowRumbleUserSettingsSubsystem_C.generated.h"

DECLARE_MULTICAST_DELEGATE(FSnowRumbleUserSettingsChanged);

UCLASS(Config = GameUserSettings)
class SNOWRUMBLE_API USnowRumbleUserSettingsSubsystem
	: public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	FSnowRumbleUserSettingsChanged OnKeyBindingsChanged;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|User Settings|Input")
	void SetKeyBinding(FName BindingId, FKey NewKey);

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|User Settings|Input")
	void ResetKeyBinding(FName BindingId);

	UFUNCTION(BlueprintPure, Category = "SnowRumble|User Settings|Input")
	FKey GetKeyBinding(FName BindingId, FKey DefaultKey) const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|User Settings|Input")
	bool HasKeyBinding(FName BindingId) const;

private:
	UPROPERTY(Config)
	TMap<FName, FKey> KeyBindings;
};

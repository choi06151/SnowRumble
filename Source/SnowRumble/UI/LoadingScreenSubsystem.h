// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LoadingScreenSubsystem.generated.h"

class ULoadingScreenWidget;

UCLASS()
class SNOWRUMBLE_API ULoadingScreenSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Loading")
	void ShowLoadingScreen(TSubclassOf<ULoadingScreenWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Loading")
	void HideLoadingScreen();

	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Loading")
	bool IsLoadingScreenVisible() const;

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Loading")
	void SetLoadingProgress(int32 LoadedPlayers, int32 ExpectedPlayers);

	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Loading")
	float GetLoadingProgress() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Loading")
	int32 GetLoadedPlayerCount() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Loading")
	int32 GetExpectedPlayerCount() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Loading")
	FText GetLoadingStatusText() const;

private:
	UPROPERTY(Transient)
	TObjectPtr<ULoadingScreenWidget> LoadingScreenWidget;

	int32 LoadedPlayerCount = 0;
	int32 ExpectedPlayerCount = 0;
};

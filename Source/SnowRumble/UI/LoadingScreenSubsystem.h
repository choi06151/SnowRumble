// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LoadingScreenSubsystem.generated.h"

class ULoadingScreenWidget;
class UTexture2D;
class UWorld;

UCLASS()
class SNOWRUMBLE_API ULoadingScreenSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Loading")
	void ShowLoadingScreen(TSubclassOf<ULoadingScreenWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Loading")
	void HideLoadingScreen();

	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Loading")
	bool IsLoadingScreenVisible() const;

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Loading")
	void SetLoadingProgress(int32 LoadedPlayers, int32 ExpectedPlayers);

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Loading")
	void SetLoadingPresentation(
		const FString& MapPackageName,
		const FText& MapDisplayName,
		TSoftObjectPtr<UTexture2D> MapLoadingImage,
		const TArray<FString>& TeamPlayerNames);

	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Loading")
	float GetLoadingProgress() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Loading")
	int32 GetLoadedPlayerCount() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Loading")
	int32 GetExpectedPlayerCount() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Loading")
	FText GetLoadingStatusText() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Loading")
	FString GetLoadingMapPackageName() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Loading")
	FText GetLoadingMapDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Loading")
	UTexture2D* GetLoadingMapImage() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Loading")
	TArray<FString> GetLoadingTeamPlayerNames() const;

private:
	void EnsureLoadingScreenWidget();
	void AddLoadingScreenWidgetToViewport();
	void StartMoviePlayerLoadingScreen();
	void StopMoviePlayerLoadingScreen();
	void HandlePostLoadMapWithWorld(UWorld* LoadedWorld);

	UPROPERTY(Transient)
	TObjectPtr<ULoadingScreenWidget> LoadingScreenWidget;

	UPROPERTY(Transient)
	TSubclassOf<ULoadingScreenWidget> LoadingScreenWidgetClass;

	int32 LoadedPlayerCount = 0;
	int32 ExpectedPlayerCount = 0;
	bool bLoadingScreenRequested = false;
	bool bMoviePlayerLoadingScreenActive = false;
	FString LoadingMapPackageName;
	FText LoadingMapDisplayName;
	TArray<FString> LoadingTeamPlayerNames;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> LoadedMapLoadingImage;
};

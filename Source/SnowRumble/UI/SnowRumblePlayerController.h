// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SnowRumblePlayerController.generated.h"

class ULoadingScreenWidget;

UCLASS(Blueprintable)
class SNOWRUMBLE_API ASnowRumblePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(Client, Reliable, Category = "SnowRumble|UI|Loading")
	void ClientShowLoadingScreen();

	UFUNCTION(Client, Reliable, Category = "SnowRumble|UI|Loading")
	void ClientHideLoadingScreen();

	UFUNCTION(Client, Reliable, Category = "SnowRumble|UI|Loading")
	void ClientUpdateLoadingProgress(int32 LoadedPlayers, int32 ExpectedPlayers);

protected:
	virtual void ClientShowLoadingScreen_Implementation();
	virtual void ClientUpdateLoadingProgress_Implementation(
		int32 LoadedPlayers,
		int32 ExpectedPlayers);
	virtual void ClientHideLoadingScreen_Implementation();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Loading")
	TSubclassOf<ULoadingScreenWidget> LoadingScreenWidgetClass;
};

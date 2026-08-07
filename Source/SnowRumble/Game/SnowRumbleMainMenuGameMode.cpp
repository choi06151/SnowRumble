// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleMainMenuGameMode.h"

#include "../UI/MainMenuPlayerController.h"

ASnowRumbleMainMenuGameMode::ASnowRumbleMainMenuGameMode()
{
	PlayerControllerClass = AMainMenuPlayerController::StaticClass();
}

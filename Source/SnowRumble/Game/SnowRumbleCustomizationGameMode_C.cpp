// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleCustomizationGameMode_C.h"

#include "../Player/SnowRumbleCharacter.h"
#include "../UI/CustomizationPlayerController_C.h"

ASnowRumbleCustomizationGameMode::ASnowRumbleCustomizationGameMode()
{
	PlayerControllerClass = ACustomizationPlayerController::StaticClass();
	DefaultPawnClass = ASnowRumbleCharacter::StaticClass();
}

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SnowRumbleCharacterAnimationTypes_C.generated.h"

UENUM(BlueprintType)
enum class ESnowRumbleCharacterAnimTrigger : uint8
{
	None,
	PickupSmallSnowball,
	PickupLargeSnowball,
	ItemInteraction,
	ThrowSmallSnowball,
	ThrowSmallSnowballInAir,
	ThrowLargeSnowball,
	ThrowSnowDuckMaker,
	HitReact
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GrabbablePhysicsObject_C.h"
#include "TambourineGrabbableObject_C.generated.h"

class USoundAttenuation;
class USoundBase;

UCLASS(Blueprintable)
class SNOWRUMBLE_API ATambourineGrabbableObject : public AGrabbablePhysicsObject
{
	GENERATED_BODY()

public:
	ATambourineGrabbableObject();

	virtual void HandleGrabbedByCharacter(ACharacter* Grabber) override;
	virtual void HandleReleasedByCharacter(ACharacter* Grabber) override;
	virtual void TickGrabbedByCharacter(
		ACharacter* Grabber,
		FVector HeldMotion,
		float DeltaTime) override;

protected:
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayJingle(
		FVector_NetQuantize Location,
		float PitchMultiplier);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Tambourine|Audio")
	TObjectPtr<USoundBase> JingleSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Tambourine|Audio")
	TObjectPtr<USoundAttenuation> JingleSoundAttenuation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Tambourine|Audio", meta = (ClampMin = "0.0"))
	float MinimumGrabberLookSpeedForJingle = 90.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Tambourine|Audio", meta = (ClampMin = "0.0"))
	float MinimumHeldRotationDegreesForJingle = 18.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Tambourine|Audio", meta = (ClampMin = "0.0"))
	float JingleVolumeMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Tambourine|Audio", meta = (ClampMin = "0.1"))
	float MinimumJinglePitch = 0.92f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Tambourine|Audio", meta = (ClampMin = "0.1"))
	float MaximumJinglePitch = 1.08f;

private:
	float AccumulatedHeldRotationDegrees = 0.0f;
	FRotator PreviousGrabberControlRotation = FRotator::ZeroRotator;
	bool bHasPreviousGrabberControlRotation = false;
};

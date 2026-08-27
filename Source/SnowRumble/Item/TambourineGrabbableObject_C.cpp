// Copyright Epic Games, Inc. All Rights Reserved.

#include "TambourineGrabbableObject_C.h"

#include "../Audio/SnowRumbleAudioHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"

ATambourineGrabbableObject::ATambourineGrabbableObject()
{
}

void ATambourineGrabbableObject::HandleGrabbedByCharacter(ACharacter* Grabber)
{
	Super::HandleGrabbedByCharacter(Grabber);

	AccumulatedHeldRotationDegrees = 0.0f;
	PreviousGrabberControlRotation = Grabber
		? Grabber->GetControlRotation()
		: FRotator::ZeroRotator;
	bHasPreviousGrabberControlRotation = Grabber != nullptr;
}

void ATambourineGrabbableObject::HandleReleasedByCharacter(ACharacter* Grabber)
{
	Super::HandleReleasedByCharacter(Grabber);

	AccumulatedHeldRotationDegrees = 0.0f;
	bHasPreviousGrabberControlRotation = false;
}

void ATambourineGrabbableObject::TickGrabbedByCharacter(
	ACharacter* Grabber,
	FVector HeldMotion,
	float DeltaTime)
{
	Super::TickGrabbedByCharacter(Grabber, HeldMotion, DeltaTime);

	if (!HasAuthority()
		|| !Grabber
		|| !JingleSound
		|| DeltaTime <= SMALL_NUMBER)
	{
		return;
	}

	const FRotator CurrentGrabberControlRotation = Grabber->GetControlRotation();
	if (!bHasPreviousGrabberControlRotation)
	{
		PreviousGrabberControlRotation = CurrentGrabberControlRotation;
		bHasPreviousGrabberControlRotation = true;
		return;
	}

	const FRotator RotationDelta = (CurrentGrabberControlRotation
		- PreviousGrabberControlRotation).GetNormalized();
	PreviousGrabberControlRotation = CurrentGrabberControlRotation;

	const float RotationDistanceDegrees =
		FMath::Abs(RotationDelta.Yaw)
		+ FMath::Abs(RotationDelta.Pitch)
		+ FMath::Abs(RotationDelta.Roll);
	const float LookSpeedDegreesPerSecond =
		RotationDistanceDegrees / DeltaTime;
	if (LookSpeedDegreesPerSecond < MinimumGrabberLookSpeedForJingle)
	{
		return;
	}

	AccumulatedHeldRotationDegrees +=
		RotationDistanceDegrees;
	if (AccumulatedHeldRotationDegrees < MinimumHeldRotationDegreesForJingle)
	{
		return;
	}

	AccumulatedHeldRotationDegrees = 0.0f;
	const float PitchMultiplier = FMath::FRandRange(
		FMath::Min(MinimumJinglePitch, MaximumJinglePitch),
		FMath::Max(MinimumJinglePitch, MaximumJinglePitch));
	MulticastPlayJingle(GetActorLocation(), PitchMultiplier);
}

void ATambourineGrabbableObject::MulticastPlayJingle_Implementation(
	FVector_NetQuantize Location,
	float PitchMultiplier)
{
	SnowRumbleAudio::PlaySoundAtLocation(
		this,
		JingleSound,
		ESnowRumbleAudioMixChannel::Gameplay,
		Location,
		JingleVolumeMultiplier,
		PitchMultiplier,
		JingleSoundAttenuation);
}

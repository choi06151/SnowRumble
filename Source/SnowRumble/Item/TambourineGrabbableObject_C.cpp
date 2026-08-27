// Copyright Epic Games, Inc. All Rights Reserved.

#include "TambourineGrabbableObject_C.h"

#include "../Audio/SnowRumbleAudioHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"

ATambourineGrabbableObject::ATambourineGrabbableObject()
{
}

void ATambourineGrabbableObject::HandleGrabbedByCharacter(ACharacter* Grabber)
{
	Super::HandleGrabbedByCharacter(Grabber);

	AccumulatedHeldTravelDistance = 0.0f;
	LastJingleTimeSeconds = GetWorld()
		? GetWorld()->GetTimeSeconds() - JingleCooldownSeconds
		: -1000.0f;
}

void ATambourineGrabbableObject::HandleReleasedByCharacter(ACharacter* Grabber)
{
	Super::HandleReleasedByCharacter(Grabber);

	AccumulatedHeldTravelDistance = 0.0f;
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

	const UCharacterMovementComponent* MovementComponent =
		Grabber->GetCharacterMovement();
	const float GrabberSpeed = MovementComponent
		? MovementComponent->Velocity.Size2D()
		: 0.0f;
	if (GrabberSpeed < MinimumGrabberSpeedForJingle)
	{
		AccumulatedHeldTravelDistance = 0.0f;
		return;
	}

	AccumulatedHeldTravelDistance += HeldMotion.Size2D();
	if (AccumulatedHeldTravelDistance < MinimumHeldTravelDistanceForJingle)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float CurrentTime = World->GetTimeSeconds();
	if (CurrentTime - LastJingleTimeSeconds < JingleCooldownSeconds)
	{
		return;
	}

	AccumulatedHeldTravelDistance = 0.0f;
	LastJingleTimeSeconds = CurrentTime;
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

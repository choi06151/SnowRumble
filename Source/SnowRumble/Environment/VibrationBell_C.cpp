// Copyright Epic Games, Inc. All Rights Reserved.

#include "VibrationBell_C.h"

#include "../Audio/SnowRumbleAudioHelpers.h"
#include "../Snowball/SnowballItem.h"
#include "Components/StaticMeshComponent.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"

AVibrationBell::AVibrationBell()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false);

	BellMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BellMeshComponent"));
	SetRootComponent(BellMeshComponent);
	BellMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BellMeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	BellMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
}

void AVibrationBell::BeginPlay()
{
	Super::BeginPlay();

	if (BellMeshComponent)
	{
		BellMeshBaseRelativeLocation = BellMeshComponent->GetRelativeLocation();
		BellMeshBaseRelativeRotation = BellMeshComponent->GetRelativeRotation();
	}
}

void AVibrationBell::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!BellMeshComponent || ShakeElapsedSeconds <= 0.0f)
	{
		return;
	}

	ShakeElapsedSeconds += FMath::Max(0.0f, DeltaSeconds);
	const float Duration = FMath::Max(0.0f, ShakeDurationSeconds);
	if (Duration <= 0.0f || ShakeElapsedSeconds >= Duration)
	{
		ShakeElapsedSeconds = 0.0f;
		BellMeshComponent->SetRelativeLocation(BellMeshBaseRelativeLocation);
		BellMeshComponent->SetRelativeRotation(BellMeshBaseRelativeRotation);
		return;
	}

	const float Frequency = FMath::Max(0.0f, ShakeFrequency);
	const float Angle = Frequency > 0.0f
		? FMath::Sin(ShakeElapsedSeconds * Frequency * UE_TWO_PI)
			* ShakeRotationAmplitudeDegrees
		: 0.0f;
	BellMeshComponent->SetRelativeRotation(
		BellMeshBaseRelativeRotation + FRotator(0.0f, Angle, 0.0f));
}

float AVibrationBell::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	const float AppliedDamage = Super::TakeDamage(
		DamageAmount,
		DamageEvent,
		EventInstigator,
		DamageCauser);

	if (HasAuthority()
		&& DamageAmount > 0.0f
		&& Cast<ASnowballItem>(DamageCauser))
	{
		MulticastPlayHitFeedback();
	}

	return AppliedDamage;
}

void AVibrationBell::StartShake()
{
	ShakeElapsedSeconds = KINDA_SMALL_NUMBER;
}

void AVibrationBell::MulticastPlayHitFeedback_Implementation()
{
	StartShake();
	SnowRumbleAudio::PlaySoundAtLocation(
		this,
		HitSound,
		ESnowRumbleAudioMixChannel::Gameplay,
		GetActorLocation(),
		1.0f,
		1.0f,
		HitSoundAttenuation);
}

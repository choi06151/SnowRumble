// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhotoInteractionActor_C.h"

#include "../Player/SnowRumbleCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"

APhotoInteractionActor::APhotoInteractionActor()
{
	bReplicates = true;

	PhotoMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PhotoMeshComponent"));
	SetRootComponent(PhotoMeshComponent);
	PhotoMeshComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	PhotoMeshComponent->SetGenerateOverlapEvents(false);

	PhotoCameraComponent =
		CreateDefaultSubobject<UCameraComponent>(TEXT("PhotoCameraComponent"));
	PhotoCameraComponent->SetupAttachment(PhotoMeshComponent);
	PhotoCameraComponent->bAutoActivate = true;
	bFindCameraComponentWhenViewTarget = true;
}

bool APhotoInteractionActor::CanInteractWith(
	const ASnowRumbleCharacter* Character) const
{
	if (!IsValid(Character) || !PhotoMeshComponent || InteractionRadius <= 0.0f)
	{
		return false;
	}

	const FBoxSphereBounds PhotoBounds = PhotoMeshComponent->Bounds;
	const FVector ClosestPoint = PhotoBounds.GetBox().GetClosestPointTo(
		Character->GetActorLocation());
	return FVector::DistSquared(
		Character->GetActorLocation(),
		ClosestPoint) <= FMath::Square(InteractionRadius);
}

void APhotoInteractionActor::Interact(ASnowRumbleCharacter* Character)
{
	if (!HasAuthority() || !CanInteractWith(Character))
	{
		return;
	}

	Character->NotifyItemInteractionSucceeded();
	Character->ClientFocusPhotoActor(this);
	OnPhotoInteraction(Character);
}

void APhotoInteractionActor::AddPhotoCameraRotation(
	float YawDelta,
	float PitchDelta)
{
	if (!PhotoCameraComponent)
	{
		return;
	}

	FRotator CameraRotation = PhotoCameraComponent->GetRelativeRotation();
	CameraRotation.Yaw = FRotator::NormalizeAxis(
		CameraRotation.Yaw + YawDelta);
	CameraRotation.Pitch = FMath::Clamp(
		CameraRotation.Pitch + PitchDelta,
		-CameraPitchLimit,
		CameraPitchLimit);
	PhotoCameraComponent->SetRelativeRotation(CameraRotation);
}

UCameraComponent* APhotoInteractionActor::GetPhotoCameraComponent() const
{
	return PhotoCameraComponent;
}

float APhotoInteractionActor::GetInteractionRadius() const
{
	return InteractionRadius;
}

void APhotoInteractionActor::BeginPlay()
{
	Super::BeginPlay();

	if (PhotoCameraComponent)
	{
		PhotoCameraComponent->SetActive(false);
	}
}

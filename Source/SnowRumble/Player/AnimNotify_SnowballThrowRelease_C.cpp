// Copyright Epic Games, Inc. All Rights Reserved.

#include "AnimNotify_SnowballThrowRelease_C.h"

#include "SnowRumbleCharacter.h"
#include "Components/SkeletalMeshComponent.h"

UAnimNotify_SnowballThrowRelease::UAnimNotify_SnowballThrowRelease()
{
}

void UAnimNotify_SnowballThrowRelease::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	ASnowRumbleCharacter* Character =
		Cast<ASnowRumbleCharacter>(MeshComp->GetOwner());
	if (!Character)
	{
		return;
	}

	Character->RequestSnowballThrowReleaseFromNotify();
}

FString UAnimNotify_SnowballThrowRelease::GetNotifyName_Implementation() const
{
	return TEXT("Snowball Throw Release");
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "AnimNotify_SnowFootstep_C.h"

#include "SnowRumbleCharacter.h"
#include "Components/SkeletalMeshComponent.h"

UAnimNotify_SnowFootstep::UAnimNotify_SnowFootstep()
{
}

void UAnimNotify_SnowFootstep::Notify(
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

	Character->RequestSnowFootstepEffect(FootSocketName);
}

FString UAnimNotify_SnowFootstep::GetNotifyName_Implementation() const
{
	return FootSocketName.IsNone()
		? TEXT("Snow Footstep")
		: FString::Printf(TEXT("Snow Footstep %s"), *FootSocketName.ToString());
}

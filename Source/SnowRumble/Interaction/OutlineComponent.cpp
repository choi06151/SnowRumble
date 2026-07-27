// Copyright Epic Games, Inc. All Rights Reserved.

#include "OutlineComponent.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"

UOutlineComponent::UOutlineComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

void UOutlineComponent::SetOutlinedActor(AActor* NewOutlinedActor)
{
	if (OutlinedActor == NewOutlinedActor)
	{
		return;
	}

	ApplyOutline(OutlinedActor, false);
	OutlinedActor = NewOutlinedActor;
	ApplyOutline(OutlinedActor, true);
}

AActor* UOutlineComponent::GetOutlinedActor() const
{
	return OutlinedActor;
}

void UOutlineComponent::ClearOutline()
{
	SetOutlinedActor(nullptr);
}

void UOutlineComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearOutline();
	Super::EndPlay(EndPlayReason);
}

void UOutlineComponent::ApplyOutline(AActor* TargetActor, bool bEnabled) const
{
	if (!IsValid(TargetActor))
	{
		return;
	}

	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(TargetActor);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent)
		{
			continue;
		}

		if (bEnabled)
		{
			PrimitiveComponent->SetCustomDepthStencilValue(CustomDepthStencilValue);
		}
		PrimitiveComponent->SetRenderCustomDepth(bEnabled);
	}
}

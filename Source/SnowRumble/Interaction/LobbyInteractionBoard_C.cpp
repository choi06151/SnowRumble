// Copyright Epic Games, Inc. All Rights Reserved.

#include "LobbyInteractionBoard_C.h"

#include "../Player/SnowRumbleCharacter.h"
#include "Components/StaticMeshComponent.h"

ALobbyInteractionBoard::ALobbyInteractionBoard()
{
	bReplicates = true;

	BoardMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoardMeshComponent"));
	SetRootComponent(BoardMeshComponent);
	BoardMeshComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	BoardMeshComponent->SetGenerateOverlapEvents(false);
}

bool ALobbyInteractionBoard::CanInteractWith(
	const ASnowRumbleCharacter* Character) const
{
	if (!IsValid(Character) || !BoardMeshComponent || InteractionRadius <= 0.0f)
	{
		return false;
	}

	const FBoxSphereBounds BoardBounds = BoardMeshComponent->Bounds;
	const FBox BoardBox = BoardBounds.GetBox();
	const FVector CharacterLocation = Character->GetActorLocation();
	const FVector ClosestBoardPoint = BoardBox.GetClosestPointTo(CharacterLocation);

	return FVector::DistSquared(CharacterLocation, ClosestBoardPoint)
		<= FMath::Square(InteractionRadius);
}

void ALobbyInteractionBoard::Interact(ASnowRumbleCharacter* Character)
{
	if (!HasAuthority() || !CanInteractWith(Character))
	{
		return;
	}

	Character->ClientFocusLobbyBoard(this);
	OnBoardInteracted(Character);
}

float ALobbyInteractionBoard::GetInteractionRadius() const
{
	return InteractionRadius;
}

FVector ALobbyInteractionBoard::GetFocusLocation() const
{
	return BoardMeshComponent
		? BoardMeshComponent->Bounds.Origin
		: GetActorLocation();
}

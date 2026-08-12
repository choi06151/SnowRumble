// Copyright Epic Games, Inc. All Rights Reserved.

#include "LobbyInteractionBoard_C.h"

#include "../Player/SnowRumbleCharacter.h"
#include "../UI/LobbyBoardWidget_C.h"
#include "../UI/LobbyPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"

ALobbyInteractionBoard::ALobbyInteractionBoard()
{
	bReplicates = true;

	BoardMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoardMeshComponent"));
	SetRootComponent(BoardMeshComponent);
	BoardMeshComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	BoardMeshComponent->SetGenerateOverlapEvents(false);

	FocusCameraComponent =
		CreateDefaultSubobject<UCameraComponent>(TEXT("FocusCameraComponent"));
	FocusCameraComponent->SetupAttachment(BoardMeshComponent);
	FocusCameraComponent->bAutoActivate = true;
	FocusCameraComponent->SetRelativeLocation(FVector(-260.0f, 0.0f, 120.0f));
	FocusCameraComponent->SetRelativeRotation(FRotator(-5.0f, 0.0f, 0.0f));

	BoardWidgetComponent =
		CreateDefaultSubobject<UWidgetComponent>(TEXT("BoardWidgetComponent"));
	BoardWidgetComponent->SetupAttachment(BoardMeshComponent);
	BoardWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	BoardWidgetComponent->SetDrawSize(FVector2D(900.0f, 600.0f));
	BoardWidgetComponent->SetTwoSided(true);
	BoardWidgetComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoardWidgetComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoardWidgetComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	BoardWidgetComponent->SetGenerateOverlapEvents(false);
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

void ALobbyInteractionBoard::HandleBoardAction(
	ASnowRumbleCharacter* Character,
	ELobbyBoardAction BoardAction)
{
	if (!HasAuthority() || !CanInteractWith(Character))
	{
		return;
	}

	OnBoardActionRequested(Character, BoardAction);
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

UWidgetComponent* ALobbyInteractionBoard::GetBoardWidgetComponent() const
{
	return BoardWidgetComponent;
}

void ALobbyInteractionBoard::GetBoardWidgetComponents(
	TArray<UWidgetComponent*>& OutWidgetComponents) const
{
	OutWidgetComponents.Reset();

	TInlineComponentArray<UWidgetComponent*> WidgetComponents(this);
	for (UWidgetComponent* WidgetComponent : WidgetComponents)
	{
		if (!WidgetComponent)
		{
			continue;
		}

		const ULobbyBoardWidget* BoardWidget =
			Cast<ULobbyBoardWidget>(WidgetComponent->GetUserWidgetObject());
		if (BoardWidget)
		{
			OutWidgetComponents.Add(WidgetComponent);
		}
	}
}

void ALobbyInteractionBoard::SetFocusedCharacter(ASnowRumbleCharacter* Character)
{
	InitializeBoardWidget();

	ALobbyPlayerController* LobbyPlayerController = Character
		? Cast<ALobbyPlayerController>(Character->GetController())
		: nullptr;
	if (LobbyPlayerController && !LobbyPlayerController->IsLocalController())
	{
		LobbyPlayerController = nullptr;
	}

	TArray<UWidgetComponent*> WidgetComponents;
	GetBoardWidgetComponents(WidgetComponents);
	for (UWidgetComponent* WidgetComponent : WidgetComponents)
	{
		if (WidgetComponent && LobbyPlayerController)
		{
			WidgetComponent->SetOwnerPlayer(
				LobbyPlayerController->GetLocalPlayer());
		}

		ULobbyBoardWidget* BoardWidget =
			Cast<ULobbyBoardWidget>(WidgetComponent->GetUserWidgetObject());
		if (BoardWidget)
		{
			BoardWidget->SetFocusedCharacter(Character);
			BoardWidget->SetFocusedPlayerController(LobbyPlayerController);
		}
	}
}

void ALobbyInteractionBoard::BeginPlay()
{
	Super::BeginPlay();

	InitializeBoardWidget();
}

void ALobbyInteractionBoard::InitializeBoardWidget()
{
	TInlineComponentArray<UWidgetComponent*> WidgetComponents(this);
	for (UWidgetComponent* WidgetComponent : WidgetComponents)
	{
		ULobbyBoardWidget* BoardWidget = WidgetComponent
			? Cast<ULobbyBoardWidget>(WidgetComponent->GetUserWidgetObject())
			: nullptr;
		if (BoardWidget)
		{
			BoardWidget->SetOwningBoard(this);
		}
	}
}

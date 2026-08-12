// Copyright Epic Games, Inc. All Rights Reserved.

#include "GiftBoxItemPickup_C.h"

#include "../Game/SnowRumbleGameState_C.h"
#include "../Game/SnowRumblePlayerState.h"
#include "../Player/SnowRumbleCharacter.h"
#include "../UI/SnowRumblePlayerController.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

AGiftBoxItemPickup::AGiftBoxItemPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComponent =
		CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->InitSphereRadius(35.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	CollisionComponent->SetGenerateOverlapEvents(false);

	ItemMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	ItemMeshComponent->SetupAttachment(CollisionComponent);
	ItemMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGiftBoxItemPickup::BeginPlay()
{
	Super::BeginPlay();

	if (ItemMeshComponent)
	{
		InitialMeshRelativeLocation = ItemMeshComponent->GetRelativeLocation();
	}

	OnRep_ItemData();
}

void AGiftBoxItemPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!ItemMeshComponent || bPickedUp)
	{
		return;
	}

	FloatTimeSeconds += DeltaSeconds;
	const float FloatOffset =
		FMath::Sin(FloatTimeSeconds * FloatSpeed * UE_TWO_PI)
		* FloatAmplitude;
	ItemMeshComponent->SetRelativeLocation(
		InitialMeshRelativeLocation + FVector::UpVector * FloatOffset);
}

void AGiftBoxItemPickup::InitializePickupFromServer(
	ESnowRumbleGiftItemType NewItemType,
	FName NewItemId,
	const FText& NewDisplayName)
{
	if (!HasAuthority())
	{
		return;
	}

	ItemType = NewItemType;
	ItemId = NewItemId;
	DisplayName = NewDisplayName;
	OnRep_ItemData();
	ForceNetUpdate();
}

bool AGiftBoxItemPickup::CanInteractWith(
	const ASnowRumbleCharacter* Character) const
{
	if (!IsValid(Character)
		|| bPickedUp
		|| InteractionRadius <= 0.0f)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	const ASnowRumbleGameState* SnowRumbleGameState = World
		? World->GetGameState<ASnowRumbleGameState>()
		: nullptr;
	if (SnowRumbleGameState
		&& (SnowRumbleGameState->IsRoundEnded()
			|| SnowRumbleGameState->IsMatchInputLocked()))
	{
		return false;
	}

	return FVector::DistSquared(
		Character->GetActorLocation(),
		GetActorLocation()) <= FMath::Square(InteractionRadius);
}

bool AGiftBoxItemPickup::TryPickup(ASnowRumbleCharacter* Character)
{
	if (!HasAuthority() || !CanInteractWith(Character))
	{
		return false;
	}

	bPickedUp = true;
	if (CollisionComponent)
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	Character->NotifyItemPickupSucceeded();
	NotifyPickedUp(Character);
	OnItemPickedUp(Character);
	SetLifeSpan(PickedUpDestroyDelaySeconds);
	ForceNetUpdate();
	return true;
}

float AGiftBoxItemPickup::GetInteractionRadius() const
{
	return InteractionRadius;
}

FText AGiftBoxItemPickup::GetDisplayName() const
{
	return DisplayName;
}

ESnowRumbleGiftItemType AGiftBoxItemPickup::GetItemType() const
{
	return ItemType;
}

void AGiftBoxItemPickup::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGiftBoxItemPickup, ItemType);
	DOREPLIFETIME(AGiftBoxItemPickup, ItemId);
	DOREPLIFETIME(AGiftBoxItemPickup, DisplayName);
	DOREPLIFETIME(AGiftBoxItemPickup, bPickedUp);
}

void AGiftBoxItemPickup::OnRep_ItemData()
{
	OnItemDataChanged(ItemType, ItemId, DisplayName);
}

void AGiftBoxItemPickup::NotifyPickedUp(
	ASnowRumbleCharacter* Character) const
{
	UWorld* World = GetWorld();
	if (!World || DisplayName.IsEmpty())
	{
		return;
	}

	const FString PlayerName = GetCharacterDisplayName(Character);
	const FText EventLogMessage = FText::Format(
		NSLOCTEXT(
			"SnowRumble",
			"EventLogGiftItemPickedUp",
			"{0}님이 {1}을 획득했습니다"),
		FText::FromString(PlayerName),
		DisplayName);
	const FText PersonalMessage = FText::Format(
		NSLOCTEXT(
			"SnowRumble",
			"PersonalGiftItemPickedUp",
			"획득: {0}"),
		DisplayName);

	for (FConstPlayerControllerIterator It =
			World->GetPlayerControllerIterator();
		It;
		++It)
	{
		ASnowRumblePlayerController* PlayerController =
			Cast<ASnowRumblePlayerController>(It->Get());
		if (!PlayerController)
		{
			continue;
		}

		PlayerController->ClientReceiveEventLogMessage(EventLogMessage);
		if (Character && PlayerController == Character->GetController())
		{
			PlayerController->ClientShowPersonalTextAlarm(PersonalMessage);
		}
	}
}

FString AGiftBoxItemPickup::GetCharacterDisplayName(
	const ASnowRumbleCharacter* Character) const
{
	const ASnowRumblePlayerState* SnowRumblePlayerState = Character
		? Character->GetPlayerState<ASnowRumblePlayerState>()
		: nullptr;
	if (SnowRumblePlayerState
		&& !SnowRumblePlayerState->GetLobbyPlayerName().IsEmpty())
	{
		return SnowRumblePlayerState->GetLobbyPlayerName();
	}

	return TEXT("Player");
}

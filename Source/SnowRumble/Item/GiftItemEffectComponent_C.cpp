// Copyright Epic Games, Inc. All Rights Reserved.

#include "GiftItemEffectComponent_C.h"

#include "Campfire_C.h"
#include "../Game/SnowRumbleGameMode.h"
#include "../Game/SnowRumblePlayerState.h"
#include "../Player/SnowRumbleCharacter.h"
#include "../Player/SnowRumbleHealthComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

UGiftItemEffectComponent::UGiftItemEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

bool UGiftItemEffectComponent::ApplyGiftItemFromServer(
	ESnowRumbleGiftItemType ItemType)
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority())
	{
		return false;
	}

	bool bApplied = true;
	switch (ItemType)
	{
	case ESnowRumbleGiftItemType::HotPack:
		bApplied = EquipHotPack();
		break;

	case ESnowRumbleGiftItemType::GoldenHotPack:
		bApplied = UseGoldenHotPackImmediately();
		break;

	case ESnowRumbleGiftItemType::HotChocolate:
	case ESnowRumbleGiftItemType::FishBread:
		bApplied = ApplyInstantHeal(InstantHealAmount);
		break;

	case ESnowRumbleGiftItemType::EnergyDrink:
		StartInvulnerability(EnergyDrinkInvulnerabilitySeconds);
		break;

	case ESnowRumbleGiftItemType::Boots:
		bHasBoots = true;
		break;

	case ESnowRumbleGiftItemType::Padding:
		bHasPadding = true;
		break;

	case ESnowRumbleGiftItemType::Gloves:
		bHasGloves = true;
		break;

	case ESnowRumbleGiftItemType::SnowShovel:
		SnowShovelDurability = FMath::RandRange(
			FMath::Min(RustyShovelMinDurability, RustyShovelMaxDurability),
			FMath::Max(RustyShovelMinDurability, RustyShovelMaxDurability));
		EquippedShovelType = ESnowRumbleGiftItemType::SnowShovel;
		break;

	case ESnowRumbleGiftItemType::GoldenShovel:
		SnowShovelDurability = FMath::Max(0, GoldenShovelDurability);
		EquippedShovelType = ESnowRumbleGiftItemType::GoldenShovel;
		break;

	case ESnowRumbleGiftItemType::SnowDuckMaker:
		bHasSnowDuckMaker = true;
		bHasGoldenSnowDuckMaker = false;
		break;

	case ESnowRumbleGiftItemType::GoldenDuckMaker:
		bHasSnowDuckMaker = false;
		bHasGoldenSnowDuckMaker = true;
		break;

	case ESnowRumbleGiftItemType::GoldenFishBread:
		StartGoldenFishBreadRegeneration();
		break;

	case ESnowRumbleGiftItemType::CampfireKit:
		bApplied = SpawnCampfireFromKit();
		break;

	default:
		bApplied = false;
		break;
	}

	if (bApplied)
	{
		OnRep_ItemEffects();
		OwningActor->ForceNetUpdate();
	}
	return bApplied;
}

bool UGiftItemEffectComponent::IsInvulnerable() const
{
	return bInvulnerable;
}

bool UGiftItemEffectComponent::HasHotPack() const
{
	return bHasHotPack;
}

bool UGiftItemEffectComponent::HasBoots() const
{
	return bHasBoots;
}

bool UGiftItemEffectComponent::HasPadding() const
{
	return bHasPadding;
}

bool UGiftItemEffectComponent::HasGloves() const
{
	return bHasGloves;
}

int32 UGiftItemEffectComponent::GetSnowShovelDurability() const
{
	return SnowShovelDurability;
}

ESnowRumbleGiftItemType UGiftItemEffectComponent::GetEquippedShovelItemType()
	const
{
	return SnowShovelDurability > 0
		? EquippedShovelType
		: ESnowRumbleGiftItemType::None;
}

ESnowRumbleGiftItemType UGiftItemEffectComponent::GetEquippedDuckMakerItemType()
	const
{
	if (bHasGoldenSnowDuckMaker)
	{
		return ESnowRumbleGiftItemType::GoldenDuckMaker;
	}

	return bHasSnowDuckMaker
		? ESnowRumbleGiftItemType::SnowDuckMaker
		: ESnowRumbleGiftItemType::None;
}

float UGiftItemEffectComponent::GetMovementSpeedMultiplier() const
{
	return bHasBoots ? BootsMovementSpeedMultiplier : 1.0f;
}

float UGiftItemEffectComponent::GetIncomingDamageMultiplier() const
{
	return bHasPadding ? PaddingIncomingDamageMultiplier : 1.0f;
}

float UGiftItemEffectComponent::GetSnowballCreationDurationMultiplier() const
{
	return bHasGloves ? GlovesCreationDurationMultiplier : 1.0f;
}

float UGiftItemEffectComponent::GetSnowballDamageMultiplier() const
{
	if (bHasGoldenSnowDuckMaker)
	{
		return GoldenSnowDuckMakerDamageMultiplier;
	}

	return bHasSnowDuckMaker ? SnowDuckMakerDamageMultiplier : 1.0f;
}

void UGiftItemEffectComponent::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGiftItemEffectComponent, bHasBoots);
	DOREPLIFETIME(UGiftItemEffectComponent, bHasPadding);
	DOREPLIFETIME(UGiftItemEffectComponent, bHasGloves);
	DOREPLIFETIME(UGiftItemEffectComponent, bHasSnowDuckMaker);
	DOREPLIFETIME(UGiftItemEffectComponent, bHasGoldenSnowDuckMaker);
	DOREPLIFETIME(UGiftItemEffectComponent, bInvulnerable);
	DOREPLIFETIME(UGiftItemEffectComponent, bHasHotPack);
	DOREPLIFETIME(UGiftItemEffectComponent, SnowShovelDurability);
	DOREPLIFETIME(UGiftItemEffectComponent, EquippedShovelType);
}

void UGiftItemEffectComponent::OnRep_ItemEffects()
{
	OnGiftItemEffectsChanged.Broadcast();
}

bool UGiftItemEffectComponent::ApplyInstantHeal(float HealAmount)
{
	AActor* OwningActor = GetOwner();
	USnowRumbleHealthComponent* HealthComponent = OwningActor
		? OwningActor->FindComponentByClass<USnowRumbleHealthComponent>()
		: nullptr;
	if (!HealthComponent)
	{
		return false;
	}

	HealthComponent->ApplyHealing(FMath::Max(0.0f, HealAmount));
	return true;
}

bool UGiftItemEffectComponent::EquipHotPack()
{
	if (bHasHotPack)
	{
		return false;
	}

	bHasHotPack = true;
	return true;
}

bool UGiftItemEffectComponent::UseGoldenHotPackImmediately()
{
	const ASnowRumbleCharacter* OwnerCharacter =
		Cast<ASnowRumbleCharacter>(GetOwner());
	const ASnowRumblePlayerState* OwnerPlayerState = OwnerCharacter
		? OwnerCharacter->GetPlayerState<ASnowRumblePlayerState>()
		: nullptr;
	if (!OwnerCharacter || !OwnerPlayerState)
	{
		return false;
	}

	const ESnowRumbleTeam OwnerTeam = OwnerPlayerState->GetLobbyTeam();
	if (OwnerTeam == ESnowRumbleTeam::None)
	{
		return false;
	}

	UWorld* World = GetWorld();
	AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	if (!GameState)
	{
		return false;
	}

	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		const ASnowRumblePlayerState* SnowRumblePlayerState =
			Cast<ASnowRumblePlayerState>(PlayerState);
		if (!SnowRumblePlayerState
			|| SnowRumblePlayerState->GetLobbyTeam() != OwnerTeam)
		{
			continue;
		}

		APawn* Pawn = SnowRumblePlayerState->GetPawn();
		ASnowRumbleCharacter* Teammate = Cast<ASnowRumbleCharacter>(Pawn);
		USnowRumbleHealthComponent* HealthComponent = Teammate
			? Teammate->FindComponentByClass<USnowRumbleHealthComponent>()
			: nullptr;
		if (HealthComponent)
		{
			HealthComponent->ReviveFromFrozen(0.5f);
		}
	}

	return true;
}

bool UGiftItemEffectComponent::SpawnCampfireFromKit()
{
	ASnowRumbleCharacter* OwnerCharacter =
		Cast<ASnowRumbleCharacter>(GetOwner());
	UWorld* World = GetWorld();
	if (!OwnerCharacter || !World)
	{
		return false;
	}

	const FVector DesiredLocation =
		OwnerCharacter->GetActorLocation()
		+ OwnerCharacter->GetActorForwardVector().GetSafeNormal2D()
			* CampfireSpawnForwardDistance;
	FVector SpawnLocation = DesiredLocation;

	FHitResult GroundHit;
	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(CampfireKitGroundTrace),
		false,
		OwnerCharacter);
	const FVector TraceStart =
		DesiredLocation + FVector::UpVector * CampfireSpawnGroundTraceDistance;
	const FVector TraceEnd =
		DesiredLocation - FVector::UpVector * CampfireSpawnGroundTraceDistance;
	if (World->LineTraceSingleByChannel(
		GroundHit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams))
	{
		SpawnLocation = GroundHit.ImpactPoint;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwnerCharacter;
	SpawnParameters.Instigator = OwnerCharacter;
	SpawnParameters.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	TSubclassOf<ACampfire> ClassToSpawn = CampfireClass;
	if (!ClassToSpawn)
	{
		ClassToSpawn = ACampfire::StaticClass();
	}
	ACampfire* Campfire = World->SpawnActor<ACampfire>(
		ClassToSpawn,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParameters);
	if (!Campfire)
	{
		return false;
	}

	Campfire->InitializeCampfireFromServer(OwnerCharacter);
	return true;
}

void UGiftItemEffectComponent::StartInvulnerability(float DurationSeconds)
{
	bInvulnerable = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InvulnerabilityTimerHandle);
		if (DurationSeconds > 0.0f)
		{
			World->GetTimerManager().SetTimer(
				InvulnerabilityTimerHandle,
				this,
				&UGiftItemEffectComponent::FinishInvulnerability,
				DurationSeconds,
				false);
		}
	}
}

void UGiftItemEffectComponent::FinishInvulnerability()
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority())
	{
		return;
	}

	bInvulnerable = false;
	OnRep_ItemEffects();
	OwningActor->ForceNetUpdate();
}

void UGiftItemEffectComponent::StartGoldenFishBreadRegeneration()
{
	UWorld* World = GetWorld();
	if (!World || GoldenFishBreadHealPerSecond <= 0.0f)
	{
		return;
	}

	GoldenFishBreadTicksRemaining =
		FMath::Max(0, FMath::RoundToInt(GoldenFishBreadDurationSeconds));
	if (GoldenFishBreadTicksRemaining <= 0)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(GoldenFishBreadRegenerationTimerHandle);
	World->GetTimerManager().SetTimer(
		GoldenFishBreadRegenerationTimerHandle,
		this,
		&UGiftItemEffectComponent::TickGoldenFishBreadRegeneration,
		1.0f,
		true);
	TickGoldenFishBreadRegeneration();
}

void UGiftItemEffectComponent::TickGoldenFishBreadRegeneration()
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority())
	{
		return;
	}

	if (GoldenFishBreadTicksRemaining <= 0)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(
				GoldenFishBreadRegenerationTimerHandle);
		}
		return;
	}

	ApplyInstantHeal(GoldenFishBreadHealPerSecond);
	--GoldenFishBreadTicksRemaining;
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleHealthComponent.h"

#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

USnowRumbleHealthComponent::USnowRumbleHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void USnowRumbleHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		CurrentHealth = MaxHealth;
		bIsFrozen = false;
	}
}

void USnowRumbleHealthComponent::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USnowRumbleHealthComponent, MaxHealth);
	DOREPLIFETIME(USnowRumbleHealthComponent, CurrentHealth);
	DOREPLIFETIME(USnowRumbleHealthComponent, bIsFrozen);
}

float USnowRumbleHealthComponent::ApplyDamage(float DamageAmount)
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority() || DamageAmount <= 0.0f || bIsFrozen)
	{
		return 0.0f;
	}

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	OnRep_CurrentHealth();

	if (CurrentHealth <= 0.0f && !bIsFrozen)
	{
		bIsFrozen = true;
		OnRep_IsFrozen();
	}

	return PreviousHealth - CurrentHealth;
}

float USnowRumbleHealthComponent::GetCurrentHealth() const
{
	return CurrentHealth;
}

float USnowRumbleHealthComponent::GetMaxHealth() const
{
	return MaxHealth;
}

bool USnowRumbleHealthComponent::IsFrozen() const
{
	return bIsFrozen;
}

void USnowRumbleHealthComponent::OnRep_CurrentHealth()
{
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void USnowRumbleHealthComponent::OnRep_IsFrozen()
{
	OnFrozenChanged.Broadcast(bIsFrozen);
}

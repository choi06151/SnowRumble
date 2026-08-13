// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleHealthComponent.h"

#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "../Game/SnowRumbleGameMode.h"
#include "TimerManager.h"

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
		bIsDead = false;
		FrozenDeathServerTime = 0.0f;
	}
}

void USnowRumbleHealthComponent::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USnowRumbleHealthComponent, MaxHealth);
	DOREPLIFETIME(USnowRumbleHealthComponent, CurrentHealth);
	DOREPLIFETIME(USnowRumbleHealthComponent, bIsFrozen);
	DOREPLIFETIME(USnowRumbleHealthComponent, bIsDead);
	DOREPLIFETIME(USnowRumbleHealthComponent, FrozenDeathServerTime);
}

float USnowRumbleHealthComponent::ApplyDamage(float DamageAmount)
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor
		|| !OwningActor->HasAuthority()
		|| DamageAmount <= 0.0f
		|| bIsFrozen
		|| bIsDead)
	{
		return 0.0f;
	}

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	OnRep_CurrentHealth();

	if (CurrentHealth <= 0.0f && !bIsFrozen)
	{
		StartFrozenState();
	}

	return PreviousHealth - CurrentHealth;
}

float USnowRumbleHealthComponent::ApplyHealing(float HealAmount)
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor
		|| !OwningActor->HasAuthority()
		|| HealAmount <= 0.0f
		|| bIsFrozen
		|| bIsDead)
	{
		return 0.0f;
	}

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth);
	if (!FMath::IsNearlyEqual(PreviousHealth, CurrentHealth))
	{
		OnRep_CurrentHealth();
	}
	return CurrentHealth - PreviousHealth;
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

bool USnowRumbleHealthComponent::IsDead() const
{
	return bIsDead;
}

float USnowRumbleHealthComponent::GetFrozenSecondsRemaining() const
{
	if (!bIsFrozen || bIsDead || FrozenDeathServerTime <= 0.0f)
	{
		return 0.0f;
	}

	const UWorld* World = GetWorld();
	const AGameStateBase* GameState = World
		? World->GetGameState()
		: nullptr;
	const float CurrentServerTime = GameState
		? GameState->GetServerWorldTimeSeconds()
		: World
			? World->GetTimeSeconds()
			: 0.0f;
	return FMath::Max(0.0f, FrozenDeathServerTime - CurrentServerTime);
}

bool USnowRumbleHealthComponent::ReviveFromFrozen(float HealthRatio)
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority() || !bIsFrozen || bIsDead)
	{
		return false;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FrozenDeathTimerHandle);
	}

	bIsFrozen = false;
	FrozenDeathServerTime = 0.0f;
	CurrentHealth = FMath::Clamp(MaxHealth * HealthRatio, 1.0f, MaxHealth);
	OnRep_CurrentHealth();
	OnRep_IsFrozen();
	if (ASnowRumbleGameMode* GameMode =
		GetWorld() ? GetWorld()->GetAuthGameMode<ASnowRumbleGameMode>() : nullptr)
	{
		GameMode->EvaluateRoundEndCondition();
	}
	return true;
}

void USnowRumbleHealthComponent::OnRep_CurrentHealth()
{
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void USnowRumbleHealthComponent::OnRep_IsFrozen()
{
	OnFrozenChanged.Broadcast(bIsFrozen);
}

void USnowRumbleHealthComponent::OnRep_IsDead()
{
	OnDeathChanged.Broadcast(bIsDead);
}

void USnowRumbleHealthComponent::StartFrozenState()
{
	bIsFrozen = true;
	FrozenDeathServerTime = 0.0f;

	UWorld* World = GetWorld();
	if (World && FrozenDeathDelaySeconds > 0.0f)
	{
		const AGameStateBase* GameState = World->GetGameState();
		const float CurrentServerTime = GameState
			? GameState->GetServerWorldTimeSeconds()
			: World->GetTimeSeconds();
		FrozenDeathServerTime =
			CurrentServerTime + FrozenDeathDelaySeconds;
		World->GetTimerManager().SetTimer(
			FrozenDeathTimerHandle,
			this,
			&USnowRumbleHealthComponent::FinishFrozenDeath,
			FrozenDeathDelaySeconds,
			false);
	}
	else
	{
		FinishFrozenDeath();
	}

	OnRep_IsFrozen();
	if (ASnowRumbleGameMode* GameMode =
		GetWorld() ? GetWorld()->GetAuthGameMode<ASnowRumbleGameMode>() : nullptr)
	{
		GameMode->EvaluateRoundEndCondition();
	}
}

void USnowRumbleHealthComponent::FinishFrozenDeath()
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority() || !bIsFrozen || bIsDead)
	{
		return;
	}

	bIsFrozen = false;
	bIsDead = true;
	FrozenDeathServerTime = 0.0f;
	OnRep_IsFrozen();
	OnRep_IsDead();
	if (ASnowRumbleGameMode* GameMode =
		GetWorld() ? GetWorld()->GetAuthGameMode<ASnowRumbleGameMode>() : nullptr)
	{
		GameMode->EvaluateRoundEndCondition();
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SnowRumbleHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnSnowRumbleHealthChanged,
	float,
	CurrentHealth,
	float,
	MaxHealth);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnSnowRumbleFrozenChanged,
	bool,
	bIsFrozen);

UCLASS(ClassGroup = (SnowRumble), meta = (BlueprintSpawnableComponent))
class SNOWRUMBLE_API USnowRumbleHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USnowRumbleHealthComponent();

	/** 서버에서 피해를 적용하고 HP와 얼기 상태를 확정한다. */
	float ApplyDamage(float DamageAmount);

	/** 현재 HP를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Health")
	float GetCurrentHealth() const;

	/** 최대 HP를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Health")
	float GetMaxHealth() const;

	/** 현재 얼기 상태를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Health")
	bool IsFrozen() const;

	UPROPERTY(BlueprintAssignable, Category = "SnowRumble|Health")
	FOnSnowRumbleHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "SnowRumble|Health")
	FOnSnowRumbleFrozenChanged OnFrozenChanged;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 복제된 HP 변경을 로컬 표현과 UI 연결 지점에 알린다. */
	UFUNCTION()
	void OnRep_CurrentHealth();

	/** 복제된 얼기 변경을 로컬 표현과 행동 제한에 알린다. */
	UFUNCTION()
	void OnRep_IsFrozen();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "SnowRumble|Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentHealth, Category = "SnowRumble|Health")
	float CurrentHealth = 100.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_IsFrozen, Category = "SnowRumble|Health")
	bool bIsFrozen = false;
};

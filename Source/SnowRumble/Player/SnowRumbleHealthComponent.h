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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnSnowRumbleDeathChanged,
	bool,
	bIsDead);

UCLASS(ClassGroup = (SnowRumble), meta = (BlueprintSpawnableComponent))
class SNOWRUMBLE_API USnowRumbleHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USnowRumbleHealthComponent();

	/** 서버에서 피해를 적용하고 HP와 얼기 상태를 확정한다. */
	float ApplyDamage(float DamageAmount);

	/** 서버에서 회복을 적용하고 HP 변경량을 반환한다. */
	float ApplyHealing(float HealAmount);

	/** 현재 HP를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Health")
	float GetCurrentHealth() const;

	/** 최대 HP를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Health")
	float GetMaxHealth() const;

	/** 현재 얼기 상태를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Health")
	bool IsFrozen() const;

	/** 현재 라운드 사망 상태를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Health")
	bool IsDead() const;

	/** 얼음 상태에서 사망까지 남은 시간을 초 단위로 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Health")
	float GetFrozenSecondsRemaining() const;

	/** 서버가 핫팩 부활 완료 시 얼음 상태를 해제하고 HP를 회복한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Health")
	bool ReviveFromFrozen(float HealthRatio);

	UPROPERTY(BlueprintAssignable, Category = "SnowRumble|Health")
	FOnSnowRumbleHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "SnowRumble|Health")
	FOnSnowRumbleFrozenChanged OnFrozenChanged;

	UPROPERTY(BlueprintAssignable, Category = "SnowRumble|Health")
	FOnSnowRumbleDeathChanged OnDeathChanged;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 복제된 HP 변경을 로컬 표현과 UI 연결 지점에 알린다. */
	UFUNCTION()
	void OnRep_CurrentHealth();

	/** 복제된 얼기 변경을 로컬 표현과 행동 제한에 알린다. */
	UFUNCTION()
	void OnRep_IsFrozen();

	/** 복제된 사망 변경을 로컬 표현과 행동 제한에 알린다. */
	UFUNCTION()
	void OnRep_IsDead();

	/** 서버가 HP 0 상태를 60초 부활 가능 얼음 상태로 전환한다. */
	void StartFrozenState();

	/** 서버가 얼음 제한시간 만료 후 라운드 사망 상태를 확정한다. */
	void FinishFrozenDeath();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "SnowRumble|Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Health", meta = (ClampMin = "0.0"))
	float FrozenDeathDelaySeconds = 60.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentHealth, Category = "SnowRumble|Health")
	float CurrentHealth = 100.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_IsFrozen, Category = "SnowRumble|Health")
	bool bIsFrozen = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_IsDead, Category = "SnowRumble|Health")
	bool bIsDead = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "SnowRumble|Health")
	float FrozenDeathServerTime = 0.0f;

	FTimerHandle FrozenDeathTimerHandle;
};

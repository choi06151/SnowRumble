// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GiftItemTypes_C.h"
#include "GiftItemEffectComponent_C.generated.h"

class USnowRumbleHealthComponent;
class ACampfire;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGiftItemEffectsChanged);

UCLASS(ClassGroup = (SnowRumble), meta = (BlueprintSpawnableComponent))
class SNOWRUMBLE_API UGiftItemEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGiftItemEffectComponent();

	/** 서버가 선물상자 아이템 획득 결과를 실제 플레이어 효과로 적용한다. */
	bool ApplyGiftItemFromServer(ESnowRumbleGiftItemType ItemType);

	/** 현재 피해를 무시하는 무적 효과가 켜져 있는지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Item|Effect")
	bool IsInvulnerable() const;

	/** 일반 핫팩을 장착 중인지 반환한다. 핫팩은 1개까지만 장착된다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Item|Effect")
	bool HasHotPack() const;

	/** 부츠를 장착 중인지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Item|Effect")
	bool HasBoots() const;

	/** 패딩을 장착 중인지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Item|Effect")
	bool HasPadding() const;

	/** 장갑을 장착 중인지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Item|Effect")
	bool HasGloves() const;

	/** 눈삽 내구도를 반환한다. 0이면 눈삽을 들고 있지 않다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Item|Effect")
	int32 GetSnowShovelDurability() const;

	/** 외형 슬롯이 표시할 현재 눈삽 종류를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Item|Effect")
	ESnowRumbleGiftItemType GetEquippedShovelItemType() const;

	/** 외형 슬롯이 표시할 현재 눈오리 제작기 종류를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Item|Effect")
	ESnowRumbleGiftItemType GetEquippedDuckMakerItemType() const;

	/** 이동속도에 곱할 패시브 배율을 반환한다. */
	float GetMovementSpeedMultiplier() const;

	/** 받는 피해에 곱할 패시브 배율을 반환한다. */
	float GetIncomingDamageMultiplier() const;

	/** 눈덩이 제작 시간에 곱할 패시브 배율을 반환한다. */
	float GetSnowballCreationDurationMultiplier() const;

	/** 눈덩이 피해에 곱할 패시브 배율을 반환한다. */
	float GetSnowballDamageMultiplier() const;

	UPROPERTY(BlueprintAssignable, Category = "SnowRumble|Item|Effect")
	FOnGiftItemEffectsChanged OnGiftItemEffectsChanged;

protected:
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_ItemEffects();

	/** 서버가 즉시 회복 아이템을 HP 컴포넌트에 적용한다. */
	bool ApplyInstantHeal(float HealAmount);

	/** 서버가 일반 핫팩을 1개까지 장착한다. */
	bool EquipHotPack();

	/** 서버가 황금 핫팩 획득 즉시 같은 팀의 얼음 상태 아군을 부활시킨다. */
	bool UseGoldenHotPackImmediately();

	/** 서버가 모닥불 키트 획득 즉시 캐릭터 앞에 모닥불을 설치한다. */
	bool SpawnCampfireFromKit();

	/** 서버가 제한시간이 있는 무적 효과를 시작한다. */
	void StartInvulnerability(float DurationSeconds);

	/** 서버가 무적 효과를 종료한다. */
	void FinishInvulnerability();

	/** 서버가 황금 붕어빵 지속 회복을 시작한다. */
	void StartGoldenFishBreadRegeneration();

	/** 서버가 황금 붕어빵 회복 Tick을 처리한다. */
	void TickGoldenFishBreadRegeneration();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Effect|Heal", meta = (ClampMin = "0.0"))
	float InstantHealAmount = 35.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Effect|Energy Drink", meta = (ClampMin = "0.0"))
	float EnergyDrinkInvulnerabilitySeconds = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Effect|Boots", meta = (ClampMin = "1.0"))
	float BootsMovementSpeedMultiplier = 1.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Effect|Padding", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PaddingIncomingDamageMultiplier = 0.9f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Effect|Gloves", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GlovesCreationDurationMultiplier = 0.85f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Effect|Duck Maker", meta = (ClampMin = "1.0"))
	float SnowDuckMakerDamageMultiplier = 1.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Effect|Duck Maker", meta = (ClampMin = "1.0"))
	float GoldenSnowDuckMakerDamageMultiplier = 1.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Effect|Golden Fish Bread", meta = (ClampMin = "0.0"))
	float GoldenFishBreadHealPerSecond = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Effect|Golden Fish Bread", meta = (ClampMin = "0.0"))
	float GoldenFishBreadDurationSeconds = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Effect|Shovel", meta = (ClampMin = "0"))
	int32 RustyShovelMinDurability = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Effect|Shovel", meta = (ClampMin = "0"))
	int32 RustyShovelMaxDurability = 4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Effect|Shovel", meta = (ClampMin = "0"))
	int32 GoldenShovelDurability = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Effect|Campfire")
	TSubclassOf<ACampfire> CampfireClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Effect|Campfire", meta = (ClampMin = "0.0"))
	float CampfireSpawnForwardDistance = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Effect|Campfire", meta = (ClampMin = "0.0"))
	float CampfireSpawnGroundTraceDistance = 500.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_ItemEffects, Category = "SnowRumble|Item|Effect")
	bool bHasBoots = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_ItemEffects, Category = "SnowRumble|Item|Effect")
	bool bHasPadding = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_ItemEffects, Category = "SnowRumble|Item|Effect")
	bool bHasGloves = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_ItemEffects, Category = "SnowRumble|Item|Effect")
	bool bHasSnowDuckMaker = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_ItemEffects, Category = "SnowRumble|Item|Effect")
	bool bHasGoldenSnowDuckMaker = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_ItemEffects, Category = "SnowRumble|Item|Effect")
	bool bInvulnerable = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_ItemEffects, Category = "SnowRumble|Item|Effect")
	bool bHasHotPack = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_ItemEffects, Category = "SnowRumble|Item|Effect")
	int32 SnowShovelDurability = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_ItemEffects, Category = "SnowRumble|Item|Effect")
	ESnowRumbleGiftItemType EquippedShovelType = ESnowRumbleGiftItemType::None;

	FTimerHandle InvulnerabilityTimerHandle;
	FTimerHandle GoldenFishBreadRegenerationTimerHandle;
	int32 GoldenFishBreadTicksRemaining = 0;
};

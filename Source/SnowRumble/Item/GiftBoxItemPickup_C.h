// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GiftItemTypes_C.h"
#include "GameFramework/Actor.h"
#include "GiftBoxItemPickup_C.generated.h"

class ASnowRumbleCharacter;
class UNiagaraSystem;
class USphereComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class SNOWRUMBLE_API AGiftBoxItemPickup : public AActor
{
	GENERATED_BODY()

public:
	AGiftBoxItemPickup();

	virtual void Tick(float DeltaSeconds) override;

	/** 서버가 선물상자에서 나온 아이템 정보를 초기화한다. */
	void InitializePickupFromServer(
		ESnowRumbleGiftItemType NewItemType,
		FName NewItemId,
		const FText& NewDisplayName);

	/** 캐릭터가 현재 아이템을 획득할 수 있는지 로컬 후보 탐색과 서버 검증에서 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Item|Pickup")
	bool CanInteractWith(const ASnowRumbleCharacter* Character) const;

	/** 서버가 아이템 획득을 확정한다. */
	bool TryPickup(ASnowRumbleCharacter* Character);

	/** 상호작용 허용 거리다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Item|Pickup")
	float GetInteractionRadius() const;

	/** 아이템 표시 이름을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Item|Pickup")
	FText GetDisplayName() const;

	/** 아이템 종류 enum을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Item|Pickup")
	ESnowRumbleGiftItemType GetItemType() const;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_ItemData();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayPickedUpEffect();

	/** 아이템 정보가 정해졌을 때 Blueprint가 모델·색·텍스트를 연결한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Item|Pickup")
	void OnItemDataChanged(
		ESnowRumbleGiftItemType NewItemType,
		FName NewItemId,
		const FText& NewDisplayName);

	/** 서버가 획득을 확정했을 때 Blueprint가 획득 연출을 연결한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Item|Pickup")
	void OnItemPickedUp(ASnowRumbleCharacter* Picker);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Pickup")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Pickup")
	TObjectPtr<UStaticMeshComponent> ItemMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Pickup", meta = (ClampMin = "0.0"))
	float InteractionRadius = 180.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Pickup", meta = (ClampMin = "0.0"))
	float PickedUpDestroyDelaySeconds = 0.2f;

	/** 아이템 획득 성공 시 모든 화면에서 한 번 재생할 공통 Niagara 이펙트다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Pickup|Effect")
	TObjectPtr<UNiagaraSystem> PickedUpEffect;

	/** 획득 이펙트를 Pickup 위치에서 얼마나 띄워 재생할지 조정한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Pickup|Effect")
	FVector PickedUpEffectLocationOffset = FVector(0.0f, 0.0f, 30.0f);

	/** 선물상자가 아니라 맵이나 BP에 직접 배치한 개발용 Pickup의 기본 아이템 종류다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Pickup|Placed")
	ESnowRumbleGiftItemType DefaultItemType = ESnowRumbleGiftItemType::None;

	/** 직접 배치한 개발용 Pickup의 기본 식별자다. 비워두면 DefaultItemType 이름을 사용한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Pickup|Placed")
	FName DefaultItemId = NAME_None;

	/** 직접 배치한 개발용 Pickup의 표시 이름이다. 비워두면 아이템 종류 이름을 사용한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Pickup|Placed")
	FText DefaultDisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Pickup|Float", meta = (ClampMin = "0.0"))
	float FloatAmplitude = 18.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Pickup|Float", meta = (ClampMin = "0.0"))
	float FloatSpeed = 5.0f;

private:
	void InitializePlacedPickupFromDefaults();
	FText GetFallbackDisplayNameForItemType() const;
	void NotifyPickedUp(ASnowRumbleCharacter* Character) const;
	void SpawnPickedUpEffect() const;
	FString GetCharacterDisplayName(const ASnowRumbleCharacter* Character) const;

	UPROPERTY(ReplicatedUsing = OnRep_ItemData)
	ESnowRumbleGiftItemType ItemType = ESnowRumbleGiftItemType::None;

	UPROPERTY(ReplicatedUsing = OnRep_ItemData)
	FName ItemId = NAME_None;

	UPROPERTY(ReplicatedUsing = OnRep_ItemData)
	FText DisplayName;

	UPROPERTY(Replicated)
	bool bPickedUp = false;

	FVector InitialMeshRelativeLocation = FVector::ZeroVector;
	float FloatTimeSeconds = 0.0f;
};

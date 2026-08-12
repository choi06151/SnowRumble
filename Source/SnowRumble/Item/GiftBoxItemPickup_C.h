// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GiftItemTypes_C.h"
#include "GameFramework/Actor.h"
#include "GiftBoxItemPickup_C.generated.h"

class ASnowRumbleCharacter;
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Pickup|Float", meta = (ClampMin = "0.0"))
	float FloatAmplitude = 18.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Pickup|Float", meta = (ClampMin = "0.0"))
	float FloatSpeed = 5.0f;

private:
	void NotifyPickedUp(ASnowRumbleCharacter* Character) const;
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

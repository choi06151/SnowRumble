// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GiftItemTypes_C.h"
#include "GameFramework/Actor.h"
#include "GiftBox_C.generated.h"

class ASnowRumbleCharacter;
class AGiftBoxItemPickup;
class UMaterialInterface;
class UNiagaraSystem;
class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class AController;
struct FDamageEvent;

UENUM(BlueprintType)
enum class ESnowRumbleGiftBoxGrade : uint8
{
	Red,
	Gold
};

USTRUCT(BlueprintType)
struct FSnowRumbleGiftBoxReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item")
	ESnowRumbleGiftItemType ItemType = ESnowRumbleGiftItemType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item")
	FName RewardId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item")
	TSubclassOf<AGiftBoxItemPickup> PickupClass;
};

UCLASS(Blueprintable)
class SNOWRUMBLE_API AGiftBox : public AActor
{
	GENERATED_BODY()

public:
	AGiftBox();

	/** 서버가 상자 등급을 확정하고 해당 등급 표현을 갱신한다. */
	void InitializeGiftBoxFromServer(ESnowRumbleGiftBoxGrade NewGrade);

	/** 캐릭터가 현재 선물상자를 열 수 있는지 로컬 후보 탐색과 서버 검증에서 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Item|Gift Box")
	bool CanInteractWith(const ASnowRumbleCharacter* Character) const;

	/** 서버가 선물상자를 열고 등급에 맞는 랜덤 보상을 확정한다. */
	bool TryOpen(ASnowRumbleCharacter* Character);

	/** 데미지가 들어오면 서버에서 선물상자를 연다. */
	virtual float TakeDamage(
		float DamageAmount,
		FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser) override;

	/** 상호작용 허용 거리다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Item|Gift Box")
	float GetInteractionRadius() const;

	/** 현재 상자 등급을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Item|Gift Box")
	ESnowRumbleGiftBoxGrade GetGiftBoxGrade() const;

	/** 서버가 확정한 보상 표시 이름을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Item|Gift Box")
	FText GetOpenedRewardName() const;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 복제된 등급에 맞춰 Blueprint 표현을 갱신한다. */
	UFUNCTION()
	void OnRep_GiftBoxGrade();

	/** 복제된 개봉 상태에 맞춰 Blueprint 표현을 갱신한다. */
	UFUNCTION()
	void OnRep_Opened();

	/** 상자 등급이 정해졌을 때 Blueprint가 빨간/황금 표현을 연결한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Item|Gift Box")
	void OnGiftBoxGradeChanged(ESnowRumbleGiftBoxGrade NewGrade);

	/** 상자가 바닥에 닿거나 ProjectileMovement가 정지했을 때 Blueprint가 착지 연출을 연결한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Item|Gift Box")
	void OnGiftBoxLanded();

	/** 상자가 서버에서 개봉됐을 때 Blueprint가 개봉 연출을 연결한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Item|Gift Box")
	void OnGiftBoxOpened(
		ASnowRumbleCharacter* Opener,
		ESnowRumbleGiftBoxGrade OpenedGrade,
		const FText& RewardName);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Gift Box")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Gift Box")
	TObjectPtr<UStaticMeshComponent> GiftBoxMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Gift Box")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Gift Box", meta = (ClampMin = "0.0"))
	float InteractionRadius = 220.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Gift Box", meta = (ClampMin = "0.0"))
	float OpenedDestroyDelaySeconds = 3.0f;

	/** 빨간/황금 선물상자 머티리얼을 적용할 StaticMesh 머티리얼 슬롯이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Gift Box|Material", meta = (ClampMin = "0"))
	int32 GradeMaterialIndex = 0;

	/** 빨간 선물상자 등급일 때 적용할 머티리얼이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Gift Box|Material")
	TObjectPtr<UMaterialInterface> RedGiftBoxMaterial;

	/** 황금 선물상자 등급일 때 적용할 머티리얼이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Gift Box|Material")
	TObjectPtr<UMaterialInterface> GoldGiftBoxMaterial;

	/** 선물상자가 열려 사라질 때 재생할 Niagara 이펙트다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Gift Box|Effect")
	TObjectPtr<UNiagaraSystem> OpenedEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Gift Box|Reward")
	TArray<FSnowRumbleGiftBoxReward> RedBoxRewards;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Gift Box|Reward")
	TArray<FSnowRumbleGiftBoxReward> GoldBoxRewards;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Gift Box|Reward")
	TSubclassOf<AGiftBoxItemPickup> DefaultPickupClass;

private:
	UFUNCTION()
	void HandleProjectileStopped(const FHitResult& Hit);

	void ApplyGradeMaterial();
	void SpawnOpenedEffect() const;
	FSnowRumbleGiftBoxReward ChooseReward() const;
	FText GetFallbackRewardName() const;
	void SpawnRewardPickup(const FSnowRumbleGiftBoxReward& Reward);

	UPROPERTY(ReplicatedUsing = OnRep_GiftBoxGrade)
	ESnowRumbleGiftBoxGrade GiftBoxGrade = ESnowRumbleGiftBoxGrade::Red;

	UPROPERTY(ReplicatedUsing = OnRep_Opened)
	bool bOpened = false;

	UPROPERTY(Replicated)
	FName OpenedRewardId = NAME_None;

	UPROPERTY(Replicated)
	FText OpenedRewardName;

	UPROPERTY(Replicated)
	TObjectPtr<ASnowRumbleCharacter> OpenedByCharacter;

	UPROPERTY(Replicated)
	TObjectPtr<AGiftBoxItemPickup> SpawnedPickup;
};

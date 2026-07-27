// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SnowballItem.generated.h"

class ASnowRumbleCharacter;
class UPrimitiveComponent;
class UProjectileMovementComponent;
class USceneComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class ESnowballItemState : uint8
{
	Ground,
	Rolling,
	Held,
	Thrown
};

UCLASS()
class SNOWRUMBLE_API ASnowballItem : public AActor
{
	GENERATED_BODY()

public:
	ASnowballItem();

	/** 서버가 바닥 눈덩이를 플레이어의 장착 위치에 귀속한다. */
	bool TrySetHeldBy(ASnowRumbleCharacter* NewHolder, USceneComponent* HoldPoint);

	/** 현재 눈덩이가 바닥에서 획득 가능한지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	bool CanBePickedUp() const;

	/** 서버가 장착된 눈덩이를 지정한 방향과 속도로 투척한다. */
	bool Throw(const FVector& ThrowDirection, float ThrowSpeed);

	/** 서버가 장착된 눈덩이를 현재 손 위치에서 바닥 상태로 놓는다. */
	bool DropToGround();

	/** 서버가 바닥 눈덩이를 지정한 플레이어의 굴리기 상태로 전환한다. */
	bool TryStartRolling(ASnowRumbleCharacter* NewRoller);

	/** 서버가 굴리기 상태를 끝내고 바닥 물리를 복구한다. */
	bool StopRolling();

	/** 서버가 굴리기 중인 눈덩이를 충돌 Sweep으로 새 위치까지 이동한다. */
	void MoveRollingSnowball(const FVector& TargetLocation);

	/** 서버가 마지막 확인 위치부터 실제 이동한 거리를 성장값에 누적한다. */
	void UpdateRollingGrowth();

	/** 현재 0~1 정규화된 성장 진행도를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	float GetGrowthProgress() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	ESnowballItemState GetItemState() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	ASnowRumbleCharacter* GetHolder() const;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 복제된 상태에 맞춰 충돌과 장착 표현을 갱신한다. */
	UFUNCTION()
	void OnRep_ItemState();

	/** 복제된 보유자에 맞춰 장착 표현을 갱신한다. */
	UFUNCTION()
	void OnRep_Holder();

	/** 복제된 성장값을 Actor Scale과 충돌 크기에 적용한다. */
	UFUNCTION()
	void OnRep_GrowthProgress();

	/** 현재 상태와 보유자를 사용해 로컬 액터 부착과 충돌을 적용한다. */
	void RefreshStatePresentation();

	/** 초기 크기를 기준으로 현재 성장값의 Actor Scale을 적용한다. */
	void ApplyGrowthScale();

	/** 서버에서 투척 눈덩이의 충돌 피해와 제거를 확정한다. */
	UFUNCTION()
	void HandleCollision(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball", meta = (ClampMin = "0.1"))
	float MaximumThrownLifeSeconds = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball", meta = (ClampMin = "0.0"))
	float Damage = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Growth", meta = (ClampMin = "1.0"))
	float MaximumScaleMultiplier = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Growth", meta = (ClampMin = "1.0"))
	float DistanceForMaximumGrowth = 1000.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_ItemState, Category = "SnowRumble|Snowball")
	ESnowballItemState ItemState = ESnowballItemState::Ground;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_Holder, Category = "SnowRumble|Snowball")
	TObjectPtr<ASnowRumbleCharacter> Holder;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_Holder, Category = "SnowRumble|Snowball")
	TObjectPtr<ASnowRumbleCharacter> Roller;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_GrowthProgress, Category = "SnowRumble|Snowball|Growth")
	float GrowthProgress = 0.0f;

	FVector InitialActorScale = FVector::OneVector;
	FVector LastRollingLocation = FVector::ZeroVector;
	float AccumulatedRollingDistance = 0.0f;
};

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
class USoundBase;

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

	/** 서버가 장착된 눈덩이를 지정한 방향·속도·충전량으로 투척한다. */
	bool Throw(
		const FVector& ThrowDirection,
		float ThrowSpeed,
		float ThrowChargeProgress);

	/** 서버가 장착된 눈덩이를 현재 손 위치에서 바닥 상태로 놓는다. */
	bool DropToGround();

	/** 서버가 바닥 눈덩이를 지정한 플레이어의 굴리기 상태로 전환한다. */
	bool TryStartRolling(ASnowRumbleCharacter* NewRoller);

	/** 서버가 굴리기 상태를 끝내고 바닥 물리를 복구한다. */
	bool StopRolling();

	/** 서버가 굴리기 충돌 프록시의 확정 위치로 눈덩이를 무충돌 이동한다. */
	void MoveRollingSnowball(const FVector& TargetLocation);

	/** 서버가 확인한 눈 표면에 눈덩이를 물리 낙하 없이 고정한다. */
	void SettleOnGroundFromSurface(
		const FVector& SurfacePoint,
		const FVector& SurfaceNormal);

	/** 서버가 마지막 확인 위치부터 실제 이동한 거리를 성장값에 누적한다. */
	void UpdateRollingGrowth();

	/** 현재 0~1 정규화된 성장 진행도를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	float GetGrowthProgress() const;

	/** 성장 오차 범위를 포함해 최대 성장 큰 눈덩이인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	bool IsFullyGrown() const;

	/** 현재 성장 크기가 적용된 굴리기 충돌 반지름을 반환한다. */
	float GetRollingCollisionRadius() const;

	/** 생성 직후 지정 Actor와 잠시 충돌하지 않도록 한다. */
	void IgnoreActorTemporarily(AActor* ActorToIgnore, float DurationSeconds);

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

	/** 복제된 지면 고정 상태에 맞춰 바닥 물리를 갱신한다. */
	UFUNCTION()
	void OnRep_IsSettledOnGround();

	/** 현재 상태와 보유자를 사용해 로컬 액터 부착과 충돌을 적용한다. */
	void RefreshStatePresentation();

	/** 서버가 현재 위치 아래 바닥을 찾아 눈덩이를 안정적으로 올려놓는다. */
	bool TrySettleOnGroundBelow(AActor* ActorToIgnore);

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

	/** 투사체 이동이 Blocking Hit로 정지하면 서버 충돌 처리를 보완한다. */
	UFUNCTION()
	void HandleProjectileStopped(const FHitResult& Hit);

	/** 서버에서 처음 확인한 투척 충돌의 피해, 이펙트와 제거를 처리한다. */
	void HandleThrownImpact(AActor* OtherActor, const FHitResult& Hit);

	/** 임시 충돌 무시가 끝난 Actor를 다시 충돌 대상으로 복구한다. */
	void RestoreTemporarilyIgnoredActor(
		TWeakObjectPtr<AActor> IgnoredActor);

	/** 서버가 확정한 충돌 이펙트를 모든 참가자 화면에서 재생한다. */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayImpactEffect(
		FVector_NetQuantize ImpactPoint,
		FVector_NetQuantizeNormal ImpactNormal);

	/** Blueprint에서 실제 충돌 Niagara, 파티클과 사운드를 재생한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Snowball|Impact")
	void PlayImpactEffect(
		FVector ImpactPoint,
		FVector ImpactNormal);

	/** 충돌 시 기본 사운드를 재생한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Impact")
	TObjectPtr<USoundBase> ImpactSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball", meta = (ClampMin = "0.1"))
	float MaximumThrownLifeSeconds = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball", meta = (ClampMin = "0.0"))
	float Damage = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Impact", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinimumDamageMultiplier = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Impact", meta = (ClampMin = "0.0"))
	float SmallSnowballMinimumKnockback = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Impact", meta = (ClampMin = "0.0"))
	float SmallSnowballMaximumKnockback = 900.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Impact|Large", meta = (ClampMin = "0.0"))
	float LargeSnowballMinimumKnockback = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Impact|Large", meta = (ClampMin = "0.0"))
	float LargeSnowballMaximumKnockback = 1400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Impact", meta = (ClampMin = "0.0"))
	float KnockbackUpwardRatio = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw", meta = (ClampMin = "0.0"))
	float SmallSnowballProjectileGravityScale = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Throw|Large", meta = (ClampMin = "0.0"))
	float LargeSnowballProjectileGravityScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Growth", meta = (ClampMin = "1.0"))
	float MaximumScaleMultiplier = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Growth", meta = (ClampMin = "1.0"))
	float DistanceForMaximumGrowth = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Grounding", meta = (ClampMin = "0.0"))
	float GroundSettleTraceUpDistance = 120.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Grounding", meta = (ClampMin = "0.0"))
	float GroundSettleTraceDownDistance = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Grounding", meta = (ClampMin = "0.0"))
	float GroundSettleExtraClearance = 0.5f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_ItemState, Category = "SnowRumble|Snowball")
	ESnowballItemState ItemState = ESnowballItemState::Ground;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_Holder, Category = "SnowRumble|Snowball")
	TObjectPtr<ASnowRumbleCharacter> Holder;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_Holder, Category = "SnowRumble|Snowball")
	TObjectPtr<ASnowRumbleCharacter> Roller;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_GrowthProgress, Category = "SnowRumble|Snowball|Growth")
	float GrowthProgress = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_IsSettledOnGround, Category = "SnowRumble|Snowball")
	bool bIsSettledOnGround = true;

	FVector InitialActorScale = FVector::OneVector;
	FVector LastRollingLocation = FVector::ZeroVector;
	float AccumulatedRollingDistance = 0.0f;
	bool bHasProcessedThrownImpact = false;
	float CurrentThrowChargeProgress = 0.0f;

	TSet<TWeakObjectPtr<AActor>> TemporarilyIgnoredActors;
	ECollisionResponse CachedPawnCollisionResponse = ECR_Block;
	bool bTemporarilyIgnoringPawnCollision = false;
};

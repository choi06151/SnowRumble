// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SnowballCreationComponent.generated.h"

class ASnowballItem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnSnowballCreatingChanged,
	bool,
	bIsCreating);

UCLASS(ClassGroup = (SnowRumble), meta = (BlueprintSpawnableComponent))
class SNOWRUMBLE_API USnowballCreationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USnowballCreationComponent();

	/** 소유 플레이어가 현재 바라보는 눈 바닥에서 제작 시작을 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Snowball")
	void StartCreatingSnowball();

	/** 입력 해제 또는 행동 불가 상태에서 진행 중인 제작을 취소한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Snowball")
	void CancelCreatingSnowball();

	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	bool IsCreatingSnowball() const;

	/** UI가 사용할 0~1 정규화된 제작 진행도를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	float GetCreationProgress() const;

	UPROPERTY(BlueprintAssignable, Category = "SnowRumble|Snowball")
	FOnSnowballCreatingChanged OnCreatingChanged;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void ServerStartCreatingSnowball(
		FVector_NetQuantize ViewLocation,
		FVector_NetQuantizeNormal ViewDirection);

	UFUNCTION(Server, Reliable)
	void ServerCancelCreatingSnowball();

	UFUNCTION()
	void OnRep_IsCreating();

	/** 서버가 제작 완료 시 상태와 바닥을 다시 검사하고 눈덩이를 생성한다. */
	void CompleteCreation();

	/** 전달받은 카메라 시점으로 제작 가능한 눈 바닥을 찾는다. */
	bool FindSnowSurface(
		const FVector& ViewLocation,
		const FVector& ViewDirection,
		FHitResult& OutHit) const;

	void SetCreatingState(bool bNewCreating);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Creation")
	TSubclassOf<ASnowballItem> SnowballItemClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Creation", meta = (ClampMin = "0.1"))
	float CreationDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Creation", meta = (ClampMin = "0.0"))
	float CreationTraceDistance = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Creation", meta = (ClampMin = "0.0"))
	float MaxCameraOriginDistance = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Creation", meta = (ClampMin = "0.0"))
	float CreationForwardDistance = 100.0f;

	/** 생성 직후 자기 플레이어와 충돌을 무시할 시간이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Creation", meta = (ClampMin = "0.0"))
	float CreatedSnowballOwnerCollisionIgnoreSeconds = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Creation")
	FName SnowSurfaceTag = TEXT("SnowSurface");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_IsCreating, Category = "SnowRumble|Snowball|Creation")
	bool bIsCreating = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "SnowRumble|Snowball|Creation")
	float CreationStartServerTime = 0.0f;

	TWeakObjectPtr<AActor> CreationSurfaceActor;
	FVector CreationSurfacePoint = FVector::ZeroVector;
	FVector CreationSurfaceNormal = FVector::UpVector;

	FTimerHandle CreationTimerHandle;
};

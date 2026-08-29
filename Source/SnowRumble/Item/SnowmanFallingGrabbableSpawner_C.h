// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SnowmanFallingGrabbableSpawner_C.generated.h"

class AGrabbablePhysicsObject;
class UStaticMesh;
class UNiagaraSystem;
class USoundAttenuation;
class USoundBase;

UCLASS(Blueprintable)
class SNOWRUMBLE_API ASnowmanFallingGrabbableSpawner : public AActor
{
	GENERATED_BODY()

public:
	ASnowmanFallingGrabbableSpawner();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	/** 눈사람 모드에서 낙하시킬 Static Mesh 후보 목록이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Grab")
	TArray<TObjectPtr<UStaticMesh>> CandidateStaticMeshes;

	/** 후보 Mesh를 장착할 물리 Grab Actor 클래스다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Grab")
	TSubclassOf<AGrabbablePhysicsObject> GrabbableObjectClass;

	/** 경기 시작 후 첫 물체가 떨어질 때까지 기다리는 시간이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Grab", meta = (ClampMin = "0.0"))
	float FirstDropDelaySeconds = 15.0f;

	/** 첫 물체 이후 다음 물체가 떨어지는 간격이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Grab", meta = (ClampMin = "0.1"))
	float DropIntervalSeconds = 20.0f;

	/** 낙하 물체를 생성할 NavMesh 검색 반경이다. 스포너 Actor 위치를 중심으로 한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Grab", meta = (ClampMin = "0.0"))
	float NavMeshSearchRadius = 2500.0f;

	/** NavMesh 지점보다 높은 곳에서 물체를 생성할 높이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Grab", meta = (ClampMin = "0.0"))
	float DropHeightOffset = 1200.0f;

	/** 변환 물체에 적용할 플레이어 밀침 힘이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Grab|Interaction", meta = (ClampMin = "0.0"))
	float PlayerPushStrength = 3000.0f;

	/** 이 횟수만큼 눈덩이 피격·플레이어 밀침이 누적되면 물체를 제거한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Grab|Interaction", meta = (ClampMin = "1"))
	int32 InteractionsToBreak = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Grab|Interaction")
	TObjectPtr<UNiagaraSystem> InteractionBreakEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Grab|Interaction")
	TObjectPtr<USoundBase> InteractionBreakSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Grab|Interaction")
	TObjectPtr<USoundAttenuation> InteractionBreakSoundAttenuation;

	/** 눈사람 모드가 실제 시작됐는지 감시한다. 서버에서만 실행된다. */
	void UpdateSnowmanModeState();

	/** NavMesh 위 랜덤 지점에 후보 Mesh를 가진 Grab 물체를 하나 생성한다. */
	void SpawnFallingGrabbableObject();

	/** 현재 눈사람 모드가 종료됐는지 확인하고 낙하 타이머를 정리한다. */
	bool IsSnowmanModeFinished() const;

	FTimerHandle SnowmanModeStateTimerHandle;
	FTimerHandle FirstDropTimerHandle;
	FTimerHandle DropTimerHandle;
	bool bDropSequenceStarted = false;
};

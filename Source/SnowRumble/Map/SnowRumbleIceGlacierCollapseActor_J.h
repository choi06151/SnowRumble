// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "SnowRumbleIceGlacierCollapseActor_J.generated.h"

class UPrimitiveComponent;
class USceneComponent;

UENUM(BlueprintType)
enum class ESnowRumbleIceGlacierCollapsePhase : uint8
{
	Stable,
	Group1Sinking,
	Group1Complete,
	Group2Sinking,
	Complete
};

struct FIceGlacierCollapseComponentCollisionState
{
	TWeakObjectPtr<UPrimitiveComponent> Component;
	ECollisionEnabled::Type CollisionEnabled = ECollisionEnabled::NoCollision;
};

struct FIceGlacierCollapseTargetState
{
	TWeakObjectPtr<AActor> Actor;
	FTransform InitialTransform = FTransform::Identity;
	TArray<FIceGlacierCollapseComponentCollisionState> CollisionStates;
	bool bCollisionDisabledAfterSink = false;
};

UCLASS(Blueprintable)
class SNOWRUMBLE_API ASnowRumbleIceGlacierCollapseActor : public AActor
{
	GENERATED_BODY()

public:
	ASnowRumbleIceGlacierCollapseActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 현재 서버가 확정한 빙판 침몰 단계다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Ice Glacier Collapse")
	ESnowRumbleIceGlacierCollapsePhase GetCurrentCollapsePhase() const;

	/** 현재 Group 1 침몰 진행도다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Ice Glacier Collapse")
	float GetGroup1Alpha() const;

	/** 현재 Group 2 침몰 진행도다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Ice Glacier Collapse")
	float GetGroup2Alpha() const;

protected:
	virtual void BeginPlay() override;

	/** 240~300초 동안 침몰할 빙판 Actor 목록이다. Final Core는 넣지 않는다. */
	UPROPERTY(EditInstanceOnly, Category = "SnowRumble|Ice Glacier Collapse|Targets")
	TArray<TObjectPtr<AActor>> CollapseGroup1;

	/** 300~360초 동안 침몰할 빙판 Actor 목록이다. Final Core는 넣지 않는다. */
	UPROPERTY(EditInstanceOnly, Category = "SnowRumble|Ice Glacier Collapse|Targets")
	TArray<TObjectPtr<AActor>> CollapseGroup2;

	/** Group 1 침몰 시작 경기 시간이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Ice Glacier Collapse|Timing", meta = (ClampMin = "0.0"))
	float Group1StartSeconds = 240.0f;

	/** Group 1 침몰 완료 경기 시간이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Ice Glacier Collapse|Timing", meta = (ClampMin = "0.0"))
	float Group1EndSeconds = 300.0f;

	/** Group 2 침몰 시작 경기 시간이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Ice Glacier Collapse|Timing", meta = (ClampMin = "0.0"))
	float Group2StartSeconds = 300.0f;

	/** Group 2 침몰 완료 경기 시간이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Ice Glacier Collapse|Timing", meta = (ClampMin = "0.0"))
	float Group2EndSeconds = 360.0f;

	/** 각 빙판이 시작 위치에서 World Z 아래로 내려갈 거리다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Ice Glacier Collapse|Tuning", meta = (ClampMin = "0.0"))
	float SinkDistance = 1200.0f;

	/** 완전히 침몰한 빙판의 Collision을 끌지 여부다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Ice Glacier Collapse|Collision")
	bool bDisableCollisionAfterFullySunk = true;

private:
	/** 복제된 서버 Phase를 클라이언트의 빙판 위치에 반영한다. */
	UFUNCTION()
	void OnRep_CollapsePhase();

	/** Editor에서 지정한 대상 Actor들의 시작 Transform과 Collision 상태를 저장한다. */
	void InitializeCollapseTargets();

	/** 한 그룹의 시작 상태를 저장한다. */
	void InitializeCollapseGroup(
		const TArray<TObjectPtr<AActor>>& SourceActors,
		TArray<FIceGlacierCollapseTargetState>& OutTargetStates);

	/** 서버 경기 시간 기준으로 현재 침몰 단계를 계산한다. */
	ESnowRumbleIceGlacierCollapsePhase CalculateCollapsePhase(
		float RoundElapsedSeconds) const;

	/** 지정 시간 구간의 침몰 진행도를 계산한다. */
	float CalculateAlpha(
		float RoundElapsedSeconds,
		float StartSeconds,
		float EndSeconds) const;

	/** 현재 Phase에서 Group 1에 적용할 침몰 진행도다. */
	float CalculateGroup1Alpha(float RoundElapsedSeconds) const;

	/** 현재 Phase에서 Group 2에 적용할 침몰 진행도다. */
	float CalculateGroup2Alpha(float RoundElapsedSeconds) const;

	/** 현재 서버 Phase와 경기 시간으로 모든 빙판 위치와 Collision을 갱신한다. */
	void ApplyCollapseState(float RoundElapsedSeconds);

	/** 한 그룹의 빙판 위치와 완료 후 Collision 상태를 갱신한다. */
	void ApplyCollapseGroup(
		TArray<FIceGlacierCollapseTargetState>& TargetStates,
		float Alpha);

	/** 저장된 시작 Transform 기준으로 빙판의 World Z만 갱신한다. */
	void ApplyTargetTransform(
		FIceGlacierCollapseTargetState& TargetState,
		float Alpha);

	/** 완전히 침몰한 빙판의 Collision을 비활성화한다. */
	void DisableTargetCollision(FIceGlacierCollapseTargetState& TargetState);

	/** 새 라운드 또는 안정 구간에서 기존 Collision 상태를 복원한다. */
	void RestoreTargetCollision(FIceGlacierCollapseTargetState& TargetState);

	/** GameState의 서버 동기화 경기 시간을 읽는다. */
	float GetRoundElapsedSeconds() const;

	UPROPERTY(VisibleDefaultsOnly, Category = "SnowRumble|Ice Glacier Collapse")
	TObjectPtr<USceneComponent> RootSceneComponent;

	TArray<FIceGlacierCollapseTargetState> Group1TargetStates;
	TArray<FIceGlacierCollapseTargetState> Group2TargetStates;

	UPROPERTY(ReplicatedUsing = OnRep_CollapsePhase)
	ESnowRumbleIceGlacierCollapsePhase CurrentCollapsePhase =
		ESnowRumbleIceGlacierCollapsePhase::Stable;
};

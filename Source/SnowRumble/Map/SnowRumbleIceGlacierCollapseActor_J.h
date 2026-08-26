// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "SnowRumbleIceGlacierCollapseActor_J.generated.h"

class UPrimitiveComponent;
class USceneComponent;
class UCameraShakeBase;
class ACharacter;

UENUM(BlueprintType)
enum class ESnowRumbleIceGlacierCollapsePhase : uint8
{
	Stable,
	Group1Sinking,
	Group1Complete,
	Group2Sinking,
	Complete
};

USTRUCT(BlueprintType)
struct FIceGlacierCollapsePiece
{
	GENERATED_BODY()

	/** 흔들리고 침몰할 빙판 조각 Actor다. Final Core는 넣지 않는다. */
	UPROPERTY(EditInstanceOnly, Category = "SnowRumble|Ice Glacier Collapse|Piece")
	TObjectPtr<AActor> TargetActor = nullptr;

	/** 그룹 시작 시간 이후 이 조각이 경고를 시작하기까지 기다릴 시간이다. */
	UPROPERTY(EditAnywhere, Category = "SnowRumble|Ice Glacier Collapse|Piece", meta = (ClampMin = "0.0"))
	float StartDelaySeconds = 0.0f;

	/** 침몰 직전에 Initial Transform 기준으로 흔들리는 시간이다. */
	UPROPERTY(EditAnywhere, Category = "SnowRumble|Ice Glacier Collapse|Piece", meta = (ClampMin = "0.0"))
	float WarningDurationSeconds = 1.0f;

	/** Warning 중 World X/Y로 흔들릴 최대 거리(cm)다. */
	UPROPERTY(EditAnywhere, Category = "SnowRumble|Ice Glacier Collapse|Piece", meta = (ClampMin = "0.0"))
	float ShakeAmplitude = 8.0f;

	/** Warning 중 1초에 반복할 흔들림 횟수다. */
	UPROPERTY(EditAnywhere, Category = "SnowRumble|Ice Glacier Collapse|Piece", meta = (ClampMin = "0.0"))
	float ShakeFrequency = 8.0f;

	/** Warning이 끝난 뒤 Initial Z에서 목표 Z까지 침몰하는 시간이다. */
	UPROPERTY(EditAnywhere, Category = "SnowRumble|Ice Glacier Collapse|Piece", meta = (ClampMin = "0.0"))
	float FallDurationSeconds = 2.0f;

	/** 이 조각이 Initial Z에서 World Z 아래로 내려갈 거리다. */
	UPROPERTY(EditAnywhere, Category = "SnowRumble|Ice Glacier Collapse|Piece", meta = (ClampMin = "0.0"))
	float SinkDistance = 1200.0f;
};

struct FIceGlacierCollapsePieceRuntimeSettings
{
	float StartDelaySeconds = 0.0f;
	float WarningDurationSeconds = 1.0f;
	float ShakeAmplitude = 8.0f;
	float ShakeFrequency = 8.0f;
	float FallDurationSeconds = 2.0f;
	float SinkDistance = 1200.0f;
};

struct FIceGlacierCollapseComponentCollisionState
{
	TWeakObjectPtr<UPrimitiveComponent> Component;
	ECollisionEnabled::Type CollisionEnabled = ECollisionEnabled::NoCollision;
};

enum class EIceGlacierCollapsePieceState : uint8
{
	Idle,
	Warning,
	Falling,
	Done
};

struct FIceGlacierCollapseTargetState
{
	TWeakObjectPtr<AActor> Actor;
	FIceGlacierCollapsePieceRuntimeSettings PieceSettings;
	FTransform InitialTransform = FTransform::Identity;
	TArray<FIceGlacierCollapseComponentCollisionState> CollisionStates;
	TArray<TWeakObjectPtr<AActor>> CarriedActors;
	FVector PreviousCarryPieceLocation = FVector::ZeroVector;
	EIceGlacierCollapsePieceState PreviousPieceState =
		EIceGlacierCollapsePieceState::Idle;
	bool bCollisionDisabledAfterSink = false;
	bool bCarryInitializedForFall = false;
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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 240~300초 동안 차례로 흔들리고 침몰할 빙판 조각 목록이다. Final Core는 넣지 않는다. */
	UPROPERTY(EditInstanceOnly, Category = "SnowRumble|Ice Glacier Collapse|Targets")
	TArray<FIceGlacierCollapsePiece> Group1Pieces;

	/** 300~360초 동안 차례로 흔들리고 침몰할 빙판 조각 목록이다. Final Core는 넣지 않는다. */
	UPROPERTY(EditInstanceOnly, Category = "SnowRumble|Ice Glacier Collapse|Targets")
	TArray<FIceGlacierCollapsePiece> Group2Pieces;

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

	/** 완전히 침몰한 빙판의 Collision을 끌지 여부다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Ice Glacier Collapse|Collision")
	bool bDisableCollisionAfterFullySunk = true;

	/** Warning 중인 빙판 위의 로컬 플레이어에게만 재생할 Camera Shake Class다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Ice Glacier Collapse|Camera Shake")
	TSubclassOf<UCameraShakeBase> WarningCameraShakeClass;

	/** Warning Camera Shake 재생 배율이다. 세부 진폭과 길이는 Camera Shake Asset에서 조정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Ice Glacier Collapse|Camera Shake", meta = (ClampMin = "0.0"))
	float WarningCameraShakeScale = 1.0f;

private:
	/** 복제된 서버 Phase를 클라이언트의 빙판 위치에 반영한다. */
	UFUNCTION()
	void OnRep_CollapsePhase();

	/** Editor에서 지정한 대상 Actor들의 시작 Transform과 Collision 상태를 저장한다. */
	void InitializeCollapseTargets();

	/** 한 그룹의 시작 상태를 저장한다. */
	void InitializeCollapseGroup(
		const TArray<FIceGlacierCollapsePiece>& SourcePieces,
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
		float RoundElapsedSeconds,
		float GroupStartSeconds,
		float GroupEndSeconds);

	/** 현재 경기 시간에서 조각 하나의 흐름 상태와 진행도를 계산한다. */
	EIceGlacierCollapsePieceState CalculatePieceState(
		const FIceGlacierCollapsePieceRuntimeSettings& PieceSettings,
		float RoundElapsedSeconds,
		float GroupStartSeconds,
		float GroupEndSeconds,
		float& OutWarningAlpha,
		float& OutFallAlpha) const;

	/** 저장된 시작 Transform 기준으로 Warning 흔들림을 적용한다. */
	void ApplyWarningTransform(
		FIceGlacierCollapseTargetState& TargetState,
		float WarningElapsedSeconds,
		float WarningAlpha);

	/** 저장된 시작 Transform 기준으로 Falling 중인 빙판 위치를 계산한다. */
	FVector CalculateFallingPieceLocation(
		const FIceGlacierCollapseTargetState& TargetState,
		float Alpha) const;

	/** 저장된 시작 Transform 기준으로 빙판의 World Z만 갱신한다. */
	void ApplyFallingTransform(
		FIceGlacierCollapseTargetState& TargetState,
		float Alpha);

	/** 서버에서 Falling 중인 빙판 위 Actor를 같은 Delta만큼 이동시킨다. */
	void UpdateFallingCarryActors(
		FIceGlacierCollapseTargetState& TargetState,
		float FallAlpha);

	/** Falling 진입 순간 해당 빙판 위에 있는 Carry 대상 Actor를 저장한다. */
	void InitializeCarryActorsForFallingPiece(
		FIceGlacierCollapseTargetState& TargetState,
		const FVector& CurrentPieceLocation);

	/** Falling 중 저장된 Carry 대상 Actor를 빙판 Delta만큼 이동시킨다. */
	void MoveCarriedActorsWithFallingPiece(
		FIceGlacierCollapseTargetState& TargetState,
		const FVector& CurrentPieceLocation);

	/** Falling 종료 또는 재초기화 시 Carry 런타임 상태를 정리한다. */
	void ClearCarryActors(FIceGlacierCollapseTargetState& TargetState);

	/** TargetActor의 현재 Primitive Bounds를 XY 사전 필터용으로 합산한다. */
	bool CalculateTargetActorBounds(
		const AActor* TargetActor,
		FBox& OutBounds) const;

	/** 후보 Actor가 Carry 가능한 현재 gameplay Actor인지 확인한다. */
	bool ShouldCarryCandidateActor(const AActor* CandidateActor) const;

	/** Falling 중 Carry를 계속 적용해도 되는지 확인한다. */
	bool ShouldKeepCarryingActor(const AActor* CandidateActor) const;

	/** 후보 Actor의 하단 근처에서 아래 방향 Trace를 시작할 위치를 구한다. */
	FVector GetCarryTraceStartLocation(const AActor* CandidateActor) const;

	/** 아래 방향 RayTrace로 후보 Actor가 현재 TargetActor 위에 있는지 확인한다. */
	bool IsCandidateStandingOnTargetActorByTrace(
		const AActor* CandidateActor,
		const AActor* TargetActor) const;

	/** 저장된 시작 Transform으로 빙판 위치와 회전을 되돌린다. */
	void ApplyInitialTransform(FIceGlacierCollapseTargetState& TargetState);

	/** 로컬 플레이어가 Warning 중인 빙판 위에 있는지 확인하고 Camera Shake 상태를 갱신한다. */
	void UpdateLocalWarningCameraShake(float RoundElapsedSeconds);

	/** WarningAlpha를 Camera Shake에 적용할 최종 Scale로 변환한다. */
	float CalculateWarningCameraShakeScale(float WarningAlpha) const;

	/** 로컬 플레이어가 현재 밟고 있는 Warning Piece의 TargetActor와 진행도를 찾는다. */
	AActor* FindWarningPieceUnderLocalPlayer(
		float RoundElapsedSeconds,
		float& OutWarningAlpha) const;

	/** 로컬 Character의 MovementBase Owner가 지정 TargetActor인지 확인한다. */
	bool IsLocalCharacterStandingOnTargetActor(
		const ACharacter* Character,
		const AActor* TargetActor) const;

	/** 로컬 플레이어 Camera Shake를 한 번 시작한다. */
	void StartLocalWarningCameraShake(AActor* TargetActor, float WarningAlpha);

	/** 실행 중인 로컬 플레이어 Camera Shake를 정지하고 상태를 초기화한다. */
	void StopLocalWarningCameraShake();

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

	TWeakObjectPtr<AActor> ActiveWarningCameraShakeTargetActor;
	TWeakObjectPtr<UCameraShakeBase> ActiveWarningCameraShakeInstance;
};

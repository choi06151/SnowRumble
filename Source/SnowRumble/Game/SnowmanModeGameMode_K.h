// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SnowmanModeGameMode_K.generated.h"

class APlayerController;
class AController;
class AActor;

UCLASS()
class SNOWRUMBLE_API ASnowmanModeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASnowmanModeGameMode();

	virtual void InitGame(
		const FString& MapName,
		const FString& Options,
		FString& ErrorMessage) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void HandleStartingNewPlayer_Implementation(
		APlayerController* NewPlayer) override;

	virtual AActor* ChoosePlayerStart_Implementation(
		AController* Player) override;

	virtual void RestartPlayerAtPlayerStart(
		AController* NewPlayer,
		AActor* StartSpot) override;

protected:
	/** 눈사람 모드 제한시간이다. K-14 전까지 승패 없이 시간 상태만 제공한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman", meta = (ClampMin = "0.0"))
	float SnowmanModeTimeLimitSeconds = 600.0f;

	/** 눈사람 모드 스폰 후 실제 시작까지 기다릴 카운트다운 시간이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman", meta = (ClampMin = "0.0"))
	float SnowmanModeStartCountdownSeconds = 3.0f;

private:
	/** 대기방에서 전달받은 이번 모드 예상 접속 인원 수다. */
	int32 ExpectedPlayerCount = 0;

	bool bLoadingScreensDismissed = false;
	bool bSnowmanTimerStarted = false;

	/** 모든 예상 플레이어가 접속하면 전체 클라이언트의 로딩창을 닫는다. */
	void TryDismissLoadingScreens();

	/** 로딩이 끝난 뒤 눈사람 모드 시작 카운트다운을 시작한다. */
	void StartSnowmanModeCountdownAfterLoading();

	/** 시작 카운트다운이 끝난 뒤 눈사람 모드 제한시간을 시작한다. */
	void StartSnowmanModeAfterCountdown();

	/** 시작 카운트다운 동안 모든 플레이어 이동과 시야 입력을 잠그거나 해제한다. */
	void ApplySnowmanModeStartInputLock(bool bShouldLockInput);

	/** 현재 눈사람 모드 레벨 접속 진행률을 접속 완료된 클라이언트들에게 보낸다. */
	void BroadcastLoadingProgress();

	/** 이번 눈사람 모드에서 이미 선택한 PlayerStart다. */
	TSet<TWeakObjectPtr<AActor>> UsedPlayerStarts;

	/** 이번 눈사람 모드에서 이미 확정한 실제 스폰 위치다. */
	TArray<FVector> UsedSpawnLocations;

	/** PlayerStart 주변에서 실제 Pawn 생성 위치를 넓게 분산할 반경이다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Spawn", meta = (ClampMin = "0.0"))
	float PlayerStartSpawnScatterRadius = 900.0f;

	/** 같은 PlayerStart 군집 안에서 플레이어끼리 확보할 최소 간격이다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Spawn", meta = (ClampMin = "0.0"))
	float PlayerStartSpawnMinimumSpacing = 240.0f;

	/** PlayerStart 주변 랜덤 스폰 위치를 찾기 위해 시도할 횟수다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Spawn", meta = (ClampMin = "1"))
	int32 PlayerStartSpawnScatterAttempts = 24;

	/** 선택된 PlayerStart를 기준으로 실제 Pawn 생성 transform을 만든다. */
	FTransform BuildScatteredPlayerStartTransform(
		const AActor* StartSpot) const;

	/** 이번 모드에서 이미 확정한 스폰 위치와 충분히 떨어져 있는지 확인한다. */
	bool IsSpawnLocationFarEnough(const FVector& CandidateLocation) const;
};

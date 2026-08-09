// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SnowRumbleGameMode.generated.h"

class APlayerController;
class AActor;
class APawn;
enum class ESnowRumbleTeam : uint8;

UCLASS()
class SNOWRUMBLE_API ASnowRumbleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASnowRumbleGameMode();

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

	/** 서버가 현재 라운드 팀 전멸 조건을 다시 검사한다. */
	void EvaluateRoundEndCondition();

	/** 플레이어 얼음/사망 상태 변경 시 라운드 종료 조건을 다시 검사한다. */
	UFUNCTION()
	void HandlePlayerLifeStateChanged(bool bUnused);

private:
	/** 대기방에서 전달받은 이번 매치 예상 접속 인원 수다. */
	int32 ExpectedPlayerCount = 0;

	bool bLoadingScreensDismissed = false;
	bool bStartCountdownStarted = false;

	/** PvP 맵 시작 후 입력을 잠글 카운트다운 시간이다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Match", meta = (ClampMin = "0.0"))
	float MatchStartCountdownSeconds = 3.0f;

	/** 로딩창 제거와 HUD 생성이 반영된 뒤 카운트다운을 시작하기 위한 지연이다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Match", meta = (ClampMin = "0.0"))
	float MatchStartCountdownDelaySeconds = 3.0f;

	/** 모든 예상 플레이어가 접속하면 전체 클라이언트의 로딩창을 닫는다. */
	void TryDismissLoadingScreens();

	/** PvP 맵 로딩이 끝난 뒤 서버 확정 시작 카운트다운을 시작한다. */
	void StartMatchCountdownAfterLoading();

	/** 새로 생성된 Pawn의 생명 상태 변경을 라운드 종료 검사에 연결한다. */
	void BindPawnLifeState(APawn* Pawn);

	/** 현재 팀이 라운드 참가 팀으로 유효한지 확인한다. */
	bool IsValidRoundTeam(ESnowRumbleTeam Team) const;

	/** 현재 PvP 레벨 접속 진행률을 접속 완료된 클라이언트들에게 보낸다. */
	void BroadcastLoadingProgress();

	/** 이번 PvP 매치에서 이미 선택한 PlayerStart다. */
	TSet<TWeakObjectPtr<AActor>> UsedPlayerStarts;

	/** 이번 PvP 매치에서 이미 확정한 실제 스폰 위치다. */
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

	/** 이번 매치에서 이미 확정한 스폰 위치와 충분히 떨어져 있는지 확인한다. */
	bool IsSpawnLocationFarEnough(const FVector& CandidateLocation) const;
};

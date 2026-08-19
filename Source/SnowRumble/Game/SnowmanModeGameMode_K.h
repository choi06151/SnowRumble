// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SnowmanModeGameMode_K.generated.h"

class APlayerController;
class AController;
class AActor;
class ASnowRumbleCharacter;
class ASnowRumblePlayerState;
class ASnowmanModeSnowmanCharacter;

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

	virtual void HandleSeamlessTravelPlayer(AController*& C) override;

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

	/** 접촉 감염 뒤 눈사람으로 확정되기까지 걸리는 시간이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Infection", meta = (ClampMin = "0.0"))
	float InfectionPendingSeconds = 10.0f;

	/** 눈사람으로 확정된 플레이어의 이동 속도 배율이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Movement", meta = (ClampMin = "1.0"))
	float SnowmanMovementSpeedMultiplier = 1.25f;

	/** 눈사람 속도 배율 계산에 사용할 일반 플레이어 기준 이동 속도다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Movement", meta = (ClampMin = "0.0"))
	float NormalPlayerReferenceWalkSpeed = 500.0f;

	/** 눈사람으로 전환될 때 스폰할 전용 캐릭터 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Pawn")
	TSubclassOf<ASnowmanModeSnowmanCharacter> SnowmanCharacterClass;

	/** 눈사람 접촉 감염을 확정할 거리 판정 반경이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Infection", meta = (ClampMin = "0.0"))
	float InfectionContactRadius = 140.0f;

	/** 서버가 접촉 감염 상태를 갱신하는 주기다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Infection", meta = (ClampMin = "0.01"))
	float InfectionScanIntervalSeconds = 0.1f;

	/** 접촉 감염 후보와 판정 결과를 서버 로그에 출력한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Debug")
	bool bLogSnowmanInfectionDebug = true;

private:
	/** 대기방에서 전달받은 이번 모드 예상 접속 인원 수다. */
	int32 ExpectedPlayerCount = 0;

	bool bLoadingScreensDismissed = false;
	bool bSnowmanTimerStarted = false;
	bool bSnowmanRolesInitialized = false;

	/** 모든 예상 플레이어가 접속하면 전체 클라이언트의 로딩창을 닫는다. */
	void TryDismissLoadingScreens();

	/** 로딩이 끝난 뒤 눈사람 모드 시작 카운트다운을 시작한다. */
	void StartSnowmanModeCountdownAfterLoading();

	/** 시작 카운트다운이 끝난 뒤 눈사람 모드 제한시간을 시작한다. */
	void StartSnowmanModeAfterCountdown();

	/** 접속자를 초기화하고 시작 눈사람을 무작위로 선택한다. */
	void InitializeSnowmanRoles();

	/** 시작 눈사람 초기화가 아직 불가능하면 짧게 지연해 재시도한다. */
	void ScheduleSnowmanRoleInitializationRetry();

	/** 감염 대기 완료와 눈사람 접촉 감염을 서버에서 처리한다. */
	void UpdateSnowmanInfectionFlow();

	/** 현재 눈사람 역할에 맞춰 모든 플레이어 이동 속도를 갱신한다. */
	void ApplySnowmanMovementSpeeds();

	/** 지정 플레이어를 눈사람 전용 Pawn으로 교체한다. */
	bool ConvertPlayerToSnowmanPawn(ASnowRumblePlayerState* PlayerState);

	/** 눈사람 전용 Pawn의 이동 속도를 모드 설정에 맞춘다. */
	void ApplySnowmanMovementSpeed(ASnowRumbleCharacter* Character);

	/** 현재 GameState의 SnowRumble PlayerState 목록을 수집한다. */
	TArray<ASnowRumblePlayerState*> CollectSnowmanPlayerStates() const;

	/** 지정 PlayerState가 소유한 캐릭터를 찾는다. */
	ASnowRumbleCharacter* FindCharacterForPlayerState(
		const ASnowRumblePlayerState* PlayerState) const;

	/** Pawn과 Controller 중 더 안정적인 경로로 캐릭터 소유 PlayerState를 찾는다. */
	ASnowRumblePlayerState* ResolvePlayerStateForCharacter(
		const ASnowRumbleCharacter* Character) const;

	/** 시작 카운트다운 동안 모든 플레이어 이동과 시야 입력을 잠그거나 해제한다. */
	void ApplySnowmanModeStartInputLock(bool bShouldLockInput);

	/** 현재 눈사람 모드 레벨 접속 진행률을 접속 완료된 클라이언트들에게 보낸다. */
	void BroadcastLoadingProgress();

	/** 이번 눈사람 모드에서 이미 선택한 PlayerStart다. */
	TSet<TWeakObjectPtr<AActor>> UsedPlayerStarts;

	/** 이번 눈사람 모드에서 이미 확정한 실제 스폰 위치다. */
	TArray<FVector> UsedSpawnLocations;

	FTimerHandle InfectionScanTimerHandle;
	FTimerHandle SnowmanRoleInitializationRetryTimerHandle;

	double LastInfectionDebugSummaryTime = -1.0;

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

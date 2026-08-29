// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SnowmanModeGameMode_K.generated.h"

class APlayerController;
class AController;
class AActor;
class ASnowRumbleCharacter;
class ASnowRumbleLobbyGameMode;
class ASnowRumblePlayerState;
class ASnowmanModeSnowmanCharacter;
class ASnowballItem;
class UTimedDropAnnouncementWidget;
enum class ESnowmanModeResult : uint8;
enum class ESnowRumbleTeam : uint8;
class USoundBase;

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

	/** URL 옵션에서 읽은 눈사람 모드 라운드 정보를 GameState 복제 상태로 넘긴다. */
	virtual void InitGameState() override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void HandleSeamlessTravelPlayer(AController*& C) override;

	virtual void HandleStartingNewPlayer_Implementation(
		APlayerController* NewPlayer) override;

	virtual AActor* ChoosePlayerStart_Implementation(
		AController* Player) override;

	virtual void RestartPlayerAtPlayerStart(
		AController* NewPlayer,
		AActor* StartSpot) override;

	/** 초기 역할이 눈사람인 플레이어는 일반 Pawn 대신 눈사람 Pawn 클래스로 스폰한다. */
	virtual UClass* GetDefaultPawnClassForController_Implementation(
		AController* InController) override;

protected:
	/** 눈사람 모드에서 재생할 배경음악이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Audio")
	TObjectPtr<USoundBase> BackgroundMusicSound;

	/** 눈사람 모드 제한시간이다. K-14 전까지 승패 없이 시간 상태만 제공한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman", meta = (ClampMin = "0.0"))
	float SnowmanModeTimeLimitSeconds = 600.0f;

	/** 눈사람 모드에서도 PvP와 같은 큰 눈덩이 낙하 이벤트를 사용할지 정한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Snowball")
	bool bEnableFallingSnowballEvent = true;

	/** 눈사람 모드 시작 후 첫 낙하 이벤트까지 기다리는 시간이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Snowball", meta = (ClampMin = "0.0"))
	float FirstFallingSnowballDelaySeconds = 30.0f;

	/** 눈사람 모드에서 낙하 이벤트가 반복되는 간격이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Snowball", meta = (ClampMin = "0.1"))
	float FallingSnowballEventIntervalSeconds = 20.0f;

	/** 한 번의 눈덩이 낙하 이벤트에서 생성할 큰 눈덩이 수다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Snowball", meta = (ClampMin = "0"))
	int32 FallingSnowballCount = 10;

	/** 낙하 이벤트에 사용할 큰 눈덩이 Blueprint 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Snowball")
	TSubclassOf<ASnowballItem> FallingSnowballClass;

	/** 큰 눈덩이 낙하 시작 시 표시할 PvP와 동일한 알림 WBP다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Snowball")
	TSubclassOf<UTimedDropAnnouncementWidget> FallingSnowballAnnouncementWidgetClass;

	/** 큰 눈덩이 낙하 알림 WBP가 화면에 유지되는 시간이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Snowball", meta = (ClampMin = "0.1"))
	float FallingSnowballAnnouncementDisplayDurationSeconds = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Snowball")
	FSoftClassPath DefaultFallingSnowballClassPath =
		FSoftClassPath(TEXT("/Game/Snowball/BP_SnowballItem.BP_SnowballItem_C"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Snowball", meta = (ClampMin = "0.0"))
	float FallingSnowballDamage = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Snowball", meta = (ClampMin = "0.0"))
	float FallingSnowballScatterRadius = 650.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Snowball", meta = (ClampMin = "0.0"))
	float FallingSnowballHeightOffset = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Snowball", meta = (ClampMin = "0.0"))
	float FallingSnowballSpawnIntervalSeconds = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Snowball", meta = (ClampMin = "0.0"))
	float FallingSnowballHorizontalSpeedMin = 80.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Falling Snowball", meta = (ClampMin = "0.0"))
	float FallingSnowballHorizontalSpeedMax = 280.0f;

	/** 눈사람 모드 스폰 후 실제 시작까지 기다릴 카운트다운 시간이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman", meta = (ClampMin = "0.0"))
	float SnowmanModeStartCountdownSeconds = 3.0f;

	/** 로딩창을 닫은 뒤 역할 초기화·인트로를 시작하기 전 PvP와 동일하게 기다릴 시간이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman", meta = (ClampMin = "0.0"))
	float SnowmanModeStartCountdownDelaySeconds = 5.0f;

	/** 첫 라운드 시작 팀 소개 인트로에서 한 팀을 보여줄 시간이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Intro", meta = (ClampMin = "0.0"))
	float SnowmanModeIntroTeamShotSeconds = 2.5f;

	/** 이전 Pending 감염 BP 호환용 값이다. 현재 접촉 감염은 즉시 전환된다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Infection", meta = (ClampMin = "0.0"))
	float InfectionPendingSeconds = 0.0f;

	/** 눈사람으로 확정된 플레이어의 이동 속도 배율이다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnowRumble|Snowman|Tuning|Movement",
		meta = (ClampMin = "1.0", DisplayName = "Snowman Movement Speed Multiplier"))
	float SnowmanMovementSpeedMultiplier = 2.3f;

	/** 눈사람 속도 배율 계산에 사용할 일반 플레이어 기준 이동 속도다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnowRumble|Snowman|Tuning|Movement",
		meta = (ClampMin = "0.0", DisplayName = "Normal Player Reference Walk Speed"))
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

	/** 스폰 또는 눈사람 전환 직후 접촉 감염을 무시할 시간이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Infection", meta = (ClampMin = "0.0"))
	float SpawnInfectionGraceSeconds = 3.0f;

	/** 접촉 감염 후보와 판정 결과를 서버 로그에 출력한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Debug")
	bool bLogSnowmanInfectionDebug = true;

	/** 눈사람 모드 결과를 보여준 뒤 포디움으로 이동하기까지 기다릴 시간이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Result", meta = (ClampMin = "0.0"))
	float SnowmanModeResultLobbyReturnDelaySeconds = 5.0f;

	/** 눈사람 모드 종료 후 이동할 전용 포디움 맵 travel URL이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Result")
	FString PodiumTravelUrl = TEXT("/Game/Maps/L_Podium_Snowman_K?listen");

	/** 눈사람 모드 종료 후 복귀할 로비 맵 travel URL이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Result")
	FString LobbyReturnTravelUrl = TEXT("/Game/Maps/L_Lobby?listen");

	/** 로비 복귀 시 이전 눈사람 GameMode travel 옵션이 남지 않도록 명시할 GameMode다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Result")
	TSubclassOf<ASnowRumbleLobbyGameMode> LobbyReturnGameModeClass;

	FString BuildNextRoundTravelUrl();
private:
	/** 대기방에서 전달받은 이번 모드 예상 접속 인원 수다. */
	int32 ExpectedPlayerCount = 0;
	int32 TotalMatchRounds = 1; // 총 설정된 판 수 (1, 3, 5 등)
	int32 CurrentRoundIndex = 1; // 현재 진행 중인 라운드 번호

	bool bLoadingScreensDismissed = false;
	bool bSnowmanTimerStarted = false;
	bool bSnowmanRolesInitialized = false;
	bool bSnowmanIntroStarted = false;
	int32 SnowmanIntroTeamIndex = 0;
	TArray<ESnowRumbleTeam> SnowmanIntroTeams;

	/** 모든 예상 플레이어가 접속하면 인트로 시작 대기를 예약한다. */
	void TryDismissLoadingScreens();

	/** PvP처럼 시작 인트로 직전에 전체 클라이언트의 로딩창을 닫는다. */
	void HideLoadingScreensBeforeIntro();

	/** 로딩이 끝난 뒤 눈사람 모드 시작 카운트다운을 시작한다. */
	void StartSnowmanModeCountdownAfterLoading();

	/** 로딩 종료 후 시작 대기가 끝나면 역할 초기화와 인트로를 시작한다. */
	void StartSnowmanModeIntroAfterLoading();

	/** 역할 초기화가 끝난 뒤 PvP와 같은 팀 소개 인트로를 시작한다. */
	void StartSnowmanModeIntroSequence();

	/** 현재 라운드에서 소개할 팀을 고정 순서로 수집한다. */
	void GetActiveSnowmanModeTeams(TArray<ESnowRumbleTeam>& OutTeams) const;

	/** 눈사람 모드 팀 소개 인트로의 다음 장면을 재생한다. */
	void AdvanceSnowmanModeIntroSequence();

	/** 팀 소개 인트로를 종료하고 눈사람 모드 카운트다운을 시작한다. */
	void FinishSnowmanModeIntroSequence();

	/** 시작 카운트다운이 끝난 뒤 눈사람 모드 제한시간을 시작한다. */
	void StartSnowmanModeAfterCountdown();

	/** 눈사람 모드에서 PvP와 동일한 큰 눈덩이 낙하 이벤트를 시작한다. */
	void ScheduleFallingSnowballEvent(float DelaySeconds);
	void SpawnFallingSnowballEvent();
	void SpawnNextFallingSnowball();
	void BroadcastFallingSnowballAnnouncement() const;

	/** 접속자를 초기화하고 시작 눈사람을 무작위로 선택한다. */
	void InitializeSnowmanRoles();

	/** 눈사람 모드 시작 시 모든 플레이어에게 실제 장갑·부츠 아이템 효과를 적용한다. */
	void GrantSnowmanModeStartingItems();

	/** 시작 눈사람 초기화가 아직 불가능하면 짧게 지연해 재시도한다. */
	void ScheduleSnowmanRoleInitializationRetry();

	/** 눈사람 접촉 감염을 서버에서 즉시 눈사람 전환으로 처리한다. */
	void UpdateSnowmanInfectionFlow();

	/** 현재 눈사람 역할에 맞춰 모든 플레이어 이동 속도를 갱신한다. */
	void ApplySnowmanMovementSpeeds();

	/** 현재 눈사람 역할과 제한시간 기준으로 모드 종료 조건을 확인한다. */
	void EvaluateSnowmanModeEndCondition();

	/** 눈사람 모드 제한시간이 끝났을 때 생존자 승리를 확정한다. */
	void HandleSnowmanModeTimeLimitExpired();

	/** 서버에서 눈사람 모드 결과를 확정하고 후속 흐름을 예약한다. */
	void EndSnowmanMode(ESnowmanModeResult Result);

	/** 결과 표시 시간이 지난 뒤 전용 포디움으로 이동한다. */
	void TravelToPodiumAfterSnowmanModeEnd();

	/** 포디움 이동이 불가능하면 로비로 복귀한다. */
	void ReturnToLobbyAfterSnowmanModeEnd();

	/** 로비 복귀용 travel URL을 절대 이동 기준으로 안전하게 구성한다. */
	FString BuildLobbyReturnTravelUrl() const;

	/** 포디움 travel URL에 결과와 승자 정보를 붙인다. */
	FString BuildPodiumTravelUrl(ESnowmanModeResult Result);

	/** 포디움에서 배치할 승자 PlayerId 목록을 URL 옵션 값으로 만든다. */
	FString BuildWinnerPlayerIdsOption(ESnowmanModeResult Result) const;

	/** 포디움에서 PlayerId가 바뀐 승자를 보조 매칭할 플레이어 이름 목록을 만든다. */
	FString BuildWinnerPlayerNamesOption(ESnowmanModeResult Result) const;

	/** 지정 플레이어를 눈사람 전용 Pawn으로 교체한다. */
	bool ConvertPlayerToSnowmanPawn(ASnowRumblePlayerState* PlayerState);

	/** 눈사람 전용 Pawn의 이동 속도를 모드 설정에 맞춘다. */
	void ApplySnowmanMovementSpeed(ASnowRumbleCharacter* Character);

	/** 현재 GameState의 SnowRumble PlayerState 목록을 수집한다. */
	TArray<ASnowRumblePlayerState*> CollectSnowmanPlayerStates() const;

	/** 지정 플레이어에게 스폰 직후 감염 면역 시간을 부여한다. */
	void GrantSpawnInfectionGrace(ASnowRumblePlayerState* PlayerState);

	/** 현재 접촉 감염 면역 시간 안에 있는지 확인한다. */
	bool IsSpawnInfectionGraceActive(
		const ASnowRumblePlayerState* PlayerState) const;

	/** 현재 참가자 전체에게 감염 면역 시간을 부여한다. */
	void GrantSpawnInfectionGraceToAllPlayers();

	/** 현재 로컬 클라이언트에 눈사람 모드 배경음악을 재생하도록 지시한다. */
	void BroadcastBackgroundMusic() const;

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

	/** PlayerState별 접촉 감염 면역 종료 월드 시간이다. */
	TMap<TWeakObjectPtr<ASnowRumblePlayerState>, double>
		SpawnInfectionGraceEndTimes;
	
	/** 같은 플레이어에게 눈사람 Pawn 전환이 중복 실행되지 않도록 막는다. */
	TSet<TWeakObjectPtr<ASnowRumblePlayerState>> ConvertingSnowmanPlayerStates;

	FTimerHandle InfectionScanTimerHandle;
	FTimerHandle SnowmanRoleInitializationRetryTimerHandle;
	FTimerHandle SnowmanIntroTimerHandle;
	FTimerHandle SnowmanModeTimeLimitTimerHandle;
	FTimerHandle SnowmanModeLobbyReturnTimerHandle;
	FTimerHandle FallingSnowballEventTimerHandle;
	FTimerHandle FallingSnowballSpawnTimerHandle;
	int32 RemainingFallingSnowballs = 0;

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

	/** 후보 위치 아래 바닥을 찾아 Pawn 캡슐 높이에 맞는 스폰 위치로 보정한다. */
	bool TryResolveSpawnLocationOnGround(
		const FVector& CandidateLocation,
		FVector& OutSpawnLocation) const;

	/** 후보 스폰 위치에 Pawn 캡슐을 놓을 수 있는지 확인한다. */
	bool IsSpawnCapsuleClear(const FVector& SpawnLocation) const;

	/** 기본 인간 Pawn과 눈사람 Pawn 중 더 큰 캡슐 크기를 스폰 검사 기준으로 반환한다. */
	void GetDefaultPawnCapsuleSize(
		float& OutCapsuleRadius,
		float& OutCapsuleHalfHeight) const;

	/** 이번 모드에서 이미 확정한 스폰 위치와 충분히 떨어져 있는지 확인한다. */
	bool IsSpawnLocationFarEnough(const FVector& CandidateLocation) const;
};

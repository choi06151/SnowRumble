// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SnowRumbleGameMode.generated.h"

class APlayerController;
class AActor;
class APawn;
class AGiftBox;
class ASnowRumbleCharacter;
enum class ESnowRumbleTeam : uint8;
enum class ESnowRumbleGiftBoxGrade : uint8;

UCLASS()
class SNOWRUMBLE_API ASnowRumbleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASnowRumbleGameMode();

	virtual void BeginPlay() override;

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

	/** 서버가 현재 라운드 팀 전멸 조건을 다시 검사한다. */
	void EvaluateRoundEndCondition();

	/** 플레이어 얼음/사망 상태 변경 시 라운드 종료 조건을 다시 검사한다. */
	UFUNCTION()
	void HandlePlayerLifeStateChanged(bool bUnused);

	/** 맵 Blueprint가 실제 축소 완료 시 호출할 수 있는 완료 신호다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Map Pressure")
	void CompleteMapShrinkFromBlueprint();

protected:
	/** 서버가 맵 축소 시점에 맵 Blueprint로 넘기는 이벤트다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Map Pressure")
	void OnMapShrinkRequested(
		int32 ShrinkStage,
		float RoundElapsedSeconds,
		float ShrinkDurationSeconds);

private:
	/** 대기방에서 전달받은 이번 매치 예상 접속 인원 수다. */
	int32 ExpectedPlayerCount = 0;

	bool bLoadingScreensDismissed = false;
	bool bLoadingScreensHidden = false;
	bool bStartCountdownStarted = false;
	bool bMatchIntroStarted = false;
	int32 MatchIntroTeamIndex = 0;

	/** PvP 맵 시작 후 입력을 잠글 카운트다운 시간이다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Match", meta = (ClampMin = "0.0"))
	float MatchStartCountdownSeconds = 3.0f;

	/** 로딩창 제거와 HUD 생성이 반영된 뒤 카운트다운을 시작하기 위한 지연이다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Match", meta = (ClampMin = "0.0"))
	float MatchStartCountdownDelaySeconds = 3.0f;

	/** PvP 시작 전 팀 소개에서 한 팀을 보여줄 시간이다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Match Intro", meta = (ClampMin = "0.0"))
	float MatchIntroTeamShotSeconds = 2.5f;

	/** 라운드 종료 결과를 보여준 뒤 다음 라운드 맵으로 이동하기까지 기다릴 시간이다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Match", meta = (ClampMin = "0.0"))
	float NextRoundTravelDelaySeconds = 5.0f;

	/** 매치 종료 결과를 보여준 뒤 로비로 복귀하기까지 기다릴 시간이다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Match", meta = (ClampMin = "0.0"))
	float MatchEndLobbyReturnDelaySeconds = 5.0f;

	/** 매치 종료 후 복귀할 로비 맵 travel URL이다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Match")
	FString LobbyReturnTravelUrl = TEXT("/Game/Maps/L_Lobby?listen");

	/** 공동 1등 발생 시 이동할 단판 승부 전용 PvP 맵 travel URL이다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Match")
	FString TiebreakerTravelUrl = TEXT("/Game/Maps/L_Tiebreaker?listen");

	/** 매치 종료 후 포디엄(시상대)으로 이동할 레벨 travel URL이다. 에디터에서 L_Podium을 준비해야 함. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Match")
	FString PodiumTravelUrl = TEXT("/Game/Maps/L_Podium?listen");

	/** 매치 종료 후 포디엄으로 이동하고 배치하기까지의 대기 시간(초) */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Match", meta = (ClampMin = "0.0"))
	float PodiumTravelDelaySeconds = 3.0f;

	/** 실제 맵 축소 완료 신호가 오기 전 임시로 축소 완료를 가정할 시간이다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Map Pressure", meta = (ClampMin = "0.0"))
	float TemporaryMapShrinkDurationSeconds = 5.0f;

	/** 모든 예상 플레이어가 접속하면 전체 클라이언트의 로딩창을 닫는다. */
	void TryDismissLoadingScreens();

	/** PvP 맵 로딩이 끝난 뒤 서버 확정 시작 카운트다운을 시작한다. */
	void StartMatchCountdownAfterLoading();

	/** PvP 맵 로딩이 끝난 뒤 팀 소개 인트로를 시작한다. */
	void StartMatchIntroAfterLoading();

	/** 현재 PvP 진입에서 팀 소개 시퀀스를 재생해야 하는지 반환한다. */
	bool ShouldPlayMatchIntroSequence() const;

	/** 다음 팀 소개 카메라 샷을 모든 클라이언트에 지시한다. */
	void AdvanceMatchIntroSequence();

	/** 팀 소개가 끝난 뒤 기존 시작 카운트다운을 확정한다. */
	void FinishMatchIntroSequence();

	/** 모든 클라이언트의 PvP 로딩창을 닫는다. */
	void HideLoadingScreensBeforeIntro();

	/** 기존 C-17 시작 카운트다운과 경기 타이머들을 시작한다. */
	void StartConfirmedMatchCountdown();

	/** 라운드 결과 표시 후 남은 라운드가 있으면 다음 PvP 맵으로 이동한다. */
	void TravelToNextRoundIfNeeded();

	/** 공동 1등 단판 승부 맵으로 이동한다. */
	void TravelToTiebreakerRound();

	/** 매치 종료 후 포디엄으로 서버가 이동하는 함수(헤더에 선언되어야 함). */
	void TravelToPodiumAfterMatchEnd();

	/** 다음 맵 축소 타이머를 예약한다. */
	void ScheduleNextMapShrink();

	/** 다음 선물상자 스폰 타이머를 예약한다. */
	void ScheduleNextGiftBoxSpawn(float DelaySeconds);

	/** 서버가 맵 축소를 시작하고 Blueprint 이벤트를 호출한다. */
	void TriggerMapShrink();

	/** 서버가 TargetPoint 후보 중 하나에 선물상자를 공중 스폰한다. */
	void SpawnGiftBox();

	/** 맵 축소 완료 상태를 확정하고 다음 축소를 예약한다. */
	void CompleteMapShrinkFromServer();

	/** 매치 결과 표시 후 로비로 복귀한다. */
	void ReturnToLobbyAfterMatchEnd();

	/** 공용 PvP travel URL에 listen과 ExpectedPlayers 옵션을 붙인다. */
	FString BuildPvPTravelUrl(const FString& BaseTravelUrl) const;

	/** 새로 생성된 Pawn의 생명 상태 변경을 라운드 종료 검사에 연결한다. */
	void BindPawnLifeState(APawn* Pawn);

	/** 새로 얼거나 죽은 플레이어를 찾아 이벤트 로그로 알린다. */
	void RefreshPlayerLifeEventLogStates();

	/** 모든 클라이언트의 이벤트 로그 UI에 메시지를 보낸다. */
	void BroadcastEventLogMessage(const FText& Message) const;

	/** 모든 클라이언트의 개인 알림 UI에 메시지를 보낸다. */
	void BroadcastPersonalTextAlarm(const FText& Message) const;

	/** 이벤트 로그에 표시할 플레이어 이름을 반환한다. */
	FString GetEventLogPlayerName(const ASnowRumbleCharacter* Character) const;

	/** 맵에 배치된 선물상자 스폰용 TargetPoint 후보를 수집한다. */
	void GetGiftBoxSpawnPointCandidates(TArray<AActor*>& OutCandidates) const;

	/** 설정된 선물상자 클래스가 없으면 기본 BP 경로를 시도해 반환한다. */
	TSubclassOf<AGiftBox> ResolveGiftBoxClass() const;

	/** 이번 스폰에서 사용할 선물상자 등급을 서버가 결정한다. */
	ESnowRumbleGiftBoxGrade ChooseGiftBoxGrade() const;

	/** 현재 팀이 라운드 참가 팀으로 유효한지 확인한다. */
	bool IsValidRoundTeam(ESnowRumbleTeam Team) const;

	/** 현재 라운드에 실제 참가 중인 팀을 고정 표시 순서로 수집한다. */
	void GetActiveRoundTeams(TArray<ESnowRumbleTeam>& OutTeams) const;

	/** 현재 PvP 레벨 접속 진행률을 접속 완료된 클라이언트들에게 보낸다. */
	void BroadcastLoadingProgress();

	/** 현재 GameInstance에 저장된 PvP 매치 상태를 반환한다. */
	class USnowRumbleMatchSubsystem* GetMatchSubsystem() const;

	/** 이번 PvP 매치에서 이미 선택한 PlayerStart다. */
	TSet<TWeakObjectPtr<AActor>> UsedPlayerStarts;

	/** 이번 PvP 매치에서 이미 확정한 실제 스폰 위치다. */
	TArray<FVector> UsedSpawnLocations;

	TArray<ESnowRumbleTeam> MatchIntroTeams;

	struct FTrackedLifeState
	{
		bool bFrozen = false;
		bool bDead = false;
	};

	TMap<TWeakObjectPtr<ASnowRumbleCharacter>, FTrackedLifeState>
		TrackedLifeStates;

	/** 현재 라운드의 맵 축소 단계 번호다. */
	int32 MapShrinkStage = 0;

	FTimerHandle MapShrinkTimerHandle;
	FTimerHandle MapShrinkCompletionTimerHandle;
	FTimerHandle GiftBoxSpawnTimerHandle;
	FTimerHandle MatchIntroTimerHandle;

	/** 서버가 스폰할 선물상자 Blueprint 클래스다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Item|Gift Box")
	TSubclassOf<AGiftBox> GiftBoxClass;

	/** GiftBoxClass가 비어 있을 때 임시로 로드할 기본 Blueprint 경로다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Item|Gift Box")
	FSoftClassPath DefaultGiftBoxClassPath =
		FSoftClassPath(TEXT("/Game/Blueprints/BP_GiftBox.BP_GiftBox_C"));

	/** 첫 선물상자가 라운드 시작 후 떨어지기까지의 시간이다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Item|Gift Box", meta = (ClampMin = "0.0"))
	float FirstGiftBoxSpawnDelaySeconds = 30.0f;

	/** 첫 상자 이후 선물상자가 반복 생성되는 간격이다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Item|Gift Box", meta = (ClampMin = "0.1"))
	float GiftBoxSpawnIntervalSeconds = 60.0f;

	/** TargetPoint 위치보다 얼마나 위에서 선물상자를 떨어뜨릴지 정한다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Item|Gift Box", meta = (ClampMin = "0.0"))
	float GiftBoxSpawnHeightOffset = 800.0f;

	/** TargetPoint 주변에서 선물상자 낙하 시작 위치를 랜덤하게 분산할 반경이다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Item|Gift Box", meta = (ClampMin = "0.0"))
	float GiftBoxSpawnScatterRadius = 450.0f;

	/** 이 태그가 붙은 TargetPoint만 우선 사용한다. 없으면 맵의 모든 TargetPoint를 사용한다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Item|Gift Box")
	FName GiftBoxSpawnPointTag = TEXT("GiftBoxSpawn");

	/** 일반 반복 스폰에서 황금 상자가 나올 확률이다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Item|Gift Box", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GoldGiftBoxSpawnChance = 0.15f;

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
	bool TryResolveSpawnLocationOnGround(
		const FVector& CandidateLocation,
		FVector& OutSpawnLocation) const;

	bool IsSpawnCapsuleClear(const FVector& SpawnLocation) const;

	void GetDefaultPawnCapsuleSize(
		float& OutCapsuleRadius,
		float& OutCapsuleHalfHeight) const;

	bool IsSpawnLocationFarEnough(const FVector& CandidateLocation) const;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SnowmanModeGameState_K.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"
#include "SnowmanModePodiumGameMode_K.generated.h"

class ACameraActor;
class APlayerStart;
class ASnowRumblePlayerState;
class USoundBase;

UCLASS()
class SNOWRUMBLE_API ASnowmanModePodiumGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASnowmanModePodiumGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void InitGame(
		const FString& MapName,
		const FString& Options,
		FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void HandleStartingNewPlayer_Implementation(
		APlayerController* NewPlayer) override;

private:
	/** 포디움 맵 접속자가 모두 준비된 뒤 배치를 다시 시도한다. */
	void ScheduleSnowmanPodiumSetup();

	/** 현재 로컬 클라이언트에 눈사람 포디움 배경음악을 재생하도록 지시한다. */
	void BroadcastBackgroundMusic() const;

	/** 서버가 눈사람 승리 진영, 캐릭터 위치, 카메라, UI 텍스트를 확정한다. */
	void SetupSnowmanPodiumFromServer();

	/** 포디움 결과를 보여준 뒤 로비로 복귀한다. */
	void ReturnToLobbyAfterSnowmanPodium();

	/** 포디움 복귀 카운트다운을 시작한다. */
	void StartSnowmanPodiumReturnCountdown();

	/** 포디움 복귀 카운트다운을 1초 갱신한다. */
	void TickSnowmanPodiumReturnCountdown();

	/** 포디움 복귀 카운트다운 텍스트를 모든 클라이언트에 보낸다. */
	void BroadcastSnowmanPodiumReturnCountdown();

	/** 승자 플레이어가 사용할 PlayerStart 후보를 1~4번 위치 기준으로 수집한다. */
	void CollectSnowmanPodiumPlayerStarts(
		TArray<APlayerStart*>& OutWinnerStarts) const;

	/** 포디움 카메라 액터를 찾는다. */
	ACameraActor* FindSnowmanPodiumCamera() const;

	/** URL 옵션의 WinnerPlayerIds 값을 파싱한다. */
	void ParseWinnerPlayerIds(const FString& WinnerPlayerIdsOption);

	/** PlayerState가 URL로 전달된 눈사람 모드 승자 목록에 포함되는지 확인한다. */
	bool IsWinnerPlayerState(const ASnowRumblePlayerState* PlayerState) const;

	/** 결과 enum에 맞는 포디움 제목을 만든다. */
	FText BuildSnowmanPodiumResultText() const;

	/** 로비 복귀 안내 문구를 만든다. */
	FText BuildSnowmanPodiumReturnSubtitle() const;

	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Snowman|Podium")
	FString LobbyReturnTravelUrl = TEXT("/Game/Maps/L_Lobby?listen");

	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Snowman|Podium", meta = (ClampMin = "0.0"))
	float PodiumReturnDelaySeconds = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Snowman|Podium", meta = (ClampMin = "0.0"))
	float PodiumSetupRetryDelaySeconds = 0.25f;

	/** 눈사람 포디움에서 재생할 배경음악이다. */
	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Audio")
	TObjectPtr<USoundBase> BackgroundMusicSound;

	int32 ExpectedPlayerCount = 0;
	bool bPodiumSetupComplete = false;
	ESnowmanModeResult SnowmanPodiumResult = ESnowmanModeResult::None;
	TSet<int32> WinnerPlayerIds;
	FTimerHandle PodiumSetupTimerHandle;
	FTimerHandle PodiumReturnCountdownTimerHandle;
	int32 PodiumReturnCountdownRemainingSeconds = 0;
};

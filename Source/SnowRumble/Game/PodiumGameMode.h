// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SnowRumbleGameMode.h"
#include "SnowRumblePlayerState.h"
#include "PodiumGameMode.generated.h"

class ACameraActor;
class APlayerStart;
class APodiumPlayerController;
class USnowRumbleMatchSubsystem;
enum class ESnowRumbleTeam : uint8;

USTRUCT()
struct FSnowRumblePodiumTeamResult
{
	GENERATED_BODY()

	ESnowRumbleTeam Team = ESnowRumbleTeam::None;
	int32 RoundWins = 0;
};

UCLASS()
class SNOWRUMBLE_API APodiumGameMode : public ASnowRumbleGameMode
{
	GENERATED_BODY()

public:
	APodiumGameMode();

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
	void SchedulePodiumSetup();

	/** 서버가 참가 팀 순위, 캐릭터 위치, 카메라, UI 텍스트를 확정한다. */
	void SetupPodiumFromServer();

	/** 포디움 결과를 10초 보여준 뒤 매치 상태를 정리하고 로비로 복귀한다. */
	void ReturnToLobbyAfterPodium();

	/** 현재 포디움 맵에 접속한 플레이어의 팀만 순위 후보로 수집한다. */
	void BuildParticipatingTeamResults(
		USnowRumbleMatchSubsystem* MatchSubsystem,
		TArray<FSnowRumblePodiumTeamResult>& OutResults) const;

	/** 순위별 PlayerStart 후보를 수집한다. */
	void CollectPodiumPlayerStarts(
		TArray<APlayerStart*>& OutFirstPlaceStarts,
		TArray<APlayerStart*>& OutSecondPlaceStarts,
		TArray<APlayerStart*>& OutThirdPlaceStarts) const;

	/** 포디움 카메라 액터를 찾는다. */
	ACameraActor* FindPodiumCamera() const;

	/** 팀 이름과 승수를 UI 표시 텍스트로 변환한다. */
	FText BuildResultText(const FSnowRumblePodiumTeamResult* Result) const;

	/** 팀 색 이름을 UI 표시 텍스트로 변환한다. */
	FText GetTeamDisplayName(ESnowRumbleTeam Team) const;

	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Podium")
	FString PodiumLobbyReturnTravelUrl = TEXT("/Game/Maps/L_Lobby?listen");

	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Podium", meta = (ClampMin = "0.0"))
	float PodiumReturnDelaySeconds = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "SnowRumble|Podium", meta = (ClampMin = "0.0"))
	float PodiumSetupRetryDelaySeconds = 0.25f;

	int32 ExpectedPlayerCount = 0;
	bool bPodiumSetupComplete = false;
	FTimerHandle PodiumSetupTimerHandle;
	FTimerHandle PodiumReturnTimerHandle;
};

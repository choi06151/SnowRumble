// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SnowRumbleLobbyGameMode.generated.h"

class APlayerController;
class AController;
class AGameModeBase;
class ASnowRumblePlayerState;
class UTexture2D;
class UWorld;
class USoundBase;
enum class ESnowRumbleTeam : uint8;

USTRUCT(BlueprintType)
struct FSnowRumbleLoadingMapPresentation
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Loading")
	TSoftObjectPtr<UWorld> Level;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Loading")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Loading")
	TSoftObjectPtr<UTexture2D> LoadingImage;
};

UCLASS()
class SNOWRUMBLE_API ASnowRumbleLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASnowRumbleLobbyGameMode();

	virtual void BeginPlay() override;

	/** 호스트 요청과 준비 상태를 검사한 뒤 게임방으로 이동한다. */
	void RequestStartMatch(APlayerController* RequestingController);

	/** 서버가 현재 로비 인원을 지정한 팀 수로 무작위 균등 배정한다. */
	void ShuffleLobbyTeamsFromServer(int32 TeamCount);

	/** 서버가 현재 로비 인원을 모두 서로 다른 팀 색으로 무작위 배정한다. */
	void ShuffleLobbyPlayersIndividuallyFromServer();

protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
	virtual void HandleStartingNewPlayer_Implementation(
		APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/** 새 참가자를 서버 기준 랜덤 팀 배정 규칙으로 배치한다. */
	void AssignLobbyTeam(APlayerController* NewPlayer);

	/** 현재 로비에서 팀 섞기에 사용할 수 있는 플레이어 목록을 반환한다. */
	TArray<ASnowRumblePlayerState*> GetShuffleableLobbyPlayers() const;

	/** 현재 대기방에서 해당 팀에 배정된 인원을 센다. */
	int32 CountLobbyTeamPlayers(ESnowRumbleTeam Team) const;

	/** PvP travel과 로딩 UI에 사용할 예상 참가 인원을 서버 상태에서 확정한다. */
	int32 ResolveExpectedMatchPlayerCount() const;

	/** 현재 참가 인원과 서버가 고른 PvP 후보 레벨을 URL 옵션에 포함한다. */
	FString BuildMatchTravelUrl(int32 ExpectedPlayerCount);

	/** 현재 참가 인원과 서버가 고른 PvP 후보 레벨을 눈사람 모드 URL 옵션에 포함한다. */
	FString BuildSnowmanModeTravelUrl(int32 ExpectedPlayerCount);

	/** 등록된 후보 중 PvP 진입에 사용할 레벨 경로를 서버에서 고른다. */
	FString SelectPvPLevelPath() const;

	/** 등록된 PvP 후보 레벨 경로 목록을 반환한다. */
	TArray<FString> GetPvPLevelCandidatePaths() const;

	/** 등록된 눈사람 전용 후보가 없으면 PvP 후보로 fallback한 레벨 경로 목록을 반환한다. */
	TArray<FString> GetSnowmanLevelCandidatePaths() const;

	/** 선택된 맵의 로딩 화면 표시 설정을 반환한다. */
	FSnowRumbleLoadingMapPresentation GetLoadingMapPresentation(
		const FString& MapPackageName) const;

	/** 특정 플레이어 기준 같은 팀 이름 목록을 반환한다. */
	TArray<FString> GetTeamPlayerNamesFor(
		const ASnowRumblePlayerState* LocalPlayerState) const;

	/** 현재 연결된 모든 클라이언트에 매치 로딩창 표시를 요청한다. */
	void ShowMatchLoadingScreens();

	/** 현재 로컬 클라이언트에 로비 배경음악을 재생하도록 지시한다. */
	void BroadcastBackgroundMusic() const;

	/** 로딩창 표시 후 지연 시간이 지나면 실제 게임 맵 이동을 실행한다. */
	void StartPendingMatchTravel();

	/** 대기방에서 PvP 시작 시 이동할 게임방 맵 경로다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Lobby")
	FString MatchTravelUrl =
		TEXT("/Game/LowpolyStyle/WinterEnvironment/Maps/DemoMap?listen");

	/** 대기방에서 PvP 시작 시 서버가 무작위로 고를 후보 레벨이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Lobby")
	TArray<TSoftObjectPtr<UWorld>> PvPLevelCandidates;

	/** 대기방에서 눈사람 모드 시작 시 서버가 무작위로 고를 전용 후보 레벨이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Lobby|Snowman")
	TArray<TSoftObjectPtr<UWorld>> SnowmanLevelCandidates;

	/** 로비에서 재생할 배경음악이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Audio")
	TObjectPtr<USoundBase> BackgroundMusicSound;

	/** PvP 후보 레벨별 로딩 화면 표시명과 이미지다. 비어 있으면 레벨 경로 이름을 사용한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Lobby")
	TArray<FSnowRumbleLoadingMapPresentation> PvPLevelLoadingPresentations;

	/** 로비에서 게임 맵으로 이동하기 전 로딩창을 보여줄 대기 시간이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Lobby", meta = (ClampMin = "0.0"))
	float MatchTravelDelaySeconds = 5.0f;

	/** 대기방에서 눈사람 모드 시작 시 사용할 GameMode 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Lobby")
	TSubclassOf<AGameModeBase> SnowmanModeGameModeClass;

	FString PendingMatchTravelUrl;
	FString PendingMatchMapPackageName;
	bool bMatchTravelPending = false;
	FTimerHandle PendingMatchTravelTimerHandle;
};

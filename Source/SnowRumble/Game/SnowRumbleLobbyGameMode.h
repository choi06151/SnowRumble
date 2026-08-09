// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SnowRumbleLobbyGameMode.generated.h"

class APlayerController;
enum class ESnowRumbleTeam : uint8;

UCLASS()
class SNOWRUMBLE_API ASnowRumbleLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASnowRumbleLobbyGameMode();

	/** 호스트 요청과 준비 상태를 검사한 뒤 게임방으로 이동한다. */
	void RequestStartMatch(APlayerController* RequestingController);

protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/** 새 참가자를 서버 기준 랜덤 팀 배정 규칙으로 배치한다. */
	void AssignLobbyTeam(APlayerController* NewPlayer);

	/** 현재 대기방에서 해당 팀에 배정된 인원을 센다. */
	int32 CountLobbyTeamPlayers(ESnowRumbleTeam Team) const;

	/** 현재 참가 인원을 URL 옵션에 포함한 PvP 맵 이동 경로를 만든다. */
	FString BuildMatchTravelUrl(int32 ExpectedPlayerCount) const;

	/** 현재 연결된 모든 클라이언트에 매치 로딩창 표시를 요청한다. */
	void ShowMatchLoadingScreens();

	/** 로딩창 표시 RPC가 나간 다음 틱에 실제 PvP 맵 이동을 실행한다. */
	void StartPendingMatchTravel();

	/** 대기방에서 PvP 시작 시 이동할 게임방 맵 경로다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Lobby")
	FString MatchTravelUrl =
		TEXT("/Game/LowpolyStyle/WinterEnvironment/Maps/DemoMap?listen");

	FString PendingMatchTravelUrl;
	bool bMatchTravelPending = false;
};

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

	/** 대기방에서 시작 시 이동할 게임방 맵 경로다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Lobby")
	FString MatchTravelUrl = TEXT("/Game/Maps/L_Prototype?listen");
};

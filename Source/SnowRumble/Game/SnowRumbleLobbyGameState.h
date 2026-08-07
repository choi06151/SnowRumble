// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SnowRumbleLobbyGameState.generated.h"

class ASnowRumblePlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSnowRumbleLobbyStateChanged);

UCLASS()
class SNOWRUMBLE_API ASnowRumbleLobbyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	/** 현재 대기방에 참가한 SnowRumble PlayerState 목록을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Lobby")
	TArray<ASnowRumblePlayerState*> GetLobbyPlayers() const;

	/** 서버 기준으로 현재 대기방에서 경기 시작이 가능한지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Lobby")
	bool CanStartLobbyMatch() const;

	/** 대기방 상태 변경을 UI에 알린다. */
	void NotifyLobbyStateChanged();

	UPROPERTY(BlueprintAssignable, Category = "SnowRumble|Lobby")
	FOnSnowRumbleLobbyStateChanged OnLobbyStateChanged;
};

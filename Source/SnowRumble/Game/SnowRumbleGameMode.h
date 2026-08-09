// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SnowRumbleGameMode.generated.h"

class APlayerController;
class AActor;

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

	virtual AActor* ChoosePlayerStart_Implementation(
		AController* Player) override;

private:
	/** 대기방에서 전달받은 이번 매치 예상 접속 인원 수다. */
	int32 ExpectedPlayerCount = 0;

	bool bLoadingScreensDismissed = false;

	/** 모든 예상 플레이어가 접속하면 전체 클라이언트의 로딩창을 닫는다. */
	void TryDismissLoadingScreens();

	/** 현재 PvP 레벨 접속 진행률을 접속 완료된 클라이언트들에게 보낸다. */
	void BroadcastLoadingProgress();

	/** 이번 PvP 매치에서 이미 선택한 PlayerStart다. */
	TSet<TWeakObjectPtr<AActor>> UsedPlayerStarts;
};

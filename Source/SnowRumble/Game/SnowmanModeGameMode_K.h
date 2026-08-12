// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SnowmanModeGameMode_K.generated.h"

class APlayerController;

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

	virtual void HandleStartingNewPlayer_Implementation(
		APlayerController* NewPlayer) override;

protected:
	/** 눈사람 모드 제한시간이다. K-14 전까지 승패 없이 시간 상태만 제공한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman", meta = (ClampMin = "0.0"))
	float SnowmanModeTimeLimitSeconds = 600.0f;

private:
	/** 대기방에서 전달받은 이번 모드 예상 접속 인원 수다. */
	int32 ExpectedPlayerCount = 0;

	bool bLoadingScreensDismissed = false;
	bool bSnowmanTimerStarted = false;

	/** 모든 예상 플레이어가 접속하면 전체 클라이언트의 로딩창을 닫는다. */
	void TryDismissLoadingScreens();

	/** 로딩이 끝난 뒤 눈사람 모드 제한시간을 시작한다. */
	void StartSnowmanModeAfterLoading();

	/** 현재 눈사람 모드 레벨 접속 진행률을 접속 완료된 클라이언트들에게 보낸다. */
	void BroadcastLoadingProgress();
};

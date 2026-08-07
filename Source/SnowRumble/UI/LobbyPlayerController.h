// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

class ULobbyWidget;

UCLASS(Blueprintable)
class SNOWRUMBLE_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** 대기방 위젯을 생성하고 UI 입력 모드로 전환한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void ShowLobby();

	/** 대기방 위젯을 제거하고 게임 입력 모드로 되돌린다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void HideLobby();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 대기방에서 자동 생성할 WBP_Lobby 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Lobby")
	TSubclassOf<ULobbyWidget> LobbyWidgetClass;

private:
	/** 대기방 위젯 인스턴스가 없으면 생성한다. */
	ULobbyWidget* EnsureLobbyWidget();

	UPROPERTY(Transient)
	TObjectPtr<ULobbyWidget> LobbyWidget;
};

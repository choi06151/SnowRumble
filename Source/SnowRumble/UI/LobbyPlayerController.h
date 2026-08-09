// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../Game/SnowRumblePlayerState.h"
#include "SnowRumblePlayerController.h"
#include "LobbyPlayerController.generated.h"

class ULobbyWidget;

UCLASS(Blueprintable)
class SNOWRUMBLE_API ALobbyPlayerController : public ASnowRumblePlayerController
{
	GENERATED_BODY()

public:
	/** 대기방 위젯을 생성하고 기본 게임 입력 모드로 전환한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void ShowLobby();

	/** 대기방 위젯을 제거하고 게임 입력 모드로 되돌린다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void HideLobby();

	/** 로비 설정 UI를 조작할 때 커서와 UI 입력을 켠다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void EnableLobbyUiInput();

	/** 로비 설정 UI 조작을 끝내고 플레이 입력으로 돌아간다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void EnableLobbyGameInput();

	/** 소유 클라이언트가 로비 닉네임을 서버 PlayerState에 적용하도록 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void RequestApplyLobbyPlayerName(const FString& NewName);

	/** 소유 클라이언트가 로비 팀 색을 서버 PlayerState에 적용하도록 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void RequestApplyLobbyTeam(ESnowRumbleTeam NewTeam);

	/** 서버가 로비 입장 완료 후 소유 클라이언트의 저장 닉네임 제출을 요청한다. */
	UFUNCTION(Client, Reliable)
	void ClientRequestApplySavedLobbyPlayerName();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void ClientShowLoadingScreen_Implementation() override;

	/** 대기방에서 자동 생성할 WBP_Lobby 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Lobby")
	TSubclassOf<ULobbyWidget> LobbyWidgetClass;

private:
	/** 로컬 GameInstance에 저장된 닉네임을 서버로 제출한다. */
	void ApplySavedLobbyPlayerName();

	UFUNCTION(Server, Reliable)
	void ServerApplyLobbyPlayerName(const FString& NewName);

	UFUNCTION(Server, Reliable)
	void ServerApplyLobbyTeam(ESnowRumbleTeam NewTeam);

	/** 대기방 위젯 인스턴스가 없으면 생성한다. */
	ULobbyWidget* EnsureLobbyWidget();

	UPROPERTY(Transient)
	TObjectPtr<ULobbyWidget> LobbyWidget;
};

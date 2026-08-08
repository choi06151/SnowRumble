// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/SnowRumblePlayerState.h"
#include "LobbyWidget.generated.h"

class ASnowRumbleLobbyGameState;
class ASnowRumblePlayerState;
class UTextBlock;

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 현재 대기방 플레이어 목록을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Lobby")
	TArray<ASnowRumblePlayerState*> GetLobbyPlayers() const;

	/** 로컬 플레이어의 대기방 이름 변경을 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void RequestSetLocalPlayerName(const FString& NewName);

	/** 로컬 플레이어의 팀 변경을 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void RequestSetLocalPlayerTeam(ESnowRumbleTeam NewTeam);

	/** 로컬 플레이어의 준비 상태 변경을 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void RequestSetLocalPlayerReady(bool bNewReady);

	/** 호스트의 경기 시작을 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void RequestStartMatch();

	/** 현재 로컬 플레이어가 호스트인지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Lobby")
	bool IsLocalPlayerHost() const;

	/** 현재 서버 기준 시작 가능 여부를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Lobby")
	bool CanStartMatch() const;

	/** 현재 방의 방 코드를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Lobby")
	FString GetCurrentRoomCode() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(
		const FGeometry& MyGeometry,
		float InDeltaTime) override;

	/** 대기방 목록 또는 플레이어 상태가 바뀌면 Blueprint UI에 알린다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Lobby")
	void OnLobbyStateChanged();

	/** 있으면 현재 방 코드를 자동 표시하는 텍스트다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Lobby")
	TObjectPtr<UTextBlock> RoomCodeTextBlock;

private:
	ASnowRumblePlayerState* GetLocalSnowRumblePlayerState() const;
	ASnowRumbleLobbyGameState* GetLobbyGameState() const;
	void ApplyLocalPlayerIdentity();
	void RefreshLobbyBindings();
	void RefreshRoomCodeText();
	void UnbindLobbyBindings();

	UFUNCTION()
	void HandleLobbyStateChanged();

	UPROPERTY(Transient)
	TObjectPtr<ASnowRumbleLobbyGameState> BoundLobbyGameState;

	UPROPERTY(Transient)
	TObjectPtr<ASnowRumblePlayerState> IdentityAppliedPlayerState;

	double LastIdentityApplyRequestTime = -1.0;
};

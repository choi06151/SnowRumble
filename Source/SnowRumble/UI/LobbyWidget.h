// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/SnowRumbleMatchSubsystem_C.h"
#include "../Game/SnowRumblePlayerState.h"
#include "LobbyWidget.generated.h"

class ASnowRumbleLobbyGameState;
class ASnowRumblePlayerState;
class UBorder;
class UTextBlock;
class UWidgetAnimation;

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 현재 대기방 플레이어 목록을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Lobby")
	TArray<ASnowRumblePlayerState*> GetLobbyPlayers() const;

	/** 로비 이벤트 로그에 새 메시지를 추가한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void AddEventLogMessage(const FText& Message);

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

	/** 예외행동 사유를 표시하고 피드백 애니메이션을 재생한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void ShowInvalidActionFeedback(const FText& ReasonText);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(
		const FGeometry& MyGeometry,
		float InDeltaTime) override;

	/** 대기방 목록 또는 플레이어 상태가 바뀌면 Blueprint UI에 알린다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Lobby")
	void OnLobbyStateChanged();

	/** 예외행동 피드백을 Blueprint가 직접 표시하거나 애니메이션으로 처리한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Lobby")
	void OnInvalidActionFeedback(const FText& ReasonText);

	/** WBP에 같은 이름으로 만든 예외행동 피드백 애니메이션이다. */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> InvalidActionAnimation;

	/** WBP에 같은 이름으로 만든 예외행동 사유 표시 TextBlock이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Lobby")
	TObjectPtr<UTextBlock> InvalidActionReasonText;

	/** WBP에 같은 이름으로 만든 누적 이벤트 로그 TextBlock이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Lobby")
	TObjectPtr<UTextBlock> EventLogText;

	/** 이벤트 로그 한 줄이 화면에 유지되는 시간이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Lobby", meta = (ClampMin = "0.0"))
	float EventLogEntryVisibleSeconds = 5.0f;

	/** 있으면 현재 방 코드를 자동 표시하는 텍스트다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Lobby")
	TObjectPtr<UTextBlock> RoomCodeTextBlock;

	/** 있으면 준비 완료한 인원 수와 전체 인원 수를 자동 표시하는 텍스트다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Lobby")
	TObjectPtr<UTextBlock> ReadyPlayerCountText;

	/** 있으면 현재 선택된 로비 게임 모드를 자동 표시하는 텍스트다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Lobby")
	TObjectPtr<UTextBlock> CurrentGameModeText;

	/** 있으면 로비에서 선택한 총 라운드 수를 자동 표시하는 텍스트다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Lobby")
	TObjectPtr<UTextBlock> MatchRoundLimitText;

	/** 있으면 로비에서 선택한 게임 속도와 축소 주기를 자동 표시하는 텍스트다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Lobby")
	TObjectPtr<UTextBlock> GameSpeedText;

	/** 있으면 로컬 플레이어 이름을 자동 표시하는 텍스트다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Lobby")
	TObjectPtr<UTextBlock> LocalPlayerNameText;

	/** 있으면 로컬 플레이어 팀 색 이름을 자동 표시하는 텍스트다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Lobby")
	TObjectPtr<UTextBlock> LocalTeamColorText;

	/** 있으면 로컬 플레이어 준비 상태를 자동 표시하는 텍스트다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Lobby")
	TObjectPtr<UTextBlock> LocalReadyStateText;

	/** 있으면 로컬 플레이어 팀 색을 배경 swatch로 자동 표시한다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Lobby")
	TObjectPtr<UBorder> LocalTeamColorBorder;

private:
	ASnowRumblePlayerState* GetLocalSnowRumblePlayerState() const;
	ASnowRumbleLobbyGameState* GetLobbyGameState() const;
	void ApplyLocalPlayerIdentity();
	void RefreshLobbyBindings();
	void RefreshRoomCodeText();
	void RefreshLobbyStatusTexts();
	void RefreshEventLogText();
	void UnbindLobbyBindings();

	UFUNCTION()
	void HandleLobbyStateChanged();

	UPROPERTY(Transient)
	TObjectPtr<ASnowRumbleLobbyGameState> BoundLobbyGameState;

	UPROPERTY(Transient)
	TObjectPtr<ASnowRumblePlayerState> IdentityAppliedPlayerState;

	struct FEventLogEntry
	{
		FText Message;
		double ExpireTimeSeconds = 0.0;
	};

	TArray<FEventLogEntry> EventLogEntries;

	double LastIdentityApplyRequestTime = -1.0;
};

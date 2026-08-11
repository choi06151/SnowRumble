// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainHUDWidget.generated.h"

class ASnowRumbleCharacter;
class ASnowRumblePlayerState;
class UHealthBarWidget;
class UImage;
class UOverheadTimedActionWidget;
class UPanelWidget;
class UProgressBar;
class UTextBlock;
class UWidget;
class UWidgetAnimation;
enum class ESnowRumbleTeam : uint8;

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API UMainHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** HUD 이벤트 로그에 새 메시지를 추가한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Event Log")
	void AddEventLogMessage(const FText& Message);

	/** 로컬 플레이어에게만 보이는 짧은 상태 알림을 표시한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Personal Alarm")
	void ShowPersonalTextAlarm(const FText& Message);

protected:
	/** HUD 생성 시 로컬 플레이어와 다른 플레이어 체력 바를 초기화한다. */
	virtual void NativeConstruct() override;

	/** 플레이어 입장·퇴장과 Pawn 변경에 대응해 체력 바 목록을 갱신한다. */
	virtual void NativeTick(
		const FGeometry& MyGeometry,
		float InDeltaTime) override;

	/** WBP에서 직접 배치한 내 체력 바 위젯에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UHealthBarWidget> LocalHealthBar;

	/** 다른 플레이어 체력 바들이 추가될 Panel에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> OtherPlayersHealthPanel;

	/** 다른 플레이어마다 동적으로 만들 체력 바 위젯 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Health")
	TSubclassOf<UHealthBarWidget> OtherPlayerHealthBarWidgetClass;

	/** WBP에서 직접 배치한 투척 충전 Progress Bar에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> AimChargeProgressBar;

	/** WBP에서 직접 배치한 조준점 위젯에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> AimCrosshair;

	/** WBP에서 직접 배치한 머리 위 행동 진행 바 위젯에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UOverheadTimedActionWidget> OverheadTimedActionBar;

	/** WBP에서 직접 배치한 PvP 시작 카운트다운 TextBlock에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StartCountdownText;

	/** WBP에서 직접 배치한 현재 라운드 표시 TextBlock에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CurrentRoundText;

	/** WBP에서 직접 배치한 현재 경기 시간 표시 TextBlock에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MatchElapsedTimeText;

	/** WBP에서 직접 배치한 다음 맵 축소 안내 TextBlock에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MapShrinkCountdownText;

	/** WBP에서 직접 배치한 라운드 종료 패널에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> EndRoundPanel;

	/** WBP에서 직접 배치한 라운드 종료 결과 TextBlock에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EndRoundResultText;

	/** WBP에서 직접 배치한 누적 이벤트 로그 TextBlock에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EventLogText;

	/** WBP에서 직접 배치한 로컬 개인 알림 TextBlock에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PersonalAlarmText;

	/** WBP에 같은 이름으로 만든 로컬 개인 알림 애니메이션이다. */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> PersonalAlarmAnimation;

	/** WBP에서 직접 배치한 현재 음성 송출 중인 플레이어 이름 TextBlock에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> VoiceSpeakingNamesText;

	/** WBP에서 직접 배치한 현재 음성 송출 아이콘 Image에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> VoiceSpeakingIcon;

	/** WBP에서 아이콘과 이름 텍스트를 함께 감싸는 위젯에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> VoiceSpeakingContainer;

	/** 이벤트 로그 한 줄이 화면에 유지되는 시간이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Event Log", meta = (ClampMin = "0.0"))
	float EventLogEntryVisibleSeconds = 5.0f;

	/** WBP에서 직접 배치한 빨강 팀 점수 TextBlock에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RedTeamScoreText;

	/** WBP에서 직접 배치한 빨강 팀 점수 행 위젯에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> RedTeamScoreRow;

	/** WBP에서 직접 배치한 하늘 팀 점수 TextBlock에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SkyTeamScoreText;

	/** WBP에서 직접 배치한 하늘 팀 점수 행 위젯에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> SkyTeamScoreRow;

	/** WBP에서 직접 배치한 초록 팀 점수 TextBlock에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GreenTeamScoreText;

	/** WBP에서 직접 배치한 초록 팀 점수 행 위젯에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> GreenTeamScoreRow;

	/** WBP에서 직접 배치한 노랑 팀 점수 TextBlock에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> YellowTeamScoreText;

	/** WBP에서 직접 배치한 노랑 팀 점수 행 위젯에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> YellowTeamScoreRow;

	/** WBP에서 직접 배치한 보라 팀 점수 TextBlock에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PurpleTeamScoreText;

	/** WBP에서 직접 배치한 보라 팀 점수 행 위젯에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> PurpleTeamScoreRow;

	/** WBP에서 직접 배치한 핑크 팀 점수 TextBlock에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PinkTeamScoreText;

	/** WBP에서 직접 배치한 핑크 팀 점수 행 위젯에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> PinkTeamScoreRow;

	/** WBP에서 직접 배치한 파랑 팀 점수 TextBlock에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BlueTeamScoreText;

	/** WBP에서 직접 배치한 파랑 팀 점수 행 위젯에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> BlueTeamScoreRow;

	/** WBP에서 직접 배치한 하양 팀 점수 TextBlock에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WhiteTeamScoreText;

	/** WBP에서 직접 배치한 하양 팀 점수 행 위젯에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> WhiteTeamScoreRow;

private:
	/** 현재 로컬 플레이어와 다른 플레이어 체력 바 연결을 갱신한다. */
	void RefreshHealthBars();

	/** 조준점과 투척 충전 바 표시를 로컬 플레이어 상태에 맞게 갱신한다. */
	void RefreshCombatHudPresentation();

	/** PvP 시작 카운트다운 표시를 현재 GameState에 맞게 갱신한다. */
	void RefreshStartCountdownPresentation();

	/** 현재 라운드 번호 표시를 현재 GameState에 맞게 갱신한다. */
	void RefreshCurrentRoundPresentation();

	/** 현재 경기 시간과 다음 맵 축소 안내를 현재 GameState에 맞게 갱신한다. */
	void RefreshMatchTimerPresentation();

	/** 라운드 종료 패널 표시를 현재 GameState에 맞게 갱신한다. */
	void RefreshEndRoundPresentation();

	/** 팀별 라운드 승리 점수 표시를 현재 GameState에 맞게 갱신한다. */
	void RefreshTeamScorePresentation();

	/** 만료된 이벤트 로그 줄을 제거하고 TextBlock을 갱신한다. */
	void RefreshEventLogText();

	/** 현재 음성 송출 중인 플레이어 이름 목록을 갱신한다. */
	void RefreshVoiceSpeakingNamesText();

	/** 음성 송출 표시 묶음과 하위 위젯의 표시 상태를 함께 갱신한다. */
	void SetVoiceSpeakingPresentationVisible(bool bVisible);

	/** 현재 로컬 플레이어가 송신자의 음성 표시를 볼 수 있는지 반환한다. */
	bool ShouldShowVoiceSpeakingPlayer(
		const ASnowRumblePlayerState* SenderPlayerState) const;

	/** 선택 바인딩된 점수 TextBlock 하나를 해당 팀 승수로 갱신한다. */
	void SetTeamScoreText(UTextBlock* ScoreText, ESnowRumbleTeam Team) const;

	/** 점수 행 위젯이 있으면 행을, 없으면 점수 TextBlock 자체를 표시 대상으로 반환한다. */
	UWidget* GetTeamScoreDisplayWidget(ESnowRumbleTeam Team) const;

	/** WBP 트리에서 팀 점수 행 위젯을 이름으로 찾는다. */
	UWidget* FindTeamScoreRowWidget(ESnowRumbleTeam Team) const;

	/** 팀 점수 TextBlock 바인딩을 반환한다. */
	UTextBlock* GetTeamScoreText(ESnowRumbleTeam Team) const;

	/** 현재 PvP에 참가 중인 팀인지 반환한다. */
	bool IsTeamParticipating(ESnowRumbleTeam Team) const;

	/** 다른 플레이어 체력 바 목록에서 더 이상 유효하지 않은 항목을 제거한다. */
	void RemoveInvalidOtherPlayerHealthBars(
		const TSet<TWeakObjectPtr<ASnowRumbleCharacter>>& ValidOtherPlayers);

	/** 로컬 플레이어와 같은 팀인 다른 플레이어만 팀원 HP 목록에 표시한다. */
	bool ShouldShowOtherPlayerHealthBar(
		const ASnowRumbleCharacter* LocalCharacter,
		const ASnowRumbleCharacter* OtherCharacter) const;

	TMap<TWeakObjectPtr<ASnowRumbleCharacter>, TWeakObjectPtr<UHealthBarWidget>>
		OtherPlayerHealthBars;

	struct FEventLogEntry
	{
		FText Message;
		double ExpireTimeSeconds = 0.0;
	};

	TArray<FEventLogEntry> EventLogEntries;
};

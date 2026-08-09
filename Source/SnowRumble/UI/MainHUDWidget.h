// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainHUDWidget.generated.h"

class ASnowRumbleCharacter;
class UHealthBarWidget;
class UOverheadTimedActionWidget;
class UPanelWidget;
class UProgressBar;
class UTextBlock;
class UWidget;

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API UMainHUDWidget : public UUserWidget
{
	GENERATED_BODY()

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

	/** WBP에서 직접 배치한 라운드 종료 패널에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UPanelWidget> EndRoundPanel;

	/** WBP에서 직접 배치한 라운드 종료 결과 TextBlock에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EndRoundResultText;

private:
	/** 현재 로컬 플레이어와 다른 플레이어 체력 바 연결을 갱신한다. */
	void RefreshHealthBars();

	/** 조준점과 투척 충전 바 표시를 로컬 플레이어 상태에 맞게 갱신한다. */
	void RefreshCombatHudPresentation();

	/** PvP 시작 카운트다운 표시를 현재 GameState에 맞게 갱신한다. */
	void RefreshStartCountdownPresentation();

	/** 라운드 종료 패널 표시를 현재 GameState에 맞게 갱신한다. */
	void RefreshEndRoundPresentation();

	/** 다른 플레이어 체력 바 목록에서 더 이상 유효하지 않은 항목을 제거한다. */
	void RemoveInvalidOtherPlayerHealthBars(
		const TSet<TWeakObjectPtr<ASnowRumbleCharacter>>& ValidOtherPlayers);

	/** 로컬 플레이어와 같은 팀인 다른 플레이어만 팀원 HP 목록에 표시한다. */
	bool ShouldShowOtherPlayerHealthBar(
		const ASnowRumbleCharacter* LocalCharacter,
		const ASnowRumbleCharacter* OtherCharacter) const;

	TMap<TWeakObjectPtr<ASnowRumbleCharacter>, TWeakObjectPtr<UHealthBarWidget>>
		OtherPlayerHealthBars;
};

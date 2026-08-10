// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Game/SnowRumblePlayerState.h"
#include "../Interaction/LobbyInteractionBoard_C.h"
#include "LobbyBoardWidget_C.generated.h"

class ASnowRumbleCharacter;
class ALobbyPlayerController;
class UButton;
class UTextBlock;
class UWidgetAnimation;

UENUM(BlueprintType)
enum class ELobbyBoardTeamColor : uint8
{
	Red,
	Sky,
	Green,
	Yellow,
	Purple,
	Pink,
	Blue,
	White
};

UENUM(BlueprintType)
enum class ELobbyBoardGameMode : uint8
{
	Pvp,
	Snowman
};

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API ULobbyBoardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 게시판 액터가 위젯 생성 후 자신을 연결한다. */
	void SetOwningBoard(ALobbyInteractionBoard* NewOwningBoard);

	/** 로컬 포커스 중 버튼 요청을 보낼 캐릭터를 갱신한다. */
	void SetFocusedCharacter(ASnowRumbleCharacter* NewFocusedCharacter);

	/** 로컬 포커스 중 팀 변경 RPC를 보낼 PlayerController를 갱신한다. */
	void SetFocusedPlayerController(ALobbyPlayerController* NewPlayerController);

	/** WBP 버튼 OnClicked에서 수동으로 팀 색 변경 요청을 보낼 때 사용한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby Board")
	void SubmitTeamColorFromBlueprint(ELobbyBoardTeamColor TeamColor);

protected:
	/** 위젯 생성 시 선택 버튼과 닫기 버튼을 연결한다. */
	virtual void NativeConstruct() override;

	/** 위젯 제거 시 버튼 이벤트 연결을 정리한다. */
	virtual void NativeDestruct() override;

	/** 팀 색 인원 수 표시를 최신 로비 상태에 맞춰 갱신한다. */
	virtual void NativeTick(
		const FGeometry& MyGeometry,
		float InDeltaTime) override;

	/** WBP에서 같은 이름으로 만든 0번 게시판 액션 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> BoardActionButton0;

	/** WBP에서 같은 이름으로 만든 1번 게시판 액션 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> BoardActionButton1;

	/** WBP에서 같은 이름으로 만든 2번 게시판 액션 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> BoardActionButton2;

	/** WBP에서 같은 이름으로 만든 3번 게시판 액션 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> BoardActionButton3;

	/** WBP에서 같은 이름으로 만든 포커스 닫기 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> CloseFocusButton;

	/** WBP에서 같은 이름으로 만든 빨간 팀 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> RedTeamButton;

	/** WBP에서 같은 이름으로 만든 하늘 팀 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> SkyTeamButton;

	/** WBP에서 같은 이름으로 만든 초록 팀 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> GreenTeamButton;

	/** WBP에서 같은 이름으로 만든 노랑 팀 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> YellowTeamButton;

	/** WBP에서 같은 이름으로 만든 보라 팀 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> PurpleTeamButton;

	/** WBP에서 같은 이름으로 만든 핑크 팀 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> PinkTeamButton;

	/** WBP에서 같은 이름으로 만든 파란 팀 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> BlueTeamButton;

	/** WBP에서 같은 이름으로 만든 하양 팀 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> WhiteTeamButton;

	/** WBP에서 같은 이름으로 만든 PVP 모드 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> PvpModeButton;

	/** WBP에서 같은 이름으로 만든 눈사람 모드 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> SnowmanModeButton;

	/** WBP에서 같은 이름으로 만든 준비 완료 또는 게임 시작 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ReadyStartButton;

	/** WBP에서 같은 이름으로 만든 준비 완료 또는 게임 시작 버튼 텍스트에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ReadyStartButtonText;

	/** WBP에서 같은 이름으로 만든 1라운드 선택 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Round1Button;

	/** WBP에서 같은 이름으로 만든 3라운드 선택 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Round3Button;

	/** WBP에서 같은 이름으로 만든 5라운드 선택 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Round5Button;

	/** WBP에서 같은 이름으로 만든 2팀 섞기 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Shuffle2TeamsButton;

	/** WBP에서 같은 이름으로 만든 3팀 섞기 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Shuffle3TeamsButton;

	/** WBP에서 같은 이름으로 만든 4팀 섞기 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Shuffle4TeamsButton;

	/** WBP에서 같은 이름으로 만든 개인전 섞기 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ShuffleSoloButton;

	/** WBP에서 같은 이름으로 만든 현재 라운드 수 표시 텍스트에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MatchRoundLimitText;

	/** WBP에서 같은 이름으로 만든 빨간 팀 인원 수 텍스트에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RedTeamCountText;

	/** WBP에서 같은 이름으로 만든 하늘 팀 인원 수 텍스트에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SkyTeamCountText;

	/** WBP에서 같은 이름으로 만든 초록 팀 인원 수 텍스트에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GreenTeamCountText;

	/** WBP에서 같은 이름으로 만든 노랑 팀 인원 수 텍스트에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> YellowTeamCountText;

	/** WBP에서 같은 이름으로 만든 보라 팀 인원 수 텍스트에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PurpleTeamCountText;

	/** WBP에서 같은 이름으로 만든 핑크 팀 인원 수 텍스트에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PinkTeamCountText;

	/** WBP에서 같은 이름으로 만든 파란 팀 인원 수 텍스트에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BlueTeamCountText;

	/** WBP에서 같은 이름으로 만든 하양 팀 인원 수 텍스트에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WhiteTeamCountText;

	/** 팀 색 버튼이 눌렸을 때 WBP가 표시 반응이나 이후 기능 연결을 처리한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Lobby Board")
	void OnTeamColorButtonClicked(ELobbyBoardTeamColor TeamColor);

	/** 로비 모드 버튼이 눌렸을 때 WBP가 표시 반응이나 이후 기능 연결을 처리한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Lobby Board")
	void OnLobbyModeButtonClicked(ELobbyBoardGameMode GameMode);

	/** 예외행동 피드백을 Blueprint가 직접 표시하거나 애니메이션으로 처리한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Lobby Board")
	void OnInvalidActionFeedback(const FText& ReasonText);

	/** WBP에 같은 이름으로 만든 예외행동 피드백 애니메이션이다. */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> InvalidActionAnimation;

	/** WBP에 같은 이름으로 만든 예외행동 사유 표시 TextBlock이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> InvalidActionReasonText;

private:
	/** WBP 내부에서 이름이 일치하는 버튼을 직접 찾아 C++ 변수에 보관한다. */
	void ResolveBoardButtons();

	/** WBP 내부에서 이름이 일치하는 팀 인원 수 텍스트를 직접 찾아 보관한다. */
	void ResolveTeamCountTexts();

	/** 게시판 버튼 클릭 이벤트를 연결한다. */
	void BindBoardButtons();

	/** 게시판 버튼 클릭 이벤트 연결을 해제한다. */
	void UnbindBoardButtons();

	UFUNCTION()
	void HandleActionButton0Clicked();

	UFUNCTION()
	void HandleActionButton1Clicked();

	UFUNCTION()
	void HandleActionButton2Clicked();

	UFUNCTION()
	void HandleActionButton3Clicked();

	UFUNCTION()
	void HandleCloseFocusButtonClicked();

	UFUNCTION()
	void HandleRedTeamButtonClicked();

	UFUNCTION()
	void HandleSkyTeamButtonClicked();

	UFUNCTION()
	void HandleGreenTeamButtonClicked();

	UFUNCTION()
	void HandleYellowTeamButtonClicked();

	UFUNCTION()
	void HandlePurpleTeamButtonClicked();

	UFUNCTION()
	void HandlePinkTeamButtonClicked();

	UFUNCTION()
	void HandleBlueTeamButtonClicked();

	UFUNCTION()
	void HandleWhiteTeamButtonClicked();

	UFUNCTION()
	void HandlePvpModeButtonClicked();

	UFUNCTION()
	void HandleSnowmanModeButtonClicked();

	UFUNCTION()
	void HandleReadyStartButtonClicked();

	UFUNCTION()
	void HandleRound1ButtonClicked();

	UFUNCTION()
	void HandleRound3ButtonClicked();

	UFUNCTION()
	void HandleRound5ButtonClicked();

	UFUNCTION()
	void HandleShuffle2TeamsButtonClicked();

	UFUNCTION()
	void HandleShuffle3TeamsButtonClicked();

	UFUNCTION()
	void HandleShuffle4TeamsButtonClicked();

	UFUNCTION()
	void HandleShuffleSoloButtonClicked();

	/** 버튼 클릭을 로컬 캐릭터의 서버 검증 요청으로 전달한다. */
	void SubmitBoardAction(ELobbyBoardAction BoardAction);

	/** 팀 색 버튼 클릭을 WBP 이벤트로 전달한다. */
	void SubmitTeamColor(ELobbyBoardTeamColor TeamColor);

	/** 로비 모드 버튼 클릭을 WBP 이벤트로 전달한다. */
	void SubmitLobbyMode(ELobbyBoardGameMode GameMode);

	/** 색별 팀 인원 수를 로비 GameState에서 읽어 숫자 텍스트로 갱신한다. */
	void RefreshTeamCountTexts();

	/** 준비 완료 또는 게임 시작 버튼 텍스트를 로컬 호스트 여부와 ready 상태에 맞춰 갱신한다. */
	void RefreshReadyStartButtonText();

	/** 현재 로비 라운드 수 설정 표시를 갱신한다. */
	void RefreshMatchRoundLimitText();

	/** 호스트가 로비 라운드 수를 서버 값으로 변경한다. */
	void SubmitMatchRoundLimit(int32 NewRoundLimit);

	/** 호스트가 현재 로비 인원을 지정 팀 수로 무작위 균등 배정한다. */
	void SubmitShuffleTeams(int32 TeamCount);

	/** 호스트가 현재 로비 인원을 모두 서로 다른 팀 색으로 무작위 배정한다. */
	void SubmitShuffleSolo();

	/** 예외행동 사유를 표시하고 피드백 애니메이션을 로컬에서 재생한다. */
	void ShowInvalidActionFeedback(const FText& ReasonText);

	/** 단일 팀 인원 수 텍스트를 갱신한다. */
	void SetTeamCountText(UTextBlock* CountText, ESnowRumbleTeam Team) const;

	/** 팀 변경 요청을 보낼 로컬 LobbyPlayerController를 찾는다. */
	ALobbyPlayerController* GetRequestingLobbyPlayerController() const;

	/** 로컬 플레이어의 SnowRumble PlayerState를 반환한다. */
	ASnowRumblePlayerState* GetRequestingPlayerState() const;

	/** 현재 로컬 플레이어가 listen host인지 반환한다. */
	bool IsRequestingPlayerHost() const;

	UPROPERTY(Transient)
	TObjectPtr<ALobbyInteractionBoard> OwningBoard;

	UPROPERTY(Transient)
	TObjectPtr<ASnowRumbleCharacter> FocusedCharacter;

	UPROPERTY(Transient)
	TObjectPtr<ALobbyPlayerController> FocusedPlayerController;
};

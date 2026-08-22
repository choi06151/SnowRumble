// Copyright Epic Games, Inc. All Rights Reserved.

#include "LobbyBoardWidget_C.h"

#include "../Audio/SnowRumbleAudioHelpers.h"
#include "../Game/SnowRumbleLobbyGameMode.h"
#include "../Game/SnowRumblePlayerState.h"
#include "../Game/SnowRumbleLobbyGameState.h"
#include "LobbyPlayerController.h"
#include "../Player/SnowRumbleCharacter.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

namespace
{
ESnowRumbleTeam ToSnowRumbleTeam(ELobbyBoardTeamColor TeamColor)
{
	switch (TeamColor)
	{
	case ELobbyBoardTeamColor::Red:
		return ESnowRumbleTeam::Red;
	case ELobbyBoardTeamColor::Sky:
		return ESnowRumbleTeam::Sky;
	case ELobbyBoardTeamColor::Green:
		return ESnowRumbleTeam::Green;
	case ELobbyBoardTeamColor::Yellow:
		return ESnowRumbleTeam::Yellow;
	case ELobbyBoardTeamColor::Purple:
		return ESnowRumbleTeam::Purple;
	case ELobbyBoardTeamColor::Pink:
		return ESnowRumbleTeam::Pink;
	case ELobbyBoardTeamColor::Blue:
		return ESnowRumbleTeam::Blue;
	case ELobbyBoardTeamColor::White:
		return ESnowRumbleTeam::White;
	default:
		return ESnowRumbleTeam::None;
	}
}

ESnowRumbleLobbyMode ToLobbyMode(ELobbyBoardGameMode GameMode)
{
	switch (GameMode)
	{
	case ELobbyBoardGameMode::Snowman:
		return ESnowRumbleLobbyMode::Snowman;
	case ELobbyBoardGameMode::Pvp:
	default:
		return ESnowRumbleLobbyMode::Pvp;
	}
}
}

void ULobbyBoardWidget::SetOwningBoard(ALobbyInteractionBoard* NewOwningBoard)
{
	OwningBoard = NewOwningBoard;
}

void ULobbyBoardWidget::SetFocusedCharacter(
	ASnowRumbleCharacter* NewFocusedCharacter)
{
	FocusedCharacter = NewFocusedCharacter;
	SetFocusedPlayerController(NewFocusedCharacter
		? Cast<ALobbyPlayerController>(NewFocusedCharacter->GetController())
		: nullptr);
}

void ULobbyBoardWidget::SetFocusedPlayerController(
	ALobbyPlayerController* NewPlayerController)
{
	FocusedPlayerController = NewPlayerController
		&& NewPlayerController->IsLocalController()
			? NewPlayerController
			: nullptr;
}

void ULobbyBoardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ResolveBoardButtons();
	ResolveTeamCountTexts();
	BindBoardButtons();
	RefreshTeamCountTexts();
	RefreshReadyStartButtonText();
	RefreshMatchRoundLimitText();
	RefreshSelectedButtonVisuals();
}

void ULobbyBoardWidget::NativeDestruct()
{
	UnbindBoardButtons();

	Super::NativeDestruct();
}

void ULobbyBoardWidget::NativeTick(
	const FGeometry& MyGeometry,
	float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshTeamCountTexts();
	RefreshReadyStartButtonText();
	RefreshMatchRoundLimitText();
	RefreshSelectedButtonVisuals();
}

void ULobbyBoardWidget::ResolveBoardButtons()
{
	if (!WidgetTree)
	{
		return;
	}

	if (!BoardActionButton0)
	{
		BoardActionButton0 =
			WidgetTree->FindWidget<UButton>(TEXT("BoardActionButton0"));
	}
	if (!BoardActionButton1)
	{
		BoardActionButton1 =
			WidgetTree->FindWidget<UButton>(TEXT("BoardActionButton1"));
	}
	if (!BoardActionButton2)
	{
		BoardActionButton2 =
			WidgetTree->FindWidget<UButton>(TEXT("BoardActionButton2"));
	}
	if (!BoardActionButton3)
	{
		BoardActionButton3 =
			WidgetTree->FindWidget<UButton>(TEXT("BoardActionButton3"));
	}
	if (!CloseFocusButton)
	{
		CloseFocusButton =
			WidgetTree->FindWidget<UButton>(TEXT("CloseFocusButton"));
	}
	if (!RedTeamButton)
	{
		RedTeamButton = WidgetTree->FindWidget<UButton>(TEXT("RedTeamButton"));
	}
	if (!SkyTeamButton)
	{
		SkyTeamButton = WidgetTree->FindWidget<UButton>(TEXT("SkyTeamButton"));
	}
	if (!GreenTeamButton)
	{
		GreenTeamButton =
			WidgetTree->FindWidget<UButton>(TEXT("GreenTeamButton"));
	}
	if (!YellowTeamButton)
	{
		YellowTeamButton =
			WidgetTree->FindWidget<UButton>(TEXT("YellowTeamButton"));
	}
	if (!PurpleTeamButton)
	{
		PurpleTeamButton =
			WidgetTree->FindWidget<UButton>(TEXT("PurpleTeamButton"));
	}
	if (!PinkTeamButton)
	{
		PinkTeamButton =
			WidgetTree->FindWidget<UButton>(TEXT("PinkTeamButton"));
	}
	if (!BlueTeamButton)
	{
		BlueTeamButton =
			WidgetTree->FindWidget<UButton>(TEXT("BlueTeamButton"));
	}
	if (!WhiteTeamButton)
	{
		WhiteTeamButton =
			WidgetTree->FindWidget<UButton>(TEXT("WhiteTeamButton"));
	}
	if (!PvpModeButton)
	{
		PvpModeButton =
			WidgetTree->FindWidget<UButton>(TEXT("PvpModeButton"));
	}
	if (!SnowmanModeButton)
	{
		SnowmanModeButton =
			WidgetTree->FindWidget<UButton>(TEXT("SnowmanModeButton"));
	}
	if (!ReadyStartButton)
	{
		ReadyStartButton =
			WidgetTree->FindWidget<UButton>(TEXT("ReadyStartButton"));
	}
	if (!ReadyStartButtonText)
	{
		ReadyStartButtonText =
			WidgetTree->FindWidget<UTextBlock>(TEXT("ReadyStartButtonText"));
	}
	if (!Round1Button)
	{
		Round1Button =
			WidgetTree->FindWidget<UButton>(TEXT("Round1Button"));
	}
	if (!Round3Button)
	{
		Round3Button =
			WidgetTree->FindWidget<UButton>(TEXT("Round3Button"));
	}
	if (!Round5Button)
	{
		Round5Button =
			WidgetTree->FindWidget<UButton>(TEXT("Round5Button"));
	}
	if (!Shuffle2TeamsButton)
	{
		Shuffle2TeamsButton =
			WidgetTree->FindWidget<UButton>(TEXT("Shuffle2TeamsButton"));
	}
	if (!Shuffle3TeamsButton)
	{
		Shuffle3TeamsButton =
			WidgetTree->FindWidget<UButton>(TEXT("Shuffle3TeamsButton"));
	}
	if (!Shuffle4TeamsButton)
	{
		Shuffle4TeamsButton =
			WidgetTree->FindWidget<UButton>(TEXT("Shuffle4TeamsButton"));
	}
	if (!ShuffleSoloButton)
	{
		ShuffleSoloButton =
			WidgetTree->FindWidget<UButton>(TEXT("ShuffleSoloButton"));
	}
	if (!SlowGameSpeedButton)
	{
		SlowGameSpeedButton =
			WidgetTree->FindWidget<UButton>(TEXT("SlowGameSpeedButton"));
	}
	if (!NormalGameSpeedButton)
	{
		NormalGameSpeedButton =
			WidgetTree->FindWidget<UButton>(TEXT("NormalGameSpeedButton"));
	}
	if (!FastGameSpeedButton)
	{
		FastGameSpeedButton =
			WidgetTree->FindWidget<UButton>(TEXT("FastGameSpeedButton"));
	}
	if (!MatchRoundLimitText)
	{
		MatchRoundLimitText =
			WidgetTree->FindWidget<UTextBlock>(TEXT("MatchRoundLimitText"));
	}
}

void ULobbyBoardWidget::ResolveTeamCountTexts()
{
	if (!WidgetTree)
	{
		return;
	}

	if (!RedTeamCountText)
	{
		RedTeamCountText =
			WidgetTree->FindWidget<UTextBlock>(TEXT("RedTeamCountText"));
	}
	if (!SkyTeamCountText)
	{
		SkyTeamCountText =
			WidgetTree->FindWidget<UTextBlock>(TEXT("SkyTeamCountText"));
	}
	if (!GreenTeamCountText)
	{
		GreenTeamCountText =
			WidgetTree->FindWidget<UTextBlock>(TEXT("GreenTeamCountText"));
	}
	if (!YellowTeamCountText)
	{
		YellowTeamCountText =
			WidgetTree->FindWidget<UTextBlock>(TEXT("YellowTeamCountText"));
	}
	if (!PurpleTeamCountText)
	{
		PurpleTeamCountText =
			WidgetTree->FindWidget<UTextBlock>(TEXT("PurpleTeamCountText"));
	}
	if (!PinkTeamCountText)
	{
		PinkTeamCountText =
			WidgetTree->FindWidget<UTextBlock>(TEXT("PinkTeamCountText"));
	}
	if (!BlueTeamCountText)
	{
		BlueTeamCountText =
			WidgetTree->FindWidget<UTextBlock>(TEXT("BlueTeamCountText"));
	}
	if (!WhiteTeamCountText)
	{
		WhiteTeamCountText =
			WidgetTree->FindWidget<UTextBlock>(TEXT("WhiteTeamCountText"));
	}
}

void ULobbyBoardWidget::BindBoardButtons()
{
	if (BoardActionButton0)
	{
		BoardActionButton0->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleActionButton0Clicked);
	}
	if (BoardActionButton1)
	{
		BoardActionButton1->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleActionButton1Clicked);
	}
	if (BoardActionButton2)
	{
		BoardActionButton2->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleActionButton2Clicked);
	}
	if (BoardActionButton3)
	{
		BoardActionButton3->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleActionButton3Clicked);
	}
	if (CloseFocusButton)
	{
		CloseFocusButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleCloseFocusButtonClicked);
	}
	if (RedTeamButton)
	{
		RedTeamButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleRedTeamButtonClicked);
	}
	if (SkyTeamButton)
	{
		SkyTeamButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleSkyTeamButtonClicked);
	}
	if (GreenTeamButton)
	{
		GreenTeamButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleGreenTeamButtonClicked);
	}
	if (YellowTeamButton)
	{
		YellowTeamButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleYellowTeamButtonClicked);
	}
	if (PurpleTeamButton)
	{
		PurpleTeamButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandlePurpleTeamButtonClicked);
	}
	if (PinkTeamButton)
	{
		PinkTeamButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandlePinkTeamButtonClicked);
	}
	if (BlueTeamButton)
	{
		BlueTeamButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleBlueTeamButtonClicked);
	}
	if (WhiteTeamButton)
	{
		WhiteTeamButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleWhiteTeamButtonClicked);
	}
	if (PvpModeButton)
	{
		PvpModeButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandlePvpModeButtonClicked);
	}
	if (SnowmanModeButton)
	{
		SnowmanModeButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleSnowmanModeButtonClicked);
	}
	if (ReadyStartButton)
	{
		ReadyStartButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleReadyStartButtonClicked);
	}
	if (Round1Button)
	{
		Round1Button->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleRound1ButtonClicked);
	}
	if (Round3Button)
	{
		Round3Button->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleRound3ButtonClicked);
	}
	if (Round5Button)
	{
		Round5Button->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleRound5ButtonClicked);
	}
	if (Shuffle2TeamsButton)
	{
		Shuffle2TeamsButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleShuffle2TeamsButtonClicked);
	}
	if (Shuffle3TeamsButton)
	{
		Shuffle3TeamsButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleShuffle3TeamsButtonClicked);
	}
	if (Shuffle4TeamsButton)
	{
		Shuffle4TeamsButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleShuffle4TeamsButtonClicked);
	}
	if (ShuffleSoloButton)
	{
		ShuffleSoloButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleShuffleSoloButtonClicked);
	}
	if (SlowGameSpeedButton)
	{
		SlowGameSpeedButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleSlowGameSpeedButtonClicked);
	}
	if (NormalGameSpeedButton)
	{
		NormalGameSpeedButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleNormalGameSpeedButtonClicked);
	}
	if (FastGameSpeedButton)
	{
		FastGameSpeedButton->OnClicked.AddUniqueDynamic(
			this,
			&ULobbyBoardWidget::HandleFastGameSpeedButtonClicked);
	}
}

void ULobbyBoardWidget::UnbindBoardButtons()
{
	if (BoardActionButton0)
	{
		BoardActionButton0->OnClicked.RemoveAll(this);
	}
	if (BoardActionButton1)
	{
		BoardActionButton1->OnClicked.RemoveAll(this);
	}
	if (BoardActionButton2)
	{
		BoardActionButton2->OnClicked.RemoveAll(this);
	}
	if (BoardActionButton3)
	{
		BoardActionButton3->OnClicked.RemoveAll(this);
	}
	if (CloseFocusButton)
	{
		CloseFocusButton->OnClicked.RemoveAll(this);
	}
	if (RedTeamButton)
	{
		RedTeamButton->OnClicked.RemoveAll(this);
	}
	if (SkyTeamButton)
	{
		SkyTeamButton->OnClicked.RemoveAll(this);
	}
	if (GreenTeamButton)
	{
		GreenTeamButton->OnClicked.RemoveAll(this);
	}
	if (YellowTeamButton)
	{
		YellowTeamButton->OnClicked.RemoveAll(this);
	}
	if (PurpleTeamButton)
	{
		PurpleTeamButton->OnClicked.RemoveAll(this);
	}
	if (PinkTeamButton)
	{
		PinkTeamButton->OnClicked.RemoveAll(this);
	}
	if (BlueTeamButton)
	{
		BlueTeamButton->OnClicked.RemoveAll(this);
	}
	if (WhiteTeamButton)
	{
		WhiteTeamButton->OnClicked.RemoveAll(this);
	}
	if (PvpModeButton)
	{
		PvpModeButton->OnClicked.RemoveAll(this);
	}
	if (SnowmanModeButton)
	{
		SnowmanModeButton->OnClicked.RemoveAll(this);
	}
	if (ReadyStartButton)
	{
		ReadyStartButton->OnClicked.RemoveAll(this);
	}
	if (Round1Button)
	{
		Round1Button->OnClicked.RemoveAll(this);
	}
	if (Round3Button)
	{
		Round3Button->OnClicked.RemoveAll(this);
	}
	if (Round5Button)
	{
		Round5Button->OnClicked.RemoveAll(this);
	}
	if (Shuffle2TeamsButton)
	{
		Shuffle2TeamsButton->OnClicked.RemoveAll(this);
	}
	if (Shuffle3TeamsButton)
	{
		Shuffle3TeamsButton->OnClicked.RemoveAll(this);
	}
	if (Shuffle4TeamsButton)
	{
		Shuffle4TeamsButton->OnClicked.RemoveAll(this);
	}
	if (ShuffleSoloButton)
	{
		ShuffleSoloButton->OnClicked.RemoveAll(this);
	}
	if (SlowGameSpeedButton)
	{
		SlowGameSpeedButton->OnClicked.RemoveAll(this);
	}
	if (NormalGameSpeedButton)
	{
		NormalGameSpeedButton->OnClicked.RemoveAll(this);
	}
	if (FastGameSpeedButton)
	{
		FastGameSpeedButton->OnClicked.RemoveAll(this);
	}
}

void ULobbyBoardWidget::HandleActionButton0Clicked()
{
	PlayBoardClickSound();
	SubmitBoardAction(ELobbyBoardAction::Action0);
}

void ULobbyBoardWidget::HandleActionButton1Clicked()
{
	PlayBoardClickSound();
	SubmitBoardAction(ELobbyBoardAction::Action1);
}

void ULobbyBoardWidget::HandleActionButton2Clicked()
{
	PlayBoardClickSound();
	SubmitBoardAction(ELobbyBoardAction::Action2);
}

void ULobbyBoardWidget::HandleActionButton3Clicked()
{
	PlayBoardClickSound();
	SubmitBoardAction(ELobbyBoardAction::Action3);
}

void ULobbyBoardWidget::HandleCloseFocusButtonClicked()
{
	PlayBoardClickSound();
	if (FocusedCharacter)
	{
		FocusedCharacter->CloseLobbyBoardFocus();
	}
}

void ULobbyBoardWidget::HandleRedTeamButtonClicked()
{
	PlayBoardClickSound();
	SubmitTeamColor(ELobbyBoardTeamColor::Red);
}

void ULobbyBoardWidget::HandleSkyTeamButtonClicked()
{
	PlayBoardClickSound();
	SubmitTeamColor(ELobbyBoardTeamColor::Sky);
}

void ULobbyBoardWidget::HandleGreenTeamButtonClicked()
{
	PlayBoardClickSound();
	SubmitTeamColor(ELobbyBoardTeamColor::Green);
}

void ULobbyBoardWidget::HandleYellowTeamButtonClicked()
{
	PlayBoardClickSound();
	SubmitTeamColor(ELobbyBoardTeamColor::Yellow);
}

void ULobbyBoardWidget::HandlePurpleTeamButtonClicked()
{
	PlayBoardClickSound();
	SubmitTeamColor(ELobbyBoardTeamColor::Purple);
}

void ULobbyBoardWidget::HandlePinkTeamButtonClicked()
{
	PlayBoardClickSound();
	SubmitTeamColor(ELobbyBoardTeamColor::Pink);
}

void ULobbyBoardWidget::HandleBlueTeamButtonClicked()
{
	PlayBoardClickSound();
	SubmitTeamColor(ELobbyBoardTeamColor::Blue);
}

void ULobbyBoardWidget::HandleWhiteTeamButtonClicked()
{
	PlayBoardClickSound();
	SubmitTeamColor(ELobbyBoardTeamColor::White);
}

void ULobbyBoardWidget::HandlePvpModeButtonClicked()
{
	PlayBoardClickSound();
	SubmitLobbyMode(ELobbyBoardGameMode::Pvp);
}

void ULobbyBoardWidget::HandleSnowmanModeButtonClicked()
{
	PlayBoardClickSound();
	SubmitLobbyMode(ELobbyBoardGameMode::Snowman);
}

void ULobbyBoardWidget::HandleReadyStartButtonClicked()
{
	PlayBoardClickSound();
	ASnowRumblePlayerState* PlayerState = GetRequestingPlayerState();
	if (!PlayerState)
	{
		return;
	}

	if (IsRequestingPlayerHost())
	{
		const UWorld* World = GetWorld();
		const ASnowRumbleLobbyGameState* LobbyGameState = World
			? World->GetGameState<ASnowRumbleLobbyGameState>()
			: nullptr;
		if (!LobbyGameState || !LobbyGameState->CanStartLobbyMatch())
		{
			ShowInvalidActionFeedback(
				LobbyGameState
					? LobbyGameState->GetStartMatchInvalidReasonText()
					: NSLOCTEXT(
						"SnowRumble",
						"LobbyBoardInvalidStartFallback",
						"게임을 시작할 수 없습니다."));
		}
		else
		{
			PlayerState->RequestStartLobbyMatch();
		}
	}
	else
	{
		PlayerState->RequestSetLobbyReady(!PlayerState->IsLobbyReady());
	}

	RefreshReadyStartButtonText();
}

void ULobbyBoardWidget::HandleRound1ButtonClicked()
{
	PlayBoardClickSound();
	SubmitMatchRoundLimit(1);
}

void ULobbyBoardWidget::HandleRound3ButtonClicked()
{
	PlayBoardClickSound();
	SubmitMatchRoundLimit(3);
}

void ULobbyBoardWidget::HandleRound5ButtonClicked()
{
	PlayBoardClickSound();
	SubmitMatchRoundLimit(5);
}

void ULobbyBoardWidget::HandleShuffle2TeamsButtonClicked()
{
	PlayBoardClickSound();
	SubmitShuffleTeams(2);
}

void ULobbyBoardWidget::HandleShuffle3TeamsButtonClicked()
{
	PlayBoardClickSound();
	SubmitShuffleTeams(3);
}

void ULobbyBoardWidget::HandleShuffle4TeamsButtonClicked()
{
	PlayBoardClickSound();
	SubmitShuffleTeams(4);
}

void ULobbyBoardWidget::HandleShuffleSoloButtonClicked()
{
	PlayBoardClickSound();
	SubmitShuffleSolo();
}

void ULobbyBoardWidget::HandleSlowGameSpeedButtonClicked()
{
	PlayBoardClickSound();
	SubmitGameSpeed(ESnowRumbleGameSpeed::Slow);
}

void ULobbyBoardWidget::HandleNormalGameSpeedButtonClicked()
{
	PlayBoardClickSound();
	SubmitGameSpeed(ESnowRumbleGameSpeed::Normal);
}

void ULobbyBoardWidget::HandleFastGameSpeedButtonClicked()
{
	PlayBoardClickSound();
	SubmitGameSpeed(ESnowRumbleGameSpeed::Fast);
}

void ULobbyBoardWidget::SubmitBoardAction(ELobbyBoardAction BoardAction)
{
	if (!FocusedCharacter || !OwningBoard)
	{
		return;
	}

	FocusedCharacter->RequestLobbyBoardAction(BoardAction);
}

void ULobbyBoardWidget::SubmitTeamColor(ELobbyBoardTeamColor TeamColor)
{
	SubmitTeamColorFromBlueprint(TeamColor);
}

void ULobbyBoardWidget::SubmitTeamColorFromBlueprint(
	ELobbyBoardTeamColor TeamColor)
{
	const ESnowRumbleTeam SnowRumbleTeam = ToSnowRumbleTeam(TeamColor);
	const ASnowRumblePlayerState* PlayerState = GetRequestingPlayerState();
	if (PlayerState && PlayerState->IsLobbyReady()
		&& PlayerState->GetLobbyTeam() != SnowRumbleTeam)
	{
		ShowInvalidActionFeedback(
			NSLOCTEXT(
				"SnowRumble",
				"LobbyBoardInvalidTeamChangeWhileReady",
				"준비 완료 상태에서는 팀 색을 변경할 수 없습니다."));
		return;
	}

	if (FocusedCharacter)
	{
		FocusedCharacter->RequestLobbyTeamSelection(SnowRumbleTeam);
	}
	else if (ALobbyPlayerController* LobbyPlayerController =
		GetRequestingLobbyPlayerController())
	{
		LobbyPlayerController->RequestApplyLobbyTeam(SnowRumbleTeam);
	}

	OnTeamColorButtonClicked(TeamColor);
}

ALobbyPlayerController* ULobbyBoardWidget::GetRequestingLobbyPlayerController()
	const
{
	if (FocusedPlayerController && FocusedPlayerController->IsLocalController())
	{
		return FocusedPlayerController;
	}

	if (ALobbyPlayerController* OwningLobbyPlayerController =
		Cast<ALobbyPlayerController>(GetOwningPlayer()))
	{
		if (OwningLobbyPlayerController->IsLocalController())
		{
			return OwningLobbyPlayerController;
		}
	}

	return nullptr;
}

void ULobbyBoardWidget::SubmitLobbyMode(ELobbyBoardGameMode GameMode)
{
	if (!IsRequestingPlayerHost())
	{
		ShowInvalidActionFeedback(
			NSLOCTEXT(
				"SnowRumble",
				"LobbyBoardInvalidHostOnlySetting",
				"방 설정은 호스트만 변경할 수 있습니다."));
		OnLobbyModeButtonClicked(GameMode);
		return;
	}

	UWorld* World = GetWorld();
	if (ASnowRumbleLobbyGameState* LobbyGameState = World
		? World->GetGameState<ASnowRumbleLobbyGameState>()
		: nullptr)
	{
		LobbyGameState->SetLobbyModeFromServer(ToLobbyMode(GameMode));
	}

	OnLobbyModeButtonClicked(GameMode);
}

void ULobbyBoardWidget::RefreshTeamCountTexts()
{
	SetTeamCountText(RedTeamCountText, ESnowRumbleTeam::Red);
	SetTeamCountText(SkyTeamCountText, ESnowRumbleTeam::Sky);
	SetTeamCountText(GreenTeamCountText, ESnowRumbleTeam::Green);
	SetTeamCountText(YellowTeamCountText, ESnowRumbleTeam::Yellow);
	SetTeamCountText(PurpleTeamCountText, ESnowRumbleTeam::Purple);
	SetTeamCountText(PinkTeamCountText, ESnowRumbleTeam::Pink);
	SetTeamCountText(BlueTeamCountText, ESnowRumbleTeam::Blue);
	SetTeamCountText(WhiteTeamCountText, ESnowRumbleTeam::White);
}

void ULobbyBoardWidget::RefreshReadyStartButtonText()
{
	if (!ReadyStartButtonText)
	{
		return;
	}

	if (IsRequestingPlayerHost())
	{
		ReadyStartButtonText->SetText(FText::FromString(TEXT("게임 시작")));
		return;
	}

	const ASnowRumblePlayerState* PlayerState = GetRequestingPlayerState();
	const bool bReady = PlayerState && PlayerState->IsLobbyReady();
	ReadyStartButtonText->SetText(FText::FromString(
		bReady ? TEXT("준비 취소") : TEXT("준비 완료")));
}

void ULobbyBoardWidget::RefreshMatchRoundLimitText()
{
	if (!MatchRoundLimitText)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const ASnowRumbleLobbyGameState* LobbyGameState = World
		? World->GetGameState<ASnowRumbleLobbyGameState>()
		: nullptr;
	const int32 MatchRoundLimit = LobbyGameState
		? LobbyGameState->GetMatchRoundLimit()
		: 1;
	MatchRoundLimitText->SetText(FText::Format(
		NSLOCTEXT("SnowRumble", "LobbyMatchRoundLimitFormat", "{0} 라운드"),
		FText::AsNumber(MatchRoundLimit)));
}

void ULobbyBoardWidget::RefreshSelectedButtonVisuals()
{
	const UWorld* World = GetWorld();
	const ASnowRumbleLobbyGameState* LobbyGameState = World
		? World->GetGameState<ASnowRumbleLobbyGameState>()
		: nullptr;
	const ASnowRumblePlayerState* PlayerState = GetRequestingPlayerState();

	const ESnowRumbleTeam SelectedTeam = PlayerState
		? PlayerState->GetLobbyTeam()
		: ESnowRumbleTeam::None;
	SetButtonSelectedVisual(
		RedTeamButton,
		SelectedTeam == ESnowRumbleTeam::Red);
	SetButtonSelectedVisual(
		SkyTeamButton,
		SelectedTeam == ESnowRumbleTeam::Sky);
	SetButtonSelectedVisual(
		GreenTeamButton,
		SelectedTeam == ESnowRumbleTeam::Green);
	SetButtonSelectedVisual(
		YellowTeamButton,
		SelectedTeam == ESnowRumbleTeam::Yellow);
	SetButtonSelectedVisual(
		PurpleTeamButton,
		SelectedTeam == ESnowRumbleTeam::Purple);
	SetButtonSelectedVisual(
		PinkTeamButton,
		SelectedTeam == ESnowRumbleTeam::Pink);
	SetButtonSelectedVisual(
		BlueTeamButton,
		SelectedTeam == ESnowRumbleTeam::Blue);
	SetButtonSelectedVisual(
		WhiteTeamButton,
		SelectedTeam == ESnowRumbleTeam::White);

	const ESnowRumbleLobbyMode LobbyMode = LobbyGameState
		? LobbyGameState->GetLobbyMode()
		: ESnowRumbleLobbyMode::Pvp;
	SetButtonSelectedVisual(
		PvpModeButton,
		LobbyMode == ESnowRumbleLobbyMode::Pvp);
	SetButtonSelectedVisual(
		SnowmanModeButton,
		LobbyMode == ESnowRumbleLobbyMode::Snowman);

	const bool bReady = PlayerState && PlayerState->IsLobbyReady();
	SetButtonSelectedVisual(
		ReadyStartButton,
		!IsRequestingPlayerHost() && bReady);

	const int32 MatchRoundLimit = LobbyGameState
		? LobbyGameState->GetMatchRoundLimit()
		: 1;
	SetButtonSelectedVisual(Round1Button, MatchRoundLimit == 1);
	SetButtonSelectedVisual(Round3Button, MatchRoundLimit == 3);
	SetButtonSelectedVisual(Round5Button, MatchRoundLimit == 5);

	const ESnowRumbleGameSpeed GameSpeed = LobbyGameState
		? LobbyGameState->GetGameSpeed()
		: ESnowRumbleGameSpeed::Normal;
	SetButtonSelectedVisual(
		SlowGameSpeedButton,
		GameSpeed == ESnowRumbleGameSpeed::Slow);
	SetButtonSelectedVisual(
		NormalGameSpeedButton,
		GameSpeed == ESnowRumbleGameSpeed::Normal);
	SetButtonSelectedVisual(
		FastGameSpeedButton,
		GameSpeed == ESnowRumbleGameSpeed::Fast);
}

void ULobbyBoardWidget::SetButtonSelectedVisual(UButton* Button, bool bSelected)
{
	if (!Button)
	{
		return;
	}

	FButtonStyle* CachedStyle = DefaultButtonStyles.Find(Button);
	if (!CachedStyle)
	{
		DefaultButtonStyles.Add(Button, Button->GetStyle());
		CachedStyle = DefaultButtonStyles.Find(Button);
	}
	if (!CachedStyle)
	{
		return;
	}

	if (!bSelected)
	{
		Button->SetStyle(*CachedStyle);
		return;
	}

	FButtonStyle SelectedStyle = *CachedStyle;
	SelectedStyle.SetNormal(CachedStyle->Pressed);
	SelectedStyle.SetHovered(CachedStyle->Pressed);
	SelectedStyle.SetPressed(CachedStyle->Pressed);
	Button->SetStyle(SelectedStyle);
}

void ULobbyBoardWidget::SubmitMatchRoundLimit(int32 NewRoundLimit)
{
	if (!IsRequestingPlayerHost())
	{
		ShowInvalidActionFeedback(
			NSLOCTEXT(
				"SnowRumble",
				"LobbyBoardInvalidHostOnlyRound",
				"라운드 수는 호스트만 변경할 수 있습니다."));
		return;
	}

	UWorld* World = GetWorld();
	if (ASnowRumbleLobbyGameState* LobbyGameState = World
		? World->GetGameState<ASnowRumbleLobbyGameState>()
		: nullptr)
	{
		LobbyGameState->SetMatchRoundLimitFromServer(NewRoundLimit);
	}
}

void ULobbyBoardWidget::PlayBoardClickSound() const
{
	SnowRumbleAudio::PlaySound2D(
		this,
		BoardClickSound,
		ESnowRumbleAudioMixChannel::UserInterface);
}

void ULobbyBoardWidget::SubmitGameSpeed(ESnowRumbleGameSpeed NewGameSpeed)
{
	if (!IsRequestingPlayerHost())
	{
		ShowInvalidActionFeedback(
			NSLOCTEXT(
				"SnowRumble",
				"LobbyBoardInvalidHostOnlyGameSpeed",
				"게임 속도는 호스트만 변경할 수 있습니다."));
		return;
	}

	UWorld* World = GetWorld();
	if (ASnowRumbleLobbyGameState* LobbyGameState = World
		? World->GetGameState<ASnowRumbleLobbyGameState>()
		: nullptr)
	{
		LobbyGameState->SetGameSpeedFromServer(NewGameSpeed);
	}
}

void ULobbyBoardWidget::SubmitShuffleTeams(int32 TeamCount)
{
	if (!IsRequestingPlayerHost())
	{
		ShowInvalidActionFeedback(
			NSLOCTEXT(
				"SnowRumble",
				"LobbyBoardInvalidHostOnlyShuffle",
				"팀 섞기는 호스트만 사용할 수 있습니다."));
		return;
	}

	const UWorld* World = GetWorld();
	const ASnowRumbleLobbyGameState* LobbyGameState = World
		? World->GetGameState<ASnowRumbleLobbyGameState>()
		: nullptr;
	const int32 LobbyPlayerCount = LobbyGameState
		? LobbyGameState->GetLobbyPlayers().Num()
		: 0;
	if (LobbyPlayerCount < TeamCount)
	{
		ShowInvalidActionFeedback(
			NSLOCTEXT(
				"SnowRumble",
				"LobbyBoardInvalidShuffleNeedPlayers",
				"선택한 팀 수보다 플레이어가 적습니다."));
		return;
	}

	ASnowRumbleLobbyGameMode* LobbyGameMode = World
		? World->GetAuthGameMode<ASnowRumbleLobbyGameMode>()
		: nullptr;
	if (LobbyGameMode)
	{
		LobbyGameMode->ShuffleLobbyTeamsFromServer(TeamCount);
	}
}

void ULobbyBoardWidget::SubmitShuffleSolo()
{
	if (!IsRequestingPlayerHost())
	{
		ShowInvalidActionFeedback(
			NSLOCTEXT(
				"SnowRumble",
				"LobbyBoardInvalidHostOnlySoloShuffle",
				"개인전 섞기는 호스트만 사용할 수 있습니다."));
		return;
	}

	ASnowRumbleLobbyGameMode* LobbyGameMode = nullptr;
	if (UWorld* World = GetWorld())
	{
		LobbyGameMode = World->GetAuthGameMode<ASnowRumbleLobbyGameMode>();
	}
	if (LobbyGameMode)
	{
		LobbyGameMode->ShuffleLobbyPlayersIndividuallyFromServer();
	}
}

void ULobbyBoardWidget::ShowInvalidActionFeedback(const FText& ReasonText)
{
	if (ALobbyPlayerController* LobbyPlayerController =
		GetRequestingLobbyPlayerController())
	{
		LobbyPlayerController->ShowLobbyInvalidActionFeedback(ReasonText);
	}

	if (InvalidActionReasonText)
	{
		InvalidActionReasonText->SetText(ReasonText);
		InvalidActionReasonText->SetVisibility(
			ESlateVisibility::SelfHitTestInvisible);
	}
	if (InvalidActionAnimation)
	{
		PlayAnimation(InvalidActionAnimation);
	}
	OnInvalidActionFeedback(ReasonText);
}

void ULobbyBoardWidget::SetTeamCountText(
	UTextBlock* CountText,
	ESnowRumbleTeam Team) const
{
	if (!CountText)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const ASnowRumbleLobbyGameState* LobbyGameState = World
		? World->GetGameState<ASnowRumbleLobbyGameState>()
		: nullptr;
	const int32 TeamPlayerCount = LobbyGameState
		? LobbyGameState->GetLobbyTeamPlayerCount(Team)
		: 0;
	CountText->SetText(FText::AsNumber(TeamPlayerCount));
}

ASnowRumblePlayerState* ULobbyBoardWidget::GetRequestingPlayerState() const
{
	ALobbyPlayerController* LobbyPlayerController =
		GetRequestingLobbyPlayerController();
	return LobbyPlayerController
		? LobbyPlayerController->GetPlayerState<ASnowRumblePlayerState>()
		: nullptr;
}

bool ULobbyBoardWidget::IsRequestingPlayerHost() const
{
	const ALobbyPlayerController* LobbyPlayerController =
		GetRequestingLobbyPlayerController();
	return LobbyPlayerController && LobbyPlayerController->IsLocalController()
		&& LobbyPlayerController->HasAuthority();
}

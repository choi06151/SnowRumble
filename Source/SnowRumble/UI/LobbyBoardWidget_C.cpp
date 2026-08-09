// Copyright Epic Games, Inc. All Rights Reserved.

#include "LobbyBoardWidget_C.h"

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
}

void ULobbyBoardWidget::HandleActionButton0Clicked()
{
	SubmitBoardAction(ELobbyBoardAction::Action0);
}

void ULobbyBoardWidget::HandleActionButton1Clicked()
{
	SubmitBoardAction(ELobbyBoardAction::Action1);
}

void ULobbyBoardWidget::HandleActionButton2Clicked()
{
	SubmitBoardAction(ELobbyBoardAction::Action2);
}

void ULobbyBoardWidget::HandleActionButton3Clicked()
{
	SubmitBoardAction(ELobbyBoardAction::Action3);
}

void ULobbyBoardWidget::HandleCloseFocusButtonClicked()
{
	if (FocusedCharacter)
	{
		FocusedCharacter->CloseLobbyBoardFocus();
	}
}

void ULobbyBoardWidget::HandleRedTeamButtonClicked()
{
	SubmitTeamColor(ELobbyBoardTeamColor::Red);
}

void ULobbyBoardWidget::HandleSkyTeamButtonClicked()
{
	SubmitTeamColor(ELobbyBoardTeamColor::Sky);
}

void ULobbyBoardWidget::HandleGreenTeamButtonClicked()
{
	SubmitTeamColor(ELobbyBoardTeamColor::Green);
}

void ULobbyBoardWidget::HandleYellowTeamButtonClicked()
{
	SubmitTeamColor(ELobbyBoardTeamColor::Yellow);
}

void ULobbyBoardWidget::HandlePurpleTeamButtonClicked()
{
	SubmitTeamColor(ELobbyBoardTeamColor::Purple);
}

void ULobbyBoardWidget::HandlePinkTeamButtonClicked()
{
	SubmitTeamColor(ELobbyBoardTeamColor::Pink);
}

void ULobbyBoardWidget::HandleBlueTeamButtonClicked()
{
	SubmitTeamColor(ELobbyBoardTeamColor::Blue);
}

void ULobbyBoardWidget::HandleWhiteTeamButtonClicked()
{
	SubmitTeamColor(ELobbyBoardTeamColor::White);
}

void ULobbyBoardWidget::HandlePvpModeButtonClicked()
{
	SubmitLobbyMode(ELobbyBoardGameMode::Pvp);
}

void ULobbyBoardWidget::HandleSnowmanModeButtonClicked()
{
	SubmitLobbyMode(ELobbyBoardGameMode::Snowman);
}

void ULobbyBoardWidget::HandleReadyStartButtonClicked()
{
	ASnowRumblePlayerState* PlayerState = GetRequestingPlayerState();
	if (!PlayerState)
	{
		return;
	}

	if (IsRequestingPlayerHost())
	{
		PlayerState->RequestStartLobbyMatch();
	}
	else
	{
		PlayerState->RequestSetLobbyReady(!PlayerState->IsLobbyReady());
	}

	RefreshReadyStartButtonText();
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
	if (IsRequestingPlayerHost())
	{
		UWorld* World = GetWorld();
		if (ASnowRumbleLobbyGameState* LobbyGameState = World
			? World->GetGameState<ASnowRumbleLobbyGameState>()
			: nullptr)
		{
			LobbyGameState->SetLobbyModeFromServer(ToLobbyMode(GameMode));
		}
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

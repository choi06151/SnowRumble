// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainHUDWidget.h"

#include "../Game/SnowmanModeGameState_K.h"
#include "../Game/SnowRumbleGameState_C.h"
#include "../Game/SnowRumblePlayerState.h"
#include "../Player/SnowRumbleCharacter.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "HealthBarWidget.h"
#include "Blueprint/WidgetTree.h"

namespace
{
const TArray<ESnowRumbleTeam>& GetScoreboardTeamOrder()
{
	static const TArray<ESnowRumbleTeam> TeamOrder = {
		ESnowRumbleTeam::Red,
		ESnowRumbleTeam::Sky,
		ESnowRumbleTeam::Green,
		ESnowRumbleTeam::Yellow,
		ESnowRumbleTeam::Purple,
		ESnowRumbleTeam::Pink,
		ESnowRumbleTeam::Blue,
		ESnowRumbleTeam::White
	};
	return TeamOrder;
}

}

void UMainHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVoiceSpeakingPresentationVisible(false);
	RefreshHealthBars();
	RefreshCombatHudPresentation();
	RefreshStartCountdownPresentation();
	RefreshCurrentRoundPresentation();
	RefreshMatchTimerPresentation();
	RefreshEndRoundPresentation();
	RefreshTeamScorePresentation();
	RefreshEventLogText();
	RefreshVoiceSpeakingNamesText();
}

void UMainHUDWidget::NativeTick(
	const FGeometry& MyGeometry,
	float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshHealthBars();
	RefreshCombatHudPresentation();
	RefreshStartCountdownPresentation();
	RefreshCurrentRoundPresentation();
	RefreshMatchTimerPresentation();
	RefreshEndRoundPresentation();
	RefreshTeamScorePresentation();
	RefreshEventLogText();
	RefreshVoiceSpeakingNamesText();
}

void UMainHUDWidget::AddEventLogMessage(const FText& Message)
{
	if (Message.IsEmpty())
	{
		return;
	}

	const UWorld* World = GetWorld();
	FEventLogEntry NewEntry;
	NewEntry.Message = Message;
	NewEntry.ExpireTimeSeconds = World
		? World->GetTimeSeconds() + EventLogEntryVisibleSeconds
		: EventLogEntryVisibleSeconds;
	EventLogEntries.Add(NewEntry);
	RefreshEventLogText();
}

void UMainHUDWidget::ShowPersonalTextAlarm(const FText& Message)
{
	if (Message.IsEmpty())
	{
		return;
	}

	if (PersonalAlarmText)
	{
		PersonalAlarmText->SetText(Message);
		PersonalAlarmText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	if (PersonalAlarmAnimation)
	{
		PlayAnimation(PersonalAlarmAnimation);
	}
}

void UMainHUDWidget::RefreshHealthBars()
{
	UWorld* World = GetWorld();
	ASnowRumbleCharacter* LocalCharacter =
		Cast<ASnowRumbleCharacter>(GetOwningPlayerPawn());
	if (!World || !LocalCharacter)
	{
		return;
	}

	if (World->GetGameState<ASnowmanModeGameState>())
	{
		if (LocalHealthBar)
		{
			LocalHealthBar->SetVisibility(ESlateVisibility::Collapsed);
			LocalHealthBar->SetObservedActor(nullptr);
		}
		TSet<TWeakObjectPtr<ASnowRumbleCharacter>> EmptyValidPlayers;
		RemoveInvalidOtherPlayerHealthBars(EmptyValidPlayers);
		return;
	}

	if (LocalHealthBar)
	{
		LocalHealthBar->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		LocalHealthBar->SetObservedActor(LocalCharacter);
	}

	TSet<TWeakObjectPtr<ASnowRumbleCharacter>> ValidOtherPlayers;
	for (TActorIterator<ASnowRumbleCharacter> It(World); It; ++It)
	{
		ASnowRumbleCharacter* Character = *It;
		if (!ShouldShowOtherPlayerHealthBar(LocalCharacter, Character))
		{
			continue;
		}

		ValidOtherPlayers.Add(Character);
		if (OtherPlayerHealthBars.Contains(Character))
		{
			continue;
		}

		if (!OtherPlayersHealthPanel)
		{
			continue;
		}

		TSubclassOf<UHealthBarWidget> HealthBarWidgetClass =
			OtherPlayerHealthBarWidgetClass;
		if (!HealthBarWidgetClass && LocalHealthBar)
		{
			HealthBarWidgetClass = LocalHealthBar->GetClass();
		}
		if (!HealthBarWidgetClass)
		{
			continue;
		}

		UHealthBarWidget* NewHealthBar =
			CreateWidget<UHealthBarWidget>(
				GetOwningPlayer(),
				HealthBarWidgetClass);
		if (!NewHealthBar)
		{
			continue;
		}

		NewHealthBar->SetObservedActor(Character);
		OtherPlayersHealthPanel->AddChild(NewHealthBar);
		OtherPlayerHealthBars.Add(Character, NewHealthBar);
	}

	RemoveInvalidOtherPlayerHealthBars(ValidOtherPlayers);
}

void UMainHUDWidget::RefreshCombatHudPresentation()
{
	ASnowRumbleCharacter* LocalCharacter =
		Cast<ASnowRumbleCharacter>(GetOwningPlayerPawn());
	if (!LocalCharacter)
	{
		if (AimChargeProgressBar)
		{
			AimChargeProgressBar->SetPercent(0.0f);
			AimChargeProgressBar->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (AimCrosshair)
		{
			AimCrosshair->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	const bool bShouldShowAimCrosshair = LocalCharacter->IsAiming();
	if (AimCrosshair)
	{
		AimCrosshair->SetVisibility(
			bShouldShowAimCrosshair
				? ESlateVisibility::SelfHitTestInvisible
				: ESlateVisibility::Collapsed);
	}

	const bool bShouldShowChargeBar =
		LocalCharacter->IsChargingSnowball();
	if (AimChargeProgressBar)
	{
		AimChargeProgressBar->SetVisibility(
			bShouldShowChargeBar
				? ESlateVisibility::SelfHitTestInvisible
				: ESlateVisibility::Collapsed);
		AimChargeProgressBar->SetPercent(
			bShouldShowChargeBar
				? LocalCharacter->GetSnowballChargeProgress()
				: 0.0f);
	}
}

void UMainHUDWidget::RemoveInvalidOtherPlayerHealthBars(
	const TSet<TWeakObjectPtr<ASnowRumbleCharacter>>& ValidOtherPlayers)
{
	for (auto It = OtherPlayerHealthBars.CreateIterator(); It; ++It)
	{
		TWeakObjectPtr<ASnowRumbleCharacter> Character = It.Key();
		TWeakObjectPtr<UHealthBarWidget> HealthBar = It.Value();
		if (Character.IsValid() && ValidOtherPlayers.Contains(Character))
		{
			continue;
		}

		if (HealthBar.IsValid())
		{
			HealthBar->RemoveFromParent();
		}
		It.RemoveCurrent();
	}
}

void UMainHUDWidget::RefreshStartCountdownPresentation()
{
	if (!StartCountdownText)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const ASnowRumbleGameState* SnowRumbleGameState = World
		? World->GetGameState<ASnowRumbleGameState>()
		: nullptr;
	const ASnowmanModeGameState* SnowmanModeGameState = World
		? World->GetGameState<ASnowmanModeGameState>()
		: nullptr;
	if (SnowmanModeGameState
		&& SnowmanModeGameState->ShouldShowSnowmanModeStartCountdown())
	{
		StartCountdownText->SetText(
			SnowmanModeGameState->GetSnowmanModeStartCountdownText());
		StartCountdownText->SetVisibility(
			ESlateVisibility::SelfHitTestInvisible);
		return;
	}

	if (!SnowRumbleGameState
		|| !SnowRumbleGameState->ShouldShowStartCountdown())
	{
		StartCountdownText->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	StartCountdownText->SetText(
		SnowRumbleGameState->GetStartCountdownText());
	StartCountdownText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UMainHUDWidget::RefreshCurrentRoundPresentation()
{
	if (!CurrentRoundText)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const ASnowRumbleGameState* SnowRumbleGameState = World
		? World->GetGameState<ASnowRumbleGameState>()
		: nullptr;
	if (!SnowRumbleGameState)
	{
		CurrentRoundText->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	CurrentRoundText->SetText(SnowRumbleGameState->IsTiebreakerRound()
		? NSLOCTEXT("SnowRumble", "CurrentRoundTiebreaker", "단판승부")
		: FText::Format(
			NSLOCTEXT("SnowRumble", "CurrentRoundFormat", "라운드 {0} / {1}"),
			FText::AsNumber(SnowRumbleGameState->GetCurrentRoundNumber()),
			FText::AsNumber(SnowRumbleGameState->GetRoundLimit())));
	CurrentRoundText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UMainHUDWidget::RefreshMatchTimerPresentation()
{
	if (!MatchElapsedTimeText && !MapShrinkCountdownText)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const ASnowRumbleGameState* SnowRumbleGameState = World
		? World->GetGameState<ASnowRumbleGameState>()
		: nullptr;
	const ASnowmanModeGameState* SnowmanModeGameState = World
		? World->GetGameState<ASnowmanModeGameState>()
		: nullptr;
	if (SnowmanModeGameState)
	{
		const bool bShouldShowSnowmanMatchTime =
			SnowmanModeGameState->IsSnowmanModeTimerActive();

		if (MatchElapsedTimeText)
		{
			if (bShouldShowSnowmanMatchTime)
			{
				MatchElapsedTimeText->SetText(
					SnowmanModeGameState->GetSnowmanModeElapsedTimeText());
				MatchElapsedTimeText->SetVisibility(
					ESlateVisibility::SelfHitTestInvisible);
			}
			else
			{
				MatchElapsedTimeText->SetVisibility(ESlateVisibility::Collapsed);
			}
		}

		if (MapShrinkCountdownText)
		{
			MapShrinkCountdownText->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	const bool bShouldShowMatchTimers =
		SnowRumbleGameState
		&& !SnowRumbleGameState->IsMatchInputLocked()
		&& !SnowRumbleGameState->IsRoundEnded();

	if (MatchElapsedTimeText)
	{
		if (bShouldShowMatchTimers)
		{
			MatchElapsedTimeText->SetText(
				SnowRumbleGameState->GetRoundElapsedTimeText());
			MatchElapsedTimeText->SetVisibility(
				ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			MatchElapsedTimeText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (MapShrinkCountdownText)
	{
		if (bShouldShowMatchTimers
			&& !SnowRumbleGameState->IsTiebreakerRound())
		{
			MapShrinkCountdownText->SetText(
				SnowRumbleGameState->GetMapShrinkCountdownText());
			MapShrinkCountdownText->SetVisibility(
				ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			MapShrinkCountdownText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UMainHUDWidget::RefreshEndRoundPresentation()
{
	if (!EndRoundPanel && !EndRoundResultText)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const ASnowRumbleGameState* SnowRumbleGameState = World
		? World->GetGameState<ASnowRumbleGameState>()
		: nullptr;
	const bool bRoundEnded = SnowRumbleGameState
		&& SnowRumbleGameState->IsRoundEnded();

	if (EndRoundPanel)
	{
		EndRoundPanel->SetVisibility(
			bRoundEnded
				? ESlateVisibility::SelfHitTestInvisible
				: ESlateVisibility::Collapsed);
	}

	if (!EndRoundResultText)
	{
		return;
	}

	if (!bRoundEnded)
	{
		EndRoundResultText->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const UEnum* TeamEnum = StaticEnum<ESnowRumbleTeam>();
	const ESnowRumbleTeam ResultTeam = SnowRumbleGameState->IsMatchEnded()
		? SnowRumbleGameState->GetMatchWinningTeam()
		: SnowRumbleGameState->GetRoundWinningTeam();
	const FText TeamText = TeamEnum
		? TeamEnum->GetDisplayNameTextByValue(static_cast<int64>(ResultTeam))
		: FText::GetEmpty();
	EndRoundResultText->SetText(SnowRumbleGameState->IsMatchEnded()
		? FText::Format(
			NSLOCTEXT("SnowRumble", "MatchWinnerFormat", "{0} 1등"),
			TeamText)
		: FText::Format(
			NSLOCTEXT("SnowRumble", "RoundWinnerFormat", "{0} 승리"),
			TeamText));
	EndRoundResultText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UMainHUDWidget::RefreshTeamScorePresentation()
{
	const UWorld* World = GetWorld();
	const ASnowRumbleGameState* SnowRumbleGameState = World
		? World->GetGameState<ASnowRumbleGameState>()
		: nullptr;

	for (const ESnowRumbleTeam Team : GetScoreboardTeamOrder())
	{
		UWidget* DisplayWidget = GetTeamScoreDisplayWidget(Team);
		if (!DisplayWidget)
		{
			continue;
		}

		const bool bParticipating = IsTeamParticipating(Team);
		DisplayWidget->SetVisibility(
			bParticipating
				? ESlateVisibility::SelfHitTestInvisible
				: ESlateVisibility::Collapsed);
		UTextBlock* ScoreText = GetTeamScoreText(Team);
		SetTeamScoreText(ScoreText, Team);
		if (ScoreText && bParticipating)
		{
			ScoreText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}

	}
}

void UMainHUDWidget::RefreshEventLogText()
{
	if (!EventLogText)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const double CurrentTimeSeconds = World ? World->GetTimeSeconds() : 0.0;
	EventLogEntries.RemoveAll(
		[CurrentTimeSeconds](const FEventLogEntry& Entry)
		{
			return Entry.ExpireTimeSeconds <= CurrentTimeSeconds;
		});

	if (EventLogEntries.IsEmpty())
	{
		EventLogText->SetText(FText::GetEmpty());
		EventLogText->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	FString CombinedLog;
	for (const FEventLogEntry& Entry : EventLogEntries)
	{
		if (!CombinedLog.IsEmpty())
		{
			CombinedLog += LINE_TERMINATOR;
		}
		CombinedLog += Entry.Message.ToString();
	}

	EventLogText->SetText(FText::FromString(CombinedLog));
	EventLogText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UMainHUDWidget::RefreshVoiceSpeakingNamesText()
{
	if (!VoiceSpeakingNamesText && !VoiceSpeakingIcon
		&& !VoiceSpeakingContainer)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	if (!GameState)
	{
		SetVoiceSpeakingPresentationVisible(false);
		return;
	}

	FString SpeakingNames;
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		const ASnowRumblePlayerState* SnowRumblePlayerState =
			Cast<ASnowRumblePlayerState>(PlayerState);
		if (!SnowRumblePlayerState
			|| !ShouldShowVoiceSpeakingPlayer(SnowRumblePlayerState))
		{
			continue;
		}

		if (!SpeakingNames.IsEmpty())
		{
			SpeakingNames += LINE_TERMINATOR;
		}
		SpeakingNames += SnowRumblePlayerState->GetLobbyPlayerName();
	}

	if (SpeakingNames.IsEmpty())
	{
		if (VoiceSpeakingNamesText)
		{
			VoiceSpeakingNamesText->SetText(FText::GetEmpty());
		}
		SetVoiceSpeakingPresentationVisible(false);
		return;
	}

	if (VoiceSpeakingNamesText)
	{
		VoiceSpeakingNamesText->SetText(FText::FromString(SpeakingNames));
	}
	SetVoiceSpeakingPresentationVisible(true);
}

void UMainHUDWidget::SetVoiceSpeakingPresentationVisible(bool bVisible)
{
	const ESlateVisibility TargetVisibility = bVisible
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed;
	if (VoiceSpeakingContainer)
	{
		VoiceSpeakingContainer->SetVisibility(TargetVisibility);
	}
	if (VoiceSpeakingNamesText)
	{
		VoiceSpeakingNamesText->SetVisibility(TargetVisibility);
	}
	if (VoiceSpeakingIcon)
	{
		VoiceSpeakingIcon->SetVisibility(TargetVisibility);
	}
}

bool UMainHUDWidget::ShouldShowVoiceSpeakingPlayer(
	const ASnowRumblePlayerState* SenderPlayerState) const
{
	if (!SenderPlayerState || !SenderPlayerState->IsVoiceSpeaking())
	{
		return false;
	}
	if (SenderPlayerState->GetVoiceChannel() == ESnowRumbleVoiceChannel::All)
	{
		return true;
	}

	const APlayerController* OwningPlayerController = GetOwningPlayer();
	const ASnowRumblePlayerState* LocalPlayerState = OwningPlayerController
		? OwningPlayerController->GetPlayerState<ASnowRumblePlayerState>()
		: nullptr;
	return LocalPlayerState
		&& SenderPlayerState->GetLobbyTeam() != ESnowRumbleTeam::None
		&& SenderPlayerState->GetLobbyTeam()
			== LocalPlayerState->GetLobbyTeam();
}

void UMainHUDWidget::SetTeamScoreText(
	UTextBlock* ScoreText,
	ESnowRumbleTeam Team) const
{
	if (!ScoreText)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const ASnowRumbleGameState* SnowRumbleGameState = World
		? World->GetGameState<ASnowRumbleGameState>()
		: nullptr;
	const int32 TeamScore = SnowRumbleGameState
		? SnowRumbleGameState->GetTeamRoundWinCount(Team)
		: 0;
	ScoreText->SetText(FText::AsNumber(TeamScore));
}

UWidget* UMainHUDWidget::GetTeamScoreDisplayWidget(
	ESnowRumbleTeam Team) const
{
	switch (Team)
	{
	case ESnowRumbleTeam::Red:
		return RedTeamScoreRow
			? RedTeamScoreRow.Get()
			: FindTeamScoreRowWidget(Team);
	case ESnowRumbleTeam::Sky:
		return SkyTeamScoreRow
			? SkyTeamScoreRow.Get()
			: FindTeamScoreRowWidget(Team);
	case ESnowRumbleTeam::Green:
		return GreenTeamScoreRow
			? GreenTeamScoreRow.Get()
			: FindTeamScoreRowWidget(Team);
	case ESnowRumbleTeam::Yellow:
		return YellowTeamScoreRow
			? YellowTeamScoreRow.Get()
			: FindTeamScoreRowWidget(Team);
	case ESnowRumbleTeam::Purple:
		return PurpleTeamScoreRow
			? PurpleTeamScoreRow.Get()
			: FindTeamScoreRowWidget(Team);
	case ESnowRumbleTeam::Pink:
		return PinkTeamScoreRow
			? PinkTeamScoreRow.Get()
			: FindTeamScoreRowWidget(Team);
	case ESnowRumbleTeam::Blue:
		return BlueTeamScoreRow
			? BlueTeamScoreRow.Get()
			: FindTeamScoreRowWidget(Team);
	case ESnowRumbleTeam::White:
		return WhiteTeamScoreRow
			? WhiteTeamScoreRow.Get()
			: FindTeamScoreRowWidget(Team);
	default:
		return nullptr;
	}
}

UWidget* UMainHUDWidget::FindTeamScoreRowWidget(
	ESnowRumbleTeam Team) const
{
	if (!WidgetTree)
	{
		return GetTeamScoreText(Team);
	}

	FName RowName = NAME_None;
	switch (Team)
	{
	case ESnowRumbleTeam::Red:
		RowName = TEXT("RedTeamScoreRow");
		break;
	case ESnowRumbleTeam::Sky:
		RowName = TEXT("SkyTeamScoreRow");
		break;
	case ESnowRumbleTeam::Green:
		RowName = TEXT("GreenTeamScoreRow");
		break;
	case ESnowRumbleTeam::Yellow:
		RowName = TEXT("YellowTeamScoreRow");
		break;
	case ESnowRumbleTeam::Purple:
		RowName = TEXT("PurpleTeamScoreRow");
		break;
	case ESnowRumbleTeam::Pink:
		RowName = TEXT("PinkTeamScoreRow");
		break;
	case ESnowRumbleTeam::Blue:
		RowName = TEXT("BlueTeamScoreRow");
		break;
	case ESnowRumbleTeam::White:
		RowName = TEXT("WhiteTeamScoreRow");
		break;
	default:
		break;
	}

	UWidget* RowWidget = RowName != NAME_None
		? WidgetTree->FindWidget(RowName)
		: nullptr;
	return RowWidget ? RowWidget : GetTeamScoreText(Team);
}

UTextBlock* UMainHUDWidget::GetTeamScoreText(
	ESnowRumbleTeam Team) const
{
	switch (Team)
	{
	case ESnowRumbleTeam::Red:
		return RedTeamScoreText;
	case ESnowRumbleTeam::Sky:
		return SkyTeamScoreText;
	case ESnowRumbleTeam::Green:
		return GreenTeamScoreText;
	case ESnowRumbleTeam::Yellow:
		return YellowTeamScoreText;
	case ESnowRumbleTeam::Purple:
		return PurpleTeamScoreText;
	case ESnowRumbleTeam::Pink:
		return PinkTeamScoreText;
	case ESnowRumbleTeam::Blue:
		return BlueTeamScoreText;
	case ESnowRumbleTeam::White:
		return WhiteTeamScoreText;
	default:
		return nullptr;
	}
}

bool UMainHUDWidget::IsTeamParticipating(ESnowRumbleTeam Team) const
{
	const UWorld* World = GetWorld();
	const ASnowRumbleGameState* SnowRumbleGameState = World
		? World->GetGameState<ASnowRumbleGameState>()
		: nullptr;
	if (!SnowRumbleGameState)
	{
		return false;
	}

	for (APlayerState* PlayerState : SnowRumbleGameState->PlayerArray)
	{
		const ASnowRumblePlayerState* SnowRumblePlayerState =
			Cast<ASnowRumblePlayerState>(PlayerState);
		if (SnowRumblePlayerState
			&& SnowRumblePlayerState->GetLobbyTeam() == Team)
		{
			return true;
		}
	}
	return false;
}

bool UMainHUDWidget::ShouldShowOtherPlayerHealthBar(
	const ASnowRumbleCharacter* LocalCharacter,
	const ASnowRumbleCharacter* OtherCharacter) const
{
	if (!LocalCharacter
		|| !OtherCharacter
		|| LocalCharacter == OtherCharacter)
	{
		return false;
	}

	const ASnowRumblePlayerState* LocalPlayerState =
		LocalCharacter->GetPlayerState<ASnowRumblePlayerState>();
	const ASnowRumblePlayerState* OtherPlayerState =
		OtherCharacter->GetPlayerState<ASnowRumblePlayerState>();
	if (!LocalPlayerState || !OtherPlayerState)
	{
		return false;
	}

	const ESnowRumbleTeam LocalTeam = LocalPlayerState->GetLobbyTeam();
	return LocalTeam != ESnowRumbleTeam::None
		&& LocalTeam == OtherPlayerState->GetLobbyTeam();
}

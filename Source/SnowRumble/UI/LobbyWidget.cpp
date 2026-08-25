// Copyright Epic Games, Inc. All Rights Reserved.

#include "LobbyWidget.h"

#include "../Game/SnowRumbleLobbyGameState.h"
#include "../Online/SnowRumbleSessionSubsystem.h"
#include "../Player/LocalPlayerIdentitySubsystem_C.h"
#include "../Player/SnowRumbleCustomizationSubsystem_C.h"
#include "LobbyPlayerController.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"

namespace
{
FText GetLobbyModeText(ESnowRumbleLobbyMode LobbyMode)
{
	switch (LobbyMode)
	{
	case ESnowRumbleLobbyMode::Pvp:
		return NSLOCTEXT("SnowRumble", "LobbyModePvp", "PvP");
	case ESnowRumbleLobbyMode::Snowman:
		return NSLOCTEXT("SnowRumble", "LobbyModeSnowman", "눈사람 모드");
	default:
		return FText::GetEmpty();
	}
}

FText GetGameSpeedDisplayText(ESnowRumbleGameSpeed GameSpeed)
{
	switch (GameSpeed)
	{
	case ESnowRumbleGameSpeed::Slow:
		return NSLOCTEXT("SnowRumble", "LobbyGameSpeedSlow", "느리게");
	case ESnowRumbleGameSpeed::Fast:
		return NSLOCTEXT("SnowRumble", "LobbyGameSpeedFast", "빠르게");
	case ESnowRumbleGameSpeed::Normal:
	default:
		return NSLOCTEXT("SnowRumble", "LobbyGameSpeedNormal", "보통");
	}
}

FText GetTeamText(ESnowRumbleTeam Team)
{
	switch (Team)
	{
	case ESnowRumbleTeam::Red:
		return NSLOCTEXT("SnowRumble", "LobbyTeamRed", "빨강");
	case ESnowRumbleTeam::Sky:
		return NSLOCTEXT("SnowRumble", "LobbyTeamSky", "하늘");
	case ESnowRumbleTeam::Green:
		return NSLOCTEXT("SnowRumble", "LobbyTeamGreen", "초록");
	case ESnowRumbleTeam::Yellow:
		return NSLOCTEXT("SnowRumble", "LobbyTeamYellow", "노랑");
	case ESnowRumbleTeam::Purple:
		return NSLOCTEXT("SnowRumble", "LobbyTeamPurple", "보라");
	case ESnowRumbleTeam::Pink:
		return NSLOCTEXT("SnowRumble", "LobbyTeamPink", "핑크");
	case ESnowRumbleTeam::Blue:
		return NSLOCTEXT("SnowRumble", "LobbyTeamBlue", "파랑");
	case ESnowRumbleTeam::Orange:
		return NSLOCTEXT("SnowRumble", "LobbyTeamOrange", "주황");
	default:
		return NSLOCTEXT("SnowRumble", "LobbyTeamUnselected", "미선택");
	}
}
}

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVoiceSpeakingPresentationVisible(false);
	RefreshLobbyBindings();
	ApplyLocalPlayerIdentity();
	ApplyLocalPlayerCustomization();
	RefreshRoomCodeText();
	RefreshLobbyStatusTexts();
	RefreshEventLogText();
	RefreshVoiceSpeakingNamesText();
	OnLobbyStateChanged();
}

void ULobbyWidget::NativeDestruct()
{
	UnbindLobbyBindings();

	Super::NativeDestruct();
}

void ULobbyWidget::NativeTick(
	const FGeometry& MyGeometry,
	float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshLobbyBindings();
	ApplyLocalPlayerIdentity();
	ApplyLocalPlayerCustomization();
	RefreshRoomCodeText();
	RefreshLobbyStatusTexts();
	RefreshEventLogText();
	RefreshVoiceSpeakingNamesText();
}

TArray<ASnowRumblePlayerState*> ULobbyWidget::GetLobbyPlayers() const
{
	if (ASnowRumbleLobbyGameState* LobbyGameState = GetLobbyGameState())
	{
		return LobbyGameState->GetLobbyPlayers();
	}
	return {};
}

void ULobbyWidget::AddEventLogMessage(const FText& Message)
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

void ULobbyWidget::RequestSetLocalPlayerName(const FString& NewName)
{
	if (!ULocalPlayerIdentitySubsystem::IsPlayerNameAllowed(NewName))
	{
		ShowInvalidActionFeedback(NSLOCTEXT(
			"SnowRumble",
			"LobbyInvalidPlayerName",
			"적합하지 않은 이름입니다."));
		return;
	}

	if (ASnowRumblePlayerState* PlayerState =
		GetLocalSnowRumblePlayerState())
	{
		PlayerState->RequestSetLobbyPlayerName(NewName);
	}
}

void ULobbyWidget::RequestSetLocalPlayerTeam(ESnowRumbleTeam NewTeam)
{
	if (ASnowRumblePlayerState* PlayerState =
		GetLocalSnowRumblePlayerState())
	{
		if (PlayerState->IsLobbyReady()
			&& PlayerState->GetLobbyTeam() != NewTeam)
		{
			ShowInvalidActionFeedback(
				NSLOCTEXT(
					"SnowRumble",
					"LobbyInvalidTeamChangeWhileReady",
					"준비 완료 상태에서는 팀 색을 변경할 수 없습니다."));
			return;
		}

		PlayerState->RequestSetLobbyTeam(NewTeam);
	}
}

void ULobbyWidget::RequestSetLocalPlayerReady(bool bNewReady)
{
	if (ASnowRumblePlayerState* PlayerState =
		GetLocalSnowRumblePlayerState())
	{
		PlayerState->RequestSetLobbyReady(bNewReady);
	}
}

void ULobbyWidget::RequestStartMatch()
{
	if (!CanStartMatch())
	{
		const ASnowRumbleLobbyGameState* LobbyGameState = GetLobbyGameState();
		ShowInvalidActionFeedback(
			LobbyGameState
				? LobbyGameState->GetStartMatchInvalidReasonText()
				: NSLOCTEXT(
					"SnowRumble",
					"LobbyInvalidStartFallback",
					"게임을 시작할 수 없습니다."));
		return;
	}

	if (ASnowRumblePlayerState* PlayerState =
		GetLocalSnowRumblePlayerState())
	{
		PlayerState->RequestStartLobbyMatch();
	}
}

bool ULobbyWidget::IsLocalPlayerHost() const
{
	const APlayerController* PlayerController = GetOwningPlayer();
	return PlayerController && PlayerController->IsLocalController()
		&& PlayerController->HasAuthority();
}

bool ULobbyWidget::CanStartMatch() const
{
	const ASnowRumbleLobbyGameState* LobbyGameState = GetLobbyGameState();
	return LobbyGameState && LobbyGameState->CanStartLobbyMatch();
}

FString ULobbyWidget::GetCurrentRoomCode() const
{
	UGameInstance* GameInstance = GetGameInstance();
	const USnowRumbleSessionSubsystem* SessionSubsystem = GameInstance
		? GameInstance->GetSubsystem<USnowRumbleSessionSubsystem>()
		: nullptr;
	return SessionSubsystem ? SessionSubsystem->GetCurrentRoomCode() : FString();
}

ASnowRumblePlayerState* ULobbyWidget::GetLocalSnowRumblePlayerState() const
{
	const APlayerController* PlayerController = GetOwningPlayer();
	return PlayerController
		? PlayerController->GetPlayerState<ASnowRumblePlayerState>()
		: nullptr;
}

ASnowRumbleLobbyGameState* ULobbyWidget::GetLobbyGameState() const
{
	UWorld* World = GetWorld();
	return World ? World->GetGameState<ASnowRumbleLobbyGameState>() : nullptr;
}

void ULobbyWidget::ApplyLocalPlayerIdentity()
{
	ASnowRumblePlayerState* PlayerState = GetLocalSnowRumblePlayerState();
	if (!PlayerState)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	const ULocalPlayerIdentitySubsystem* IdentitySubsystem = GameInstance
		? GameInstance->GetSubsystem<ULocalPlayerIdentitySubsystem>()
		: nullptr;
	if (!IdentitySubsystem || !IdentitySubsystem->HasDesiredPlayerName())
	{
		return;
	}

	const FString DesiredPlayerName = IdentitySubsystem->GetDesiredPlayerName();
	if (PlayerState->GetLobbyPlayerName() == DesiredPlayerName)
	{
		IdentityAppliedPlayerState = PlayerState;
		return;
	}

	const UWorld* World = GetWorld();
	const double CurrentTime = World ? World->GetTimeSeconds() : 0.0;
	constexpr double RetryIntervalSeconds = 0.5;
	if (IdentityAppliedPlayerState == PlayerState
		&& CurrentTime - LastIdentityApplyRequestTime < RetryIntervalSeconds)
	{
		return;
	}

	if (ALobbyPlayerController* LobbyPlayerController =
		Cast<ALobbyPlayerController>(GetOwningPlayer()))
	{
		LobbyPlayerController->RequestApplyLobbyPlayerName(DesiredPlayerName);
	}
	else
	{
		PlayerState->RequestSetLobbyPlayerName(DesiredPlayerName);
	}
	IdentityAppliedPlayerState = PlayerState;
	LastIdentityApplyRequestTime = CurrentTime;
}

void ULobbyWidget::ApplyLocalPlayerCustomization()
{
	ASnowRumblePlayerState* PlayerState = GetLocalSnowRumblePlayerState();
	if (!PlayerState)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	const USnowRumbleCustomizationSubsystem* CustomizationSubsystem = GameInstance
		? GameInstance->GetSubsystem<USnowRumbleCustomizationSubsystem>()
		: nullptr;
	if (!CustomizationSubsystem)
	{
		return;
	}

	const FSnowRumbleCustomizationData DesiredCustomizationData =
		CustomizationSubsystem->GetCustomizationData();
	if (PlayerState->GetCustomizationData() == DesiredCustomizationData)
	{
		CustomizationAppliedPlayerState = PlayerState;
		return;
	}

	const UWorld* World = GetWorld();
	const double CurrentTime = World ? World->GetTimeSeconds() : 0.0;
	constexpr double RetryIntervalSeconds = 0.5;
	if (CustomizationAppliedPlayerState == PlayerState
		&& CurrentTime - LastCustomizationApplyRequestTime
			< RetryIntervalSeconds)
	{
		return;
	}

	if (ALobbyPlayerController* LobbyPlayerController =
		Cast<ALobbyPlayerController>(GetOwningPlayer()))
	{
		LobbyPlayerController->RequestApplyCustomizationData(
			DesiredCustomizationData);
	}
	else
	{
		PlayerState->RequestSetCustomizationData(DesiredCustomizationData);
	}
	CustomizationAppliedPlayerState = PlayerState;
	LastCustomizationApplyRequestTime = CurrentTime;
}

void ULobbyWidget::RefreshLobbyBindings()
{
	ASnowRumbleLobbyGameState* CurrentGameState = GetLobbyGameState();
	if (BoundLobbyGameState == CurrentGameState)
	{
		return;
	}

	UnbindLobbyBindings();
	BoundLobbyGameState = CurrentGameState;
	if (BoundLobbyGameState)
	{
		BoundLobbyGameState->OnLobbyStateChanged.AddUniqueDynamic(
			this,
			&ULobbyWidget::HandleLobbyStateChanged);
		OnLobbyStateChanged();
	}
}

void ULobbyWidget::RefreshRoomCodeText()
{
	if (RoomCodeTextBlock)
	{
		RoomCodeTextBlock->SetText(FText::FromString(GetCurrentRoomCode()));
	}
}

void ULobbyWidget::RefreshLobbyStatusTexts()
{
	const ASnowRumbleLobbyGameState* LobbyGameState = GetLobbyGameState();
	const ASnowRumblePlayerState* PlayerState = GetLocalSnowRumblePlayerState();

	if (ReadyPlayerCountText)
	{
		const int32 ReadyPlayerCount = LobbyGameState
			? LobbyGameState->GetReadyPlayerCount()
			: 0;
		const int32 ReadyRequiredPlayerCount = LobbyGameState
			? LobbyGameState->GetReadyRequiredPlayerCount()
			: 0;
		ReadyPlayerCountText->SetText(FText::FromString(FString::Printf(
			TEXT("%d / %d"),
			ReadyPlayerCount,
			ReadyRequiredPlayerCount)));
	}

	if (CurrentGameModeText)
	{
		CurrentGameModeText->SetText(LobbyGameState
			? GetLobbyModeText(LobbyGameState->GetLobbyMode())
			: FText::FromString(TEXT("-")));
	}

	if (MatchRoundLimitText)
	{
		MatchRoundLimitText->SetText(LobbyGameState
			? FText::Format(
				NSLOCTEXT("SnowRumble", "LobbyRoundLimitFormat", "{0} 라운드"),
				FText::AsNumber(LobbyGameState->GetMatchRoundLimit()))
			: FText::FromString(TEXT("-")));
	}

	if (GameSpeedText)
	{
		const ESnowRumbleGameSpeed GameSpeed = LobbyGameState
			? LobbyGameState->GetGameSpeed()
			: ESnowRumbleGameSpeed::Normal;
		GameSpeedText->SetText(LobbyGameState
			? GetGameSpeedDisplayText(GameSpeed)
			: FText::FromString(TEXT("-")));
	}

	if (LocalPlayerNameText)
	{
		LocalPlayerNameText->SetText(PlayerState
			? FText::FromString(PlayerState->GetLobbyPlayerName())
			: FText::FromString(TEXT("-")));
	}

	if (LocalTeamColorText)
	{
		LocalTeamColorText->SetText(PlayerState
			? GetTeamText(PlayerState->GetLobbyTeam())
			: NSLOCTEXT("SnowRumble", "LobbyTeamUnselectedFallback", "미선택"));
	}

	if (LocalReadyStateText)
	{
		const bool bReady = PlayerState && PlayerState->IsLobbyReady();
		LocalReadyStateText->SetText(bReady
			? NSLOCTEXT("SnowRumble", "LobbyReadyStateReady", "준비 완료")
			: NSLOCTEXT("SnowRumble", "LobbyReadyStateNotReady", "준비 전"));
	}

	if (LocalTeamColorBorder)
	{
		LocalTeamColorBorder->SetBrushColor(PlayerState
			? PlayerState->GetLobbyTeamColor()
			: FLinearColor::White);
	}
}

void ULobbyWidget::RefreshEventLogText()
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

void ULobbyWidget::RefreshVoiceSpeakingNamesText()
{
	if (!VoiceSpeakingNamesText && !VoiceSpeakingIcon
		&& !VoiceSpeakingContainer)
	{
		return;
	}

	FString SpeakingNames;
	for (const ASnowRumblePlayerState* PlayerState : GetLobbyPlayers())
	{
		if (!ShouldShowVoiceSpeakingPlayer(PlayerState))
		{
			continue;
		}

		if (!SpeakingNames.IsEmpty())
		{
			SpeakingNames += LINE_TERMINATOR;
		}
		SpeakingNames += PlayerState->GetLobbyPlayerName();
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

void ULobbyWidget::SetVoiceSpeakingPresentationVisible(bool bVisible)
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

bool ULobbyWidget::ShouldShowVoiceSpeakingPlayer(
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

	const ASnowRumblePlayerState* LocalPlayerState =
		GetLocalSnowRumblePlayerState();
	return LocalPlayerState
		&& SenderPlayerState->GetLobbyTeam() != ESnowRumbleTeam::None
		&& SenderPlayerState->GetLobbyTeam()
			== LocalPlayerState->GetLobbyTeam();
}

void ULobbyWidget::ShowInvalidActionFeedback(const FText& ReasonText)
{
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

void ULobbyWidget::ShowPersonalTextAlarm(const FText& Message)
{
	if (Message.IsEmpty())
	{
		return;
	}

	UTextBlock* TargetAlarmText = PersonalAlarmText
		? PersonalAlarmText.Get()
		: InvalidActionReasonText.Get();
	UWidgetAnimation* TargetAnimation = PersonalAlarmAnimation
		? PersonalAlarmAnimation.Get()
		: InvalidActionAnimation.Get();

	if (TargetAlarmText)
	{
		TargetAlarmText->SetText(Message);
		TargetAlarmText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	if (TargetAnimation)
	{
		PlayAnimation(TargetAnimation);
	}
}

void ULobbyWidget::UnbindLobbyBindings()
{
	if (BoundLobbyGameState)
	{
		BoundLobbyGameState->OnLobbyStateChanged.RemoveAll(this);
		BoundLobbyGameState = nullptr;
	}
}

void ULobbyWidget::HandleLobbyStateChanged()
{
	OnLobbyStateChanged();
}

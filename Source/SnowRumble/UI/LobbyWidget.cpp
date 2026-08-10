// Copyright Epic Games, Inc. All Rights Reserved.

#include "LobbyWidget.h"

#include "../Game/SnowRumbleLobbyGameState.h"
#include "../Online/SnowRumbleSessionSubsystem.h"
#include "../Player/LocalPlayerIdentitySubsystem_C.h"
#include "LobbyPlayerController.h"
#include "Components/Border.h"
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
		return FText::FromString(TEXT("PvP"));
	case ESnowRumbleLobbyMode::Snowman:
		return FText::FromString(TEXT("눈사람 모드"));
	default:
		return FText::FromString(TEXT("-"));
	}
}

FText GetTeamText(ESnowRumbleTeam Team)
{
	switch (Team)
	{
	case ESnowRumbleTeam::Red:
		return FText::FromString(TEXT("빨강"));
	case ESnowRumbleTeam::Sky:
		return FText::FromString(TEXT("하늘"));
	case ESnowRumbleTeam::Green:
		return FText::FromString(TEXT("초록"));
	case ESnowRumbleTeam::Yellow:
		return FText::FromString(TEXT("노랑"));
	case ESnowRumbleTeam::Purple:
		return FText::FromString(TEXT("보라"));
	case ESnowRumbleTeam::Pink:
		return FText::FromString(TEXT("핑크"));
	case ESnowRumbleTeam::Blue:
		return FText::FromString(TEXT("파랑"));
	case ESnowRumbleTeam::White:
		return FText::FromString(TEXT("하양"));
	default:
		return FText::FromString(TEXT("미선택"));
	}
}
}

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshLobbyBindings();
	ApplyLocalPlayerIdentity();
	RefreshRoomCodeText();
	RefreshLobbyStatusTexts();
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
	RefreshRoomCodeText();
	RefreshLobbyStatusTexts();
}

TArray<ASnowRumblePlayerState*> ULobbyWidget::GetLobbyPlayers() const
{
	if (ASnowRumbleLobbyGameState* LobbyGameState = GetLobbyGameState())
	{
		return LobbyGameState->GetLobbyPlayers();
	}
	return {};
}

void ULobbyWidget::RequestSetLocalPlayerName(const FString& NewName)
{
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
			: FText::FromString(TEXT("미선택")));
	}

	if (LocalReadyStateText)
	{
		const bool bReady = PlayerState && PlayerState->IsLobbyReady();
		LocalReadyStateText->SetText(FText::FromString(
			bReady ? TEXT("준비 완료") : TEXT("준비 전")));
	}

	if (LocalTeamColorBorder)
	{
		LocalTeamColorBorder->SetBrushColor(PlayerState
			? PlayerState->GetLobbyTeamColor()
			: FLinearColor::White);
	}
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

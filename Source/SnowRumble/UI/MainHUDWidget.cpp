// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainHUDWidget.h"

#include "../Game/SnowRumbleGameState_C.h"
#include "../Game/SnowRumblePlayerState.h"
#include "../Player/SnowRumbleCharacter.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "EngineUtils.h"
#include "HealthBarWidget.h"

void UMainHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshHealthBars();
	RefreshCombatHudPresentation();
	RefreshStartCountdownPresentation();
	RefreshEndRoundPresentation();
}

void UMainHUDWidget::NativeTick(
	const FGeometry& MyGeometry,
	float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshHealthBars();
	RefreshCombatHudPresentation();
	RefreshStartCountdownPresentation();
	RefreshEndRoundPresentation();
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

	if (LocalHealthBar)
	{
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

	const UEnum* TeamEnum =
		StaticEnum<ESnowRumbleTeam>();
	const FText TeamText = TeamEnum
		? TeamEnum->GetDisplayNameTextByValue(
			static_cast<int64>(SnowRumbleGameState->GetRoundWinningTeam()))
		: FText::GetEmpty();
	EndRoundResultText->SetText(FText::Format(
		NSLOCTEXT("SnowRumble", "RoundWinnerFormat", "{0} 승리"),
		TeamText));
	EndRoundResultText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
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

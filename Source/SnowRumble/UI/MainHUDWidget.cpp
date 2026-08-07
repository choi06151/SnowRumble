// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainHUDWidget.h"

#include "../Player/SnowRumbleCharacter.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "EngineUtils.h"
#include "HealthBarWidget.h"

void UMainHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshHealthBars();
	RefreshCombatHudPresentation();
}

void UMainHUDWidget::NativeTick(
	const FGeometry& MyGeometry,
	float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshHealthBars();
	RefreshCombatHudPresentation();
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
		if (!Character || Character == LocalCharacter)
		{
			continue;
		}

		ValidOtherPlayers.Add(Character);
		if (OtherPlayerHealthBars.Contains(Character))
		{
			continue;
		}

		if (!OtherPlayersHealthPanel || !OtherPlayerHealthBarWidgetClass)
		{
			continue;
		}

		UHealthBarWidget* NewHealthBar =
			CreateWidget<UHealthBarWidget>(
				GetOwningPlayer(),
				OtherPlayerHealthBarWidgetClass);
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

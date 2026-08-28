// Copyright Epic Games, Inc. All Rights Reserved.

#include "OverheadTimedActionWidget.h"

#include "../Player/SnowRumbleCharacter.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "GameFramework/PlayerController.h"

void UOverheadTimedActionWidget::SetObservedCharacter(
	ASnowRumbleCharacter* NewCharacter)
{
	ObservedCharacter = NewCharacter;
}

void UOverheadTimedActionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	SetRenderOpacity(0.0f);
	if (TimedActionProgressBar)
	{
		TimedActionProgressBar->SetPercent(0.0f);
		TimedActionProgressBar->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UOverheadTimedActionWidget::NativeTick(
	const FGeometry& MyGeometry,
	float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	RefreshActionProgressPresentation();
}

void UOverheadTimedActionWidget::RefreshActionProgressPresentation()
{
	if (!TimedActionProgressBar)
	{
		return;
	}

	APlayerController* OwningPlayer = GetOwningPlayer();
	ASnowRumbleCharacter* Character =
		ObservedCharacter
			? ObservedCharacter.Get()
			: Cast<ASnowRumbleCharacter>(GetOwningPlayerPawn());
	const bool bIsRemoteFrozenTimer = ObservedCharacter != nullptr;
	const bool bShouldShow =
		OwningPlayer
		&& Character
		&& (bIsRemoteFrozenTimer
			? Character->IsFrozen()
			: Character->IsLocallyControlled()
				&& Character->GetTimedActionState()
					!= ESnowRumbleTimedActionState::None);

	TimedActionProgressBar->SetVisibility(
		bShouldShow
			? ESlateVisibility::SelfHitTestInvisible
			: ESlateVisibility::Collapsed);
	SetRenderOpacity(bShouldShow ? 1.0f : 0.0f);
	if (!bShouldShow)
	{
		TimedActionProgressBar->SetPercent(0.0f);
		return;
	}

	TimedActionProgressBar->SetPercent(
		bIsRemoteFrozenTimer
			? Character->GetFrozenProgress()
			: Character->GetTimedActionProgress());

	FVector2D ScreenPosition = FVector2D::ZeroVector;
	const bool bProjected =
		UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
			OwningPlayer,
			Character->GetActorLocation() + OverheadWorldOffset,
			ScreenPosition,
			true);
	if (!bProjected)
	{
		TimedActionProgressBar->SetVisibility(ESlateVisibility::Collapsed);
		SetRenderOpacity(0.0f);
		return;
	}

	if (UCanvasPanelSlot* CanvasSlot =
		Cast<UCanvasPanelSlot>(Slot))
	{
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetPosition(ScreenPosition + ScreenPositionOffset);
	}
}

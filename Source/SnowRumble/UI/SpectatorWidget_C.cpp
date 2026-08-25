// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpectatorWidget_C.h"

#include "../Player/SnowRumbleCharacter.h"
#include "Components/TextBlock.h"

void USpectatorWidget::SetSpectatorViewTarget(
	ASnowRumbleCharacter* NewViewTarget)
{
	CurrentViewTarget = NewViewTarget;
	RefreshCurrentViewTargetIdText();
}

int32 USpectatorWidget::GetCurrentViewTargetId() const
{
	const APlayerState* PlayerState = CurrentViewTarget
		? CurrentViewTarget->GetPlayerState()
		: nullptr;
	return PlayerState ? PlayerState->GetPlayerId() : INDEX_NONE;
}

void USpectatorWidget::RefreshCurrentViewTargetIdText()
{
	if (!CurrentViewTargetIdText)
	{
		return;
	}

	const int32 ViewTargetId = GetCurrentViewTargetId();
	CurrentViewTargetIdText->SetText(
		ViewTargetId == INDEX_NONE
			? FText::GetEmpty()
			: FText::Format(
				NSLOCTEXT("SnowRumble", "SpectatorViewTargetId", "ID: {0}"),
				ViewTargetId));
}

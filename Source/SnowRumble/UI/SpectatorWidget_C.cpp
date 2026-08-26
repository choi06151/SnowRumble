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

	const FString ViewTargetName = CurrentViewTarget
		? CurrentViewTarget->GetOverheadPlayerName()
		: FString();
	CurrentViewTargetIdText->SetText(
		ViewTargetName.IsEmpty()
			? FText::GetEmpty()
			: FText::FromString(ViewTargetName));
}

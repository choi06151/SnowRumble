// Copyright Epic Games, Inc. All Rights Reserved.

#include "VoiceMutePlayerRowWidget_C.h"

#include "../Game/SnowRumblePlayerState.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "SnowRumblePlayerController.h"

void UVoiceMutePlayerRowWidget::SetVoiceMutePlayer(
	ASnowRumblePlayerState* NewPlayerState,
	ASnowRumblePlayerController* NewPlayerController)
{
	ObservedPlayerState = NewPlayerState;
	VoicePlayerController = NewPlayerController;
	RefreshRow();
}

ASnowRumblePlayerState*
UVoiceMutePlayerRowWidget::GetObservedPlayerState() const
{
	return ObservedPlayerState;
}

void UVoiceMutePlayerRowWidget::RefreshRow()
{
	const bool bIsMuted =
		VoicePlayerController
		&& ObservedPlayerState
		&& VoicePlayerController->IsVoicePlayerManuallyMuted(
			ObservedPlayerState);

	if (PlayerNameText)
	{
		PlayerNameText->SetText(
			ObservedPlayerState
				? FText::FromString(ObservedPlayerState->GetLobbyPlayerName())
				: FText::GetEmpty());
	}
	if (MuteButtonText)
	{
		MuteButtonText->SetText(
			bIsMuted
				? NSLOCTEXT("SnowRumble", "VoiceMuteUnmuteButton", "해제")
				: NSLOCTEXT("SnowRumble", "VoiceMuteMuteButton", "음소거"));
	}
	if (MuteButton)
	{
		MuteButton->SetIsEnabled(
			VoicePlayerController != nullptr
			&& ObservedPlayerState != nullptr);
	}

	OnVoiceMutePlayerRowChanged(ObservedPlayerState, bIsMuted);
}

void UVoiceMutePlayerRowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindRowButton();
	RefreshRow();
}

void UVoiceMutePlayerRowWidget::NativeDestruct()
{
	UnbindRowButton();

	Super::NativeDestruct();
}

void UVoiceMutePlayerRowWidget::HandleMuteButtonClicked()
{
	if (VoicePlayerController && ObservedPlayerState)
	{
		VoicePlayerController->ToggleManualVoiceMute(ObservedPlayerState);
		RefreshRow();
	}
}

void UVoiceMutePlayerRowWidget::BindRowButton()
{
	if (MuteButton)
	{
		MuteButton->OnClicked.AddUniqueDynamic(
			this,
			&UVoiceMutePlayerRowWidget::HandleMuteButtonClicked);
	}
}

void UVoiceMutePlayerRowWidget::UnbindRowButton()
{
	if (MuteButton)
	{
		MuteButton->OnClicked.RemoveAll(this);
	}
}

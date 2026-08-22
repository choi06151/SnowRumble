// Copyright Epic Games, Inc. All Rights Reserved.

#include "VoiceMuteMenuWidget_C.h"

#include "../Audio/SnowRumbleAudioHelpers.h"
#include "../Game/SnowRumblePlayerState.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "SnowRumblePlayerController.h"
#include "VoiceMutePlayerRowWidget_C.h"

void UVoiceMuteMenuWidget::SetVoicePlayerController(
	ASnowRumblePlayerController* NewPlayerController)
{
	VoicePlayerController = NewPlayerController;
	RefreshPlayerList();
}

void UVoiceMuteMenuWidget::RefreshPlayerList()
{
	if (!PlayerListBox)
	{
		return;
	}

	PlayerListBox->ClearChildren();

	const UWorld* World = GetWorld();
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	if (!GameState || !VoicePlayerController)
	{
		OnVoiceMutePlayerListRefreshed(0);
		return;
	}

	int32 AddedPlayerCount = 0;
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		ASnowRumblePlayerState* SnowRumblePlayerState =
			Cast<ASnowRumblePlayerState>(PlayerState);
		if (!SnowRumblePlayerState
			|| SnowRumblePlayerState
				== VoicePlayerController->GetPlayerState<ASnowRumblePlayerState>())
		{
			continue;
		}

		if (!PlayerRowWidgetClass)
		{
			continue;
		}

		UVoiceMutePlayerRowWidget* RowWidget =
			CreateWidget<UVoiceMutePlayerRowWidget>(
				GetOwningPlayer(),
				PlayerRowWidgetClass);
		if (!RowWidget)
		{
			continue;
		}

		RowWidget->SetVoiceMutePlayer(
			SnowRumblePlayerState,
			VoicePlayerController);
		PlayerListBox->AddChild(RowWidget);
		++AddedPlayerCount;
	}

	CachedPlayerListSignature = GetPlayerStateListSignature();
	OnVoiceMutePlayerListRefreshed(AddedPlayerCount);
}

void UVoiceMuteMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindMenuButtons();
	RefreshPlayerList();
}

void UVoiceMuteMenuWidget::NativeDestruct()
{
	UnbindMenuButtons();

	Super::NativeDestruct();
}

void UVoiceMuteMenuWidget::NativeTick(
	const FGeometry& MyGeometry,
	float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const FString CurrentSignature = GetPlayerStateListSignature();
	if (CurrentSignature != CachedPlayerListSignature)
	{
		RefreshPlayerList();
	}
}

void UVoiceMuteMenuWidget::HandleCloseButtonClicked()
{
	PlayMenuClickSound();
	if (VoicePlayerController)
	{
		VoicePlayerController->HideVoiceMuteMenu();
	}
}

void UVoiceMuteMenuWidget::PlayMenuClickSound() const
{
	SnowRumbleAudio::PlaySound2D(
		this,
		MenuClickSound,
		ESnowRumbleAudioMixChannel::UserInterface);
}

void UVoiceMuteMenuWidget::BindMenuButtons()
{
	if (CloseButton)
	{
		CloseButton->OnClicked.AddUniqueDynamic(
			this,
			&UVoiceMuteMenuWidget::HandleCloseButtonClicked);
	}
}

void UVoiceMuteMenuWidget::UnbindMenuButtons()
{
	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveAll(this);
	}
}

FString UVoiceMuteMenuWidget::GetPlayerStateListSignature() const
{
	const UWorld* World = GetWorld();
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	if (!GameState)
	{
		return FString();
	}

	FString Signature;
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		const ASnowRumblePlayerState* SnowRumblePlayerState =
			Cast<ASnowRumblePlayerState>(PlayerState);
		if (!SnowRumblePlayerState)
		{
			continue;
		}

		Signature += FString::Printf(
			TEXT("%d:%s|"),
			SnowRumblePlayerState->GetPlayerId(),
			*SnowRumblePlayerState->GetLobbyPlayerName());
	}
	return Signature;
}

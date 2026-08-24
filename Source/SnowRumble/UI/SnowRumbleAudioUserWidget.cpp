// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleAudioUserWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "SnowRumble/Audio/SnowRumbleAudioHelpers.h"
#include "Sound/SoundBase.h"

void USnowRumbleAudioUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!WidgetTree)
	{
		return;
	}

	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		if (UButton* Button = Cast<UButton>(Widget))
		{
			Button->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleButtonClicked);
			Button->OnHovered.AddUniqueDynamic(this, &ThisClass::HandleButtonHovered);
		}
	});
}

void USnowRumbleAudioUserWidget::HandleButtonClicked()
{
	SnowRumbleAudio::PlaySound2D(
		this,
		ButtonClickSound,
		ESnowRumbleAudioMixChannel::UserInterface);
}

void USnowRumbleAudioUserWidget::HandleButtonHovered()
{
	SnowRumbleAudio::PlaySound2D(
		this,
		ButtonHoverSound,
		ESnowRumbleAudioMixChannel::UserInterface);
}

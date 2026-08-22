// Copyright Epic Games, Inc. All Rights Reserved.

#include "EmoteRadialMenuWidget.h"

#include "../Audio/SnowRumbleAudioHelpers.h"
#include "../Player/SnowRumbleCharacter.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"

void UEmoteRadialMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Collapsed);
	ResolveEmoteButtons();
	BindEmoteButtons();
}

void UEmoteRadialMenuWidget::NativeDestruct()
{
	UnbindEmoteButtons();

	Super::NativeDestruct();
}

void UEmoteRadialMenuWidget::OpenEmoteMenu()
{
	HoveredEmoteIndex = INDEX_NONE;
	SetVisibility(ESlateVisibility::Visible);
}

void UEmoteRadialMenuWidget::CloseEmoteMenu()
{
	SetVisibility(ESlateVisibility::Collapsed);
	HoveredEmoteIndex = INDEX_NONE;
}

bool UEmoteRadialMenuWidget::SubmitHoveredEmote()
{
	if (HoveredEmoteIndex == INDEX_NONE)
	{
		return false;
	}

	SelectEmote(HoveredEmoteIndex);
	return true;
}

void UEmoteRadialMenuWidget::HandleEmoteButton0Clicked()
{
	SelectEmote(0);
}

void UEmoteRadialMenuWidget::HandleEmoteButton1Clicked()
{
	SelectEmote(1);
}

void UEmoteRadialMenuWidget::HandleEmoteButton2Clicked()
{
	SelectEmote(2);
}

void UEmoteRadialMenuWidget::HandleEmoteButton3Clicked()
{
	SelectEmote(3);
}

void UEmoteRadialMenuWidget::HandleEmoteButton4Clicked()
{
	SelectEmote(4);
}

void UEmoteRadialMenuWidget::HandleEmoteButton5Clicked()
{
	SelectEmote(5);
}

void UEmoteRadialMenuWidget::HandleEmoteButton6Clicked()
{
	SelectEmote(6);
}

void UEmoteRadialMenuWidget::HandleEmoteButton7Clicked()
{
	SelectEmote(7);
}

void UEmoteRadialMenuWidget::HandleEmoteButton0Hovered()
{
	SetHoveredEmoteIndex(0);
}

void UEmoteRadialMenuWidget::HandleEmoteButton1Hovered()
{
	SetHoveredEmoteIndex(1);
}

void UEmoteRadialMenuWidget::HandleEmoteButton2Hovered()
{
	SetHoveredEmoteIndex(2);
}

void UEmoteRadialMenuWidget::HandleEmoteButton3Hovered()
{
	SetHoveredEmoteIndex(3);
}

void UEmoteRadialMenuWidget::HandleEmoteButton4Hovered()
{
	SetHoveredEmoteIndex(4);
}

void UEmoteRadialMenuWidget::HandleEmoteButton5Hovered()
{
	SetHoveredEmoteIndex(5);
}

void UEmoteRadialMenuWidget::HandleEmoteButton6Hovered()
{
	SetHoveredEmoteIndex(6);
}

void UEmoteRadialMenuWidget::HandleEmoteButton7Hovered()
{
	SetHoveredEmoteIndex(7);
}

void UEmoteRadialMenuWidget::HandleEmoteButton0Unhovered()
{
	ClearHoveredEmoteIndex(0);
}

void UEmoteRadialMenuWidget::HandleEmoteButton1Unhovered()
{
	ClearHoveredEmoteIndex(1);
}

void UEmoteRadialMenuWidget::HandleEmoteButton2Unhovered()
{
	ClearHoveredEmoteIndex(2);
}

void UEmoteRadialMenuWidget::HandleEmoteButton3Unhovered()
{
	ClearHoveredEmoteIndex(3);
}

void UEmoteRadialMenuWidget::HandleEmoteButton4Unhovered()
{
	ClearHoveredEmoteIndex(4);
}

void UEmoteRadialMenuWidget::HandleEmoteButton5Unhovered()
{
	ClearHoveredEmoteIndex(5);
}

void UEmoteRadialMenuWidget::HandleEmoteButton6Unhovered()
{
	ClearHoveredEmoteIndex(6);
}

void UEmoteRadialMenuWidget::HandleEmoteButton7Unhovered()
{
	ClearHoveredEmoteIndex(7);
}

void UEmoteRadialMenuWidget::SelectEmote(int32 EmoteIndex)
{
	PlayEmoteClickSound();
	ASnowRumbleCharacter* Character =
		Cast<ASnowRumbleCharacter>(GetOwningPlayerPawn());
	if (!Character)
	{
		return;
	}

	Character->RequestPlayEmote(EmoteIndex);

	if (bCloseAfterSelection)
	{
		Character->CloseEmoteRadialMenu();
	}
}

void UEmoteRadialMenuWidget::PlayEmoteClickSound() const
{
	SnowRumbleAudio::PlaySound2D(
		this,
		EmoteClickSound,
		ESnowRumbleAudioMixChannel::UserInterface);
}

void UEmoteRadialMenuWidget::SetHoveredEmoteIndex(int32 EmoteIndex)
{
	HoveredEmoteIndex = EmoteIndex;
}

void UEmoteRadialMenuWidget::ClearHoveredEmoteIndex(int32 EmoteIndex)
{
	if (HoveredEmoteIndex == EmoteIndex)
	{
		HoveredEmoteIndex = INDEX_NONE;
	}
}

void UEmoteRadialMenuWidget::ResolveEmoteButtons()
{
	if (!WidgetTree)
	{
		return;
	}

	if (!EmoteButton0)
	{
		EmoteButton0 = WidgetTree->FindWidget<UButton>(TEXT("EmoteButton0"));
	}
	if (!EmoteButton1)
	{
		EmoteButton1 = WidgetTree->FindWidget<UButton>(TEXT("EmoteButton1"));
	}
	if (!EmoteButton2)
	{
		EmoteButton2 = WidgetTree->FindWidget<UButton>(TEXT("EmoteButton2"));
	}
	if (!EmoteButton3)
	{
		EmoteButton3 = WidgetTree->FindWidget<UButton>(TEXT("EmoteButton3"));
	}
	if (!EmoteButton4)
	{
		EmoteButton4 = WidgetTree->FindWidget<UButton>(TEXT("EmoteButton4"));
	}
	if (!EmoteButton5)
	{
		EmoteButton5 = WidgetTree->FindWidget<UButton>(TEXT("EmoteButton5"));
	}
	if (!EmoteButton6)
	{
		EmoteButton6 = WidgetTree->FindWidget<UButton>(TEXT("EmoteButton6"));
	}
	if (!EmoteButton7)
	{
		EmoteButton7 = WidgetTree->FindWidget<UButton>(TEXT("EmoteButton7"));
	}

}

void UEmoteRadialMenuWidget::BindEmoteButtons()
{
	if (EmoteButton0)
	{
		EmoteButton0->OnClicked.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton0Clicked);
		EmoteButton0->OnHovered.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton0Hovered);
		EmoteButton0->OnUnhovered.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton0Unhovered);
	}
	if (EmoteButton1)
	{
		EmoteButton1->OnClicked.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton1Clicked);
		EmoteButton1->OnHovered.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton1Hovered);
		EmoteButton1->OnUnhovered.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton1Unhovered);
	}
	if (EmoteButton2)
	{
		EmoteButton2->OnClicked.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton2Clicked);
		EmoteButton2->OnHovered.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton2Hovered);
		EmoteButton2->OnUnhovered.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton2Unhovered);
	}
	if (EmoteButton3)
	{
		EmoteButton3->OnClicked.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton3Clicked);
		EmoteButton3->OnHovered.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton3Hovered);
		EmoteButton3->OnUnhovered.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton3Unhovered);
	}
	if (EmoteButton4)
	{
		EmoteButton4->OnClicked.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton4Clicked);
		EmoteButton4->OnHovered.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton4Hovered);
		EmoteButton4->OnUnhovered.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton4Unhovered);
	}
	if (EmoteButton5)
	{
		EmoteButton5->OnClicked.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton5Clicked);
		EmoteButton5->OnHovered.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton5Hovered);
		EmoteButton5->OnUnhovered.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton5Unhovered);
	}
	if (EmoteButton6)
	{
		EmoteButton6->OnClicked.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton6Clicked);
		EmoteButton6->OnHovered.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton6Hovered);
		EmoteButton6->OnUnhovered.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton6Unhovered);
	}
	if (EmoteButton7)
	{
		EmoteButton7->OnClicked.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton7Clicked);
		EmoteButton7->OnHovered.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton7Hovered);
		EmoteButton7->OnUnhovered.AddUniqueDynamic(
			this,
			&UEmoteRadialMenuWidget::HandleEmoteButton7Unhovered);
	}
}

void UEmoteRadialMenuWidget::UnbindEmoteButtons()
{
	if (EmoteButton0)
	{
		EmoteButton0->OnClicked.RemoveAll(this);
	}
	if (EmoteButton1)
	{
		EmoteButton1->OnClicked.RemoveAll(this);
	}
	if (EmoteButton2)
	{
		EmoteButton2->OnClicked.RemoveAll(this);
	}
	if (EmoteButton3)
	{
		EmoteButton3->OnClicked.RemoveAll(this);
	}
	if (EmoteButton4)
	{
		EmoteButton4->OnClicked.RemoveAll(this);
	}
	if (EmoteButton5)
	{
		EmoteButton5->OnClicked.RemoveAll(this);
	}
	if (EmoteButton6)
	{
		EmoteButton6->OnClicked.RemoveAll(this);
	}
	if (EmoteButton7)
	{
		EmoteButton7->OnClicked.RemoveAll(this);
	}
}

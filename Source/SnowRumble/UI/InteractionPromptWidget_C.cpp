// Copyright Epic Games, Inc. All Rights Reserved.

#include "InteractionPromptWidget_C.h"

#include "Components/TextBlock.h"

void UInteractionPromptWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ClearPrompt();
}

void UInteractionPromptWidget::SetPromptText(const FText& NewPromptText)
{
	if (PromptText)
	{
		PromptText->SetText(NewPromptText);
	}
	SetVisibility(
		NewPromptText.IsEmpty()
			? ESlateVisibility::Collapsed
			: ESlateVisibility::SelfHitTestInvisible);
}

void UInteractionPromptWidget::ClearPrompt()
{
	if (PromptText)
	{
		PromptText->SetText(FText::GetEmpty());
	}
	SetVisibility(ESlateVisibility::Collapsed);
}

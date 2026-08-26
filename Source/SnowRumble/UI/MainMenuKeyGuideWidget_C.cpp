// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainMenuKeyGuideWidget_C.h"

#include "Components/Button.h"
#include "MainMenuPlayerController.h"

void UMainMenuKeyGuideWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddUniqueDynamic(
			this,
			&UMainMenuKeyGuideWidget::HandleCloseButtonClicked);
	}
}

void UMainMenuKeyGuideWidget::HandleCloseButtonClicked()
{
	if (AMainMenuPlayerController* MainMenuPlayerController =
		Cast<AMainMenuPlayerController>(GetOwningPlayer()))
	{
		MainMenuPlayerController->CloseKeyGuideWidget();
	}
}

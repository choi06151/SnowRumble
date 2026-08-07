// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainMenuPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.h"

void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		ShowMainMenu();
	}
}

void AMainMenuPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	HideMainMenu();

	Super::EndPlay(EndPlayReason);
}

void AMainMenuPlayerController::ShowMainMenu()
{
	if (!IsLocalController())
	{
		return;
	}

	UMainMenuWidget* Widget = EnsureMainMenuWidget();
	if (!Widget)
	{
		return;
	}

	if (!Widget->IsInViewport())
	{
		Widget->AddToViewport();
	}

	bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(Widget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AMainMenuPlayerController::HideMainMenu()
{
	if (MainMenuWidget)
	{
		MainMenuWidget->RemoveFromParent();
	}

	if (IsLocalController())
	{
		bShowMouseCursor = false;
		SetInputMode(FInputModeGameOnly());
	}
}

UMainMenuWidget* AMainMenuPlayerController::EnsureMainMenuWidget()
{
	if (MainMenuWidget)
	{
		return MainMenuWidget;
	}

	if (!MainMenuWidgetClass)
	{
		return nullptr;
	}

	MainMenuWidget = CreateWidget<UMainMenuWidget>(this, MainMenuWidgetClass);
	return MainMenuWidget;
}

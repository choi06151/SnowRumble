// Copyright Epic Games, Inc. All Rights Reserved.

#include "LobbyPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "LobbyWidget.h"

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		ShowLobby();
	}
}

void ALobbyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	HideLobby();

	Super::EndPlay(EndPlayReason);
}

void ALobbyPlayerController::ShowLobby()
{
	if (!IsLocalController())
	{
		return;
	}

	ULobbyWidget* Widget = EnsureLobbyWidget();
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

void ALobbyPlayerController::HideLobby()
{
	if (LobbyWidget)
	{
		LobbyWidget->RemoveFromParent();
	}

	if (IsLocalController())
	{
		bShowMouseCursor = false;
		SetInputMode(FInputModeGameOnly());
	}
}

ULobbyWidget* ALobbyPlayerController::EnsureLobbyWidget()
{
	if (LobbyWidget)
	{
		return LobbyWidget;
	}

	if (!LobbyWidgetClass)
	{
		return nullptr;
	}

	LobbyWidget = CreateWidget<ULobbyWidget>(this, LobbyWidgetClass);
	return LobbyWidget;
}

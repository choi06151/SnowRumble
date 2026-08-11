// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainMenuPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.h"
#include "OptionsWidget_C.h"
#include "../Online/SnowRumbleSessionSubsystem.h"

void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (USnowRumbleSessionSubsystem* SessionSubsystem =
				GameInstance->GetSubsystem<USnowRumbleSessionSubsystem>())
			{
				SessionSubsystem->LeaveLanSession();
			}
		}

		ShowMainMenu();
	}
}

void AMainMenuPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OptionsWidget)
	{
		OptionsWidget->RemoveFromParent();
		OptionsWidget = nullptr;
	}
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

void AMainMenuPlayerController::ShowOptionsMenu()
{
	if (!IsLocalController())
	{
		return;
	}

	UOptionsWidget* Widget = EnsureOptionsWidget();
	if (!Widget)
	{
		return;
	}

	if (!Widget->IsInViewport())
	{
		Widget->AddToViewport(200);
	}
	Widget->SetKeyboardFocus();

	bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(Widget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AMainMenuPlayerController::HideOptionsMenu()
{
	if (OptionsWidget)
	{
		OptionsWidget->DiscardPendingOptionChanges();
		OptionsWidget->RemoveFromParent();
	}

	if (IsLocalController())
	{
		ShowMainMenu();
	}
}

void AMainMenuPlayerController::TravelToCustomizationLevel()
{
	if (!IsLocalController() || CustomizationLevelUrl.IsEmpty())
	{
		return;
	}

	ClientTravel(CustomizationLevelUrl, TRAVEL_Absolute);
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

UOptionsWidget* AMainMenuPlayerController::EnsureOptionsWidget()
{
	if (OptionsWidget)
	{
		return OptionsWidget;
	}

	if (!OptionsWidgetClass)
	{
		return nullptr;
	}

	OptionsWidget = CreateWidget<UOptionsWidget>(this, OptionsWidgetClass);
	if (OptionsWidget)
	{
		OptionsWidget->OnOptionsCloseRequestedNative.AddUObject(
			this,
			&AMainMenuPlayerController::HideOptionsMenu);
	}
	return OptionsWidget;
}

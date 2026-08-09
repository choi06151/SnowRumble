// Copyright Epic Games, Inc. All Rights Reserved.

#include "LobbyPlayerController.h"

#include "../Game/SnowRumblePlayerState.h"
#include "../Player/LocalPlayerIdentitySubsystem_C.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "LobbyWidget.h"

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		ShowLobby();
		ApplySavedLobbyPlayerName();
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

	EnableLobbyGameInput();
}

void ALobbyPlayerController::EnableLobbyUiInput()
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

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(Widget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
}

void ALobbyPlayerController::EnableLobbyGameInput()
{
	if (!IsLocalController())
	{
		return;
	}

	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());
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

void ALobbyPlayerController::RequestApplyLobbyPlayerName(
	const FString& NewName)
{
	if (HasAuthority())
	{
		ServerApplyLobbyPlayerName_Implementation(NewName);
	}
	else
	{
		ServerApplyLobbyPlayerName(NewName);
	}
}

void ALobbyPlayerController::RequestApplyLobbyTeam(ESnowRumbleTeam NewTeam)
{
	if (HasAuthority())
	{
		ServerApplyLobbyTeam_Implementation(NewTeam);
	}
	else
	{
		ServerApplyLobbyTeam(NewTeam);
	}
}

void ALobbyPlayerController::ClientRequestApplySavedLobbyPlayerName_Implementation()
{
	ApplySavedLobbyPlayerName();
}

void ALobbyPlayerController::ClientShowLoadingScreen_Implementation()
{
	HideLobby();

	Super::ClientShowLoadingScreen_Implementation();
}

void ALobbyPlayerController::ApplySavedLobbyPlayerName()
{
	if (!IsLocalController())
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	const ULocalPlayerIdentitySubsystem* IdentitySubsystem = GameInstance
		? GameInstance->GetSubsystem<ULocalPlayerIdentitySubsystem>()
		: nullptr;
	if (IdentitySubsystem && IdentitySubsystem->HasDesiredPlayerName())
	{
		RequestApplyLobbyPlayerName(IdentitySubsystem->GetDesiredPlayerName());
	}
}

void ALobbyPlayerController::ServerApplyLobbyPlayerName_Implementation(
	const FString& NewName)
{
	if (ASnowRumblePlayerState* SnowRumblePlayerState =
		GetPlayerState<ASnowRumblePlayerState>())
	{
		SnowRumblePlayerState->RequestSetLobbyPlayerName(NewName);
	}
}

void ALobbyPlayerController::ServerApplyLobbyTeam_Implementation(
	ESnowRumbleTeam NewTeam)
{
	if (ASnowRumblePlayerState* SnowRumblePlayerState =
		GetPlayerState<ASnowRumblePlayerState>())
	{
		SnowRumblePlayerState->RequestSetLobbyTeam(NewTeam);
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

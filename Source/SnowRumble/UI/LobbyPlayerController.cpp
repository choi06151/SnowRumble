// Copyright Epic Games, Inc. All Rights Reserved.

#include "LobbyPlayerController.h"

#include "../Game/SnowRumblePlayerState.h"
#include "../Player/LocalPlayerIdentitySubsystem_C.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "InputCoreTypes.h"
#include "LobbyEscapeMenuWidget.h"
#include "LobbyWidget.h"
#include "OptionsWidget_C.h"
#include "../Online/SnowRumbleSessionSubsystem.h"

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
	if (OptionsWidget)
	{
		OptionsWidget->RemoveFromParent();
		OptionsWidget = nullptr;
	}
	HideLobbyEscapeMenu();
	HideLobby();

	Super::EndPlay(EndPlayReason);
}

void ALobbyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		InputComponent->BindKey(
			EKeys::Escape,
			IE_Pressed,
			this,
			&ALobbyPlayerController::HandleEscapePressed);
		InputComponent->BindKey(
			EKeys::F10,
			IE_Pressed,
			this,
			&ALobbyPlayerController::HandleEscapePressed);
	}
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
	ApplyDefaultMouseCursorWidget();

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

void ALobbyPlayerController::ShowLobbyEscapeMenu()
{
	if (!IsLocalController())
	{
		return;
	}

	ULobbyEscapeMenuWidget* Widget = EnsureLobbyEscapeMenuWidget();
	if (!Widget)
	{
		return;
	}

	if (!Widget->IsInViewport())
	{
		Widget->AddToViewport(100);
	}
	Widget->SetKeyboardFocus();

	ResetIgnoreMoveInput();
	ResetIgnoreLookInput();
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
	SetShowMouseCursor(true);
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CurrentMouseCursor = EMouseCursor::Default;
	SetMouseCursorWidget(EMouseCursor::Default, nullptr);

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(Widget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void ALobbyPlayerController::HideLobbyEscapeMenu()
{
	if (OptionsWidget && OptionsWidget->IsInViewport())
	{
		OptionsWidget->RemoveFromParent();
	}

	if (LobbyEscapeMenuWidget)
	{
		LobbyEscapeMenuWidget->RemoveFromParent();
	}

	if (IsLocalController())
	{
		ResetIgnoreMoveInput();
		ResetIgnoreLookInput();
		EnableLobbyGameInput();
	}
}

void ALobbyPlayerController::ShowOptionsMenu()
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

	ResetIgnoreMoveInput();
	ResetIgnoreLookInput();
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
	SetShowMouseCursor(true);
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CurrentMouseCursor = EMouseCursor::Default;
	SetMouseCursorWidget(EMouseCursor::Default, nullptr);

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(Widget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void ALobbyPlayerController::HideOptionsMenu()
{
	if (OptionsWidget)
	{
		OptionsWidget->DiscardPendingOptionChanges();
		OptionsWidget->RemoveFromParent();
	}

	if (IsLocalController())
	{
		ShowLobbyEscapeMenu();
	}
}

void ALobbyPlayerController::RequestReturnToMainMenu()
{
	if (!IsLocalController())
	{
		return;
	}

	HideLobbyEscapeMenu();

	if (MainMenuTravelUrl.IsEmpty())
	{
		return;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USnowRumbleSessionSubsystem* SessionSubsystem =
			GameInstance->GetSubsystem<USnowRumbleSessionSubsystem>())
		{
			SessionSubsystem->LeaveLanSession();
		}
	}

	if (HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			World->ServerTravel(MainMenuTravelUrl);
		}
		return;
	}

	FString ClientTravelUrl = MainMenuTravelUrl;
	FString MapPath;
	FString TravelOptions;
	if (ClientTravelUrl.Split(TEXT("?"), &MapPath, &TravelOptions))
	{
		ClientTravelUrl = MapPath;
	}
	ClientTravel(ClientTravelUrl, TRAVEL_Absolute);
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
	if (const ASnowRumblePlayerState* SnowRumblePlayerState =
		GetPlayerState<ASnowRumblePlayerState>())
	{
		if (SnowRumblePlayerState->IsLobbyReady()
			&& SnowRumblePlayerState->GetLobbyTeam() != NewTeam)
		{
			ShowLobbyInvalidActionFeedback(
				NSLOCTEXT(
					"SnowRumble",
					"LobbyControllerInvalidTeamChangeWhileReady",
					"준비 완료 상태에서는 팀 색을 변경할 수 없습니다."));
			return;
		}
	}

	if (HasAuthority())
	{
		ServerApplyLobbyTeam_Implementation(NewTeam);
	}
	else
	{
		ServerApplyLobbyTeam(NewTeam);
	}
}

void ALobbyPlayerController::RequestApplyCustomizationData(
	const FSnowRumbleCustomizationData& NewData)
{
	if (HasAuthority())
	{
		ServerApplyCustomizationData_Implementation(NewData);
	}
	else
	{
		ServerApplyCustomizationData(NewData);
	}
}

void ALobbyPlayerController::ShowLobbyInvalidActionFeedback(
	const FText& ReasonText)
{
	if (!IsLocalController())
	{
		return;
	}

	if (ULobbyWidget* Widget = EnsureLobbyWidget())
	{
		if (!Widget->IsInViewport())
		{
			Widget->AddToViewport();
		}
		Widget->ShowInvalidActionFeedback(ReasonText);
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

bool ALobbyPlayerController::CanOpenChatInput() const
{
	return Super::CanOpenChatInput()
		&& (!LobbyEscapeMenuWidget || !LobbyEscapeMenuWidget->IsInViewport())
		&& (!OptionsWidget || !OptionsWidget->IsInViewport());
}

bool ALobbyPlayerController::IsGameplayUiInputOpen() const
{
	return Super::IsGameplayUiInputOpen()
		|| (LobbyEscapeMenuWidget && LobbyEscapeMenuWidget->IsInViewport())
		|| (OptionsWidget && OptionsWidget->IsInViewport());
}

bool ALobbyPlayerController::SupportsTeamChat() const
{
	return false;
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

void ALobbyPlayerController::HandleEscapePressed()
{
	if (!IsLocalController())
	{
		return;
	}

	if (OptionsWidget && OptionsWidget->IsInViewport())
	{
		HideOptionsMenu();
	}
	else if (LobbyEscapeMenuWidget && LobbyEscapeMenuWidget->IsInViewport())
	{
		HideLobbyEscapeMenu();
	}
	else
	{
		ShowLobbyEscapeMenu();
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

void ALobbyPlayerController::ServerApplyCustomizationData_Implementation(
	const FSnowRumbleCustomizationData& NewData)
{
	if (ASnowRumblePlayerState* SnowRumblePlayerState =
		GetPlayerState<ASnowRumblePlayerState>())
	{
		SnowRumblePlayerState->RequestSetCustomizationData(NewData);
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

ULobbyEscapeMenuWidget* ALobbyPlayerController::EnsureLobbyEscapeMenuWidget()
{
	if (LobbyEscapeMenuWidget)
	{
		return LobbyEscapeMenuWidget;
	}

	if (!LobbyEscapeMenuWidgetClass)
	{
		return nullptr;
	}

	LobbyEscapeMenuWidget =
		CreateWidget<ULobbyEscapeMenuWidget>(this, LobbyEscapeMenuWidgetClass);
	if (LobbyEscapeMenuWidget)
	{
		LobbyEscapeMenuWidget->SetLobbyPlayerController(this);
	}
	return LobbyEscapeMenuWidget;
}

UOptionsWidget* ALobbyPlayerController::EnsureOptionsWidget()
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
			&ALobbyPlayerController::HideOptionsMenu);
	}
	return OptionsWidget;
}

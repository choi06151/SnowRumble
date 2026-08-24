// Copyright Epic Games, Inc. All Rights Reserved.

#include "LobbyPlayerController.h"

#include "../Game/SnowRumblePlayerState.h"
#include "../Player/LocalPlayerIdentitySubsystem_C.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "InputCoreTypes.h"
#include "LobbyEscapeMenuWidget.h"
#include "LobbyBoardWidget_C.h"
#include "LobbyWidget.h"
#include "OptionsWidget_C.h"
#include "../Interaction/LobbyInteractionBoard_C.h"
#include "../Online/SnowRumbleSessionSubsystem.h"
#include "Components/WidgetComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameModeBase.h"

namespace
{
constexpr const TCHAR* LobbyMainMenuGameModeTravelPath =
	TEXT("/Game/Game/BP_MainMenuGameMode.BP_MainMenuGameMode_C");

void EnsureTravelOptionValue(
	FString& TravelUrl,
	const TCHAR* OptionName,
	const TCHAR* OptionValue)
{
	if (TravelUrl.IsEmpty() || !OptionName || !OptionValue)
	{
		return;
	}

	const FString OptionPrefix = FString::Printf(TEXT("%s="), OptionName);
	if (TravelUrl.Contains(OptionPrefix, ESearchCase::IgnoreCase))
	{
		return;
	}

	TravelUrl += TravelUrl.Contains(TEXT("?")) ? TEXT("?") : TEXT("?");
	TravelUrl += FString::Printf(TEXT("%s=%s"), OptionName, OptionValue);
}
}

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

void ALobbyPlayerController::ClientShowLobbyBoardInvalidActionFeedback_Implementation(
	const FText& ReasonText)
{
	if (!IsLocalController() || ReasonText.IsEmpty())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ALobbyInteractionBoard> It(World); It; ++It)
	{
		TArray<UWidgetComponent*> WidgetComponents;
		It->GetBoardWidgetComponents(WidgetComponents);
		for (UWidgetComponent* WidgetComponent : WidgetComponents)
		{
			ULobbyBoardWidget* BoardWidget = WidgetComponent
				? Cast<ULobbyBoardWidget>(WidgetComponent->GetUserWidgetObject())
				: nullptr;
			if (BoardWidget)
			{
				BoardWidget->ShowInvalidActionFeedbackForController(this, ReasonText);
			}
		}
	}
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
	FString ResolvedMainMenuTravelUrl = MainMenuTravelUrl;
	EnsureTravelOptionValue(
		ResolvedMainMenuTravelUrl,
		TEXT("game"),
		LobbyMainMenuGameModeTravelPath);

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
			// 메인 메뉴는 로비 PlayerController를 이어받지 않고 새로 생성되어야
			// 프리뷰 캐릭터의 중력·이동 잠금이 메인 메뉴 경로와 동일하게 적용된다.
			if (AGameModeBase* GameMode = World->GetAuthGameMode())
			{
				GameMode->bUseSeamlessTravel = false;
			}
			World->ServerTravel(ResolvedMainMenuTravelUrl);
		}
		return;
	}

	ClientTravel(ResolvedMainMenuTravelUrl, TRAVEL_Absolute);
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

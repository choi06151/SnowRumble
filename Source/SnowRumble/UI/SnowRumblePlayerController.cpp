// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumblePlayerController.h"

#include "../Game/SnowRumblePlayerState.h"
#include "ChatWidget_C.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerState.h"
#include "InputCoreTypes.h"
#include "LoadingScreenSubsystem.h"

void ASnowRumblePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		if (UChatWidget* Widget = EnsureChatWidget())
		{
			if (!Widget->IsInViewport())
			{
				Widget->AddToViewport(50);
			}
		}
	}
}

void ASnowRumblePlayerController::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	if (ChatWidget)
	{
		ChatWidget->RemoveFromParent();
		ChatWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void ASnowRumblePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		InputComponent->BindKey(
			EKeys::Enter,
			IE_Pressed,
			this,
			&ASnowRumblePlayerController::HandleChatInputPressed);
		InputComponent->BindKey(
			EKeys::Tab,
			IE_Pressed,
			this,
			&ASnowRumblePlayerController::HandleChatChannelTogglePressed);
	}
}

void ASnowRumblePlayerController::OpenChatInput(
	ESnowRumbleChatChannel InitialChannel)
{
	if (!IsLocalController() || !CanOpenChatInput())
	{
		return;
	}

	UChatWidget* Widget = EnsureChatWidget();
	if (!Widget)
	{
		return;
	}

	if (!Widget->IsInViewport())
	{
		Widget->AddToViewport(50);
	}

	bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(Widget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	Widget->OpenChatInput(InitialChannel);
}

void ASnowRumblePlayerController::CloseChatInput()
{
	if (ChatWidget)
	{
		ChatWidget->CloseChatInput();
	}

	if (IsLocalController())
	{
		bShowMouseCursor = false;
		SetInputMode(FInputModeGameOnly());
	}
}

void ASnowRumblePlayerController::SubmitChatMessage(
	const FString& Message,
	ESnowRumbleChatChannel Channel)
{
	if (!IsLocalController())
	{
		return;
	}

	const FString TrimmedMessage = Message.TrimStartAndEnd();
	if (TrimmedMessage.IsEmpty())
	{
		return;
	}
	const ESnowRumbleChatChannel RequestedChannel = SupportsTeamChat()
		? Channel
		: ESnowRumbleChatChannel::All;

	if (HasAuthority())
	{
		ServerSubmitChatMessage_Implementation(TrimmedMessage, RequestedChannel);
	}
	else
	{
		ServerSubmitChatMessage(TrimmedMessage, RequestedChannel);
	}
}

bool ASnowRumblePlayerController::IsTeamChatAvailable() const
{
	return SupportsTeamChat();
}

bool ASnowRumblePlayerController::IsChatInputOpen() const
{
	return ChatWidget && ChatWidget->IsChatInputOpen();
}

void ASnowRumblePlayerController::ClientShowLoadingScreen_Implementation()
{
	UGameInstance* GameInstance = GetGameInstance();
	ULoadingScreenSubsystem* LoadingScreenSubsystem = GameInstance
		? GameInstance->GetSubsystem<ULoadingScreenSubsystem>()
		: nullptr;
	if (LoadingScreenSubsystem)
	{
		LoadingScreenSubsystem->ShowLoadingScreen(LoadingScreenWidgetClass);
	}
}

bool ASnowRumblePlayerController::CanOpenChatInput() const
{
	return IsLocalController();
}

bool ASnowRumblePlayerController::SupportsTeamChat() const
{
	return true;
}

void ASnowRumblePlayerController::ServerSubmitChatMessage_Implementation(
	const FString& Message,
	ESnowRumbleChatChannel Channel)
{
	const FString TrimmedMessage = Message.TrimStartAndEnd();
	if (TrimmedMessage.IsEmpty())
	{
		return;
	}

	const FString SenderName = GetChatSenderName();
	constexpr int32 MaxChatMessageLength = 120;
	const FString ClampedMessage = TrimmedMessage.Left(MaxChatMessageLength);
	const APlayerState* SenderPlayerState = PlayerState;
	const ESnowRumbleChatChannel RoutedChannel = SupportsTeamChat()
		? Channel
		: ESnowRumbleChatChannel::All;

	if (UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It =
				World->GetPlayerControllerIterator();
			It;
			++It)
		{
			ASnowRumblePlayerController* Receiver =
				Cast<ASnowRumblePlayerController>(It->Get());
			if (Receiver
				&& Receiver->ShouldReceiveChatMessage(
					SenderPlayerState,
					RoutedChannel))
			{
				Receiver->ClientReceiveChatMessage(
					RoutedChannel,
					SenderName,
					ClampedMessage);
			}
		}
	}
}

void ASnowRumblePlayerController::ClientReceiveChatMessage_Implementation(
	ESnowRumbleChatChannel Channel,
	const FString& SenderName,
	const FString& Message)
{
	if (UChatWidget* Widget = EnsureChatWidget())
	{
		if (!Widget->IsInViewport())
		{
			Widget->AddToViewport(50);
		}
		Widget->AddChatMessage(Channel, SenderName, Message);
	}
}

void ASnowRumblePlayerController::HandleChatInputPressed()
{
	if (ChatWidget && ChatWidget->IsChatInputOpen())
	{
		return;
	}

	OpenChatInput(ESnowRumbleChatChannel::All);
}

void ASnowRumblePlayerController::HandleChatChannelTogglePressed()
{
	if (ChatWidget && ChatWidget->IsChatInputOpen())
	{
		ChatWidget->ToggleChatChannel();
	}
}

UChatWidget* ASnowRumblePlayerController::EnsureChatWidget()
{
	if (ChatWidget)
	{
		return ChatWidget;
	}

	if (!ChatWidgetClass)
	{
		return nullptr;
	}

	ChatWidget = CreateWidget<UChatWidget>(this, ChatWidgetClass);
	if (ChatWidget)
	{
		ChatWidget->SetChatPlayerController(this);
	}
	return ChatWidget;
}

bool ASnowRumblePlayerController::ShouldReceiveChatMessage(
	const APlayerState* SenderPlayerState,
	ESnowRumbleChatChannel Channel) const
{
	if (Channel == ESnowRumbleChatChannel::All)
	{
		return true;
	}

	const ASnowRumblePlayerState* SenderSnowRumblePlayerState =
		Cast<ASnowRumblePlayerState>(SenderPlayerState);
	const ASnowRumblePlayerState* ReceiverSnowRumblePlayerState =
		GetPlayerState<ASnowRumblePlayerState>();
	return SenderSnowRumblePlayerState
		&& ReceiverSnowRumblePlayerState
		&& SenderSnowRumblePlayerState->GetLobbyTeam()
			!= ESnowRumbleTeam::None
		&& SenderSnowRumblePlayerState->GetLobbyTeam()
			== ReceiverSnowRumblePlayerState->GetLobbyTeam();
}

FString ASnowRumblePlayerController::GetChatSenderName() const
{
	const ASnowRumblePlayerState* SnowRumblePlayerState =
		GetPlayerState<ASnowRumblePlayerState>();
	if (SnowRumblePlayerState
		&& !SnowRumblePlayerState->GetLobbyPlayerName().IsEmpty())
	{
		return SnowRumblePlayerState->GetLobbyPlayerName();
	}

	return PlayerState ? PlayerState->GetPlayerName() : TEXT("Player");
}

void ASnowRumblePlayerController::ClientUpdateLoadingProgress_Implementation(
	int32 LoadedPlayers,
	int32 ExpectedPlayers)
{
	UGameInstance* GameInstance = GetGameInstance();
	ULoadingScreenSubsystem* LoadingScreenSubsystem = GameInstance
		? GameInstance->GetSubsystem<ULoadingScreenSubsystem>()
		: nullptr;
	if (LoadingScreenSubsystem)
	{
		LoadingScreenSubsystem->SetLoadingProgress(
			LoadedPlayers,
			ExpectedPlayers);
	}
}

void ASnowRumblePlayerController::ClientHideLoadingScreen_Implementation()
{
	UGameInstance* GameInstance = GetGameInstance();
	ULoadingScreenSubsystem* LoadingScreenSubsystem = GameInstance
		? GameInstance->GetSubsystem<ULoadingScreenSubsystem>()
		: nullptr;
	if (LoadingScreenSubsystem)
	{
		LoadingScreenSubsystem->HideLoadingScreen();
	}
}

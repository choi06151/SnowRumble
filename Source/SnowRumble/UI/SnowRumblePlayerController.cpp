// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumblePlayerController.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "../Audio/SnowRumbleBackgroundMusicSubsystem_C.h"
#include "../Game/SnowRumblePlayerState.h"
#include "../Game/SnowRumbleGameMode.h"
#include "../Game/SnowRumbleGameState_C.h"
#include "../Player/SnowRumbleUserSettingsSubsystem_C.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "ChatWidget_C.h"
#include "Components/InputComponent.h"
#include "Components/Widget.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/Pawn.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerState.h"
#include "InputCoreTypes.h"
#include "LobbyWidget.h"
#include "LoadingScreenSubsystem.h"
#include "MainHUDWidget.h"
#include "TimedDropAnnouncementWidget.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/VoiceInterface.h"
#include "Sound/SoundBase.h"
#include "VoiceMuteMenuWidget_C.h"
#include "../Player/SnowRumbleCharacter.h"

DEFINE_LOG_CATEGORY_STATIC(LogSnowRumbleVoice, Log, All);

void ASnowRumblePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() && HasAttachedLocalPlayer())
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
				GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>())
			{
				UserSettingsSubsystem->OnKeyBindingsChanged.AddUObject(
					this,
					&ASnowRumblePlayerController::HandleUserKeyBindingsChanged);
				UserSettingsSubsystem->OnMicrophoneSettingsChanged.AddUObject(
					this,
					&ASnowRumblePlayerController::HandleMicrophoneSettingsChanged);
			}
		}

		if (UChatWidget* Widget = EnsureChatWidget())
		{
			if (!Widget->IsInViewport())
			{
				Widget->AddToViewport(50);
			}
			if (bPvpIntroWidgetsHidden)
			{
				Widget->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
		EnsureLocalVoiceTalkerReady();
		EnsureRemoteVoiceTalkersReady();
	}
}

void ASnowRumblePlayerController::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	ApplyNetworkVoiceInputState(false);
	ApplyReplicatedVoiceSpeakingState(false);
	GameplayUnmuteAllPlayers();
	RegisteredRemoteVoiceTalkerIds.Reset();
	bLocalVoiceTalkerReady = false;

	if (ChatWidget)
	{
		ChatWidget->RemoveFromParent();
		ChatWidget = nullptr;
	}
	if (VoiceMuteMenuWidget)
	{
		VoiceMuteMenuWidget->RemoveFromParent();
		VoiceMuteMenuWidget = nullptr;
	}
	if (PvpIntroCameraActor)
	{
		PvpIntroCameraActor->Destroy();
		PvpIntroCameraActor = nullptr;
	}
	GetWorldTimerManager().ClearTimer(PvpIntroCameraDestroyTimerHandle);
	DefaultMouseCursorWidget = nullptr;

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
			GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>())
		{
			UserSettingsSubsystem->OnKeyBindingsChanged.RemoveAll(this);
			UserSettingsSubsystem->OnMicrophoneSettingsChanged.RemoveAll(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

bool ASnowRumblePlayerController::InputKey(const FInputKeyEventArgs& Params)
{
	if (IsLocalController()
		&& Params.Event == IE_Pressed
		&& (Params.Key == BoundVoiceTargetMuteKey
			|| (!BoundVoiceTargetMuteKey.IsValid() && Params.Key == EKeys::M)))
	{
		RequestVoiceTargetMute();
		return true;
	}

	return Super::InputKey(Params);
}

void ASnowRumblePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	RebindConfiguredInputKeys();
}

void ASnowRumblePlayerController::RebindConfiguredInputKeys()
{
	if (!InputComponent)
	{
		return;
	}

	InputComponent->KeyBindings.RemoveAll([this](const FInputKeyBinding& Binding)
	{
		return (Binding.KeyEvent == IE_Pressed
				&& (Binding.Chord.Key == BoundChatInputKey
					|| Binding.Chord.Key == BoundChatChannelToggleKey
					|| Binding.Chord.Key == BoundMicrophonePushToTalkKey
					|| Binding.Chord.Key == BoundVoiceChannelToggleKey
					|| Binding.Chord.Key == BoundVoiceTargetMuteKey))
			|| (Binding.KeyEvent == IE_Released
				&& Binding.Chord.Key == BoundMicrophonePushToTalkKey);
	});

	const UGameInstance* GameInstance = GetGameInstance();
	const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
		GameInstance
			? GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>()
			: nullptr;

	BoundChatInputKey = UserSettingsSubsystem
		? UserSettingsSubsystem->GetKeyBinding(TEXT("Chat"), EKeys::Enter)
		: EKeys::Enter;
	BoundChatChannelToggleKey = UserSettingsSubsystem
		? UserSettingsSubsystem->GetKeyBinding(TEXT("ChatChannelToggle"), EKeys::Tab)
		: EKeys::Tab;
	BoundMicrophonePushToTalkKey = UserSettingsSubsystem
		? UserSettingsSubsystem->GetKeyBinding(
			TEXT("MicrophonePushToTalk"),
			EKeys::K)
		: EKeys::K;
	BoundVoiceChannelToggleKey = UserSettingsSubsystem
		? UserSettingsSubsystem->GetKeyBinding(
			TEXT("MicrophoneChannelToggle"),
			EKeys::N)
		: EKeys::N;
	BoundVoiceTargetMuteKey = UserSettingsSubsystem
		? UserSettingsSubsystem->GetKeyBinding(
			TEXT("VoiceTargetMute"),
			EKeys::M)
		: EKeys::M;

	// 플레이어 지정 음소거는 M 키가 다른 음성 입력 설정에 남아 있어도
	// 항상 우선 처리한다. 이후 바인딩에서 같은 키를 건너뛰어 입력 소비 충돌을 막는다.
	if (BoundVoiceTargetMuteKey.IsValid())
	{
		InputComponent->BindKey(
			BoundVoiceTargetMuteKey,
			IE_Pressed,
			this,
			&ASnowRumblePlayerController::RequestVoiceTargetMute);
	}

	if (BoundChatInputKey.IsValid())
	{
		if (BoundChatInputKey != BoundVoiceTargetMuteKey)
		{
			InputComponent->BindKey(
				BoundChatInputKey,
				IE_Pressed,
				this,
				&ASnowRumblePlayerController::HandleChatInputPressed);
		}
	}
	if (BoundChatChannelToggleKey.IsValid()
		&& BoundChatChannelToggleKey != BoundChatInputKey
		&& BoundChatChannelToggleKey != BoundVoiceTargetMuteKey)
	{
		FInputKeyBinding& ChatChannelToggleBinding = InputComponent->BindKey(
			BoundChatChannelToggleKey,
			IE_Pressed,
			this,
			&ASnowRumblePlayerController::HandleChatChannelTogglePressed);
		ChatChannelToggleBinding.bConsumeInput = false;
	}
	if (BoundMicrophonePushToTalkKey.IsValid()
		&& BoundMicrophonePushToTalkKey != BoundVoiceTargetMuteKey)
	{
		InputComponent->BindKey(
			BoundMicrophonePushToTalkKey,
			IE_Pressed,
			this,
			&ASnowRumblePlayerController::HandleMicrophonePushToTalkPressed);
		InputComponent->BindKey(
			BoundMicrophonePushToTalkKey,
			IE_Released,
			this,
			&ASnowRumblePlayerController::HandleMicrophonePushToTalkReleased);
	}
	if (BoundVoiceChannelToggleKey.IsValid()
		&& BoundVoiceChannelToggleKey != BoundChatInputKey
		&& BoundVoiceChannelToggleKey != BoundChatChannelToggleKey
		&& BoundVoiceChannelToggleKey != BoundVoiceTargetMuteKey)
	{
		InputComponent->BindKey(
			BoundVoiceChannelToggleKey,
			IE_Pressed,
			this,
			&ASnowRumblePlayerController::RequestVoiceChannelToggle);
	}
	RefreshMicrophoneInputState();
	ApplyReplicatedVoiceChannel(LocalVoiceChannel);
}

void ASnowRumblePlayerController::HandleUserKeyBindingsChanged()
{
	RebindConfiguredInputKeys();
}

void ASnowRumblePlayerController::HandleMicrophoneSettingsChanged()
{
	RefreshMicrophoneInputState();
}

void ASnowRumblePlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (IsLocalController())
	{
		RefreshGameplayVoiceMutes();
		UpdatePvpIntroCamera(DeltaTime);
		if (bPvpIntroWidgetsHidden)
		{
			if (ASnowRumbleCharacter* LocalCharacter =
				Cast<ASnowRumbleCharacter>(GetPawn()))
			{
				LocalCharacter->SetPvpIntroWidgetsHidden(true);
			}
		}
		TrySchedulePvpReadyHandshake();
		TryNotifyPvpReady();
		ApplyVoiceMuteMenuInputState();
	}
}

void ASnowRumblePlayerController::TrySchedulePvpReadyHandshake()
{
	if (!IsLocalController() || !GetWorld())
	{
		return;
	}

	if (!GetWorld()->GetGameState<ASnowRumbleGameState>())
	{
		return;
	}

	const FString CurrentMapName = GetWorld()->GetMapName();
	if (PvpReadyMapName != CurrentMapName)
	{
		PvpReadyMapName = CurrentMapName;
		PvpReadyWarmupElapsedSeconds = 0.0f;
		bPvpReadySubmitted = false;
	}
}

void ASnowRumblePlayerController::TryNotifyPvpReady()
{
	if (!IsLocalController()
		|| bPvpReadySubmitted
		|| !GetWorld()->GetGameState<ASnowRumbleGameState>()
		|| !GetPawn()
		|| !PlayerState)
	{
		return;
	}

	PvpReadyWarmupElapsedSeconds += GetWorld()->GetDeltaSeconds();
	if (PvpReadyWarmupElapsedSeconds < 0.5f)
	{
		return;
	}

	bPvpReadySubmitted = true;
	ServerNotifyPvpReady(PvpReadyMapName);
}

void ASnowRumblePlayerController::ServerNotifyPvpReady_Implementation(
	const FString& PvpMapName)
{
	if (!HasAuthority() || !GetWorld() || PvpMapName.IsEmpty())
	{
		return;
	}

	ASnowRumbleGameMode* SnowRumbleGameMode =
		Cast<ASnowRumbleGameMode>(GetWorld()->GetAuthGameMode());
	if (SnowRumbleGameMode)
	{
		SnowRumbleGameMode->NotifyPvpPlayerReady(this);
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

	SetShowMouseCursor(true);
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	ApplyDefaultMouseCursorWidget();
	if (!bChatInputIgnoringPawnInput)
	{
		SetIgnoreMoveInput(true);
		SetIgnoreLookInput(true);
		bChatInputIgnoringPawnInput = true;
	}

	Widget->OpenChatInput(InitialChannel);

	FInputModeGameAndUI InputMode;
	if (UWidget* FocusWidget = Widget->GetChatInputFocusWidget())
	{
		InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
	}
	else
	{
		InputMode.SetWidgetToFocus(Widget->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	Widget->FocusChatInputTextBox();
}

void ASnowRumblePlayerController::CloseChatInput()
{
	if (ChatWidget)
	{
		ChatWidget->CloseChatInput();
	}

	if (IsLocalController())
	{
		if (bChatInputIgnoringPawnInput)
		{
			ResetIgnoreMoveInput();
			ResetIgnoreLookInput();
			bChatInputIgnoringPawnInput = false;
		}
		SetShowMouseCursor(false);
		SetInputMode(FInputModeGameOnly());
	}
}

void ASnowRumblePlayerController::EnableDefaultCursorUiInput(
	UUserWidget* WidgetToFocus,
	bool bUseCustomCursorWidget)
{
	if (!IsLocalController())
	{
		return;
	}

	SetShowMouseCursor(true);
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CurrentMouseCursor = EMouseCursor::Default;
	if (bUseCustomCursorWidget)
	{
		ApplyDefaultMouseCursorWidget();
	}
	else
	{
		SetMouseCursorWidget(EMouseCursor::Default, nullptr);
	}

	FInputModeGameAndUI InputMode;
	if (WidgetToFocus)
	{
		InputMode.SetWidgetToFocus(WidgetToFocus->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
}

void ASnowRumblePlayerController::RestoreGameOnlyInput()
{
	if (!IsLocalController())
	{
		return;
	}

	if (IsVoiceMuteMenuOpen())
	{
		ApplyVoiceMuteMenuInputState(true);
		return;
	}

	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());
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

bool ASnowRumblePlayerController::IsGameplayUiInputOpen() const
{
	return IsChatInputOpen()
		|| IsVoiceMuteMenuOpen();
}

bool ASnowRumblePlayerController::IsVoiceMuteMenuOpen() const
{
	return VoiceMuteMenuWidget && VoiceMuteMenuWidget->IsInViewport();
}

bool ASnowRumblePlayerController::IsMicrophoneInputActive() const
{
	return bMicrophoneInputActive;
}

ESnowRumbleMicrophoneMode
ASnowRumblePlayerController::GetMicrophoneMode() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
		GameInstance
			? GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>()
			: nullptr;
	return UserSettingsSubsystem
		? UserSettingsSubsystem->GetMicrophoneMode()
		: ESnowRumbleMicrophoneMode::PushToTalk;
}

ESnowRumbleVoiceChannel ASnowRumblePlayerController::GetVoiceChannel() const
{
	return LocalVoiceChannel;
}

void ASnowRumblePlayerController::RequestMicrophonePushToTalkStarted()
{
	HandleMicrophonePushToTalkPressed();
}

void ASnowRumblePlayerController::RequestMicrophonePushToTalkCompleted()
{
	HandleMicrophonePushToTalkReleased();
}

void ASnowRumblePlayerController::RequestVoiceChannelToggle()
{
	const UWorld* World = GetWorld();
	const double CurrentTimeSeconds = World ? World->GetTimeSeconds() : 0.0;
	if (CurrentTimeSeconds - LastVoiceChannelToggleTimeSeconds < 0.1)
	{
		return;
	}
	LastVoiceChannelToggleTimeSeconds = CurrentTimeSeconds;

	HandleVoiceChannelTogglePressed();
}

void ASnowRumblePlayerController::RequestVoiceTargetMute()
{
	ToggleVoiceMuteMenu();
	OnVoiceTargetMuteRequested();
	ApplyVoiceMuteMenuInputState(true);
}

void ASnowRumblePlayerController::HideVoiceMuteMenu()
{
	if (!VoiceMuteMenuWidget)
	{
		return;
	}

	VoiceMuteMenuWidget->RemoveFromParent();
	VoiceMuteMenuWidget = nullptr;
	bVoiceMuteMenuInputStateApplied = false;

	ResetIgnoreMoveInput();
	ResetIgnoreLookInput();
	SetShowMouseCursor(false);
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}

void ASnowRumblePlayerController::ApplyVoiceMuteMenuInputState(bool bForce)
{
	if (!IsLocalController()
		|| !VoiceMuteMenuWidget
		|| !VoiceMuteMenuWidget->IsInViewport())
	{
		bVoiceMuteMenuInputStateApplied = false;
		return;
	}

	if (bVoiceMuteMenuInputStateApplied && !bForce)
	{
		return;
	}

	ResetIgnoreMoveInput();
	ResetIgnoreLookInput();
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
	SetShowMouseCursor(true);
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	ApplyDefaultMouseCursorWidget();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(VoiceMuteMenuWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	VoiceMuteMenuWidget->SetKeyboardFocus();
	bVoiceMuteMenuInputStateApplied = true;
}

void ASnowRumblePlayerController::ToggleManualVoiceMute(
	ASnowRumblePlayerState* TargetPlayerState)
{
	const FString PlayerKey = GetVoiceMutePlayerKey(TargetPlayerState);
	if (PlayerKey.IsEmpty())
	{
		return;
	}

	const bool bWasMuted = ManuallyMutedVoicePlayerKeys.Contains(PlayerKey);
	if (bWasMuted)
	{
		ManuallyMutedVoicePlayerKeys.Remove(PlayerKey);
	}
	else
	{
		ManuallyMutedVoicePlayerKeys.Add(PlayerKey);
	}

	ApplyManualVoiceMuteState(TargetPlayerState, !bWasMuted);
	RefreshGameplayVoiceMutes();
}

void ASnowRumblePlayerController::SetBackgroundMusicPreviewVolume(
	float MasterVolume,
	float BgmVolume)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USnowRumbleBackgroundMusicSubsystem* BackgroundMusicSubsystem =
			GameInstance->GetSubsystem<USnowRumbleBackgroundMusicSubsystem>())
		{
			BackgroundMusicSubsystem->SetBackgroundMusicPreviewVolume(
				MasterVolume,
				BgmVolume);
		}
	}
}

bool ASnowRumblePlayerController::IsVoicePlayerManuallyMuted(
	const ASnowRumblePlayerState* TargetPlayerState) const
{
	const FString PlayerKey = GetVoiceMutePlayerKey(TargetPlayerState);
	return !PlayerKey.IsEmpty()
		&& ManuallyMutedVoicePlayerKeys.Contains(PlayerKey);
}

void ASnowRumblePlayerController::ClientShowLoadingScreen_Implementation()
{
	UGameInstance* GameInstance = GetGameInstance();
	ULoadingScreenSubsystem* LoadingScreenSubsystem = GameInstance
		? GameInstance->GetSubsystem<ULoadingScreenSubsystem>()
		: nullptr;
	if (LoadingScreenSubsystem)
	{
	LoadingScreenSubsystem->ShowLoadingScreen(
		bUseSnowmanLoadingScreen && SnowmanLoadingScreenWidgetClass
			? SnowmanLoadingScreenWidgetClass
			: LoadingScreenWidgetClass);
	}
}

void ASnowRumblePlayerController::ClientSetLoadingPresentation_Implementation(
	const FString& MapPackageName,
	const FText& MapDisplayName,
	const TSoftObjectPtr<UTexture2D>& MapLoadingImage,
	const TArray<FString>& TeamPlayerNames,
	bool bIsSnowmanMode)
{
	bUseSnowmanLoadingScreen = bIsSnowmanMode;
	UGameInstance* GameInstance = GetGameInstance();
	ULoadingScreenSubsystem* LoadingScreenSubsystem = GameInstance
		? GameInstance->GetSubsystem<ULoadingScreenSubsystem>()
		: nullptr;
	if (LoadingScreenSubsystem)
	{
		LoadingScreenSubsystem->SetLoadingPresentation(
			MapPackageName,
			MapDisplayName,
			MapLoadingImage,
			TeamPlayerNames);
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
		if (bPvpIntroWidgetsHidden)
		{
			Widget->SetVisibility(ESlateVisibility::Collapsed);
		}
		Widget->AddChatMessage(Channel, SenderName, Message);
	}
}

void ASnowRumblePlayerController::ClientReceiveEventLogMessage_Implementation(
	const FText& Message)
{
	if (!IsLocalController() || Message.IsEmpty())
	{
		return;
	}

	TArray<UUserWidget*> LobbyWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
		this,
		LobbyWidgets,
		ULobbyWidget::StaticClass(),
		false);
	for (UUserWidget* UserWidget : LobbyWidgets)
	{
		ULobbyWidget* LobbyWidget = Cast<ULobbyWidget>(UserWidget);
		if (LobbyWidget && LobbyWidget->GetOwningPlayer() == this)
		{
			LobbyWidget->AddEventLogMessage(Message);
		}
	}

	TArray<UUserWidget*> MainHUDWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
		this,
		MainHUDWidgets,
		UMainHUDWidget::StaticClass(),
		false);
	for (UUserWidget* UserWidget : MainHUDWidgets)
	{
		UMainHUDWidget* MainHUDWidget = Cast<UMainHUDWidget>(UserWidget);
		if (MainHUDWidget && MainHUDWidget->GetOwningPlayer() == this)
		{
			MainHUDWidget->AddEventLogMessage(Message);
		}
	}
}

void ASnowRumblePlayerController::ClientShowPersonalTextAlarm_Implementation(
	const FText& Message)
{
	if (!IsLocalController() || Message.IsEmpty())
	{
		return;
	}

	TArray<UUserWidget*> LobbyWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
		this,
		LobbyWidgets,
		ULobbyWidget::StaticClass(),
		false);
	for (UUserWidget* UserWidget : LobbyWidgets)
	{
		ULobbyWidget* LobbyWidget = Cast<ULobbyWidget>(UserWidget);
		if (LobbyWidget && LobbyWidget->GetOwningPlayer() == this)
		{
			LobbyWidget->ShowPersonalTextAlarm(Message);
		}
	}

	TArray<UUserWidget*> MainHUDWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
		this,
		MainHUDWidgets,
		UMainHUDWidget::StaticClass(),
		false);
	for (UUserWidget* UserWidget : MainHUDWidgets)
	{
		UMainHUDWidget* MainHUDWidget = Cast<UMainHUDWidget>(UserWidget);
		if (MainHUDWidget && MainHUDWidget->GetOwningPlayer() == this)
		{
			MainHUDWidget->ShowPersonalTextAlarm(Message);
		}
	}
}

void ASnowRumblePlayerController::ClientShowTimedDropAnnouncement_Implementation(
	TSubclassOf<UTimedDropAnnouncementWidget> WidgetClass,
	float DisplayDurationSeconds)
{
	if (!IsLocalController() || !HasAttachedLocalPlayer() || !WidgetClass)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(TimedDropAnnouncementTimerHandle);
	if (TimedDropAnnouncementWidget)
	{
		TimedDropAnnouncementWidget->RemoveFromParent();
		TimedDropAnnouncementWidget = nullptr;
	}

	TimedDropAnnouncementWidget = CreateWidget<UTimedDropAnnouncementWidget>(
		this,
		WidgetClass);
	if (!TimedDropAnnouncementWidget)
	{
		return;
	}

	TimedDropAnnouncementWidget->AddToViewport(200);
	TimedDropAnnouncementWidget->StartAnnouncementAnimation();

	FTimerDelegate RemoveAnnouncementDelegate = FTimerDelegate::CreateWeakLambda(
		this,
		[this]()
		{
			if (TimedDropAnnouncementWidget)
			{
				TimedDropAnnouncementWidget->RemoveFromParent();
				TimedDropAnnouncementWidget = nullptr;
			}
		});
	GetWorldTimerManager().SetTimer(
		TimedDropAnnouncementTimerHandle,
		RemoveAnnouncementDelegate,
		FMath::Max(0.1f, DisplayDurationSeconds),
		false);
}

void ASnowRumblePlayerController::ClientPlayPvpTeamIntroShot_Implementation(
	ESnowRumbleTeam Team,
	float ShotDurationSeconds)
{
	if (!IsLocalController())
	{
		return;
	}

	SetPvpIntroWidgetsHidden(true);

	const FText TeamDisplayText = GetPvpIntroTeamDisplayText(Team);
	OnPvpTeamIntroShot(Team, TeamDisplayText, ShotDurationSeconds);
	ClientShowPersonalTextAlarm_Implementation(TeamDisplayText);
	if (PlayerCameraManager)
	{
		PlayerCameraManager->StopCameraFade();
	}

	TArray<APawn*> TeamPawns;
	GetPvpIntroTeamPawns(Team, TeamPawns);

	FTransform DollyStartTransform;
	FTransform DollyEndTransform;
	if (!BuildPvpIntroCameraTransform(
		TeamPawns,
		DollyStartTransform,
		DollyEndTransform))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(PvpIntroCameraDestroyTimerHandle);

	FVector CurrentCameraLocation = FVector::ZeroVector;
	FRotator CurrentCameraRotation = FRotator::ZeroRotator;
	GetPlayerViewPoint(CurrentCameraLocation, CurrentCameraRotation);
	PvpIntroCameraCurrentStartTransform =
		FTransform(CurrentCameraRotation, CurrentCameraLocation);
	PvpIntroCameraDollyStartTransform = DollyStartTransform;
	PvpIntroCameraDollyEndTransform = DollyEndTransform;
	PvpIntroCameraElapsedSeconds = 0.0f;
	PvpIntroCameraDurationSeconds =
		FMath::Max(0.01f, ShotDurationSeconds);
	bPvpIntroCameraActive = true;

	if (!PvpIntroCameraActor)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		PvpIntroCameraActor = World->SpawnActor<ACameraActor>(
			ACameraActor::StaticClass(),
			PvpIntroCameraCurrentStartTransform,
			SpawnParameters);
		if (PvpIntroCameraActor)
		{
			if (UCameraComponent* CameraComponent =
				PvpIntroCameraActor->GetCameraComponent())
			{
				CameraComponent->SetConstraintAspectRatio(true);
				CameraComponent->SetAspectRatio(
					FMath::Max(1.0f, PvpIntroCinematicAspectRatio));
			}
		}
	}
	else
	{
		PvpIntroCameraActor->SetActorTransform(
			PvpIntroCameraCurrentStartTransform);
	}

	if (PvpIntroCameraActor)
	{
		SetViewTargetWithBlend(
			PvpIntroCameraActor,
			0.0f);
	}
}

void ASnowRumblePlayerController::ClientStartPvpIntroFadeOut_Implementation(
	float FadeOutSeconds)
{
	if (!IsLocalController() || !PlayerCameraManager)
	{
		return;
	}

	const float ClampedFadeOutSeconds =
		FMath::Max(0.01f, FadeOutSeconds);
	PlayerCameraManager->StartCameraFade(
		1.0f,
		0.0f,
		ClampedFadeOutSeconds,
		FLinearColor::Black,
		false,
		false);
}

void ASnowRumblePlayerController::ClientPreparePvpIntroWidgets_Implementation()
{
	PreparePvpIntroWidgetsForLocalIntro();
}

void ASnowRumblePlayerController::ClientFinishPvpTeamIntro_Implementation()
{
	if (!IsLocalController())
	{
		return;
	}

	RestorePvpIntroWidgetsForLocalIntro();

	if (APawn* ControlledPawn = GetPawn())
	{
		SetViewTargetWithBlend(
			ControlledPawn,
			PvpIntroCameraReturnBlendSeconds);
	}

	bPvpIntroCameraActive = false;
	if (PvpIntroCameraActor)
	{
		GetWorldTimerManager().SetTimer(
			PvpIntroCameraDestroyTimerHandle,
			[this]()
			{
				if (PvpIntroCameraActor)
				{
					PvpIntroCameraActor->Destroy();
					PvpIntroCameraActor = nullptr;
				}
			},
			PvpIntroCameraReturnBlendSeconds,
			false);
	}
}

void ASnowRumblePlayerController::ClientPlayBackgroundMusic_Implementation(
	USoundBase* BackgroundMusicSound)
{
	if (!IsLocalController())
	{
		return;
	}

	PlayBackgroundMusic(BackgroundMusicSound);
}

void ASnowRumblePlayerController::ClientStopBackgroundMusic_Implementation()
{
	if (!IsLocalController())
	{
		return;
	}

	StopBackgroundMusic();
}

void ASnowRumblePlayerController::HandleChatInputPressed()
{
	if (ChatWidget && ChatWidget->IsChatInputOpen())
	{
		return;
	}

	const ESnowRumbleChatChannel InitialChannel =
		ChatWidget && SupportsTeamChat()
			? ChatWidget->GetActiveChatChannel()
			: ESnowRumbleChatChannel::All;
	OpenChatInput(InitialChannel);
}

void ASnowRumblePlayerController::HandleChatChannelTogglePressed()
{
	if (ChatWidget && ChatWidget->IsChatInputOpen())
	{
		ChatWidget->ToggleChatChannel();
	}
}

void ASnowRumblePlayerController::HandleVoiceChannelTogglePressed()
{
	LocalVoiceChannel = LocalVoiceChannel == ESnowRumbleVoiceChannel::All
		? ESnowRumbleVoiceChannel::Team
		: ESnowRumbleVoiceChannel::All;

	ApplyReplicatedVoiceChannel(LocalVoiceChannel);
	ShowVoiceChannelFeedback(LocalVoiceChannel);
	RefreshGameplayVoiceMutes();
}

void ASnowRumblePlayerController::HandleMicrophonePushToTalkPressed()
{
	if (GetMicrophoneMode() == ESnowRumbleMicrophoneMode::PushToTalk)
	{
		SetMicrophoneInputActive(true);
	}
}

void ASnowRumblePlayerController::HandleMicrophonePushToTalkReleased()
{
	if (GetMicrophoneMode() == ESnowRumbleMicrophoneMode::PushToTalk)
	{
		SetMicrophoneInputActive(false);
	}
}

void ASnowRumblePlayerController::SetMicrophoneInputActive(bool bNewActive)
{
	const bool bNewVoiceSpeaking =
		bNewActive && ShouldMirrorMicrophoneInputToVoiceSpeaking();

	if (bMicrophoneInputActive == bNewActive)
	{
		ApplyReplicatedVoiceSpeakingState(bNewVoiceSpeaking);
		return;
	}

	bMicrophoneInputActive = bNewActive;
	UE_LOG(
		LogSnowRumbleVoice,
		Log,
		TEXT("Microphone input %s. Controller=%s Mode=%d"),
		bMicrophoneInputActive ? TEXT("ON") : TEXT("OFF"),
		*GetNameSafe(this),
		static_cast<int32>(GetMicrophoneMode()));
	ApplyNetworkVoiceInputState(bMicrophoneInputActive);
	ApplyReplicatedVoiceSpeakingState(bNewVoiceSpeaking);
	OnMicrophoneInputStateChanged(bMicrophoneInputActive);
}

void ASnowRumblePlayerController::ApplyNetworkVoiceInputState(
	bool bShouldSpeak)
{
	if (!IsLocalController())
	{
		UE_LOG(
			LogSnowRumbleVoice,
			Warning,
			TEXT("Network voice input skipped: controller is not local. Controller=%s ShouldSpeak=%d"),
			*GetNameSafe(this),
			bShouldSpeak ? 1 : 0);
		return;
	}

	if (bNetworkVoiceInputActive == bShouldSpeak)
	{
		UE_LOG(
			LogSnowRumbleVoice,
			Verbose,
			TEXT("Network voice input unchanged. Controller=%s ShouldSpeak=%d"),
			*GetNameSafe(this),
			bShouldSpeak ? 1 : 0);
		return;
	}

	bNetworkVoiceInputActive = bShouldSpeak;
	if (bNetworkVoiceInputActive)
	{
		EnsureLocalVoiceTalkerReady();
		UE_LOG(
			LogSnowRumbleVoice,
			Log,
			TEXT("StartTalking requested. Controller=%s"),
			*GetNameSafe(this));
		StartTalking();
	}
	else
	{
		UE_LOG(
			LogSnowRumbleVoice,
			Log,
			TEXT("StopTalking requested. Controller=%s"),
			*GetNameSafe(this));
		StopTalking();
	}
}

bool ASnowRumblePlayerController::EnsureLocalVoiceTalkerReady()
{
	if (!IsLocalController())
	{
		return false;
	}

	const ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (!LocalPlayer)
	{
		UE_LOG(
			LogSnowRumbleVoice,
			Warning,
			TEXT("Voice setup skipped: local player is missing. Controller=%s"),
			*GetNameSafe(this));
		return false;
	}

	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	if (!OnlineSubsystem)
	{
		UE_LOG(
			LogSnowRumbleVoice,
			Warning,
			TEXT("Voice setup failed: OnlineSubsystem is unavailable."));
		return false;
	}

	IOnlineVoicePtr VoiceInterface = OnlineSubsystem->GetVoiceInterface();
	if (!VoiceInterface.IsValid())
	{
		UE_LOG(
			LogSnowRumbleVoice,
			Warning,
			TEXT("Voice setup failed: VoiceInterface is unavailable. Subsystem=%s"),
			*OnlineSubsystem->GetSubsystemName().ToString());
		return false;
	}

	const uint32 LocalUserNum =
		static_cast<uint32>(LocalPlayer->GetControllerId());
	const bool bRegistered = VoiceInterface->RegisterLocalTalker(LocalUserNum);
	const bool bWasReady = bLocalVoiceTalkerReady;
	bLocalVoiceTalkerReady = bRegistered;
	if (bRegistered && !bWasReady && !bNetworkVoiceInputActive)
	{
		StopTalking();
	}
	const bool bHeadsetPresent = VoiceInterface->IsHeadsetPresent(LocalUserNum);
	UE_LOG(
		LogSnowRumbleVoice,
		Log,
		TEXT("Voice local talker ready check. User=%u Registered=%d HeadsetPresent=%d Subsystem=%s"),
		LocalUserNum,
		bRegistered ? 1 : 0,
		bHeadsetPresent ? 1 : 0,
		*OnlineSubsystem->GetSubsystemName().ToString());

	return bRegistered;
}

void ASnowRumblePlayerController::EnsureRemoteVoiceTalkersReady()
{
	if (!IsLocalController())
	{
		return;
	}

	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	IOnlineVoicePtr VoiceInterface = OnlineSubsystem
		? OnlineSubsystem->GetVoiceInterface()
		: nullptr;
	if (!VoiceInterface.IsValid())
	{
		return;
	}

	UWorld* World = GetWorld();
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	const ASnowRumblePlayerState* LocalPlayerState =
		GetPlayerState<ASnowRumblePlayerState>();
	if (!GameState || !LocalPlayerState)
	{
		return;
	}

	for (APlayerState* CandidatePlayerState : GameState->PlayerArray)
	{
		const ASnowRumblePlayerState* SnowRumblePlayerState =
			Cast<ASnowRumblePlayerState>(CandidatePlayerState);
		if (!SnowRumblePlayerState
			|| SnowRumblePlayerState == LocalPlayerState)
		{
			continue;
		}

		const FUniqueNetIdRepl& PlayerNetId =
			SnowRumblePlayerState->GetUniqueId();
		if (!PlayerNetId.IsValid() || !PlayerNetId.IsV1())
		{
			continue;
		}

		const FString TalkerId = PlayerNetId->ToString();
		if (RegisteredRemoteVoiceTalkerIds.Contains(TalkerId))
		{
			continue;
		}

		const bool bRegistered =
			VoiceInterface->RegisterRemoteTalker(*PlayerNetId);
		UE_LOG(
			LogSnowRumbleVoice,
			Log,
			TEXT("Voice remote talker register. Player=%s Registered=%d Subsystem=%s"),
			*GetNameSafe(SnowRumblePlayerState),
			bRegistered ? 1 : 0,
			*OnlineSubsystem->GetSubsystemName().ToString());
		if (bRegistered)
		{
			RegisteredRemoteVoiceTalkerIds.Add(TalkerId);
		}
	}

}

void ASnowRumblePlayerController::SetPvpIntroWidgetsHidden(bool bShouldHide)
{
	if (!IsLocalController())
	{
		return;
	}

	if (bShouldHide)
	{
		if (bPvpIntroWidgetsHidden)
		{
			CollapseViewportMainHUDWidgets();
			if (ChatWidget)
			{
				ChatWidget->SetVisibility(ESlateVisibility::Collapsed);
			}
			if (VoiceMuteMenuWidget)
			{
				HideVoiceMuteMenu();
			}
			if (ASnowRumbleCharacter* LocalCharacter =
				Cast<ASnowRumbleCharacter>(GetPawn()))
			{
				LocalCharacter->SetPvpIntroWidgetsHidden(true);
			}
			RestoreGameOnlyInput();
			return;
		}

		bPvpIntroWidgetsHidden = true;
		CloseChatInput();
		CollapseViewportMainHUDWidgets();
		if (ChatWidget)
		{
			PvpIntroChatVisibility = ChatWidget->GetVisibility();
			ChatWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (VoiceMuteMenuWidget)
		{
			HideVoiceMuteMenu();
		}
		if (ASnowRumbleCharacter* LocalCharacter =
			Cast<ASnowRumbleCharacter>(GetPawn()))
		{
			LocalCharacter->SetPvpIntroWidgetsHidden(true);
		}
		RestoreGameOnlyInput();
		return;
	}

	if (!bPvpIntroWidgetsHidden)
	{
		return;
	}

	bPvpIntroWidgetsHidden = false;
	if (ChatWidget)
	{
		ChatWidget->SetVisibility(PvpIntroChatVisibility);
	}
	if (ASnowRumbleCharacter* LocalCharacter =
		Cast<ASnowRumbleCharacter>(GetPawn()))
	{
		LocalCharacter->SetPvpIntroWidgetsHidden(false);
	}
}

bool ASnowRumblePlayerController::IsPvpIntroWidgetsHidden() const
{
	return bPvpIntroWidgetsHidden;
}

void ASnowRumblePlayerController::PreparePvpIntroWidgetsForLocalIntro()
{
	if (!IsLocalController())
	{
		return;
	}

	SetPvpIntroWidgetsHidden(true);
}

void ASnowRumblePlayerController::RestorePvpIntroWidgetsForLocalIntro()
{
	if (!IsLocalController())
	{
		return;
	}

	bPvpIntroWidgetsHidden = false;
	if (ChatWidget)
	{
		ChatWidget->SetVisibility(PvpIntroChatVisibility);
	}
	if (ASnowRumbleCharacter* LocalCharacter =
		Cast<ASnowRumbleCharacter>(GetPawn()))
	{
		LocalCharacter->SetPvpIntroWidgetsHidden(false);
	}
	RestoreViewportMainHUDWidgets();
}

bool ASnowRumblePlayerController::ShouldMirrorMicrophoneInputToVoiceSpeaking()
	const
{
	return GetMicrophoneMode() == ESnowRumbleMicrophoneMode::PushToTalk;
}

void ASnowRumblePlayerController::ApplyReplicatedVoiceSpeakingState(
	bool bNewVoiceSpeaking)
{
	if (!IsLocalController())
	{
		return;
	}

	if (HasAuthority())
	{
		ServerSetVoiceSpeaking_Implementation(bNewVoiceSpeaking);
	}
	else
	{
		ServerSetVoiceSpeaking(bNewVoiceSpeaking);
	}
}

void ASnowRumblePlayerController::ApplyReplicatedVoiceChannel(
	ESnowRumbleVoiceChannel NewVoiceChannel)
{
	if (!IsLocalController())
	{
		return;
	}

	if (HasAuthority())
	{
		ServerSetVoiceChannel_Implementation(NewVoiceChannel);
	}
	else
	{
		ServerSetVoiceChannel(NewVoiceChannel);
	}
}

void ASnowRumblePlayerController::ToggleVoiceMuteMenu()
{
	if (VoiceMuteMenuWidget && VoiceMuteMenuWidget->IsInViewport())
	{
		HideVoiceMuteMenu();
		return;
	}

	ShowVoiceMuteMenu();
}

void ASnowRumblePlayerController::ShowVoiceMuteMenu()
{
	if (!IsLocalController() || !HasAttachedLocalPlayer())
	{
		return;
	}

	if (!VoiceMuteMenuWidget)
	{
		if (!VoiceMuteMenuWidgetClass)
		{
			ClientShowPersonalTextAlarm_Implementation(
				NSLOCTEXT(
					"SnowRumble",
					"VoiceMuteMenuMissing",
					"음소거 메뉴가 설정되지 않았습니다"));
			return;
		}

		VoiceMuteMenuWidget =
			CreateWidget<UVoiceMuteMenuWidget>(
				this,
				VoiceMuteMenuWidgetClass);
	}

	if (!VoiceMuteMenuWidget)
	{
		return;
	}

	VoiceMuteMenuWidget->SetVoicePlayerController(this);
	if (!VoiceMuteMenuWidget->IsInViewport())
	{
		VoiceMuteMenuWidget->AddToViewport(80);
	}
	VoiceMuteMenuWidget->RefreshPlayerList();
	ApplyVoiceMuteMenuInputState(true);
}

void ASnowRumblePlayerController::ShowVoiceChannelFeedback(
	ESnowRumbleVoiceChannel NewVoiceChannel)
{
	const FText FeedbackText = NewVoiceChannel == ESnowRumbleVoiceChannel::Team
		? NSLOCTEXT("SnowRumble", "VoiceChannelTeam", "팀으로 말하기")
		: NSLOCTEXT("SnowRumble", "VoiceChannelAll", "전체로 말하기");
	ClientShowPersonalTextAlarm_Implementation(FeedbackText);
}

void ASnowRumblePlayerController::RefreshGameplayVoiceMutes()
{
	UWorld* World = GetWorld();
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	if (!GameState)
	{
		return;
	}

	EnsureRemoteVoiceTalkersReady();

	const ASnowRumblePlayerState* LocalPlayerState =
		GetPlayerState<ASnowRumblePlayerState>();
	for (APlayerState* CandidatePlayerState : GameState->PlayerArray)
	{
		const ASnowRumblePlayerState* SnowRumblePlayerState =
			Cast<ASnowRumblePlayerState>(CandidatePlayerState);
		if (!SnowRumblePlayerState
			|| SnowRumblePlayerState == LocalPlayerState)
		{
			continue;
		}

		const FUniqueNetIdRepl& PlayerNetId =
			SnowRumblePlayerState->GetUniqueId();
		if (!PlayerNetId.IsValid())
		{
			continue;
		}

		if (ShouldHearVoiceFrom(SnowRumblePlayerState))
		{
			GameplayUnmutePlayer(PlayerNetId);
		}
		else
		{
			GameplayMutePlayer(PlayerNetId);
		}
	}
}

bool ASnowRumblePlayerController::ShouldHearVoiceFrom(
	const ASnowRumblePlayerState* SenderPlayerState) const
{
	if (!SenderPlayerState)
	{
		return true;
	}
	if (IsVoicePlayerManuallyMuted(SenderPlayerState))
	{
		return false;
	}
	if (!SenderPlayerState->IsVoiceSpeaking()
		|| SenderPlayerState->GetVoiceChannel() == ESnowRumbleVoiceChannel::All)
	{
		return true;
	}

	const ASnowRumblePlayerState* ReceiverPlayerState =
		GetPlayerState<ASnowRumblePlayerState>();
	return ReceiverPlayerState
		&& SenderPlayerState->GetLobbyTeam() != ESnowRumbleTeam::None
		&& SenderPlayerState->GetLobbyTeam()
			== ReceiverPlayerState->GetLobbyTeam();
}

FString ASnowRumblePlayerController::GetVoiceMutePlayerKey(
	const ASnowRumblePlayerState* TargetPlayerState) const
{
	if (!TargetPlayerState)
	{
		return FString();
	}

	return FString::Printf(
		TEXT("PlayerId:%d:%s"),
		TargetPlayerState->GetPlayerId(),
		*TargetPlayerState->GetLobbyPlayerName());
}

void ASnowRumblePlayerController::ApplyManualVoiceMuteState(
	const ASnowRumblePlayerState* TargetPlayerState,
	bool bShouldMute)
{
	if (!TargetPlayerState)
	{
		return;
	}

	const FUniqueNetIdRepl& PlayerNetId = TargetPlayerState->GetUniqueId();
	if (!PlayerNetId.IsValid())
	{
		return;
	}

	if (bShouldMute)
	{
		GameplayMutePlayer(PlayerNetId);
	}
	else if (ShouldHearVoiceFrom(TargetPlayerState))
	{
		GameplayUnmutePlayer(PlayerNetId);
	}
}

void ASnowRumblePlayerController::RefreshMicrophoneInputState()
{
	const UGameInstance* GameInstance = GetGameInstance();
	const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
		GameInstance
			? GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>()
			: nullptr;
	const ESnowRumbleMicrophoneMode Mode = UserSettingsSubsystem
		? UserSettingsSubsystem->GetMicrophoneMode()
		: ESnowRumbleMicrophoneMode::PushToTalk;
	const float Volume = UserSettingsSubsystem
		? UserSettingsSubsystem->GetMicrophoneVolume()
		: 1.0f;

	SetMicrophoneInputActive(Mode == ESnowRumbleMicrophoneMode::AlwaysOn);
	OnMicrophoneSettingsApplied(Mode, Volume);
}

void ASnowRumblePlayerController::UpdatePvpIntroCamera(float DeltaTime)
{
	if (!bPvpIntroCameraActive || !PvpIntroCameraActor)
	{
		return;
	}

	PvpIntroCameraElapsedSeconds =
		FMath::Min(
			PvpIntroCameraElapsedSeconds + DeltaTime,
			PvpIntroCameraDurationSeconds);

	const float TotalDuration =
		FMath::Max(0.01f, PvpIntroCameraDurationSeconds);
	const float BlendDuration =
		FMath::Clamp(
			PvpIntroCameraBlendSeconds,
			0.0f,
			TotalDuration * 0.5f);

	FTransform NewTransform;
	if (BlendDuration > 0.0f
		&& PvpIntroCameraElapsedSeconds < BlendDuration)
	{
		const float BlendAlpha =
			FMath::SmoothStep(
				0.0f,
				1.0f,
				PvpIntroCameraElapsedSeconds / BlendDuration);
		NewTransform.Blend(
			PvpIntroCameraCurrentStartTransform,
			PvpIntroCameraDollyStartTransform,
			BlendAlpha);
	}
	else
	{
		const float DollyDuration =
			FMath::Max(0.01f, TotalDuration - BlendDuration);
		const float DollyAlpha =
			FMath::Clamp(
				(PvpIntroCameraElapsedSeconds - BlendDuration)
				/ DollyDuration,
				0.0f,
				1.0f);
		const float SmoothedDollyAlpha =
			FMath::SmoothStep(0.0f, 1.0f, DollyAlpha);
		NewTransform.Blend(
			PvpIntroCameraDollyStartTransform,
			PvpIntroCameraDollyEndTransform,
			SmoothedDollyAlpha);
	}

	PvpIntroCameraActor->SetActorTransform(NewTransform);
}

void ASnowRumblePlayerController::GetPvpIntroTeamPawns(
	ESnowRumbleTeam Team,
	TArray<APawn*>& OutPawns) const
{
	OutPawns.Reset();

	const UWorld* World = GetWorld();
	const AGameStateBase* CurrentGameState =
		World ? World->GetGameState() : nullptr;
	if (!CurrentGameState || Team == ESnowRumbleTeam::None)
	{
		return;
	}

	for (APlayerState* CandidatePlayerState : CurrentGameState->PlayerArray)
	{
		const ASnowRumblePlayerState* SnowRumblePlayerState =
			Cast<ASnowRumblePlayerState>(CandidatePlayerState);
		APawn* CandidatePawn = SnowRumblePlayerState
			? SnowRumblePlayerState->GetPawn()
			: nullptr;
		if (SnowRumblePlayerState
			&& SnowRumblePlayerState->GetLobbyTeam() == Team
			&& CandidatePawn)
		{
			OutPawns.Add(CandidatePawn);
		}
	}
}

bool ASnowRumblePlayerController::BuildPvpIntroCameraTransform(
	const TArray<APawn*>& TeamPawns,
	FTransform& OutStartTransform,
	FTransform& OutEndTransform) const
{
	if (TeamPawns.IsEmpty())
	{
		return false;
	}

	FVector Center = FVector::ZeroVector;
	FVector AverageForward = FVector::ZeroVector;
	float MaxRadius = 0.0f;
	int32 ValidPawnCount = 0;

	for (const APawn* TeamPawn : TeamPawns)
	{
		if (!TeamPawn)
		{
			continue;
		}

		FVector Origin = FVector::ZeroVector;
		FVector BoxExtent = FVector::ZeroVector;
		TeamPawn->GetActorBounds(true, Origin, BoxExtent);
		Center += Origin;
		AverageForward += TeamPawn->GetActorForwardVector();
		MaxRadius = FMath::Max(MaxRadius, BoxExtent.Size2D());
		++ValidPawnCount;
	}

	if (ValidPawnCount <= 0)
	{
		return false;
	}

	Center /= static_cast<float>(ValidPawnCount);
	if (AverageForward.IsNearlyZero())
	{
		AverageForward = GetPawn()
			? GetPawn()->GetActorForwardVector()
			: FVector::ForwardVector;
	}
	AverageForward = AverageForward.GetSafeNormal2D();
	if (AverageForward.IsNearlyZero())
	{
		AverageForward = FVector::ForwardVector;
	}

	const FVector RightVector =
		FVector::CrossProduct(FVector::UpVector, AverageForward)
			.GetSafeNormal();
	const FVector LookAtLocation =
		Center + FVector::UpVector * (PvpIntroCameraHeight * 0.45f);
	const FVector BaseCameraLocation =
		Center
		- AverageForward * (PvpIntroCameraDistance + MaxRadius)
		+ RightVector * PvpIntroCameraSideOffset
		+ FVector::UpVector * PvpIntroCameraHeight;
	const FVector DollyOffset =
		RightVector * (PvpIntroCameraDollyDistance * 0.5f);
	const FVector StartCameraLocation =
		BaseCameraLocation - DollyOffset;
	const FVector EndCameraLocation =
		BaseCameraLocation + DollyOffset;
	const FRotator StartCameraRotation =
		(LookAtLocation - StartCameraLocation).Rotation();
	const FRotator EndCameraRotation =
		(LookAtLocation - EndCameraLocation).Rotation();

	OutStartTransform =
		FTransform(StartCameraRotation, StartCameraLocation);
	OutEndTransform =
		FTransform(EndCameraRotation, EndCameraLocation);
	return true;
}

FText ASnowRumblePlayerController::GetPvpIntroTeamDisplayText(
	ESnowRumbleTeam Team) const
{
	switch (Team)
	{
	case ESnowRumbleTeam::Red:
		return NSLOCTEXT("SnowRumble", "PvpIntroRedTeam", "빨간팀");
	case ESnowRumbleTeam::Sky:
		return NSLOCTEXT("SnowRumble", "PvpIntroSkyTeam", "하늘팀");
	case ESnowRumbleTeam::Green:
		return NSLOCTEXT("SnowRumble", "PvpIntroGreenTeam", "초록팀");
	case ESnowRumbleTeam::Yellow:
		return NSLOCTEXT("SnowRumble", "PvpIntroYellowTeam", "노란팀");
	case ESnowRumbleTeam::Purple:
		return NSLOCTEXT("SnowRumble", "PvpIntroPurpleTeam", "보라팀");
	case ESnowRumbleTeam::Pink:
		return NSLOCTEXT("SnowRumble", "PvpIntroPinkTeam", "분홍팀");
	case ESnowRumbleTeam::Blue:
		return NSLOCTEXT("SnowRumble", "PvpIntroBlueTeam", "파란팀");
	case ESnowRumbleTeam::Orange:
		return NSLOCTEXT("SnowRumble", "PvpIntroOrangeTeam", "주황팀");
	default:
		return NSLOCTEXT("SnowRumble", "PvpIntroUnknownTeam", "팀 소개");
	}
}

void ASnowRumblePlayerController::ServerSetVoiceSpeaking_Implementation(
	bool bNewVoiceSpeaking)
{
	if (ASnowRumblePlayerState* SnowRumblePlayerState =
		GetPlayerState<ASnowRumblePlayerState>())
	{
		SnowRumblePlayerState->SetVoiceSpeakingFromServer(
			bNewVoiceSpeaking);
	}
}

void ASnowRumblePlayerController::ServerSetVoiceChannel_Implementation(
	ESnowRumbleVoiceChannel NewVoiceChannel)
{
	if (ASnowRumblePlayerState* SnowRumblePlayerState =
		GetPlayerState<ASnowRumblePlayerState>())
	{
		SnowRumblePlayerState->SetVoiceChannelFromServer(NewVoiceChannel);
	}
}

UChatWidget* ASnowRumblePlayerController::EnsureChatWidget()
{
	if (ChatWidget)
	{
		return ChatWidget;
	}

	if (!HasAttachedLocalPlayer() || !ChatWidgetClass)
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

void ASnowRumblePlayerController::ApplyDefaultMouseCursorWidget()
{
	if (!IsLocalController() || !HasAttachedLocalPlayer())
	{
		return;
	}

	DefaultMouseCursor = EMouseCursor::Default;
	CurrentMouseCursor = EMouseCursor::Default;

	if (!DefaultMouseCursorWidgetClass)
	{
		SetMouseCursorWidget(EMouseCursor::Default, nullptr);
		return;
	}

	if (!DefaultMouseCursorWidget)
	{
		DefaultMouseCursorWidget =
			CreateWidget<UUserWidget>(this, DefaultMouseCursorWidgetClass);
	}
	if (!DefaultMouseCursorWidget)
	{
		SetMouseCursorWidget(EMouseCursor::Default, nullptr);
		return;
	}

	SetMouseCursorWidget(EMouseCursor::Default, DefaultMouseCursorWidget);
}

bool ASnowRumblePlayerController::HasAttachedLocalPlayer() const
{
	return GetLocalPlayer() != nullptr;
}

void ASnowRumblePlayerController::CollapseViewportMainHUDWidgets()
{
	TArray<UUserWidget*> MainHUDWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
		this,
		MainHUDWidgets,
		UMainHUDWidget::StaticClass(),
		false);
	for (UUserWidget* UserWidget : MainHUDWidgets)
	{
		UMainHUDWidget* MainHUDWidget = Cast<UMainHUDWidget>(UserWidget);
		if (MainHUDWidget
			&& (!MainHUDWidget->GetOwningPlayer()
				|| MainHUDWidget->GetOwningPlayer() == this))
		{
			MainHUDWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void ASnowRumblePlayerController::RestoreViewportMainHUDWidgets()
{
	TArray<UUserWidget*> MainHUDWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
		this,
		MainHUDWidgets,
		UMainHUDWidget::StaticClass(),
		false);
	for (UUserWidget* UserWidget : MainHUDWidgets)
	{
		UMainHUDWidget* MainHUDWidget = Cast<UMainHUDWidget>(UserWidget);
		if (MainHUDWidget
			&& (!MainHUDWidget->GetOwningPlayer()
				|| MainHUDWidget->GetOwningPlayer() == this))
		{
			MainHUDWidget->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void ASnowRumblePlayerController::StopBackgroundMusic()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USnowRumbleBackgroundMusicSubsystem* BackgroundMusicSubsystem =
			GameInstance->GetSubsystem<USnowRumbleBackgroundMusicSubsystem>())
		{
			BackgroundMusicSubsystem->StopBackgroundMusic();
		}
	}
}

void ASnowRumblePlayerController::PlayBackgroundMusic(
	USoundBase* BackgroundMusicSound)
{
	if (!IsLocalController())
	{
		return;
	}

	if (!BackgroundMusicSound)
	{
		StopBackgroundMusic();
		return;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USnowRumbleBackgroundMusicSubsystem* BackgroundMusicSubsystem =
			GameInstance->GetSubsystem<USnowRumbleBackgroundMusicSubsystem>())
		{
			BackgroundMusicSubsystem->PlayBackgroundMusic(BackgroundMusicSound);
		}
	}
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

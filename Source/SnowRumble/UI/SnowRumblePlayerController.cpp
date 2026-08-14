// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumblePlayerController.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "../Game/SnowRumblePlayerState.h"
#include "../Player/SnowRumbleUserSettingsSubsystem_C.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "ChatWidget_C.h"
#include "Components/InputComponent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/Pawn.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerState.h"
#include "InputCoreTypes.h"
#include "LobbyWidget.h"
#include "LoadingScreenSubsystem.h"
#include "MainHUDWidget.h"
#include "VoiceMuteMenuWidget_C.h"

void ASnowRumblePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
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
		}
	}
}

void ASnowRumblePlayerController::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	ApplyNetworkVoiceInputState(false);
	ApplyReplicatedVoiceSpeakingState(false);
	GameplayUnmuteAllPlayers();

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

	if (BoundChatInputKey.IsValid())
	{
		InputComponent->BindKey(
			BoundChatInputKey,
			IE_Pressed,
			this,
			&ASnowRumblePlayerController::HandleChatInputPressed);
	}
	if (BoundChatChannelToggleKey.IsValid()
		&& BoundChatChannelToggleKey != BoundChatInputKey)
	{
		InputComponent->BindKey(
			BoundChatChannelToggleKey,
			IE_Pressed,
			this,
			&ASnowRumblePlayerController::HandleChatChannelTogglePressed);
	}
	if (BoundMicrophonePushToTalkKey.IsValid())
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
	ApplyDefaultMouseCursorWidget();
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
	HandleVoiceChannelTogglePressed();
}

void ASnowRumblePlayerController::RequestVoiceTargetMute()
{
	ToggleVoiceMuteMenu();
	OnVoiceTargetMuteRequested();
}

void ASnowRumblePlayerController::HideVoiceMuteMenu()
{
	if (!VoiceMuteMenuWidget)
	{
		return;
	}

	VoiceMuteMenuWidget->RemoveFromParent();
	VoiceMuteMenuWidget = nullptr;

	bShowMouseCursor = false;
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
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

void ASnowRumblePlayerController::ClientPlayPvpTeamIntroShot_Implementation(
	ESnowRumbleTeam Team,
	float ShotDurationSeconds)
{
	if (!IsLocalController())
	{
		return;
	}

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

void ASnowRumblePlayerController::ClientFinishPvpTeamIntro_Implementation()
{
	if (!IsLocalController())
	{
		return;
	}

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
	ApplyNetworkVoiceInputState(bMicrophoneInputActive);
	ApplyReplicatedVoiceSpeakingState(bNewVoiceSpeaking);
	OnMicrophoneInputStateChanged(bMicrophoneInputActive);
}

void ASnowRumblePlayerController::ApplyNetworkVoiceInputState(
	bool bShouldSpeak)
{
	if (!IsLocalController() || bNetworkVoiceInputActive == bShouldSpeak)
	{
		return;
	}

	bNetworkVoiceInputActive = bShouldSpeak;
	if (bNetworkVoiceInputActive)
	{
		StartTalking();
	}
	else
	{
		StopTalking();
	}
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
	if (!IsLocalController())
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

	bShowMouseCursor = true;
	ApplyDefaultMouseCursorWidget();
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(VoiceMuteMenuWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
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
	case ESnowRumbleTeam::White:
		return NSLOCTEXT("SnowRumble", "PvpIntroWhiteTeam", "하얀팀");
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

void ASnowRumblePlayerController::ApplyDefaultMouseCursorWidget()
{
	if (!IsLocalController() || !DefaultMouseCursorWidgetClass)
	{
		return;
	}

	if (!DefaultMouseCursorWidget)
	{
		DefaultMouseCursorWidget =
			CreateWidget<UUserWidget>(this, DefaultMouseCursorWidgetClass);
	}
	if (!DefaultMouseCursorWidget)
	{
		return;
	}

	SetMouseCursorWidget(EMouseCursor::Default, DefaultMouseCursorWidget);
	DefaultMouseCursor = EMouseCursor::Default;
	CurrentMouseCursor = EMouseCursor::Default;
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

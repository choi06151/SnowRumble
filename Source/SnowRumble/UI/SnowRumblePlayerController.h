// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChatWidget_C.h"
#include "GameFramework/PlayerController.h"
#include "../Game/SnowRumblePlayerState.h"
#include "../Player/SnowRumbleUserSettingsSubsystem_C.h"
#include "SnowRumblePlayerController.generated.h"

class UChatWidget;
class ULoadingScreenWidget;
class UUserWidget;
class UVoiceMuteMenuWidget;
class ACameraActor;

UCLASS(Blueprintable)
class SNOWRUMBLE_API ASnowRumblePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** 로컬 채팅 입력창을 연다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Chat")
	void OpenChatInput(ESnowRumbleChatChannel InitialChannel);

	/** 로컬 채팅 입력창을 닫는다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Chat")
	void CloseChatInput();

	/** 로컬 플레이어가 채팅 메시지를 서버로 제출한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Chat")
	void SubmitChatMessage(
		const FString& Message,
		ESnowRumbleChatChannel Channel);

	/** 현재 모드에서 팀 채팅을 사용할 수 있는지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Chat")
	bool IsTeamChatAvailable() const;

	/** 현재 로컬 채팅 입력창이 열려 있는지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Chat")
	bool IsChatInputOpen() const;

	/** 캐릭터 게임 입력을 막아야 하는 UI가 열려 있는지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI")
	virtual bool IsGameplayUiInputOpen() const;

	/** 현재 로컬 마이크 입력 상태가 켜져 있는지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Voice")
	bool IsMicrophoneInputActive() const;

	/** 현재 로컬 마이크 방식을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Voice")
	ESnowRumbleMicrophoneMode GetMicrophoneMode() const;

	/** 현재 로컬 음성 채널을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Voice")
	ESnowRumbleVoiceChannel GetVoiceChannel() const;

	/** 눌러서 말하기 입력 시작을 로컬 음성 송출 상태로 반영한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Voice")
	void RequestMicrophonePushToTalkStarted();

	/** 눌러서 말하기 입력 종료를 로컬 음성 송출 상태로 반영한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Voice")
	void RequestMicrophonePushToTalkCompleted();

	/** 로컬 음성 송출 채널을 전체/팀으로 전환한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Voice")
	void RequestVoiceChannelToggle();

	/** 플레이어 지정 음소거 입력을 Blueprint 또는 후속 C++ 로직에 전달한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Voice")
	void RequestVoiceTargetMute();

	/** 플레이어 지정 음소거 메뉴를 닫고 게임 입력으로 복구한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Voice")
	void HideVoiceMuteMenu();

	/** 대상 플레이어의 수동 음소거 상태를 토글한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Voice")
	void ToggleManualVoiceMute(ASnowRumblePlayerState* TargetPlayerState);

	/** 대상 플레이어가 수동 음소거 상태인지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Voice")
	bool IsVoicePlayerManuallyMuted(
		const ASnowRumblePlayerState* TargetPlayerState) const;

	/** 로컬 UI 입력으로 전환하고 기본 커서 위젯을 표시한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Cursor")
	void EnableDefaultCursorUiInput(
		UUserWidget* WidgetToFocus,
		bool bUseCustomCursorWidget = true);

	/** 로컬 게임 입력으로 돌아가고 마우스 커서를 숨긴다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Cursor")
	void RestoreGameOnlyInput();

	UFUNCTION(Client, Reliable, Category = "SnowRumble|UI|Loading")
	void ClientShowLoadingScreen();

	UFUNCTION(Client, Reliable, Category = "SnowRumble|UI|Loading")
	void ClientHideLoadingScreen();

	UFUNCTION(Client, Reliable, Category = "SnowRumble|UI|Loading")
	void ClientUpdateLoadingProgress(int32 LoadedPlayers, int32 ExpectedPlayers);

	UFUNCTION(Client, Reliable, Category = "SnowRumble|UI|Event Log")
	void ClientReceiveEventLogMessage(const FText& Message);

	UFUNCTION(Client, Reliable, Category = "SnowRumble|UI|Personal Alarm")
	void ClientShowPersonalTextAlarm(const FText& Message);

	UFUNCTION(Client, Reliable, Category = "SnowRumble|Match Intro")
	void ClientPlayPvpTeamIntroShot(
		ESnowRumbleTeam Team,
		float ShotDurationSeconds);

	UFUNCTION(Client, Reliable, Category = "SnowRumble|Match Intro")
	void ClientStartPvpIntroFadeOut(float FadeOutSeconds);

	UFUNCTION(Client, Reliable, Category = "SnowRumble|Match Intro")
	void ClientFinishPvpTeamIntro();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void ClientShowLoadingScreen_Implementation();
	virtual void ClientUpdateLoadingProgress_Implementation(
		int32 LoadedPlayers,
		int32 ExpectedPlayers);
	virtual void ClientHideLoadingScreen_Implementation();
	virtual void ClientReceiveEventLogMessage_Implementation(
		const FText& Message);
	virtual void ClientShowPersonalTextAlarm_Implementation(
		const FText& Message);
	virtual void ClientPlayPvpTeamIntroShot_Implementation(
		ESnowRumbleTeam Team,
		float ShotDurationSeconds);
	virtual void ClientStartPvpIntroFadeOut_Implementation(
		float FadeOutSeconds);
	virtual void ClientFinishPvpTeamIntro_Implementation();

	/** 현재 상태에서 Enter 채팅 입력을 열 수 있는지 반환한다. */
	virtual bool CanOpenChatInput() const;

	/** 현재 모드에서 팀 채팅을 허용하는지 반환한다. */
	virtual bool SupportsTeamChat() const;

	/** 마이크 입력 상태가 바뀔 때 실제 음성 시스템 또는 WBP가 반응할 수 있다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Voice")
	void OnMicrophoneInputStateChanged(bool bIsActive);

	/** 마이크 설정이 적용될 때 실제 음성 시스템 또는 WBP가 반응할 수 있다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Voice")
	void OnMicrophoneSettingsApplied(
		ESnowRumbleMicrophoneMode Mode,
		float Volume);

	/** 플레이어 지정 음소거 입력이 들어왔을 때 대상 선택 UI가 반응할 수 있다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Voice")
	void OnVoiceTargetMuteRequested();

	/** PvP 시작 팀 소개 UI를 WBP에서 표시할 수 있는 로컬 이벤트다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Match Intro")
	void OnPvpTeamIntroShot(
		ESnowRumbleTeam Team,
		const FText& TeamDisplayText,
		float ShotDurationSeconds);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Loading")
	TSubclassOf<ULoadingScreenWidget> LoadingScreenWidgetClass;

	/** 로비, PvP, 추후 모드에서 공통으로 사용할 채팅 WBP 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Chat")
	TSubclassOf<UChatWidget> ChatWidgetClass;

	/** M 입력으로 열 플레이어 지정 음소거 WBP 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Voice Mute")
	TSubclassOf<UVoiceMuteMenuWidget> VoiceMuteMenuWidgetClass;

	/** 로비, PvP, 채팅, ESC 메뉴 등 인게임 UI에서 공통으로 사용할 기본 마우스 커서 위젯이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Cursor")
	TSubclassOf<UUserWidget> DefaultMouseCursorWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Match Intro", meta = (ClampMin = "0.0"))
	float PvpIntroCameraBlendSeconds = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Match Intro", meta = (ClampMin = "0.0"))
	float PvpIntroCameraReturnBlendSeconds = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Match Intro", meta = (ClampMin = "0.0"))
	float PvpIntroCameraDistance = 650.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Match Intro", meta = (ClampMin = "0.0"))
	float PvpIntroCameraHeight = 260.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Match Intro", meta = (ClampMin = "0.0"))
	float PvpIntroCameraSideOffset = 420.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Match Intro", meta = (ClampMin = "0.0"))
	float PvpIntroCameraDollyDistance = 360.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Match Intro", meta = (ClampMin = "1.0"))
	float PvpIntroCinematicAspectRatio = 2.39f;

	/** 기본 마우스 커서 위젯 슬롯을 소프트웨어 커서로 적용한다. */
	void ApplyDefaultMouseCursorWidget();

private:
	/** 로컬 옵션 설정 기준으로 채팅 직접 키 바인딩을 다시 묶는다. */
	void RebindConfiguredInputKeys();

	/** 사용자 설정 변경 시 로컬 직접 키 바인딩을 갱신한다. */
	void HandleUserKeyBindingsChanged();

	/** 마이크 설정 변경 시 입력 상태와 Blueprint 알림을 갱신한다. */
	void HandleMicrophoneSettingsChanged();

	UFUNCTION(Server, Reliable)
	void ServerSubmitChatMessage(
		const FString& Message,
		ESnowRumbleChatChannel Channel);

	UFUNCTION(Client, Reliable)
	void ClientReceiveChatMessage(
		ESnowRumbleChatChannel Channel,
		const FString& SenderName,
		const FString& Message);

	UFUNCTION(Server, Reliable)
	void ServerSetVoiceSpeaking(bool bNewVoiceSpeaking);

	UFUNCTION(Server, Reliable)
	void ServerSetVoiceChannel(ESnowRumbleVoiceChannel NewVoiceChannel);

	/** Enter 입력으로 채팅 입력창을 연다. */
	void HandleChatInputPressed();

	/** 채팅 입력이 열려 있을 때 전체/팀 채널을 전환한다. */
	void HandleChatChannelTogglePressed();

	/** M 입력으로 음성 송출 채널을 전체/팀으로 전환한다. */
	void HandleVoiceChannelTogglePressed();

	/** 눌러서 말하기 마이크 입력을 시작한다. */
	void HandleMicrophonePushToTalkPressed();

	/** 눌러서 말하기 마이크 입력을 종료한다. */
	void HandleMicrophonePushToTalkReleased();

	/** 마이크 입력 상태를 변경하고 Blueprint에 알린다. */
	void SetMicrophoneInputActive(bool bNewActive);

	/** 엔진 네트워크 음성 송출 시작/중지를 실제로 호출한다. */
	void ApplyNetworkVoiceInputState(bool bShouldSpeak);

	/** 현재 마이크 입력 상태를 speaking 표시로 그대로 쓸 수 있는지 반환한다. */
	bool ShouldMirrorMicrophoneInputToVoiceSpeaking() const;

	/** 로컬 음성 송출 상태를 서버 복제 PlayerState에 반영한다. */
	void ApplyReplicatedVoiceSpeakingState(bool bNewVoiceSpeaking);

	/** 로컬 음성 채널을 서버 복제 PlayerState에 반영한다. */
	void ApplyReplicatedVoiceChannel(ESnowRumbleVoiceChannel NewVoiceChannel);

	/** 플레이어 지정 음소거 메뉴를 열거나 이미 열려 있으면 닫는다. */
	void ToggleVoiceMuteMenu();

	/** 플레이어 지정 음소거 메뉴를 생성하고 화면에 표시한다. */
	void ShowVoiceMuteMenu();

	/** 현재 음성 채널 상태를 로컬 personal alarm으로 알린다. */
	void ShowVoiceChannelFeedback(ESnowRumbleVoiceChannel NewVoiceChannel);

	/** 팀 음성 채널로 말하는 다른 팀 플레이어를 로컬에서 gameplay mute 처리한다. */
	void RefreshGameplayVoiceMutes();

	/** 로컬 플레이어가 특정 송신자의 음성을 들을 수 있는지 반환한다. */
	bool ShouldHearVoiceFrom(
		const ASnowRumblePlayerState* SenderPlayerState) const;

	/** 수동 음소거 목록에 쓸 안정적인 로컬 키를 만든다. */
	FString GetVoiceMutePlayerKey(
		const ASnowRumblePlayerState* TargetPlayerState) const;

	/** 수동 음소거 목록에 맞춰 실제 엔진 gameplay mute 상태를 반영한다. */
	void ApplyManualVoiceMuteState(
		const ASnowRumblePlayerState* TargetPlayerState,
		bool bShouldMute);

	/** 현재 저장된 마이크 설정에 맞춰 로컬 입력 상태를 갱신한다. */
	void RefreshMicrophoneInputState();

	/** PvP 팀 소개용 임시 카메라 이동을 갱신한다. */
	void UpdatePvpIntroCamera(float DeltaTime);

	/** 현재 로컬 월드에 존재하는 특정 팀 Pawn들을 찾는다. */
	void GetPvpIntroTeamPawns(
		ESnowRumbleTeam Team,
		TArray<APawn*>& OutPawns) const;

	/** 팀 Pawn bounds 기준으로 시작 소개 카메라 transform을 계산한다. */
	bool BuildPvpIntroCameraTransform(
		const TArray<APawn*>& TeamPawns,
		FTransform& OutStartTransform,
		FTransform& OutEndTransform) const;

	FText GetPvpIntroTeamDisplayText(ESnowRumbleTeam Team) const;

	/** 채팅 위젯 인스턴스가 없으면 생성한다. */
	UChatWidget* EnsureChatWidget();

	/** 서버가 채팅을 받을 클라이언트인지 확인한다. */
	bool ShouldReceiveChatMessage(
		const APlayerState* SenderPlayerState,
		ESnowRumbleChatChannel Channel) const;

	/** 채팅 표시 이름을 반환한다. */
	FString GetChatSenderName() const;

	UPROPERTY(Transient)
	TObjectPtr<UChatWidget> ChatWidget;

	UPROPERTY(Transient)
	TObjectPtr<UVoiceMuteMenuWidget> VoiceMuteMenuWidget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> DefaultMouseCursorWidget;

	UPROPERTY(Transient)
	TObjectPtr<ACameraActor> PvpIntroCameraActor;

	bool bChatInputIgnoringPawnInput = false;

	FTransform PvpIntroCameraCurrentStartTransform;
	FTransform PvpIntroCameraDollyStartTransform;
	FTransform PvpIntroCameraDollyEndTransform;
	float PvpIntroCameraElapsedSeconds = 0.0f;
	float PvpIntroCameraDurationSeconds = 0.0f;
	bool bPvpIntroCameraActive = false;
	FTimerHandle PvpIntroCameraDestroyTimerHandle;

	FKey BoundChatInputKey = EKeys::Invalid;
	FKey BoundChatChannelToggleKey = EKeys::Invalid;
	FKey BoundMicrophonePushToTalkKey = EKeys::Invalid;
	FKey BoundVoiceChannelToggleKey = EKeys::Invalid;
	FKey BoundVoiceTargetMuteKey = EKeys::Invalid;

	bool bMicrophoneInputActive = false;

	bool bNetworkVoiceInputActive = false;

	ESnowRumbleVoiceChannel LocalVoiceChannel =
		ESnowRumbleVoiceChannel::All;

	TSet<FString> ManuallyMutedVoicePlayerKeys;
};

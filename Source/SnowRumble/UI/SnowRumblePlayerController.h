// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChatWidget_C.h"
#include "GameFramework/PlayerController.h"
#include "SnowRumblePlayerController.generated.h"

class UChatWidget;
class ULoadingScreenWidget;

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

	UFUNCTION(Client, Reliable, Category = "SnowRumble|UI|Loading")
	void ClientShowLoadingScreen();

	UFUNCTION(Client, Reliable, Category = "SnowRumble|UI|Loading")
	void ClientHideLoadingScreen();

	UFUNCTION(Client, Reliable, Category = "SnowRumble|UI|Loading")
	void ClientUpdateLoadingProgress(int32 LoadedPlayers, int32 ExpectedPlayers);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void ClientShowLoadingScreen_Implementation();
	virtual void ClientUpdateLoadingProgress_Implementation(
		int32 LoadedPlayers,
		int32 ExpectedPlayers);
	virtual void ClientHideLoadingScreen_Implementation();

	/** 현재 상태에서 Enter 채팅 입력을 열 수 있는지 반환한다. */
	virtual bool CanOpenChatInput() const;

	/** 현재 모드에서 팀 채팅을 허용하는지 반환한다. */
	virtual bool SupportsTeamChat() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Loading")
	TSubclassOf<ULoadingScreenWidget> LoadingScreenWidgetClass;

	/** 로비, PvP, 추후 모드에서 공통으로 사용할 채팅 WBP 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Chat")
	TSubclassOf<UChatWidget> ChatWidgetClass;

private:
	UFUNCTION(Server, Reliable)
	void ServerSubmitChatMessage(
		const FString& Message,
		ESnowRumbleChatChannel Channel);

	UFUNCTION(Client, Reliable)
	void ClientReceiveChatMessage(
		ESnowRumbleChatChannel Channel,
		const FString& SenderName,
		const FString& Message);

	/** Enter 입력으로 채팅 입력창을 연다. */
	void HandleChatInputPressed();

	/** 채팅 입력이 열려 있을 때 전체/팀 채널을 전환한다. */
	void HandleChatChannelTogglePressed();

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
};

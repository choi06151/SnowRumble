// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Fonts/SlateFontInfo.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "ChatWidget_C.generated.h"

class ASnowRumblePlayerController;
class UBorder;
class UEditableTextBox;
class UScrollBox;
class UTextBlock;
class UWidget;

UENUM(BlueprintType)
enum class ESnowRumbleChatChannel : uint8
{
	All,
	Team
};

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API UChatWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UChatWidget(const FObjectInitializer& ObjectInitializer);

	/** 채팅 위젯이 메시지 요청을 보낼 PlayerController를 연결한다. */
	void SetChatPlayerController(ASnowRumblePlayerController* NewPlayerController);

	/** 입력창을 열고 키보드 포커스를 준다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Chat")
	void OpenChatInput(ESnowRumbleChatChannel InitialChannel);

	/** 입력창을 닫고 현재 입력값을 비운다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Chat")
	void CloseChatInput();

	UWidget* GetChatInputFocusWidget() const;

	void FocusChatInputTextBox();

	/** 현재 입력창이 열려 있는지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Chat")
	bool IsChatInputOpen() const;

	/** 새 채팅 메시지를 로그에 추가한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Chat")
	void AddChatMessage(
		ESnowRumbleChatChannel Channel,
		const FString& SenderName,
		const FString& Message);

	/** 현재 채팅 채널을 바꾼다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Chat")
	void SetActiveChatChannel(ESnowRumbleChatChannel NewChannel);

	/** 현재 채팅 채널을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Chat")
	ESnowRumbleChatChannel GetActiveChatChannel() const;

	/** 팀 채팅이 가능한 상태면 전체/팀 채널을 전환한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Chat")
	void ToggleChatChannel();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(
		const FGeometry& MyGeometry,
		float InDeltaTime) override;
	virtual FReply NativeOnPreviewKeyDown(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnKeyDown(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseWheel(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	/** WBP에서 같은 이름으로 만든 메시지 목록 ScrollBox다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Chat")
	TObjectPtr<UScrollBox> ChatLogScrollBox;

	/** WBP에서 같은 이름으로 만든 채팅 로그 장식용 Border다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Chat")
	TObjectPtr<UBorder> ChatLogBorder;

	/** WBP에서 같은 이름으로 만든 기존 호환용 채팅 로그 TextBlock이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Chat")
	TObjectPtr<UTextBlock> ChatLogText;

	/** WBP에서 같은 이름으로 만든 채팅 입력창이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Chat")
	TObjectPtr<UEditableTextBox> ChatInputTextBox;

	/** WBP에서 같은 이름으로 만든 현재 채널 표시 TextBlock이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Chat")
	TObjectPtr<UTextBlock> ChatChannelText;

	/** C++가 새로 생성하는 채팅 메시지 TextBlock 행에 적용할 폰트다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Chat")
	FSlateFontInfo ChatMessageFont;

	/** C++가 새로 생성하는 전체 채팅 메시지 행에 적용할 색이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Chat")
	FLinearColor AllChatMessageColor = FLinearColor::White;

	/** C++가 새로 생성하는 팀 채팅 메시지 행에 적용할 색이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Chat")
	FLinearColor TeamChatMessageColor = FLinearColor(0.35f, 0.85f, 1.0f, 1.0f);

	/** 채널 표시 TextBlock에 적용할 폰트다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Chat")
	FSlateFontInfo ChatChannelFont;

	/** 마지막 채팅 갱신 후 채팅 로그가 완전히 보이는 시간이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Chat", meta = (ClampMin = "0.0"))
	float ChatVisibleDuration = 5.0f;

	/** 채팅 로그가 사라지는 데 걸리는 시간이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Chat", meta = (ClampMin = "0.05"))
	float ChatFadeOutDuration = 1.5f;

	/** 새 메시지를 Blueprint가 별도 행 UI로 표시할 때 사용한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Chat")
	void OnChatMessageAdded(
		ESnowRumbleChatChannel Channel,
		const FString& SenderName,
		const FString& Message,
		const FText& DisplayText);

	/** 입력창이 열리거나 닫힐 때 Blueprint 표현을 갱신한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Chat")
	void OnChatInputOpenChanged(bool bOpen);

private:
	UFUNCTION()
	void HandleChatTextCommitted(
		const FText& Text,
		ETextCommit::Type CommitMethod);

	/** 현재 채널 표시 텍스트를 갱신한다. */
	void RefreshChannelText();

	/** 채팅 WBP 안의 고정 TextBlock 폰트를 갱신한다. */
	void ApplyConfiguredFonts();

	/** 입력 상태에 따라 로그 스크롤바와 장식 테두리 표시를 갱신한다. */
	void RefreshChatLogChrome();

	/** 현재 시간 기준으로 채팅 로그 투명도를 갱신한다. */
	void RefreshChatVisibility();

	/** 채팅 로그를 즉시 다시 보이게 하고 fade 기준 시간을 갱신한다. */
	void ShowChatLog();

	/** ScrollBox에 메시지 TextBlock 행을 추가한다. */
	void AddMessageTextRow(
		ESnowRumbleChatChannel Channel,
		const FText& DisplayText);

	/** 채팅 로그 TextBlock에 누적 표시할 문자열이다. */
	FString ChatLog;

	UPROPERTY(Transient)
	TObjectPtr<ASnowRumblePlayerController> ChatPlayerController;

	FLinearColor OriginalChatLogBorderBrushColor = FLinearColor::Transparent;

	float LastChatVisibilityRefreshTime = 0.0f;
	uint64 LastChatChannelToggleFrameNumber = 0;
	bool bChatChannelToggleKeyWasDown = false;

	ESnowRumbleChatChannel ActiveChatChannel = ESnowRumbleChatChannel::All;
	bool bChatInputOpen = false;
};

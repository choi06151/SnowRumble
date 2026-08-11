// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VoiceMutePlayerRowWidget_C.generated.h"

class ASnowRumblePlayerController;
class ASnowRumblePlayerState;
class UButton;
class UTextBlock;

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API UVoiceMutePlayerRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 이 행이 표시할 플레이어와 로컬 컨트롤러를 설정한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Voice Mute")
	void SetVoiceMutePlayer(
		ASnowRumblePlayerState* NewPlayerState,
		ASnowRumblePlayerController* NewPlayerController);

	/** 현재 표시 중인 플레이어 상태를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Voice Mute")
	ASnowRumblePlayerState* GetObservedPlayerState() const;

	/** 현재 mute 상태에 맞춰 텍스트와 버튼 표시를 갱신한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Voice Mute")
	void RefreshRow();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 행 데이터가 바뀌면 Blueprint가 추가 표시를 갱신할 수 있다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Voice Mute")
	void OnVoiceMutePlayerRowChanged(
		ASnowRumblePlayerState* NewPlayerState,
		bool bIsMuted);

	/** WBP 행 안 플레이어 이름 TextBlock에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Voice Mute")
	TObjectPtr<UTextBlock> PlayerNameText;

	/** WBP 행 안 mute 토글 Button에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Voice Mute")
	TObjectPtr<UButton> MuteButton;

	/** WBP 행 안 mute 버튼 라벨 TextBlock에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Voice Mute")
	TObjectPtr<UTextBlock> MuteButtonText;

private:
	UFUNCTION()
	void HandleMuteButtonClicked();

	void BindRowButton();
	void UnbindRowButton();

	UPROPERTY(Transient)
	TObjectPtr<ASnowRumblePlayerState> ObservedPlayerState;

	UPROPERTY(Transient)
	TObjectPtr<ASnowRumblePlayerController> VoicePlayerController;
};

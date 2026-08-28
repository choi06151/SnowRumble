// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SnowRumbleAudioUserWidget.h"
#include "VoiceMuteMenuWidget_C.generated.h"

class ASnowRumblePlayerController;
class ASnowRumblePlayerState;
class UButton;
class UPanelWidget;
class UVoiceMutePlayerRowWidget;

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API UVoiceMuteMenuWidget : public USnowRumbleAudioUserWidget
{
	GENERATED_BODY()

public:
	/** 이 메뉴를 소유한 로컬 플레이어 컨트롤러를 설정한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Voice Mute")
	void SetVoicePlayerController(
		ASnowRumblePlayerController* NewPlayerController);

	/** 현재 인게임 플레이어 목록 기준으로 mute 행을 다시 만든다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Voice Mute")
	void RefreshPlayerList();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(
		const FGeometry& MyGeometry,
		float InDeltaTime) override;

	/** 플레이어 목록이 갱신되면 Blueprint가 추가 표시를 갱신할 수 있다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Voice Mute")
	void OnVoiceMutePlayerListRefreshed(int32 PlayerCount);

	/** WBP에서 플레이어 행들이 들어갈 패널에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Voice Mute")
	TObjectPtr<UPanelWidget> PlayerListBox;

	/** WBP에서 메뉴 닫기 버튼이 있으면 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Voice Mute")
	TObjectPtr<UButton> CloseButton;

	/** 플레이어마다 동적으로 만들 행 WBP 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Voice Mute")
	TSubclassOf<UVoiceMutePlayerRowWidget> PlayerRowWidgetClass;

private:
	UFUNCTION()
	void HandleCloseButtonClicked();

	void BindMenuButtons();
	void UnbindMenuButtons();
	FString GetPlayerStateListSignature() const;

	UPROPERTY(Transient)
	TObjectPtr<ASnowRumblePlayerController> VoicePlayerController;

	FString CachedPlayerListSignature;
};

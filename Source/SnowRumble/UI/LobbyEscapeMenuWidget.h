// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SnowRumbleAudioUserWidget.h"
#include "Styling/SlateTypes.h"
#include "LobbyEscapeMenuWidget.generated.h"

class ALobbyPlayerController;
class UButton;
class UTextBlock;

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API ULobbyEscapeMenuWidget : public USnowRumbleAudioUserWidget
{
	GENERATED_BODY()

public:
	/** 이 메뉴를 소유한 로비 PlayerController를 지정한다. */
	void SetLobbyPlayerController(ALobbyPlayerController* NewPlayerController);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(
		const FGeometry& MyGeometry,
		float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent) override;

	/** 메인메뉴로 이동하기 버튼 클릭을 Blueprint가 필요하면 처리한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Lobby Escape")
	void OnReturnToMainMenuRequested();

	/** 설정 버튼 클릭을 Blueprint가 필요하면 처리한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Lobby Escape")
	void OnSettingsRequested();

	/** 친구 부르기 버튼 클릭을 Blueprint가 필요하면 처리한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Lobby Escape")
	void OnInviteFriendsRequested();

	/** WBP에서 같은 이름으로 만든 메인메뉴로 이동하기 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Lobby Escape")
	TObjectPtr<UButton> ReturnToMainMenuButton;

	/** 기존 WBP 이름 호환용 메인메뉴 이동 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Lobby Escape")
	TObjectPtr<UButton> ReturnToLobbyButton;

	/** WBP에서 같은 이름으로 만든 설정 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Lobby Escape")
	TObjectPtr<UButton> SettingsButton;

	/** WBP에서 같은 이름으로 만든 친구 부르기 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Lobby Escape")
	TObjectPtr<UButton> InviteFriendsButton;

	/** WBP에서 같은 이름으로 만든 돌아가기 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Lobby Escape")
	TObjectPtr<UButton> BackButton;

private:
	UFUNCTION()
	void HandleReturnToMainMenuButtonClicked();

	UFUNCTION()
	void HandleSettingsButtonClicked();

	UFUNCTION()
	void HandleInviteFriendsButtonClicked();

	UFUNCTION()
	void HandleBackButtonClicked();

	void BindMenuButtons();
	void UnbindMenuButtons();
	void BindTargetButtonTextColor(UButton* Button);
	void UnbindTargetButtonTextColor(UButton* Button);

	UFUNCTION()
	void RefreshTargetButtonTextColors();

	void CloseMenu();

	UPROPERTY(Transient)
	TObjectPtr<ALobbyPlayerController> LobbyPlayerController;

	TMap<UTextBlock*, FSlateColor> TargetButtonTextDefaultColors;

};

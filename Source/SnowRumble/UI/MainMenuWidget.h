// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Online/SnowRumbleSessionSubsystem.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;
class UWidget;
class UWidgetAnimation;

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 시작화면에서 LAN 호스트 생성을 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Main Menu")
	void HostLanGame(int32 MaxPlayers = 8, const FString& RoomName = FString());

	/** 시작화면에서 LAN 세션 검색을 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Main Menu")
	void FindLanGames();

	/** 빈자리가 있는 첫 번째 LAN 방에 자동 참가한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Main Menu")
	void QuickJoinLanGame();

	/** 검색 결과 인덱스로 LAN 세션 참가를 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Main Menu")
	void JoinLanGame(int32 ResultIndex);

	/** 입력한 방 코드와 일치하는 LAN 방에 참가한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Main Menu")
	void JoinLanGameByRoomCode(const FString& RoomCode);

	/** 마지막 LAN 세션 검색 결과를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Main Menu")
	const TArray<FSnowRumbleSessionInfo>& GetLanSearchResults() const;

	/** 현재 세션 작업 상태를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Main Menu")
	ESnowRumbleSessionState GetSessionState() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(
		const FGeometry& MyGeometry,
		float InDeltaTime) override;

	/** 있으면 자동으로 HostLanGame(8)에 연결되는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Main Menu")
	TObjectPtr<UButton> HostButton;

	/** 있으면 빈자리가 있는 LAN 방에 자동 참가하는 빠른 참여 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Main Menu")
	TObjectPtr<UButton> QuickJoinButton;

	/** 있으면 방 코드 입력 UI를 표시하는 방 찾기 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Main Menu")
	TObjectPtr<UButton> FindButton;

	/** 있으면 공통 옵션 메뉴를 여는 설정 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Main Menu")
	TObjectPtr<UButton> SettingsButton;

	/** 있으면 커스터마이징 레벨로 이동하는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Main Menu")
	TObjectPtr<UButton> CustomizationButton;

	/** 있으면 참가하기 버튼을 눌렀을 때 표시되는 방 코드 입력 패널이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Main Menu")
	TObjectPtr<UWidget> RoomCodeJoinPanel;

	/** 있으면 방 코드 참가 확인 시 값을 읽는 입력창이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Main Menu")
	TObjectPtr<UEditableTextBox> RoomCodeTextBox;

	/** 있으면 Host, 빠른 참여, 방 코드 참가 전에 로컬 닉네임으로 저장하는 입력창이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Main Menu")
	TObjectPtr<UEditableTextBox> PlayerNameTextBox;

	/** 있으면 입력한 방 코드로 참가를 요청하는 확인 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Main Menu")
	TObjectPtr<UButton> ConfirmRoomCodeJoinButton;

	/** 있으면 방 코드 입력 패널을 닫는 취소 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Main Menu")
	TObjectPtr<UButton> CancelRoomCodeJoinButton;

	/** 있으면 세션 상태와 검색 결과 개수를 자동 표시하는 임시 상태 텍스트다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Main Menu")
	TObjectPtr<UTextBlock> StatusTextBlock;

	/** 있으면 메인메뉴 진입 알림을 자동 표시하는 전용 TextBlock이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Main Menu")
	TObjectPtr<UTextBlock> MainMenuAlarmText;

	/** MainMenuAlarmText 대신 AlarmText 이름을 쓴 WBP용 호환 바인딩이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Main Menu")
	TObjectPtr<UTextBlock> AlarmText;

	/** WBP에 같은 이름으로 만든 메인메뉴 진입 알림 애니메이션이다. */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> MainMenuAlarmAnimation;

	/** MainMenuAlarmAnimation 대신 AlarmAnimation 이름을 쓴 WBP용 호환 바인딩이다. */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> AlarmAnimation;

	/** 세션 작업 상태 변경을 Blueprint UI에 전달한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Main Menu")
	void OnMainMenuSessionStateChanged(
		ESnowRumbleSessionOperation Operation,
		ESnowRumbleSessionState State,
		const FString& Message);

	/** 세션 검색 결과 변경을 Blueprint UI에 전달한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Main Menu")
	void OnMainMenuSearchCompleted(
		const TArray<FSnowRumbleSessionInfo>& Results);

	/** 방 코드 입력 패널이 열렸을 때 WBP가 추가 연출을 연결할 수 있다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Main Menu")
	void OnRoomCodeJoinPromptRequested();

	/** 메인메뉴 진입 알림이 표시될 때 WBP가 추가 연출을 연결할 수 있다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Main Menu")
	void OnMainMenuAlarmRequested(const FText& Message);

private:
	USnowRumbleSessionSubsystem* GetSessionSubsystem() const;

	UFUNCTION()
	void HandleSessionStateChanged(
		ESnowRumbleSessionOperation Operation,
		ESnowRumbleSessionState State,
		const FString& Message);

	UFUNCTION()
	void HandleSearchCompleted(const TArray<FSnowRumbleSessionInfo>& Results);

	UFUNCTION()
	void HandleHostButtonClicked();

	UFUNCTION()
	void HandleQuickJoinButtonClicked();

	UFUNCTION()
	void HandleFindButtonClicked();

	UFUNCTION()
	void HandleSettingsButtonClicked();

	UFUNCTION()
	void HandleCustomizationButtonClicked();

	UFUNCTION()
	void HandleConfirmRoomCodeJoinClicked();

	UFUNCTION()
	void HandleCancelRoomCodeJoinClicked();

	UFUNCTION()
	void HandlePlayerNameTextCommitted(
		const FText& Text,
		ETextCommit::Type CommitMethod);

	void BindMenuButtons();
	void UnbindMenuButtons();
	void SetRoomCodeJoinPanelVisible(bool bVisible);
	void InitializePlayerNameInput();
	void SavePlayerNameInput();
	bool ValidateAndSavePlayerNameInput();
	void RestorePlayerNameInput();
	void SetStatusMessage(const FString& Message);
	void ConsumePendingMainMenuAlarm();
	void ShowMainMenuAlarm(const FText& Message);
	FText GetSessionProgressAlarmText(
		ESnowRumbleSessionOperation Operation) const;
	void RefreshJoinButtonEnabled();

	static const TArray<FSnowRumbleSessionInfo> EmptyResults;
};

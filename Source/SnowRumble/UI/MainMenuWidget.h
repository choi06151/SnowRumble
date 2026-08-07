// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Online/SnowRumbleSessionSubsystem.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 시작화면에서 LAN 호스트 생성을 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Main Menu")
	void HostLanGame(int32 MaxPlayers = 16);

	/** 시작화면에서 LAN 세션 검색을 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Main Menu")
	void FindLanGames();

	/** 검색 결과 인덱스로 LAN 세션 참가를 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Main Menu")
	void JoinLanGame(int32 ResultIndex);

	/** 마지막 LAN 세션 검색 결과를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Main Menu")
	const TArray<FSnowRumbleSessionInfo>& GetLanSearchResults() const;

	/** 현재 세션 작업 상태를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Main Menu")
	ESnowRumbleSessionState GetSessionState() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 있으면 자동으로 HostLanGame(16)에 연결되는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Main Menu")
	TObjectPtr<UButton> HostButton;

	/** 있으면 자동으로 FindLanGames에 연결되는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Main Menu")
	TObjectPtr<UButton> FindButton;

	/** 있으면 자동으로 JoinLanGame(0)에 연결되는 임시 Join 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Main Menu")
	TObjectPtr<UButton> JoinFirstButton;

	/** 있으면 세션 상태와 검색 결과 개수를 자동 표시하는 임시 상태 텍스트다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Main Menu")
	TObjectPtr<UTextBlock> StatusTextBlock;

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
	void HandleFindButtonClicked();

	UFUNCTION()
	void HandleJoinFirstButtonClicked();

	void BindMenuButtons();
	void UnbindMenuButtons();
	void SetStatusMessage(const FString& Message);
	void RefreshJoinButtonEnabled();

	static const TArray<FSnowRumbleSessionInfo> EmptyResults;
};

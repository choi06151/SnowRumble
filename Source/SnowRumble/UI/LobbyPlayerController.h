// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../Game/SnowRumblePlayerState.h"
#include "SnowRumblePlayerController.h"
#include "LobbyPlayerController.generated.h"

class ULobbyEscapeMenuWidget;
class ULobbyWidget;
class UOptionsWidget;

UCLASS(Blueprintable)
class SNOWRUMBLE_API ALobbyPlayerController : public ASnowRumblePlayerController
{
	GENERATED_BODY()

public:
	/** 대기방 위젯을 생성하고 기본 게임 입력 모드로 전환한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void ShowLobby();

	/** 대기방 위젯을 제거하고 게임 입력 모드로 되돌린다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void HideLobby();

	/** 로비 설정 UI를 조작할 때 커서와 UI 입력을 켠다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void EnableLobbyUiInput();

	/** 로비 설정 UI 조작을 끝내고 플레이 입력으로 돌아간다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void EnableLobbyGameInput();

	/** 소유 클라이언트가 로비 닉네임을 서버 PlayerState에 적용하도록 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void RequestApplyLobbyPlayerName(const FString& NewName);

	/** 소유 클라이언트가 로비 팀 색을 서버 PlayerState에 적용하도록 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void RequestApplyLobbyTeam(ESnowRumbleTeam NewTeam);

	/** 소유 클라이언트가 저장된 커스터마이징 데이터를 서버 PlayerState에 적용하도록 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void RequestApplyCustomizationData(
		const FSnowRumbleCustomizationData& NewData);

	/** 로비 UI에 예외행동 사유를 표시하고 피드백 애니메이션을 재생한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void ShowLobbyInvalidActionFeedback(const FText& ReasonText);

	/** 로비 ESC 메뉴를 열고 마우스 UI 입력으로 전환한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void ShowLobbyEscapeMenu();

	/** 로비 ESC 메뉴를 닫고 로비 게임 입력으로 되돌린다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void HideLobbyEscapeMenu();

	/** 로비 ESC 메뉴에서 메인메뉴 맵으로 이동한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Lobby")
	void RequestReturnToMainMenu();

	/** 로비에서 공통 옵션 위젯을 연다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Options")
	void ShowOptionsMenu();

	/** 로비에서 공통 옵션 위젯을 닫고 ESC 메뉴 입력으로 돌아간다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Options")
	void HideOptionsMenu();

	/** 서버가 로비 입장 완료 후 소유 클라이언트의 저장 닉네임 제출을 요청한다. */
	UFUNCTION(Client, Reliable)
	void ClientRequestApplySavedLobbyPlayerName();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void ClientShowLoadingScreen_Implementation() override;
	virtual bool CanOpenChatInput() const override;
	virtual bool IsGameplayUiInputOpen() const override;
	virtual bool SupportsTeamChat() const override;

	/** 대기방에서 자동 생성할 WBP_Lobby 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Lobby")
	TSubclassOf<ULobbyWidget> LobbyWidgetClass;

	/** ESC를 눌렀을 때 자동 생성할 로비 전용 메뉴 WBP 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Lobby")
	TSubclassOf<ULobbyEscapeMenuWidget> LobbyEscapeMenuWidgetClass;

	/** 메인메뉴와 로비에서 공통으로 사용할 옵션 WBP 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Options")
	TSubclassOf<UOptionsWidget> OptionsWidgetClass;

	/** ESC 메뉴에서 메인메뉴로 이동할 때 사용할 travel URL이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Lobby")
	FString MainMenuTravelUrl = TEXT("/Game/Maps/L_MainMenu");

private:
	/** 로컬 GameInstance에 저장된 닉네임을 서버로 제출한다. */
	void ApplySavedLobbyPlayerName();

	/** ESC 키 입력으로 로비 메뉴를 토글한다. */
	void HandleEscapePressed();

	UFUNCTION(Server, Reliable)
	void ServerApplyLobbyPlayerName(const FString& NewName);

	UFUNCTION(Server, Reliable)
	void ServerApplyLobbyTeam(ESnowRumbleTeam NewTeam);

	UFUNCTION(Server, Reliable)
	void ServerApplyCustomizationData(
		const FSnowRumbleCustomizationData& NewData);

	/** 대기방 위젯 인스턴스가 없으면 생성한다. */
	ULobbyWidget* EnsureLobbyWidget();

	/** ESC 메뉴 위젯 인스턴스가 없으면 생성한다. */
	ULobbyEscapeMenuWidget* EnsureLobbyEscapeMenuWidget();

	/** 옵션 위젯 인스턴스가 없으면 생성한다. */
	UOptionsWidget* EnsureOptionsWidget();

	UPROPERTY(Transient)
	TObjectPtr<ULobbyWidget> LobbyWidget;

	UPROPERTY(Transient)
	TObjectPtr<ULobbyEscapeMenuWidget> LobbyEscapeMenuWidget;

	UPROPERTY(Transient)
	TObjectPtr<UOptionsWidget> OptionsWidget;
};

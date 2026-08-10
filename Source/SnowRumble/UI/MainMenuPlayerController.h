// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainMenuPlayerController.generated.h"

class UMainMenuWidget;
class UOptionsWidget;

UCLASS(Blueprintable)
class SNOWRUMBLE_API AMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** 시작화면 위젯을 생성하고 UI 입력 모드로 전환한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Main Menu")
	void ShowMainMenu();

	/** 시작화면 위젯을 제거하고 게임 입력 모드로 되돌린다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Main Menu")
	void HideMainMenu();

	/** 공통 옵션 위젯을 열고 UI 입력을 옵션에 맞춘다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Options")
	void ShowOptionsMenu();

	/** 공통 옵션 위젯을 닫고 메인메뉴 입력으로 돌아간다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Options")
	void HideOptionsMenu();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 시작화면에서 자동 생성할 WBP_MainMenu 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Main Menu")
	TSubclassOf<UMainMenuWidget> MainMenuWidgetClass;

	/** 메인메뉴와 로비에서 공통으로 사용할 옵션 WBP 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Options")
	TSubclassOf<UOptionsWidget> OptionsWidgetClass;

private:
	/** 시작화면 위젯 인스턴스가 없으면 생성한다. */
	UMainMenuWidget* EnsureMainMenuWidget();

	/** 옵션 위젯 인스턴스가 없으면 생성한다. */
	UOptionsWidget* EnsureOptionsWidget();

	UPROPERTY(Transient)
	TObjectPtr<UMainMenuWidget> MainMenuWidget;

	UPROPERTY(Transient)
	TObjectPtr<UOptionsWidget> OptionsWidget;
};

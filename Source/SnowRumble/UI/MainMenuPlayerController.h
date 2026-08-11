// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainMenuPlayerController.generated.h"

class UMainMenuWidget;
class UOptionsWidget;
class UUserWidget;

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

	/** 메인메뉴에서 커스터마이징 레벨로 이동한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void TravelToCustomizationLevel();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 시작화면에서 자동 생성할 WBP_MainMenu 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Main Menu")
	TSubclassOf<UMainMenuWidget> MainMenuWidgetClass;

	/** 메인메뉴와 로비에서 공통으로 사용할 옵션 WBP 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Options")
	TSubclassOf<UOptionsWidget> OptionsWidgetClass;

	/** 메인메뉴 커스터마이징 버튼으로 이동할 맵 URL이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization")
	FString CustomizationLevelUrl = TEXT("/Game/Maps/L_Customization");

	/** 메인메뉴와 메인메뉴 옵션에서 사용할 기본 마우스 커서 위젯이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Cursor")
	TSubclassOf<UUserWidget> DefaultMouseCursorWidgetClass;

private:
	/** 시작화면 위젯 인스턴스가 없으면 생성한다. */
	UMainMenuWidget* EnsureMainMenuWidget();

	/** 옵션 위젯 인스턴스가 없으면 생성한다. */
	UOptionsWidget* EnsureOptionsWidget();

	/** 기본 마우스 커서 위젯 슬롯을 소프트웨어 커서로 적용한다. */
	void ApplyDefaultMouseCursorWidget();

	UPROPERTY(Transient)
	TObjectPtr<UMainMenuWidget> MainMenuWidget;

	UPROPERTY(Transient)
	TObjectPtr<UOptionsWidget> OptionsWidget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> DefaultMouseCursorWidget;
};

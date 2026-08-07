// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainMenuPlayerController.generated.h"

class UMainMenuWidget;

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

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 시작화면에서 자동 생성할 WBP_MainMenu 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Main Menu")
	TSubclassOf<UMainMenuWidget> MainMenuWidgetClass;

private:
	/** 시작화면 위젯 인스턴스가 없으면 생성한다. */
	UMainMenuWidget* EnsureMainMenuWidget();

	UPROPERTY(Transient)
	TObjectPtr<UMainMenuWidget> MainMenuWidget;
};

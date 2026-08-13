// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../UI/SnowRumblePlayerController.h"
#include "PodiumPlayerController.generated.h"

class UPodiumWidget;

UCLASS()
class SNOWRUMBLE_API APodiumPlayerController : public ASnowRumblePlayerController
{
	GENERATED_BODY()

public:
	APodiumPlayerController();

	/** 서버가 확정한 포디움 결과 문구를 로컬 위젯에 전달한다. */
	UFUNCTION(Client, Reliable)
	void ClientSetPodiumResults(
		const FText& FirstPlace,
		const FText& SecondPlace,
		const FText& ThirdPlace,
		const FText& Subtitle);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 포디움에서 로컬 플레이어에게 보여줄 UI WBP 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Podium|UI")
	TSubclassOf<UPodiumWidget> PodiumWidgetClass;

	/** 현재 로컬 화면에 띄운 포디움 UI 인스턴스다. */
	UPROPERTY(Transient)
	TObjectPtr<UPodiumWidget> PodiumWidget;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadTimedActionWidget.generated.h"

class UProgressBar;
class ASnowRumbleCharacter;

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API UOverheadTimedActionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 이 위젯이 화면에 투영해 표시할 캐릭터를 지정한다. */
	void SetObservedCharacter(ASnowRumbleCharacter* NewCharacter);

protected:
	/** 위젯 생성 시 진행 바를 초기화하고 자기 전용 표시 상태를 준비한다. */
	virtual void NativeConstruct() override;

	/** 로컬 캐릭터의 진행형 행동 값과 머리 위 화면 위치를 자동 갱신한다. */
	virtual void NativeTick(
		const FGeometry& MyGeometry,
		float InDeltaTime) override;

	/** WBP에서 같은 이름으로 만든 별도 Progress Bar에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> TimedActionProgressBar;

	/** 캐릭터 원점에서 머리 위 UI 목표 위치까지의 월드 오프셋이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Timed Action")
	FVector OverheadWorldOffset = FVector(0.0f, 0.0f, 130.0f);

	/** 월드 위치를 화면에 투영한 뒤 적용할 추가 픽셀 오프셋이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Timed Action")
	FVector2D ScreenPositionOffset = FVector2D::ZeroVector;

private:
	/** 로컬 또는 지정 캐릭터의 진행도와 화면 위치를 한 번 갱신한다. */
	void RefreshActionProgressPresentation();

	/** 원격 캐릭터를 지정한 인스턴스는 얼음 타이머만 표시한다. */
	UPROPERTY(Transient)
	TObjectPtr<ASnowRumbleCharacter> ObservedCharacter;
};

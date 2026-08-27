// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.generated.h"

class UProgressBar;
class USnowRumbleHealthComponent;
class UTextBlock;
class AActor;

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 이 체력 바가 표시할 액터의 HP 컴포넌트를 찾아 연결한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Health")
	void SetObservedActor(AActor* NewObservedActor);

	/** 이 체력 바가 표시할 HP 컴포넌트를 직접 연결한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Health")
	void SetObservedHealthComponent(USnowRumbleHealthComponent* NewHealthComponent);

protected:
	/** 위젯 생성 시 현재 연결된 HP 컴포넌트 값으로 표시를 초기화한다. */
	virtual void NativeConstruct() override;

	/** 위젯 제거 시 HP 변경 이벤트 연결을 정리한다. */
	virtual void NativeDestruct() override;

	/** HP 컴포넌트의 현재 값을 표시해 복제 초기화 순서 차이를 보정한다. */
	virtual void NativeTick(
		const FGeometry& MyGeometry,
		float InDeltaTime) override;

	/** WBP에서 같은 이름으로 만든 Progress Bar에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthProgressBar;

	/** WBP에서 같은 이름으로 만들면 현재 HP와 최대 HP를 텍스트로 표시한다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthTextBlock;

	/** WBP에서 같은 이름으로 만들면 현재 체력 대상 플레이어 이름을 표시한다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PlayerNameText;

private:
	/** HP 컴포넌트 이벤트를 해제한다. */
	void UnbindObservedHealthComponent();

	/** 눈사람 모드에서는 HP 바 위젯 자체를 숨겨 다른 PvP UI는 유지한다. */
	bool ShouldHideForSnowmanMode() const;

	/** HP 값으로 Progress Bar와 선택 텍스트를 갱신한다. */
	void UpdateHealthPresentation(float CurrentHealth, float MaxHealth);

	/** 복제된 PlayerState에서 체력 대상 플레이어 이름을 갱신한다. */
	void UpdateObservedPlayerName();

	/** HP 컴포넌트의 복제 변경 이벤트를 받아 UI를 갱신한다. */
	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth);

	UPROPERTY(Transient)
	TObjectPtr<USnowRumbleHealthComponent> ObservedHealthComponent;

	UPROPERTY(Transient)
	TObjectPtr<AActor> ObservedActor;
};

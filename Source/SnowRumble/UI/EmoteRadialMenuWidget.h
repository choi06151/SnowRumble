// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SnowRumbleAudioUserWidget.h"
#include "EmoteRadialMenuWidget.generated.h"

class UButton;

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API UEmoteRadialMenuWidget : public USnowRumbleAudioUserWidget
{
	GENERATED_BODY()

public:
	/** Tab 입력으로 이모션 원형 메뉴를 화면에 표시한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Emote")
	void OpenEmoteMenu();

	/** Tab 입력 해제 또는 선택 완료 시 이모션 원형 메뉴를 숨긴다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Emote")
	void CloseEmoteMenu();

	/** 현재 마우스가 올라간 버튼의 이모션을 선택하고 성공 여부를 반환한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Emote")
	bool SubmitHoveredEmote();

protected:
	/** 위젯 생성 시 8개 버튼을 순서대로 이모션 선택에 연결한다. */
	virtual void NativeConstruct() override;

	/** 위젯 제거 시 버튼 이벤트 연결을 정리한다. */
	virtual void NativeDestruct() override;

	/** WBP에서 같은 이름으로 만든 0번 이모션 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> EmoteButton0;

	/** WBP에서 같은 이름으로 만든 1번 이모션 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> EmoteButton1;

	/** WBP에서 같은 이름으로 만든 2번 이모션 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> EmoteButton2;

	/** WBP에서 같은 이름으로 만든 3번 이모션 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> EmoteButton3;

	/** WBP에서 같은 이름으로 만든 4번 이모션 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> EmoteButton4;

	/** WBP에서 같은 이름으로 만든 5번 이모션 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> EmoteButton5;

	/** WBP에서 같은 이름으로 만든 6번 이모션 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> EmoteButton6;

	/** WBP에서 같은 이름으로 만든 7번 이모션 버튼에 자동 연결된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> EmoteButton7;

	/** 선택 후 임시 메뉴를 자동으로 닫을지 결정한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Emote")
	bool bCloseAfterSelection = true;

private:
	/** WBP 내부에서 이름이 일치하는 버튼을 직접 찾아 C++ 변수에 보관한다. */
	void ResolveEmoteButtons();

	UFUNCTION()
	void HandleEmoteButton0Clicked();

	UFUNCTION()
	void HandleEmoteButton1Clicked();

	UFUNCTION()
	void HandleEmoteButton2Clicked();

	UFUNCTION()
	void HandleEmoteButton3Clicked();

	UFUNCTION()
	void HandleEmoteButton4Clicked();

	UFUNCTION()
	void HandleEmoteButton5Clicked();

	UFUNCTION()
	void HandleEmoteButton6Clicked();

	UFUNCTION()
	void HandleEmoteButton7Clicked();

	UFUNCTION()
	void HandleEmoteButton0Hovered();

	UFUNCTION()
	void HandleEmoteButton1Hovered();

	UFUNCTION()
	void HandleEmoteButton2Hovered();

	UFUNCTION()
	void HandleEmoteButton3Hovered();

	UFUNCTION()
	void HandleEmoteButton4Hovered();

	UFUNCTION()
	void HandleEmoteButton5Hovered();

	UFUNCTION()
	void HandleEmoteButton6Hovered();

	UFUNCTION()
	void HandleEmoteButton7Hovered();

	UFUNCTION()
	void HandleEmoteButton0Unhovered();

	UFUNCTION()
	void HandleEmoteButton1Unhovered();

	UFUNCTION()
	void HandleEmoteButton2Unhovered();

	UFUNCTION()
	void HandleEmoteButton3Unhovered();

	UFUNCTION()
	void HandleEmoteButton4Unhovered();

	UFUNCTION()
	void HandleEmoteButton5Unhovered();

	UFUNCTION()
	void HandleEmoteButton6Unhovered();

	UFUNCTION()
	void HandleEmoteButton7Unhovered();

	/** 버튼 클릭을 캐릭터의 이모션 선택 요청으로 전달한다. */
	void SelectEmote(int32 EmoteIndex);

	/** 현재 마우스가 올라간 이모션 인덱스를 갱신한다. */
	void SetHoveredEmoteIndex(int32 EmoteIndex);

	/** 현재 마우스가 내려간 버튼과 같은 인덱스일 때만 호버 상태를 해제한다. */
	void ClearHoveredEmoteIndex(int32 EmoteIndex);

	/** 8개 버튼의 클릭 이벤트를 연결한다. */
	void BindEmoteButtons();

	/** 8개 버튼의 클릭 이벤트 연결을 해제한다. */
	void UnbindEmoteButtons();

	int32 HoveredEmoteIndex = INDEX_NONE;

};

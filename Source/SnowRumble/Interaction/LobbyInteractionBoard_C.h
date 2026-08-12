// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyInteractionBoard_C.generated.h"

class ASnowRumbleCharacter;
class UCameraComponent;
class UStaticMeshComponent;
class UWidgetComponent;

UENUM(BlueprintType)
enum class ELobbyBoardAction : uint8
{
	Action0,
	Action1,
	Action2,
	Action3
};

UCLASS(Blueprintable)
class SNOWRUMBLE_API ALobbyInteractionBoard : public AActor
{
	GENERATED_BODY()

public:
	ALobbyInteractionBoard();

	/** 캐릭터가 현재 게시판과 상호작용할 수 있는지 서버와 로컬 후보 탐색에서 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Lobby|Board")
	bool CanInteractWith(const ASnowRumbleCharacter* Character) const;

	/** 캐릭터가 게시판과 상호작용하도록 서버에서 확정한다. */
	void Interact(ASnowRumbleCharacter* Character);

	/** 서버가 게시판 UI 버튼 액션을 검증한 뒤 Blueprint 이벤트로 전달한다. */
	void HandleBoardAction(
		ASnowRumbleCharacter* Character,
		ELobbyBoardAction BoardAction);

	/** 게시판 상호작용 허용 거리다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Lobby|Board")
	float GetInteractionRadius() const;

	/** 카메라가 게시판을 바라볼 때 사용할 중심 위치를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Lobby|Board")
	FVector GetFocusLocation() const;

	/** 포커스 중 클릭 가능한 게시판 월드 UI 컴포넌트를 반환한다. */
	UWidgetComponent* GetBoardWidgetComponent() const;

	/** 포커스 중 클릭 후보가 되는 게시판 월드 UI 컴포넌트들을 반환한다. */
	void GetBoardWidgetComponents(TArray<UWidgetComponent*>& OutWidgetComponents) const;

	/** 로컬 포커스 상태에 맞춰 게시판 위젯이 요청을 보낼 캐릭터를 갱신한다. */
	void SetFocusedCharacter(ASnowRumbleCharacter* Character);

protected:
	virtual void BeginPlay() override;

	/** 게시판 상호작용이 서버에서 확정됐을 때 Blueprint가 UI나 연출을 연결한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Lobby|Board")
	void OnBoardInteracted(ASnowRumbleCharacter* Character);

	/** 게시판 UI 버튼 액션이 서버에서 확정됐을 때 Blueprint가 실제 동작을 연결한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Lobby|Board")
	void OnBoardActionRequested(
		ASnowRumbleCharacter* Character,
		ELobbyBoardAction BoardAction);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Lobby|Board")
	TObjectPtr<UStaticMeshComponent> BoardMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Lobby|Board")
	TObjectPtr<UCameraComponent> FocusCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Lobby|Board")
	TObjectPtr<UWidgetComponent> BoardWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Lobby|Board", meta = (ClampMin = "0.0"))
	float InteractionRadius = 320.0f;

private:
	void InitializeBoardWidget();
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SnowRumbleCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
struct FInputActionValue;

UCLASS()
class SNOWRUMBLE_API ASnowRumbleCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASnowRumbleCharacter();

protected:
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** 이동 입력을 카메라의 수평 방향 기준 이동으로 변환한다. */
	void Move(const FInputActionValue& Value);

	/** 마우스 입력을 로컬 플레이어의 카메라 회전에 적용한다. */
	void Look(const FInputActionValue& Value);

	/** 점프 입력이 시작되면 캐릭터 점프를 요청한다. */
	void StartJump();

	/** 점프 입력이 끝나면 점프 요청을 해제한다. */
	void StopJump();

	/** 상호작용 입력의 누름 또는 해제 상태를 Blueprint에 전달한다. */
	void HandleInteractStarted();

	/** 상호작용 입력 해제 상태를 Blueprint에 전달한다. */
	void HandleInteractCompleted();

	/** 조준 입력 누름 상태를 Blueprint에 전달한다. */
	void HandleAimStarted();

	/** 조준 입력 해제 상태를 Blueprint에 전달한다. */
	void HandleAimCompleted();

	/** 상황별 행동 입력 누름 상태를 Blueprint에 전달한다. */
	void HandleActionStarted();

	/** 상황별 행동 입력 해제 상태를 Blueprint에 전달한다. */
	void HandleActionCompleted();

	/** 장비 내려놓기 입력을 Blueprint에 전달한다. */
	void HandleDropEquipment();

	/** 이모션 입력 누름 상태를 Blueprint에 전달한다. */
	void HandleEmoteStarted();

	/** 이모션 입력 해제 상태를 Blueprint에 전달한다. */
	void HandleEmoteCompleted();

	/** 로컬 플레이어에게 현재 캐릭터의 입력 매핑 컨텍스트를 적용한다. */
	void ApplyInputMappingContext();

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Input")
	void OnInteractInput(bool bPressed);

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Input")
	void OnAimInput(bool bPressed);

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Input")
	void OnActionInput(bool bPressed);

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Input")
	void OnDropEquipmentInput();

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Input")
	void OnEmoteInput(bool bPressed);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputMappingContext> PlayerMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> ActionAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> DropEquipmentAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> EmoteAction;
};

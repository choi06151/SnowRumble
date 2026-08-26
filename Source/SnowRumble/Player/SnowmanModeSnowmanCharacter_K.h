// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SnowRumbleCharacter.h"
#include "SnowmanModeSnowmanCharacter_K.generated.h"

UCLASS()
class SNOWRUMBLE_API ASnowmanModeSnowmanCharacter : public ASnowRumbleCharacter
{
	GENERATED_BODY()

public:
	ASnowmanModeSnowmanCharacter();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Snowman GameMode가 확정한 눈사람 이동 속도를 캐릭터 내부 상태에 적용한다. */
	void SetSnowmanWalkSpeedFromMode(float NewSnowmanWalkSpeed);

	/** 눈사람 모드에서 인간 눈덩이에 맞았을 때 잠시 이동을 멈춘다. */
	void ApplySnowballHitStunFromServer();

protected:
	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(
		UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Movement", meta = (ClampMin = "0.0"))
	float SnowmanWalkSpeed = 625.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Hit", meta = (ClampMin = "0.0"))
	float SnowballHitStunSeconds = 1.0f;

private:
	UFUNCTION()
	void OnRep_SnowballHitStunned();

	void ClearSnowballHitStun();

	void ApplySnowballHitStunMovementState();

	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing = OnRep_SnowballHitStunned, Category = "SnowRumble|Snowman|Hit")
	bool bSnowballHitStunned = false;

	FTimerHandle SnowballHitStunTimerHandle;
};

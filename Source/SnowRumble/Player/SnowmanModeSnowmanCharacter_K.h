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

	/** 눈사람 모드에서 인간 눈덩이에 맞았을 때 기절 시간을 새로 시작한다. */
	void ApplySnowballHitStunFromServer();

	/** UI와 Blueprint에서 현재 눈덩이 피격 기절 상태인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowman|Hit")
	bool IsSnowballHitStunned() const;

	/** UI와 Blueprint에서 서버 시각 기준 눈덩이 피격 기절 남은 시간을 초 단위로 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowman|Hit")
	float GetSnowballHitStunSecondsRemaining() const;

protected:
	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(
		UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Movement", meta = (ClampMin = "0.0"))
	float SnowmanWalkSpeed = 625.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Hit", meta = (ClampMin = "0.0"))
	float SnowballHitStunSeconds = 10.0f;

private:
	UFUNCTION()
	void OnRep_SnowballHitStunned();

	void ClearSnowballHitStun();

	void ApplySnowballHitStunMovementState();

	float GetReplicatedServerTimeSeconds() const;

	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing = OnRep_SnowballHitStunned, Category = "SnowRumble|Snowman|Hit")
	bool bSnowballHitStunned = false;

	UPROPERTY(VisibleInstanceOnly, Replicated, Category = "SnowRumble|Snowman|Hit")
	float SnowballHitStunEndServerTime = 0.0f;

	FTimerHandle SnowballHitStunTimerHandle;
};

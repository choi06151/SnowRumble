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

	/** Snowman GameMode가 확정한 눈사람 이동 속도를 캐릭터 내부 상태에 적용한다. */
	void SetSnowmanWalkSpeedFromMode(float NewSnowmanWalkSpeed);

protected:
	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(
		UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowman|Movement", meta = (ClampMin = "0.0"))
	float SnowmanWalkSpeed = 625.0f;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VibrationBell_C.generated.h"

class AController;
class UStaticMeshComponent;
class USoundAttenuation;
class USoundBase;
struct FDamageEvent;

/** 눈덩이에 맞으면 소리와 좌우 흔들림을 재생하는 배치용 진동벨이다. */
UCLASS(Blueprintable)
class SNOWRUMBLE_API AVibrationBell : public AActor
{
	GENERATED_BODY()

public:
	AVibrationBell();

	virtual float TakeDamage(
		float DamageAmount,
		FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** 모든 참가자 화면에서 피격음과 흔들림을 재생한다. */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayHitFeedback();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Vibration Bell")
	TObjectPtr<UStaticMeshComponent> BellMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Vibration Bell|Audio")
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Vibration Bell|Audio")
	TObjectPtr<USoundAttenuation> HitSoundAttenuation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Vibration Bell|Animation", meta = (ClampMin = "0.0"))
	float ShakeRotationAmplitudeDegrees = 6.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Vibration Bell|Animation", meta = (ClampMin = "0.0"))
	float ShakeFrequency = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Vibration Bell|Animation", meta = (ClampMin = "0.0"))
	float ShakeDurationSeconds = 0.45f;

private:
	void StartShake();

	FVector BellMeshBaseRelativeLocation = FVector::ZeroVector;
	FRotator BellMeshBaseRelativeRotation = FRotator::ZeroRotator;
	float ShakeElapsedSeconds = 0.0f;
};

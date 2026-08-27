// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "GrabbablePhysicsObject_C.generated.h"

class UPrimitiveComponent;
class UStaticMeshComponent;
class UNiagaraSystem;
class ACharacter;

UCLASS(Blueprintable)
class SNOWRUMBLE_API AGrabbablePhysicsObject : public AActor
{
	GENERATED_BODY()

public:
	AGrabbablePhysicsObject();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab|Physics")
	UPrimitiveComponent* GetPhysicsComponent() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab|Physics")
	bool CanBeGrabbed() const;

	void PushCharacterFromGrabMotion(
		ACharacter* Character,
		FVector MotionDirection);

	void RegisterInteraction();

	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab|Interaction")
	int32 GetInteractionCount() const;

	virtual void HandleGrabbedByCharacter(ACharacter* Grabber);
	virtual void HandleReleasedByCharacter(ACharacter* Grabber);
	virtual void TickGrabbedByCharacter(
		ACharacter* Grabber,
		FVector HeldMotion,
		float DeltaTime);

protected:
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayInteractionBreakEffect(FVector_NetQuantize Location);

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Grab|Interaction")
	void OnInteractionBreak();

	UFUNCTION()
	void HandleComponentHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Grab|Physics")
	TObjectPtr<UStaticMeshComponent> PhysicsComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Physics", meta = (ClampMin = "0.0"))
	float PlayerPushStrength = 700.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Physics")
	bool bCanBeGrabbed = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Interaction", meta = (ClampMin = "1"))
	int32 InteractionsToBreak = 5;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Interaction")
	int32 InteractionCount = 0;

	/** 현재 플레이어에게 잡혀 있는 동안에만 일반 충돌 밀침을 허용한다. */
	UPROPERTY(Transient)
	bool bIsHeldByCharacter = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Interaction")
	TObjectPtr<UNiagaraSystem> InteractionBreakEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Interaction", meta = (ClampMin = "0.0"))
	float InteractionCooldownSeconds = 0.2f;

	float LastInteractionTimeSeconds = -BIG_NUMBER;
};

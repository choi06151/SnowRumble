// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "GrabbablePhysicsObject_C.generated.h"

class UPrimitiveComponent;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;
class UNiagaraSystem;
class ACharacter;
class USoundAttenuation;
class USoundBase;

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

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Grab|Interaction")
	void ConfigureInteractionSettings(
		float NewPlayerPushStrength,
		int32 NewInteractionsToBreak,
		UNiagaraSystem* NewInteractionBreakEffect,
		USoundBase* NewInteractionBreakSound,
		USoundAttenuation* NewInteractionBreakSoundAttenuation);

	/** 서버가 변환한 Mesh와 Material을 복제 상태로 설정하고 로컬 물리 컴포넌트에 적용한다. */
	void ConfigureReplicatedVisuals(
		UStaticMesh* NewStaticMesh,
		const TArray<UMaterialInterface*>& NewMaterials);

	virtual void HandleGrabbedByCharacter(ACharacter* Grabber);
	virtual void HandleReleasedByCharacter(ACharacter* Grabber);
	virtual void TickGrabbedByCharacter(
		ACharacter* Grabber,
		FVector HeldMotion,
		float DeltaTime);

protected:
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayInteractionBreakEffect(FVector_NetQuantize Location);

	/** 서버가 파괴를 확정한 위치에서 모든 클라이언트에 파괴음을 재생한다. */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayInteractionBreakSound(FVector_NetQuantize Location);

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Grab|Interaction")
	void OnInteractionBreak();

	UFUNCTION()
	void HandleComponentHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UFUNCTION()
	void OnRep_ReplicatedStaticMesh();

	void ApplyReplicatedVisuals();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Grab|Physics")
	TObjectPtr<UStaticMeshComponent> PhysicsComponent;

	/** 런타임에 StaticMeshActor를 변환한 경우에도 클라이언트가 같은 Mesh를 표시하도록 복제한다. */
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedStaticMesh)
	TObjectPtr<UStaticMesh> ReplicatedStaticMesh;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedStaticMesh)
	TArray<TObjectPtr<UMaterialInterface>> ReplicatedMaterials;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Physics", meta = (ClampMin = "0.0"))
	float PlayerPushStrength = 3000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Physics")
	bool bCanBeGrabbed = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Interaction", meta = (ClampMin = "1"))
	int32 InteractionsToBreak = 1;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Interaction")
	int32 InteractionCount = 0;

	/** 현재 플레이어에게 잡혀 있는 동안에만 일반 충돌 밀침을 허용한다. */
	UPROPERTY(Transient)
	bool bIsHeldByCharacter = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Interaction")
	TObjectPtr<UNiagaraSystem> InteractionBreakEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Interaction")
	TObjectPtr<USoundBase> InteractionBreakSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Interaction")
	TObjectPtr<USoundAttenuation> InteractionBreakSoundAttenuation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab|Interaction", meta = (ClampMin = "0.0"))
	float InteractionCooldownSeconds = 0.2f;

	float LastInteractionTimeSeconds = -BIG_NUMBER;
};

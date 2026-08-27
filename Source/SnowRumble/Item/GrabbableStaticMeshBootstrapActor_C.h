#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrabbableStaticMeshBootstrapActor_C.generated.h"

class AGrabbablePhysicsObject;
class UNiagaraSystem;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class SNOWRUMBLE_API AGrabbableStaticMeshBootstrapActor : public AActor
{
	GENERATED_BODY()

public:
	AGrabbableStaticMeshBootstrapActor();

	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Grab|Bootstrap")
	TSubclassOf<AGrabbablePhysicsObject> GrabbableObjectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Grab|Bootstrap")
	TArray<FName> GrabbableTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Grab|Bootstrap")
	bool bHideOriginalMeshes = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Grab|Bootstrap")
	bool bDisableOriginalCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Grab|Bootstrap")
	bool bConvertActorTaggedStaticMeshes = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Grab|Bootstrap|Converted Object")
	float ConvertedPlayerPushStrength = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Grab|Bootstrap|Converted Object", meta = (ClampMin = "1"))
	int32 ConvertedInteractionsToBreak = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Grab|Bootstrap|Converted Object")
	TObjectPtr<UNiagaraSystem> ConvertedInteractionBreakEffect;

	void ConvertTaggedStaticMeshes();
	bool ShouldConvertStaticMeshComponent(const AActor* SourceActor, const UStaticMeshComponent* SourceComponent) const;
	void ConvertStaticMeshComponent(AActor* SourceActor, UStaticMeshComponent* SourceComponent) const;
	void ApplyOriginalMeshSuppression(AActor* SourceActor, UStaticMeshComponent* SourceComponent) const;
};

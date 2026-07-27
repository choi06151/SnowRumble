// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OutlineComponent.generated.h"

UCLASS(ClassGroup = (SnowRumble), meta = (BlueprintSpawnableComponent))
class SNOWRUMBLE_API UOutlineComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOutlineComponent();

	/** 로컬 화면에서 새 대상 액터의 모든 Primitive Component에 아웃라인을 적용한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Outline")
	void SetOutlinedActor(AActor* NewOutlinedActor);

	/** 현재 로컬 화면에서 아웃라인을 표시 중인 액터를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Outline")
	AActor* GetOutlinedActor() const;

	/** 현재 대상에 적용한 아웃라인을 해제한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Outline")
	void ClearOutline();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 대상의 모든 Primitive Component에 Custom Depth 설정을 적용하거나 해제한다. */
	void ApplyOutline(AActor* TargetActor, bool bEnabled) const;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "SnowRumble|Outline",
		meta = (ClampMin = "0", ClampMax = "255"))
	int32 CustomDepthStencilValue = 1;

	UPROPERTY(Transient)
	TObjectPtr<AActor> OutlinedActor;
};

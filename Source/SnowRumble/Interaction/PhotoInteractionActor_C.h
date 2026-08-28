// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhotoInteractionActor_C.generated.h"

class ASnowRumbleCharacter;
class UCameraComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class SNOWRUMBLE_API APhotoInteractionActor : public AActor
{
	GENERATED_BODY()

public:
	APhotoInteractionActor();

	/** 캐릭터가 사진 촬영 액터와 상호작용할 수 있는지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Photo")
	bool CanInteractWith(const ASnowRumbleCharacter* Character) const;

	/** 서버가 사진 촬영 상호작용을 확정한다. */
	void Interact(ASnowRumbleCharacter* Character);

	/** 사진 상태에서 마우스 입력으로 액터 카메라 회전을 조정한다. */
	void AddPhotoCameraRotation(float YawDelta, float PitchDelta);

	/** 사진 상태에서 사용할 카메라 컴포넌트를 반환한다. */
	UCameraComponent* GetPhotoCameraComponent() const;

	/** 사진 액터 상호작용 허용 거리다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Photo")
	float GetInteractionRadius() const;

protected:
	virtual void BeginPlay() override;

	/** 사진 촬영 상호작용이 서버에서 확정됐을 때 Blueprint 연출을 연결한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Photo")
	void OnPhotoInteraction(ASnowRumbleCharacter* Character);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Photo")
	TObjectPtr<UStaticMeshComponent> PhotoMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Photo")
	TObjectPtr<UCameraComponent> PhotoCameraComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Photo", meta = (ClampMin = "0.0"))
	float InteractionRadius = 320.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Photo|Camera", meta = (ClampMin = "1.0", ClampMax = "89.0"))
	float CameraPitchLimit = 80.0f;
};

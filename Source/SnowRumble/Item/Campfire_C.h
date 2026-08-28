// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Campfire_C.generated.h"

class ASnowRumbleCharacter;
class UNiagaraComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class SNOWRUMBLE_API ACampfire : public AActor
{
	GENERATED_BODY()

public:
	ACampfire();

	/** 서버가 설치자를 기록하고 모닥불 수명을 시작한다. */
	void InitializeCampfireFromServer(ASnowRumbleCharacter* NewInstaller);

	/** 서버 물 침수 판정으로 모닥불을 즉시 끈다. */
	void ExtinguishFromWater();

	/** 눈덩이 등 공격을 받으면 서버가 모닥불 내구도를 감소시킨다. */
	virtual float TakeDamage(
		float DamageAmount,
		FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 서버 상태 변화에 맞춰 Blueprint가 불꽃 크기와 꺼짐 표현을 연결한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Item|Campfire")
	void OnCampfireStateChanged(int32 NewRemainingHitPoints, bool bExtinguished);

	/** 서버가 범위 안의 모든 플레이어를 주기적으로 회복한다. */
	void HealOverlappingCharacters(float DeltaSeconds);

	/** 모닥불이 꺼졌을 때 서버와 클라이언트 표현을 갱신한다. */
	void ExtinguishCampfire();

	/** 현재 내구도와 꺼짐 상태에 맞춰 C++ 기본 컴포넌트 표현을 갱신한다. */
	void RefreshCampfirePresentation();

	UFUNCTION()
	void OnRep_RemainingHitPoints();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Campfire")
	TObjectPtr<USphereComponent> HealRadiusComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Campfire")
	TObjectPtr<UStaticMeshComponent> CampfireMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Campfire|VFX")
	TObjectPtr<UNiagaraComponent> FireVfxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Campfire|VFX")
	TObjectPtr<UNiagaraComponent> HealRadiusVfxComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Campfire", meta = (ClampMin = "0.0"))
	float HealRadius = 320.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Campfire", meta = (ClampMin = "0.0"))
	float HealPerSecond = 4.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Campfire", meta = (ClampMin = "1"))
	int32 MaximumHitPoints = 2;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_RemainingHitPoints, Category = "SnowRumble|Item|Campfire")
	int32 RemainingHitPoints = 2;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "SnowRumble|Item|Campfire")
	TObjectPtr<ASnowRumbleCharacter> Installer;
};

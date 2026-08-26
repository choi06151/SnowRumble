// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JukeboxActor_C.generated.h"

class ASnowRumbleCharacter;
class ASpotLight;
class UBoxComponent;
class USoundAttenuation;
class USoundBase;
class USceneComponent;
class UStaticMeshComponent;
class UNiagaraComponent;

UCLASS(Blueprintable)
class SNOWRUMBLE_API AJukeboxActor : public AActor
{
	GENERATED_BODY()

public:
	AJukeboxActor();
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|Jukebox")
	bool CanInteractWith(const ASnowRumbleCharacter* Character) const;

	void Interact(ASnowRumbleCharacter* Character);

	UFUNCTION(BlueprintPure, Category = "SnowRumble|Jukebox")
	float GetInteractionRadius() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|Jukebox")
	bool IsPlaying() const;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Jukebox")
	void OnJukeboxStarted(ASnowRumbleCharacter* Character);

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Jukebox")
	void OnJukeboxFinished();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Jukebox")
	TObjectPtr<USceneComponent> JukeboxRootComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Jukebox")
	TObjectPtr<UStaticMeshComponent> JukeboxMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Jukebox")
	TObjectPtr<UBoxComponent> JumpBoxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Jukebox")
	TObjectPtr<UNiagaraComponent> JukeboxNiagaraComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Jukebox", meta = (ClampMin = "0.0"))
	float InteractionRadius = 320.0f;

	/** 재생할 Sound 후보 목록이다. 상호작용마다 서버가 하나를 선택한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Jukebox|Audio")
	TArray<TObjectPtr<USoundBase>> JukeboxSounds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Jukebox|Audio")
	TObjectPtr<USoundAttenuation> JukeboxSoundAttenuation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Jukebox|Movement", meta = (ClampMin = "0.05"))
	float JumpInterval = 0.75f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Jukebox|Movement", meta = (ClampMin = "0.0"))
	float JumpVelocity = 420.0f;

	/** 인스턴스에서 직접 지정할 주크박스 Spotlight 목록이다. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "SnowRumble|Jukebox|Lighting")
	TArray<TObjectPtr<ASpotLight>> JukeboxSpotlights;

	/** 재생 중 Spotlight가 바뀌는 간격이다. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "SnowRumble|Jukebox|Lighting", meta = (ClampMin = "0.05"))
	float SpotlightChangeInterval = 0.35f;

	/** Spotlight가 선택될 때 사용할 색상 후보 목록이다. 비어 있으면 완전 랜덤 색상을 사용한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Jukebox|Lighting")
	TArray<FLinearColor> SpotlightColors;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Jukebox|Animation", meta = (ClampMin = "0.0"))
	float BounceAmplitude = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Jukebox|Animation", meta = (ClampMin = "0.0"))
	float BounceFrequency = 5.0f;

private:
	void ApplyJumpPulse();
	void FinishPlayback();
	void CycleSpotlight();
	void RefreshPlaybackPresentation();

	UFUNCTION()
	void OnRep_IsPlaying();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayJukeboxSound(int32 SoundIndex);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastSetActiveSpotlight(int32 SpotlightIndex, FLinearColor LightColor);

	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing = OnRep_IsPlaying, Category = "SnowRumble|Jukebox")
	bool bIsPlaying = false;

	FTimerHandle JumpTimerHandle;
	FTimerHandle PlaybackTimerHandle;
	FTimerHandle SpotlightTimerHandle;
	int32 ActiveSpotlightIndex = INDEX_NONE;
	int32 ActiveSoundIndex = INDEX_NONE;
	FVector JukeboxMeshBaseRelativeLocation = FVector::ZeroVector;
	float BounceTime = 0.0f;
};

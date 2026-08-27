// Copyright Epic Games, Inc. All Rights Reserved.

#include "JukeboxActor_C.h"

#include "../Audio/SnowRumbleAudioHelpers.h"
#include "../Player/SnowRumbleCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/LightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/SpotLight.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

AJukeboxActor::AJukeboxActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	JukeboxRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("JukeboxRootComponent"));
	SetRootComponent(JukeboxRootComponent);

	JukeboxMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("JukeboxMeshComponent"));
	JukeboxMeshComponent->SetupAttachment(JukeboxRootComponent);
	JukeboxMeshComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	JumpBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("JumpBoxComponent"));
	JumpBoxComponent->SetupAttachment(RootComponent);
	JumpBoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	JumpBoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	JumpBoxComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	JumpBoxComponent->SetGenerateOverlapEvents(true);

	JukeboxNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("JukeboxNiagaraComponent"));
	JukeboxNiagaraComponent->SetupAttachment(JukeboxRootComponent);
	JukeboxNiagaraComponent->SetAutoActivate(false);
}

void AJukeboxActor::BeginPlay()
{
	Super::BeginPlay();
	SetReplicateMovement(false);
	JukeboxMeshBaseRelativeLocation = JukeboxMeshComponent
		? JukeboxMeshComponent->GetRelativeLocation()
		: FVector::ZeroVector;
	RefreshPlaybackPresentation();

	for (ASpotLight* Spotlight : JukeboxSpotlights)
	{
		if (Spotlight && Spotlight->GetLightComponent())
		{
			Spotlight->GetLightComponent()->SetVisibility(false);
		}
	}
}

bool AJukeboxActor::IsJukeboxMeshComponent(const UPrimitiveComponent* Component) const
{
	return Component && Component == JukeboxMeshComponent;
}

void AJukeboxActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!JukeboxMeshComponent)
	{
		return;
	}

	if (bIsPlaying && BounceAmplitude > 0.0f && BounceFrequency > 0.0f)
	{
		BounceTime += DeltaSeconds * BounceFrequency * UE_TWO_PI;
		JukeboxMeshComponent->SetRelativeLocation(
			JukeboxMeshBaseRelativeLocation
			+ FVector(0.0f, 0.0f, FMath::Sin(BounceTime) * BounceAmplitude));
	}
	else
	{
		BounceTime = 0.0f;
		JukeboxMeshComponent->SetRelativeLocation(JukeboxMeshBaseRelativeLocation);
	}
}

void AJukeboxActor::OnRep_IsPlaying()
{
	RefreshPlaybackPresentation();
}

void AJukeboxActor::RefreshPlaybackPresentation()
{
	if (!JukeboxNiagaraComponent)
	{
		return;
	}

	if (bIsPlaying)
	{
		JukeboxNiagaraComponent->Activate(true);
	}
	else
	{
		JukeboxNiagaraComponent->Deactivate();
	}
}

bool AJukeboxActor::CanInteractWith(const ASnowRumbleCharacter* Character) const
{
	if (!IsValid(Character)
		|| (!bIsPlaying && JukeboxSounds.IsEmpty())
		|| InteractionRadius <= 0.0f)
	{
		return false;
	}

	return FVector::DistSquared2D(GetActorLocation(), Character->GetActorLocation())
		<= FMath::Square(InteractionRadius);
}

void AJukeboxActor::Interact(ASnowRumbleCharacter* Character)
{
	if (!HasAuthority() || !CanInteractWith(Character))
	{
		return;
	}

	if (bIsPlaying)
	{
		if (OptedOutCharacters.Contains(Character))
		{
			OptedOutCharacters.Remove(Character);
			ApplyJumpPulse();
		}
		else
		{
			OptedOutCharacters.Add(Character);
		}

		ForceNetUpdate();
		return;
	}

	TArray<int32> ValidSoundIndices;
	for (int32 SoundIndex = 0; SoundIndex < JukeboxSounds.Num(); ++SoundIndex)
	{
		if (JukeboxSounds[SoundIndex])
		{
			ValidSoundIndices.Add(SoundIndex);
		}
	}
	if (ValidSoundIndices.IsEmpty())
	{
		return;
	}

	ActiveSoundIndex = ValidSoundIndices[FMath::RandRange(0, ValidSoundIndices.Num() - 1)];
	USoundBase* ActiveSound = JukeboxSounds[ActiveSoundIndex];
	const float SoundDuration = ActiveSound->GetDuration();
	if (SoundDuration <= 0.0f)
	{
		return;
	}

	bIsPlaying = true;
	OptedOutCharacters.Reset();
	RefreshPlaybackPresentation();
	OnJukeboxStarted(Character);
	MulticastPlayJukeboxSound(ActiveSoundIndex);
	CycleSpotlight();
	GetWorldTimerManager().SetTimer(
		SpotlightTimerHandle,
		this,
		&AJukeboxActor::CycleSpotlight,
		FMath::Max(0.05f, SpotlightChangeInterval),
		true,
		FMath::Max(0.05f, SpotlightChangeInterval));

	ApplyJumpPulse();
	GetWorldTimerManager().SetTimer(
		JumpTimerHandle,
		this,
		&AJukeboxActor::ApplyJumpPulse,
		FMath::Max(0.05f, JumpInterval),
		true,
		FMath::Max(0.05f, JumpInterval));
	GetWorldTimerManager().SetTimer(
		PlaybackTimerHandle,
		this,
		&AJukeboxActor::FinishPlayback,
		SoundDuration,
		false);
}

float AJukeboxActor::GetInteractionRadius() const
{
	return InteractionRadius;
}

bool AJukeboxActor::IsPlaying() const
{
	return bIsPlaying;
}

bool AJukeboxActor::IsCharacterParticipating(
	const ASnowRumbleCharacter* Character) const
{
	return bIsPlaying
		&& IsValid(Character)
		&& !OptedOutCharacters.Contains(Character);
}

void AJukeboxActor::ApplyJumpPulse()
{
	if (!HasAuthority() || !bIsPlaying || !JumpBoxComponent)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	JumpBoxComponent->GetOverlappingActors(OverlappingActors, ASnowRumbleCharacter::StaticClass());
	for (AActor* OverlappingActor : OverlappingActors)
	{
		ASnowRumbleCharacter* Character = Cast<ASnowRumbleCharacter>(OverlappingActor);
		if (!Character
			|| !IsCharacterParticipating(Character)
			|| Character->IsDead()
			|| Character->IsFrozen())
		{
			continue;
		}

		Character->LaunchCharacter(FVector(0.0f, 0.0f, JumpVelocity), false, true);
	}
}

void AJukeboxActor::FinishPlayback()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(JumpTimerHandle);
	GetWorldTimerManager().ClearTimer(SpotlightTimerHandle);
	bIsPlaying = false;
	OptedOutCharacters.Reset();
	RefreshPlaybackPresentation();
	ActiveSoundIndex = INDEX_NONE;
	for (ASpotLight* Spotlight : JukeboxSpotlights)
	{
		if (Spotlight && Spotlight->GetLightComponent())
		{
			Spotlight->GetLightComponent()->SetVisibility(false);
		}
	}
	ActiveSpotlightIndex = INDEX_NONE;
	MulticastSetActiveSpotlight(INDEX_NONE, FLinearColor::Black);
	OnJukeboxFinished();
}

void AJukeboxActor::CycleSpotlight()
{
	if (!HasAuthority() || !bIsPlaying || JukeboxSpotlights.IsEmpty())
	{
		return;
	}

	int32 NewSpotlightIndex = FMath::RandRange(0, JukeboxSpotlights.Num() - 1);
	if (JukeboxSpotlights.Num() > 1 && NewSpotlightIndex == ActiveSpotlightIndex)
	{
		NewSpotlightIndex = (NewSpotlightIndex + 1) % JukeboxSpotlights.Num();
	}

	const FLinearColor NewLightColor = SpotlightColors.IsEmpty()
		? FLinearColor::MakeRandomColor()
		: SpotlightColors[FMath::RandRange(0, SpotlightColors.Num() - 1)];
	ActiveSpotlightIndex = NewSpotlightIndex;
	MulticastSetActiveSpotlight(NewSpotlightIndex, NewLightColor);
}

void AJukeboxActor::MulticastPlayJukeboxSound_Implementation(int32 SoundIndex)
{
	if (!JukeboxSounds.IsValidIndex(SoundIndex) || !JukeboxSounds[SoundIndex])
	{
		return;
	}

	SnowRumbleAudio::PlaySoundAtLocation(
		this,
		JukeboxSounds[SoundIndex],
		ESnowRumbleAudioMixChannel::Gameplay,
		GetActorLocation(),
		1.0f,
		1.0f,
		JukeboxSoundAttenuation);
}

void AJukeboxActor::MulticastSetActiveSpotlight_Implementation(
	int32 SpotlightIndex,
	FLinearColor LightColor)
{
	for (int32 Index = 0; Index < JukeboxSpotlights.Num(); ++Index)
	{
		if (ASpotLight* Spotlight = JukeboxSpotlights[Index])
		{
			if (Spotlight->GetLightComponent())
			{
				Spotlight->GetLightComponent()->SetLightColor(LightColor);
				Spotlight->GetLightComponent()->SetVisibility(Index == SpotlightIndex);
			}
		}
	}
}

void AJukeboxActor::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AJukeboxActor, bIsPlaying);
	DOREPLIFETIME(AJukeboxActor, OptedOutCharacters);
}

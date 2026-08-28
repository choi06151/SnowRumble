// Copyright Epic Games, Inc. All Rights Reserved.

#include "Campfire_C.h"

#include "../Player/SnowRumbleCharacter.h"
#include "../Player/SnowRumbleHealthComponent.h"
#include "../Snowball/SnowballItem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"

ACampfire::ACampfire()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	HealRadiusComponent =
		CreateDefaultSubobject<USphereComponent>(TEXT("HealRadiusComponent"));
	SetRootComponent(HealRadiusComponent);
	HealRadiusComponent->InitSphereRadius(HealRadius);
	HealRadiusComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HealRadiusComponent->SetCollisionObjectType(ECC_WorldDynamic);
	HealRadiusComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	HealRadiusComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	HealRadiusComponent->SetGenerateOverlapEvents(true);

	CampfireMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CampfireMeshComponent"));
	CampfireMeshComponent->SetupAttachment(HealRadiusComponent);
	CampfireMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CampfireMeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CampfireMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

	FireVfxComponent =
		CreateDefaultSubobject<UNiagaraComponent>(TEXT("FireVfxComponent"));
	FireVfxComponent->SetupAttachment(CampfireMeshComponent);
	FireVfxComponent->SetAutoActivate(true);

	HealRadiusVfxComponent =
		CreateDefaultSubobject<UNiagaraComponent>(TEXT("HealRadiusVfxComponent"));
	HealRadiusVfxComponent->SetupAttachment(HealRadiusComponent);
	HealRadiusVfxComponent->SetAutoActivate(true);
}

void ACampfire::BeginPlay()
{
	Super::BeginPlay();

	RemainingHitPoints = FMath::Max(1, MaximumHitPoints);
	if (HealRadiusComponent)
	{
		HealRadiusComponent->SetSphereRadius(FMath::Max(0.0f, HealRadius));
	}
	RefreshCampfirePresentation();
	OnCampfireStateChanged(RemainingHitPoints, false);
}

void ACampfire::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority() && RemainingHitPoints > 0)
	{
		HealOverlappingCharacters(DeltaSeconds);
	}
}

void ACampfire::InitializeCampfireFromServer(
	ASnowRumbleCharacter* NewInstaller)
{
	if (!HasAuthority())
	{
		return;
	}

	Installer = NewInstaller;
	ForceNetUpdate();
}

void ACampfire::ExtinguishFromWater()
{
	if (!HasAuthority() || RemainingHitPoints <= 0)
	{
		return;
	}

	ExtinguishCampfire();
}

float ACampfire::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	const float AppliedDamage = Super::TakeDamage(
		DamageAmount,
		DamageEvent,
		EventInstigator,
		DamageCauser);

	if (!HasAuthority() || DamageAmount <= 0.0f || RemainingHitPoints <= 0)
	{
		return AppliedDamage;
	}

	const ASnowballItem* DamageSnowball = Cast<ASnowballItem>(DamageCauser);
	if (DamageSnowball && DamageSnowball->IsFullyGrown())
	{
		ExtinguishCampfire();
		return AppliedDamage;
	}

	--RemainingHitPoints;
	if (RemainingHitPoints <= 0)
	{
		ExtinguishCampfire();
	}
	else
	{
		OnRep_RemainingHitPoints();
		ForceNetUpdate();
	}

	return AppliedDamage;
}

void ACampfire::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACampfire, RemainingHitPoints);
	DOREPLIFETIME(ACampfire, Installer);
}

void ACampfire::HealOverlappingCharacters(float DeltaSeconds)
{
	if (HealPerSecond <= 0.0f || DeltaSeconds <= 0.0f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float EffectiveHealRadius = FMath::Max(0.0f, HealRadius);
	if (EffectiveHealRadius <= 0.0f)
	{
		return;
	}

	TArray<FOverlapResult> OverlapResults;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(CampfireHealOverlap),
		false,
		this);
	World->OverlapMultiByObjectType(
		OverlapResults,
		GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(EffectiveHealRadius),
		QueryParams);

	TSet<USnowRumbleHealthComponent*> HealedComponents;
	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		ASnowRumbleCharacter* Character =
			Cast<ASnowRumbleCharacter>(OverlapResult.GetActor());
		USnowRumbleHealthComponent* HealthComponent = Character
			? Character->FindComponentByClass<USnowRumbleHealthComponent>()
			: nullptr;
		if (HealthComponent && !HealedComponents.Contains(HealthComponent))
		{
			HealedComponents.Add(HealthComponent);
			HealthComponent->ApplyHealing(HealPerSecond * DeltaSeconds);
		}
	}
}

void ACampfire::ExtinguishCampfire()
{
	RemainingHitPoints = 0;
	if (HealRadiusComponent)
	{
		HealRadiusComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (CampfireMeshComponent)
	{
		CampfireMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	OnRep_RemainingHitPoints();
	ForceNetUpdate();
}

void ACampfire::RefreshCampfirePresentation()
{
	const bool bExtinguished = RemainingHitPoints <= 0;
	if (FireVfxComponent)
	{
		FireVfxComponent->SetActive(!bExtinguished, true);
	}
	if (HealRadiusVfxComponent)
	{
		HealRadiusVfxComponent->SetActive(!bExtinguished, true);
		HealRadiusVfxComponent->SetWorldScale3D(
			FVector::OneVector * FMath::Max(0.0f, HealRadius) / 100.0f);
	}
}

void ACampfire::OnRep_RemainingHitPoints()
{
	RefreshCampfirePresentation();
	OnCampfireStateChanged(RemainingHitPoints, RemainingHitPoints <= 0);
}

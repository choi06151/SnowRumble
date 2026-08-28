// Copyright Epic Games, Inc. All Rights Reserved.

#include "GiftBox_C.h"

#include "../Game/SnowRumbleGameState_C.h"
#include "GiftBoxItemPickup_C.h"
#include "../Player/SnowRumbleCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"

AGiftBox::AGiftBox()
{
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComponent =
		CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->InitSphereRadius(55.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	CollisionComponent->SetGenerateOverlapEvents(false);

	GiftBoxMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GiftBoxMeshComponent"));
	GiftBoxMeshComponent->SetupAttachment(CollisionComponent);
	GiftBoxMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement =
		CreateDefaultSubobject<UProjectileMovementComponent>(
			TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 0.0f;
	ProjectileMovement->MaxSpeed = 1600.0f;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->bRotationFollowsVelocity = false;

	GradeVfxComponent =
		CreateDefaultSubobject<UNiagaraComponent>(TEXT("GradeVfxComponent"));
	GradeVfxComponent->SetupAttachment(GiftBoxMeshComponent);
	GradeVfxComponent->SetAutoActivate(false);
	GradeVfxComponent->SetVisibility(false, true);
}

void AGiftBox::BeginPlay()
{
	Super::BeginPlay();

	if (ProjectileMovement)
	{
		ProjectileMovement->OnProjectileStop.AddDynamic(
			this,
			&AGiftBox::HandleProjectileStopped);
	}

	if (RedBoxRewards.IsEmpty())
	{
		RedBoxRewards.Add({
			ESnowRumbleGiftItemType::Boots,
			TEXT("Boots"),
			NSLOCTEXT("SnowRumble", "GiftRewardBoots", "부츠"),
			nullptr});
		RedBoxRewards.Add({
			ESnowRumbleGiftItemType::Padding,
			TEXT("Padding"),
			NSLOCTEXT("SnowRumble", "GiftRewardPadding", "패딩"),
			nullptr});
		RedBoxRewards.Add({
			ESnowRumbleGiftItemType::Gloves,
			TEXT("Gloves"),
			NSLOCTEXT("SnowRumble", "GiftRewardGloves", "장갑"),
			nullptr});
		RedBoxRewards.Add({
			ESnowRumbleGiftItemType::SnowShovel,
			TEXT("RustyShovel"),
			NSLOCTEXT("SnowRumble", "GiftRewardRustyShovel", "녹슨 눈삽"),
			nullptr});
		RedBoxRewards.Add({
			ESnowRumbleGiftItemType::SnowDuckMaker,
			TEXT("RustyDuckMaker"),
			NSLOCTEXT("SnowRumble", "GiftRewardRustyDuckMaker", "녹슨 눈오리 제작기"),
			nullptr});
	}
	if (GoldBoxRewards.IsEmpty())
	{
		GoldBoxRewards.Add({
			ESnowRumbleGiftItemType::GoldenHotPack,
			TEXT("GoldenHotPack"),
			NSLOCTEXT("SnowRumble", "GiftRewardGoldenHotPack", "황금 핫팩"),
			nullptr});
		GoldBoxRewards.Add({
			ESnowRumbleGiftItemType::GoldenShovel,
			TEXT("GoldenShovel"),
			NSLOCTEXT("SnowRumble", "GiftRewardGoldenShovel", "황금 눈삽"),
			nullptr});
		GoldBoxRewards.Add({
			ESnowRumbleGiftItemType::GoldenDuckMaker,
			TEXT("GoldenDuckMaker"),
			NSLOCTEXT("SnowRumble", "GiftRewardGoldenDuckMaker", "황금 눈오리 제작기"),
			nullptr});
		GoldBoxRewards.Add({
			ESnowRumbleGiftItemType::GoldenFishBread,
			TEXT("GoldenFishBread"),
			NSLOCTEXT("SnowRumble", "GiftRewardGoldenFishBread", "황금 붕어빵"),
			nullptr});
		GoldBoxRewards.Add({
			ESnowRumbleGiftItemType::CampfireKit,
			TEXT("CampfireKit"),
			NSLOCTEXT("SnowRumble", "GiftRewardCampfireKit", "모닥불 키트"),
			nullptr});
	}

	OnRep_GiftBoxGrade();
}

void AGiftBox::InitializeGiftBoxFromServer(
	ESnowRumbleGiftBoxGrade NewGrade)
{
	if (!HasAuthority())
	{
		return;
	}

	GiftBoxGrade = NewGrade;
	OnRep_GiftBoxGrade();
	ForceNetUpdate();
}

bool AGiftBox::CanInteractWith(
	const ASnowRumbleCharacter* Character) const
{
	if (!IsValid(Character)
		|| bOpened
		|| InteractionRadius <= 0.0f)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	const ASnowRumbleGameState* SnowRumbleGameState = World
		? World->GetGameState<ASnowRumbleGameState>()
		: nullptr;
	if (SnowRumbleGameState
		&& (SnowRumbleGameState->IsRoundEnded()
			|| SnowRumbleGameState->IsMatchInputLocked()))
	{
		return false;
	}

	return FVector::DistSquared(
		Character->GetActorLocation(),
		GetActorLocation()) <= FMath::Square(InteractionRadius);
}

bool AGiftBox::TryOpen(ASnowRumbleCharacter* Character)
{
	if (!HasAuthority()
		|| bOpened
		|| (Character && !CanInteractWith(Character)))
	{
		return false;
	}

	const FSnowRumbleGiftBoxReward Reward = ChooseReward();
	OpenedRewardId = Reward.RewardId;
	OpenedRewardName = Reward.DisplayName.IsEmpty()
		? GetFallbackRewardName()
		: Reward.DisplayName;
	OpenedByCharacter = Character;
	bOpened = true;

	SpawnRewardPickup(Reward);
	OnRep_Opened();

	SetLifeSpan(OpenedDestroyDelaySeconds);
	ForceNetUpdate();
	return true;
}

float AGiftBox::TakeDamage(
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

	if (HasAuthority() && DamageAmount > 0.0f && !bOpened)
	{
		TryOpen(nullptr);
	}

	return AppliedDamage;
}

float AGiftBox::GetInteractionRadius() const
{
	return InteractionRadius;
}

ESnowRumbleGiftBoxGrade AGiftBox::GetGiftBoxGrade() const
{
	return GiftBoxGrade;
}

FText AGiftBox::GetOpenedRewardName() const
{
	return OpenedRewardName;
}

void AGiftBox::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGiftBox, GiftBoxGrade);
	DOREPLIFETIME(AGiftBox, bOpened);
	DOREPLIFETIME(AGiftBox, OpenedRewardId);
	DOREPLIFETIME(AGiftBox, OpenedRewardName);
	DOREPLIFETIME(AGiftBox, OpenedByCharacter);
	DOREPLIFETIME(AGiftBox, SpawnedPickup);
}

void AGiftBox::OnRep_GiftBoxGrade()
{
	ApplyGradeMaterial();
	ApplyGradeEffect();
	OnGiftBoxGradeChanged(GiftBoxGrade);
}

void AGiftBox::OnRep_Opened()
{
	if (!bOpened)
	{
		return;
	}

	if (CollisionComponent)
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}
	DeactivateGradeEffect();

	SpawnOpenedEffect();
	OnGiftBoxOpened(
		OpenedByCharacter,
		GiftBoxGrade,
		OpenedRewardName);
}

void AGiftBox::HandleProjectileStopped(const FHitResult& Hit)
{
	OnGiftBoxLanded();
}

void AGiftBox::ApplyGradeMaterial()
{
	if (!GiftBoxMeshComponent || GradeMaterialIndex < 0)
	{
		return;
	}

	UMaterialInterface* GradeMaterial =
		GiftBoxGrade == ESnowRumbleGiftBoxGrade::Gold
			? GoldGiftBoxMaterial.Get()
			: RedGiftBoxMaterial.Get();
	if (GradeMaterial)
	{
		GiftBoxMeshComponent->SetMaterial(GradeMaterialIndex, GradeMaterial);
	}
}

void AGiftBox::ApplyGradeEffect()
{
	if (!GradeVfxComponent)
	{
		return;
	}

	UNiagaraSystem* GradeEffect =
		GiftBoxGrade == ESnowRumbleGiftBoxGrade::Gold
			? GoldGiftBoxEffect.Get()
			: RedGiftBoxEffect.Get();
	if (!GradeEffect || bOpened)
	{
		DeactivateGradeEffect();
		return;
	}

	GradeVfxComponent->SetAsset(GradeEffect);
	GradeVfxComponent->SetVisibility(true, true);
	GradeVfxComponent->Activate(true);
}

void AGiftBox::DeactivateGradeEffect()
{
	if (!GradeVfxComponent)
	{
		return;
	}

	GradeVfxComponent->Deactivate();
	GradeVfxComponent->SetVisibility(false, true);
}

void AGiftBox::SpawnOpenedEffect() const
{
	if (!OpenedEffect)
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		this,
		OpenedEffect,
		GetActorLocation(),
		GetActorRotation());
}

FSnowRumbleGiftBoxReward AGiftBox::ChooseReward() const
{
	const TArray<FSnowRumbleGiftBoxReward>& CandidateRewards =
		GiftBoxGrade == ESnowRumbleGiftBoxGrade::Gold
			? GoldBoxRewards
			: RedBoxRewards;
	if (CandidateRewards.IsEmpty())
	{
		return {
			ESnowRumbleGiftItemType::None,
			NAME_None,
			GetFallbackRewardName(),
			nullptr };
	}

	return CandidateRewards[
		FMath::RandRange(0, CandidateRewards.Num() - 1)];
}

FText AGiftBox::GetFallbackRewardName() const
{
	return GiftBoxGrade == ESnowRumbleGiftBoxGrade::Gold
		? NSLOCTEXT("SnowRumble", "GiftRewardGoldFallback", "황금 아이템")
		: NSLOCTEXT("SnowRumble", "GiftRewardRedFallback", "빨간 상자 아이템");
}

void AGiftBox::SpawnRewardPickup(const FSnowRumbleGiftBoxReward& Reward)
{
	UWorld* World = GetWorld();
	if (!HasAuthority() || !World)
	{
		return;
	}

	TSubclassOf<AGiftBoxItemPickup> PickupClass = Reward.PickupClass
		? Reward.PickupClass
		: DefaultPickupClass;
	if (!PickupClass)
	{
		PickupClass = AGiftBoxItemPickup::StaticClass();
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	SpawnedPickup = World->SpawnActor<AGiftBoxItemPickup>(
		PickupClass,
		GetActorLocation(),
		GetActorRotation(),
		SpawnParameters);
	if (SpawnedPickup)
	{
		SpawnedPickup->InitializePickupFromServer(
			Reward.ItemType,
			OpenedRewardId,
			OpenedRewardName);
	}
}

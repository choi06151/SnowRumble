// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowballProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ASnowballProjectile::ASnowballProjectile()
{
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(15.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	CollisionComponent->OnComponentHit.AddDynamic(this, &ASnowballProjectile::HandleCollision);
	RootComponent = CollisionComponent;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 1800.0f;
	ProjectileMovement->MaxSpeed = 1800.0f;
	ProjectileMovement->ProjectileGravityScale = 0.25f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->Velocity = FVector::ForwardVector * ProjectileMovement->InitialSpeed;
}

void ASnowballProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* OwningActor = GetOwner())
	{
		CollisionComponent->IgnoreActorWhenMoving(OwningActor, true);
	}

	if (HasAuthority())
	{
		SetLifeSpan(MaximumLifeSeconds);
	}
}

void ASnowballProjectile::HandleCollision(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!HasAuthority() || !OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	UGameplayStatics::ApplyDamage(
		OtherActor,
		Damage,
		GetInstigatorController(),
		this,
		UDamageType::StaticClass());

	Destroy();
}

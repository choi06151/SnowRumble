// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerGrabComponent_C.h"

#include "SnowRumbleCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

UPlayerGrabComponent::UPlayerGrabComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	SetIsReplicatedByDefault(true);
}

void UPlayerGrabComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlayerGrabComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const float TargetGrabReachAlpha = bIsGrabReaching ? 1.0f : 0.0f;
	const float InterpSpeed =
		bIsGrabReaching ? GrabReachRaiseInterpSpeed : GrabReachLowerInterpSpeed;
	CurrentGrabReachAlpha = FMath::FInterpTo(
		CurrentGrabReachAlpha,
		TargetGrabReachAlpha,
		DeltaTime,
		InterpSpeed);

	if (!GetOwner() || !GetOwner()->HasAuthority() || !bIsGrabReaching)
	{
		return;
	}

	if (GrabbedCharacter)
	{
		UpdateGrabbedCharacterTether(DeltaTime);
		return;
	}
	if (GrabAttachmentType == ESnowRumbleGrabAttachmentType::World)
	{
		UpdateWorldGrabTether(DeltaTime);
		return;
	}

	ASnowRumbleCharacter* TargetCharacter = nullptr;
	USkeletalMeshComponent* TargetMesh = nullptr;
	FName TargetBoneName = NAME_None;
	FVector AttachedWorldLocation = FVector::ZeroVector;
	ESnowRumbleGrabAttachmentType AttachmentType =
		ESnowRumbleGrabAttachmentType::None;
	if (FindGrabCandidate(
		TargetCharacter,
		TargetMesh,
		TargetBoneName,
		AttachedWorldLocation,
		AttachmentType))
	{
		if (AttachmentType == ESnowRumbleGrabAttachmentType::Character)
		{
			AttachGrabConstraint(
				TargetCharacter,
				TargetMesh,
				TargetBoneName,
				AttachedWorldLocation);
		}
		else if (AttachmentType == ESnowRumbleGrabAttachmentType::World)
		{
			AttachWorldGrab(AttachedWorldLocation);
		}
	}
}

void UPlayerGrabComponent::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPlayerGrabComponent, bIsGrabReaching);
	DOREPLIFETIME(UPlayerGrabComponent, GrabbedCharacter);
	DOREPLIFETIME(UPlayerGrabComponent, GrabAttachmentType);
	DOREPLIFETIME(UPlayerGrabComponent, GrabAttachedWorldLocation);
	DOREPLIFETIME(UPlayerGrabComponent, GrabbedTargetBoneName);
	DOREPLIFETIME(UPlayerGrabComponent, ActiveGrabHand);
}

void UPlayerGrabComponent::StartGrabReach()
{
	ASnowRumbleCharacter* Character = GetOwnerCharacter();
	if (!Character || (!Character->HasAuthority() && !Character->IsLocallyControlled()))
	{
		return;
	}

	if (!CanStartGrabReach())
	{
		return;
	}

	bIsGrabReaching = true;
	OnRep_IsGrabReaching();

	if (Character->HasAuthority())
	{
		ServerStartGrabReach_Implementation();
		return;
	}

	ServerStartGrabReach();
}

void UPlayerGrabComponent::StopGrabReach()
{
	ASnowRumbleCharacter* Character = GetOwnerCharacter();
	if (!Character || (!Character->HasAuthority() && !Character->IsLocallyControlled()))
	{
		return;
	}

	bIsGrabReaching = false;
	OnRep_IsGrabReaching();
	ClearGrabConstraint();

	if (Character->HasAuthority())
	{
		ServerStopGrabReach_Implementation();
		return;
	}

	ServerStopGrabReach();
}

bool UPlayerGrabComponent::IsGrabReaching() const
{
	return bIsGrabReaching;
}

bool UPlayerGrabComponent::IsGrabbingCharacter() const
{
	return GrabbedCharacter != nullptr;
}

bool UPlayerGrabComponent::IsGrabAttached() const
{
	return GrabAttachmentType != ESnowRumbleGrabAttachmentType::None;
}

bool UPlayerGrabComponent::IsHangingFromWorldGrab() const
{
	return GrabAttachmentType == ESnowRumbleGrabAttachmentType::World;
}

FVector UPlayerGrabComponent::GetGrabAttachedWorldLocation() const
{
	return GrabAttachedWorldLocation;
}

FVector UPlayerGrabComponent::GetRightHandGrabTargetLocation() const
{
	if (IsGrabAttached() && ActiveGrabHand == ESnowRumbleGrabHand::Right)
	{
		return GrabAttachedWorldLocation;
	}
	return BuildHandGrabTargetLocation(ESnowRumbleGrabHand::Right);
}

FVector UPlayerGrabComponent::GetLeftHandGrabTargetLocation() const
{
	if (IsGrabAttached() && ActiveGrabHand == ESnowRumbleGrabHand::Left)
	{
		return GrabAttachedWorldLocation;
	}
	return BuildHandGrabTargetLocation(ESnowRumbleGrabHand::Left);
}

float UPlayerGrabComponent::GetGrabReachAlpha() const
{
	return FMath::Clamp(CurrentGrabReachAlpha, 0.0f, 1.0f);
}

void UPlayerGrabComponent::ServerStartGrabReach_Implementation()
{
	if (!CanStartGrabReach())
	{
		return;
	}

	bIsGrabReaching = true;
	OnRep_IsGrabReaching();
	if (AActor* Owner = GetOwner())
	{
		Owner->ForceNetUpdate();
	}
}

void UPlayerGrabComponent::ServerStopGrabReach_Implementation()
{
	bIsGrabReaching = false;
	OnRep_IsGrabReaching();
	ClearGrabConstraint();
	if (AActor* Owner = GetOwner())
	{
		Owner->ForceNetUpdate();
	}
}

void UPlayerGrabComponent::OnRep_IsGrabReaching()
{
	if (!bIsGrabReaching)
	{
		ClearGrabConstraint();
	}
}

void UPlayerGrabComponent::OnRep_GrabbedCharacter()
{
	if (!GrabbedCharacter)
	{
		ClearGrabConstraint();
	}
}

void UPlayerGrabComponent::OnRep_GrabAttachmentType()
{
	if (GrabAttachmentType == ESnowRumbleGrabAttachmentType::None)
	{
		ClearGrabConstraint();
	}
}

bool UPlayerGrabComponent::CanStartGrabReach() const
{
	const ASnowRumbleCharacter* Character = GetOwnerCharacter();
	return Character
		&& Character->CanStartPlayerGrabReach();
}

bool UPlayerGrabComponent::FindGrabCandidate(
	ASnowRumbleCharacter*& OutCharacter,
	USkeletalMeshComponent*& OutMesh,
	FName& OutBoneName,
	FVector& OutAttachedWorldLocation,
	ESnowRumbleGrabAttachmentType& OutAttachmentType) const
{
	OutCharacter = nullptr;
	OutMesh = nullptr;
	OutBoneName = NAME_None;
	OutAttachedWorldLocation = FVector::ZeroVector;
	OutAttachmentType = ESnowRumbleGrabAttachmentType::None;

	const ASnowRumbleCharacter* Character = GetOwnerCharacter();
	const UWorld* World = GetWorld();
	if (!Character || !World)
	{
		return false;
	}

	const FVector TargetLocation = BuildHandGrabTargetLocation(ActiveGrabHand);
	const FVector Forward = Character->GetActorForwardVector();
	const FVector TraceStart =
		TargetLocation - Forward * GrabTraceBacktrackDistance;
	const FVector TraceEnd =
		TargetLocation + Forward * GrabTraceForwardDistance;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PlayerGrabTrace), false);
	QueryParams.AddIgnoredActor(Character);

	TArray<FHitResult> Hits;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	const bool bHit = World->SweepMultiByObjectType(
		Hits,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(GrabTraceRadius),
		QueryParams);
	if (!bHit)
	{
		return false;
	}

	for (const FHitResult& Hit : Hits)
	{
		ASnowRumbleCharacter* HitCharacter =
			Cast<ASnowRumbleCharacter>(Hit.GetActor());
		if (!HitCharacter)
		{
			if (Hit.bBlockingHit && Hit.GetActor() && Hit.GetActor() != Character)
			{
				OutAttachedWorldLocation = Hit.ImpactPoint;
				OutAttachmentType = ESnowRumbleGrabAttachmentType::World;
				return true;
			}
			continue;
		}
		if (HitCharacter == Character || HitCharacter->IsDead())
		{
			continue;
		}

		USkeletalMeshComponent* HitMesh =
			Cast<USkeletalMeshComponent>(Hit.GetComponent());
		if (!HitMesh)
		{
			HitMesh = HitCharacter->GetMesh();
		}
		if (!HitMesh)
		{
			continue;
		}

		OutCharacter = HitCharacter;
		OutMesh = HitMesh;
		OutBoneName = Hit.BoneName;
		OutAttachedWorldLocation =
			Hit.ImpactPoint.IsNearlyZero() ? Hit.Location : Hit.ImpactPoint;
		OutAttachmentType = ESnowRumbleGrabAttachmentType::Character;
		return true;
	}

	return false;
}

void UPlayerGrabComponent::AttachGrabConstraint(
	ASnowRumbleCharacter* TargetCharacter,
	USkeletalMeshComponent* TargetMesh,
	FName TargetBoneName,
	FVector AttachedWorldLocation)
{
	ASnowRumbleCharacter* Character = GetOwnerCharacter();
	if (!Character || !Character->HasAuthority() || !TargetCharacter || !TargetMesh)
	{
		return;
	}

	USkeletalMeshComponent* OwnerMesh = Character->GetMesh();
	if (!OwnerMesh)
	{
		return;
	}

	ClearGrabConstraint();

	GrabbedCharacter = TargetCharacter;
	GrabbedTargetBoneName = TargetBoneName;
	GrabAttachedWorldLocation = AttachedWorldLocation;
	GrabAttachmentType = ESnowRumbleGrabAttachmentType::Character;
	GrabbedActorLocationOffsetFromAttachedPoint =
		TargetCharacter->GetActorLocation() - AttachedWorldLocation;
	TargetCharacter->ApplyGrabbedByCharacter(Character);

	GrabConstraintComponent =
		NewObject<UPhysicsConstraintComponent>(Character, TEXT("GrabConstraint"));
	if (!GrabConstraintComponent)
	{
		return;
	}

	GrabConstraintComponent->RegisterComponent();
	GrabConstraintComponent->AttachToComponent(
		Character->GetRootComponent(),
		FAttachmentTransformRules::KeepWorldTransform);
	GrabConstraintComponent->SetWorldLocation(
		BuildHandGrabTargetLocation(ActiveGrabHand));
	GrabConstraintComponent->SetDisableCollision(true);
	GrabConstraintComponent->SetLinearXLimit(LCM_Limited, GrabTetherSlackDistance);
	GrabConstraintComponent->SetLinearYLimit(LCM_Limited, GrabTetherSlackDistance);
	GrabConstraintComponent->SetLinearZLimit(LCM_Limited, GrabTetherSlackDistance);
	GrabConstraintComponent->SetAngularSwing1Limit(ACM_Limited, 35.0f);
	GrabConstraintComponent->SetAngularSwing2Limit(ACM_Limited, 35.0f);
	GrabConstraintComponent->SetAngularTwistLimit(ACM_Limited, 45.0f);
	GrabConstraintComponent->SetConstrainedComponents(
		OwnerMesh,
		ActiveGrabHand == ESnowRumbleGrabHand::Right
			? RightGrabHandBoneName
			: LeftGrabHandBoneName,
		TargetMesh,
		TargetBoneName);

	Character->ForceNetUpdate();
	TargetCharacter->ForceNetUpdate();
}

void UPlayerGrabComponent::AttachWorldGrab(FVector AttachedWorldLocation)
{
	ASnowRumbleCharacter* Character = GetOwnerCharacter();
	if (!Character || !Character->HasAuthority())
	{
		return;
	}

	ClearGrabConstraint();

	GrabAttachedWorldLocation = AttachedWorldLocation;
	GrabAttachmentType = ESnowRumbleGrabAttachmentType::World;
	GrabbedActorLocationOffsetFromAttachedPoint = FVector::ZeroVector;
	Character->HandleWorldGrabChanged(true);
	Character->ForceNetUpdate();
}

void UPlayerGrabComponent::ClearGrabConstraint()
{
	ASnowRumbleCharacter* PreviousGrabbedCharacter = GrabbedCharacter.Get();
	if (GrabConstraintComponent)
	{
		GrabConstraintComponent->BreakConstraint();
		GrabConstraintComponent->DestroyComponent();
		GrabConstraintComponent = nullptr;
	}

	if (AActor* Owner = GetOwner(); Owner && Owner->HasAuthority())
	{
		if (PreviousGrabbedCharacter)
		{
			PreviousGrabbedCharacter->ClearGrabbedByCharacter(GetOwnerCharacter());
		}
		GrabbedCharacter = nullptr;
		GrabbedTargetBoneName = NAME_None;
		GrabAttachmentType = ESnowRumbleGrabAttachmentType::None;
		GrabAttachedWorldLocation = FVector::ZeroVector;
		GrabbedActorLocationOffsetFromAttachedPoint = FVector::ZeroVector;
		if (ASnowRumbleCharacter* OwnerCharacter = GetOwnerCharacter())
		{
			OwnerCharacter->HandleWorldGrabChanged(false);
		}
	}
}

void UPlayerGrabComponent::UpdateGrabbedCharacterTether(float DeltaTime)
{
	ASnowRumbleCharacter* Character = GetOwnerCharacter();
	ASnowRumbleCharacter* TargetCharacter = GrabbedCharacter.Get();
	if (!Character || !Character->HasAuthority() || !TargetCharacter)
	{
		return;
	}

	UCharacterMovementComponent* TargetMovement =
		TargetCharacter->GetCharacterMovement();
	if (!TargetMovement)
	{
		return;
	}

	GrabAttachedWorldLocation = BuildHandGrabTargetLocation(ActiveGrabHand);
	const FVector DesiredTargetLocation =
		GrabAttachedWorldLocation + GrabbedActorLocationOffsetFromAttachedPoint;
	const FVector CurrentTargetLocation = TargetCharacter->GetActorLocation();

	const FVector ToGrabPoint = GrabAttachedWorldLocation - CurrentTargetLocation;
	const FVector FlatToGrabPoint(ToGrabPoint.X, ToGrabPoint.Y, 0.0f);
	if (!FlatToGrabPoint.IsNearlyZero())
	{
		const FRotator DesiredFacingRotation =
			FlatToGrabPoint.ToOrientationRotator();
		const FRotator CurrentRotation = TargetCharacter->GetActorRotation();
		const FRotator NewRotation = FMath::RInterpTo(
			CurrentRotation,
			FRotator(0.0f, DesiredFacingRotation.Yaw, 0.0f),
			DeltaTime,
			GrabbedCharacterFacingInterpSpeed);
		TargetCharacter->SetActorRotation(NewRotation);
	}

	const FVector ToDesired = DesiredTargetLocation - CurrentTargetLocation;
	const float Distance = ToDesired.Size();
	if (Distance <= GrabTetherSlackDistance)
	{
		TargetMovement->Velocity.Z =
			FMath::Min(TargetMovement->Velocity.Z, 0.0f)
			* GrabTetherVelocityDamping;
		return;
	}

	const FVector PullDirection = ToDesired / Distance;
	const float PullSpeed = FMath::Clamp(
		(Distance - GrabTetherSlackDistance) * GrabTetherPullStrength,
		0.0f,
		GrabTetherMaxPullSpeed);
	const FVector CorrectionVelocity = PullDirection * PullSpeed;
	TargetMovement->Velocity =
		TargetMovement->Velocity * GrabbedCharacterInputVelocityRetention
		+ CorrectionVelocity;
	TargetMovement->Velocity = TargetMovement->Velocity.GetClampedToMaxSize(
		GrabTetherMaxPullSpeed);
	TargetCharacter->ForceNetUpdate();
	Character->ForceNetUpdate();
}

void UPlayerGrabComponent::UpdateWorldGrabTether(float DeltaTime)
{
	ASnowRumbleCharacter* Character = GetOwnerCharacter();
	if (!Character || !Character->HasAuthority())
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent =
		Character->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	const FVector ToGrabPoint = GrabAttachedWorldLocation - Character->GetActorLocation();
	const FVector FlatToGrabPoint(ToGrabPoint.X, ToGrabPoint.Y, 0.0f);
	if (!FlatToGrabPoint.IsNearlyZero())
	{
		const FRotator DesiredFacingRotation =
			FlatToGrabPoint.ToOrientationRotator();
		const FRotator CurrentRotation = Character->GetActorRotation();
		const FRotator NewRotation = FMath::RInterpTo(
			CurrentRotation,
			FRotator(0.0f, DesiredFacingRotation.Yaw, 0.0f),
			DeltaTime,
			WorldGrabFacingInterpSpeed);
		Character->SetActorRotation(NewRotation);
	}

	const FVector DesiredBodyLocation =
		GrabAttachedWorldLocation
		- Character->GetActorForwardVector() * WorldGrabBodyBackOffset
		- FVector::UpVector * WorldGrabBodyDownOffset;
	const FVector CurrentLocation = Character->GetActorLocation();
	const FVector ToDesired = DesiredBodyLocation - CurrentLocation;
	const float Distance = ToDesired.Size();
	if (Distance <= WorldGrabTetherSlackDistance)
	{
		MovementComponent->Velocity.Z =
			FMath::Min(MovementComponent->Velocity.Z, 0.0f)
			* GrabTetherVelocityDamping;
		return;
	}

	const FVector PullDirection = ToDesired / Distance;
	const float PullSpeed = FMath::Clamp(
		(Distance - WorldGrabTetherSlackDistance) * WorldGrabTetherPullStrength,
		0.0f,
		WorldGrabTetherMaxPullSpeed);
	const FVector CorrectionVelocity = PullDirection * PullSpeed;
	MovementComponent->Velocity =
		MovementComponent->Velocity * WorldGrabInputVelocityRetention
		+ CorrectionVelocity;
	MovementComponent->Velocity = MovementComponent->Velocity.GetClampedToMaxSize(
		WorldGrabTetherMaxPullSpeed);
	Character->ForceNetUpdate();
}

FVector UPlayerGrabComponent::BuildHandGrabTargetLocation(
	ESnowRumbleGrabHand Hand) const
{
	const ASnowRumbleCharacter* Character = GetOwnerCharacter();
	if (!Character)
	{
		return FVector::ZeroVector;
	}

	const FVector Origin =
		Character->GetActorLocation()
		+ FVector::UpVector * Character->GetGrabReachOriginHeight();
	const float SideSign =
		Hand == ESnowRumbleGrabHand::Right ? 1.0f : -1.0f;
	return Origin
		+ Character->GetActorForwardVector() * GrabReachForwardDistance
		+ FVector::UpVector * GrabReachUpOffset
		+ Character->GetActorRightVector() * GrabReachSideOffset * SideSign;
}

ASnowRumbleCharacter* UPlayerGrabComponent::GetOwnerCharacter() const
{
	return Cast<ASnowRumbleCharacter>(GetOwner());
}

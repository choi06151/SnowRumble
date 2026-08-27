// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerGrabComponent_C.h"

#include "../Audio/SnowRumbleAudioHelpers.h"
#include "../Item/GrabbablePhysicsObject_C.h"
#include "SnowRumbleCharacter.h"
#include "../Game/SnowRumblePlayerState.h"
#include "Components/PrimitiveComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "DrawDebugHelpers.h"

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

	if (IsGrabAttached()
		&& GrabAttachmentType != ESnowRumbleGrabAttachmentType::PhysicsObject
		&& MaximumGrabHoldSeconds > 0.0f
		&& GetGrabRemainingTimeProgress() <= 0.0f)
	{
		ServerStopGrabReach_Implementation();
		return;
	}

	if (GrabbedCharacter)
	{
		if (GrabbedCharacter->IsDead())
		{
			ServerStopGrabReach_Implementation();
			return;
		}
		UpdateGrabbedCharacterTether(DeltaTime);
		return;
	}
	if (GrabAttachmentType == ESnowRumbleGrabAttachmentType::World)
	{
		UpdateWorldGrabTether(DeltaTime);
		return;
	}
	if (GrabAttachmentType == ESnowRumbleGrabAttachmentType::PhysicsObject)
	{
		UpdatePhysicsObjectGrabTether(DeltaTime);
		return;
	}

	ASnowRumbleCharacter* TargetCharacter = nullptr;
	USkeletalMeshComponent* TargetMesh = nullptr;
	AGrabbablePhysicsObject* TargetPhysicsObject = nullptr;
	UPrimitiveComponent* TargetPhysicsComponent = nullptr;
	FName TargetBoneName = NAME_None;
	FVector AttachedWorldLocation = FVector::ZeroVector;
	ESnowRumbleGrabAttachmentType AttachmentType =
		ESnowRumbleGrabAttachmentType::None;
	const UWorld* World = GetWorld();
	const bool bAllowWorldAttachment =
		World
		&& World->GetTimeSeconds() - GrabReachStartedTimeSeconds
			>= WorldGrabMinReachHoldSeconds
		&& CurrentGrabReachAlpha >= MinGrabReachAlphaForAttachment;
	if (CurrentGrabReachAlpha < MinGrabReachAlphaForAttachment)
	{
		return;
	}
	if (FindGrabCandidate(
		TargetCharacter,
		TargetMesh,
		TargetPhysicsObject,
		TargetPhysicsComponent,
		TargetBoneName,
		AttachedWorldLocation,
		AttachmentType,
		bAllowWorldAttachment))
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
		else if (AttachmentType == ESnowRumbleGrabAttachmentType::PhysicsObject)
		{
			AttachPhysicsObject(
				TargetPhysicsObject,
				TargetPhysicsComponent,
				AttachedWorldLocation);
		}
	}
}

void UPlayerGrabComponent::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPlayerGrabComponent, bIsGrabReaching);
	DOREPLIFETIME(UPlayerGrabComponent, GrabbedCharacter);
	DOREPLIFETIME(UPlayerGrabComponent, GrabbedPhysicsObject);
	DOREPLIFETIME(UPlayerGrabComponent, GrabAttachmentType);
	DOREPLIFETIME(UPlayerGrabComponent, GrabAttachedWorldLocation);
	DOREPLIFETIME(UPlayerGrabComponent, GrabbedTargetBoneName);
	DOREPLIFETIME(UPlayerGrabComponent, ActiveGrabHand);
	DOREPLIFETIME(UPlayerGrabComponent, GrabReachStartedServerTime);
	DOREPLIFETIME(UPlayerGrabComponent, GrabAttachmentStartedServerTime);
	DOREPLIFETIME(UPlayerGrabComponent, GrabHoldProgress);
	DOREPLIFETIME(UPlayerGrabComponent, GrabProgressAtAttachmentStart);
	DOREPLIFETIME(UPlayerGrabComponent, GrabRecoveryStartedServerTime);
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
	if (const UWorld* World = GetWorld())
	{
		GrabReachStartedTimeSeconds = World->GetTimeSeconds();
		const AGameStateBase* GameState = World->GetGameState();
		GrabReachStartedServerTime = GameState
			? GameState->GetServerWorldTimeSeconds()
			: World->GetTimeSeconds();
	}
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
	GrabReachStartedTimeSeconds = 0.0;
	GrabReachStartedServerTime = 0.0f;
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

bool UPlayerGrabComponent::IsCarryingOpposingFrozenCharacter() const
{
	const ASnowRumbleCharacter* OwnerCharacter = GetOwnerCharacter();
	const ASnowRumbleCharacter* TargetCharacter = GrabbedCharacter.Get();
	if (!OwnerCharacter || !TargetCharacter || !TargetCharacter->IsFrozen())
	{
		return false;
	}

	const ASnowRumblePlayerState* OwnerPlayerState =
		OwnerCharacter->GetPlayerState<ASnowRumblePlayerState>();
	const ASnowRumblePlayerState* TargetPlayerState =
		TargetCharacter->GetPlayerState<ASnowRumblePlayerState>();
	return OwnerPlayerState
		&& TargetPlayerState
		&& OwnerPlayerState->GetLobbyTeam() != ESnowRumbleTeam::None
		&& TargetPlayerState->GetLobbyTeam() != ESnowRumbleTeam::None
		&& OwnerPlayerState->GetLobbyTeam()
			!= TargetPlayerState->GetLobbyTeam();
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

float UPlayerGrabComponent::GetGrabRemainingTimeProgress() const
{
	if (GrabAttachmentType == ESnowRumbleGrabAttachmentType::PhysicsObject)
	{
		return 1.0f;
	}

	if (MaximumGrabHoldSeconds <= 0.0f)
	{
		return 1.0f;
	}

	if (IsGrabAttached()
		&& GrabAttachmentStartedServerTime > 0.0f)
	{
		const float ElapsedSeconds = FMath::Max(
			0.0f,
			GetCurrentServerTimeSeconds() - GrabAttachmentStartedServerTime);
		return FMath::Clamp(
			GrabProgressAtAttachmentStart
				- ElapsedSeconds / MaximumGrabHoldSeconds,
			0.0f,
			1.0f);
	}

	if (GrabRecoveryStartedServerTime <= 0.0f
		|| GrabHoldProgress >= 1.0f
		|| GrabRecoverySeconds <= 0.0f)
	{
		return FMath::Clamp(GrabHoldProgress, 0.0f, 1.0f);
	}

	const float RecoveryElapsedSeconds = FMath::Max(
		0.0f,
		GetCurrentServerTimeSeconds() - GrabRecoveryStartedServerTime);
	return FMath::Clamp(
		GrabHoldProgress
			+ RecoveryElapsedSeconds / GrabRecoverySeconds,
		0.0f,
		1.0f);
}

void UPlayerGrabComponent::ServerStartGrabReach_Implementation()
{
	if (!CanStartGrabReach())
	{
		return;
	}

	bIsGrabReaching = true;
	if (const UWorld* World = GetWorld())
	{
		GrabReachStartedTimeSeconds = World->GetTimeSeconds();
		const AGameStateBase* GameState = World->GetGameState();
		GrabReachStartedServerTime = GameState
			? GameState->GetServerWorldTimeSeconds()
			: World->GetTimeSeconds();
	}
	OnRep_IsGrabReaching();
	if (AActor* Owner = GetOwner())
	{
		Owner->ForceNetUpdate();
	}
}

void UPlayerGrabComponent::ServerStopGrabReach_Implementation()
{
	bIsGrabReaching = false;
	GrabReachStartedTimeSeconds = 0.0;
	GrabReachStartedServerTime = 0.0f;
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
	if (!GrabbedCharacter
		&& GrabAttachmentType != ESnowRumbleGrabAttachmentType::PhysicsObject)
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
	else if (GrabAttachmentType == ESnowRumbleGrabAttachmentType::PhysicsObject)
	{
		ApplyPhysicsObjectRotationMode();
	}
}

void UPlayerGrabComponent::OnRep_GrabbedPhysicsObject()
{
	if (!GrabbedPhysicsObject)
	{
		ClearGrabConstraint();
	}
	else if (GrabAttachmentType == ESnowRumbleGrabAttachmentType::PhysicsObject)
	{
		ApplyPhysicsObjectRotationMode();
	}
}

bool UPlayerGrabComponent::CanStartGrabReach() const
{
	const ASnowRumbleCharacter* Character = GetOwnerCharacter();
	return Character
		&& Character->CanStartPlayerGrabReach()
		&& (MaximumGrabHoldSeconds <= 0.0f
			|| GetGrabRemainingTimeProgress() > 0.0f);
}

bool UPlayerGrabComponent::FindGrabCandidate(
	ASnowRumbleCharacter*& OutCharacter,
	USkeletalMeshComponent*& OutMesh,
	AGrabbablePhysicsObject*& OutPhysicsObject,
	UPrimitiveComponent*& OutPhysicsComponent,
	FName& OutBoneName,
	FVector& OutAttachedWorldLocation,
	ESnowRumbleGrabAttachmentType& OutAttachmentType,
	bool bAllowWorldAttachment) const
{
	OutCharacter = nullptr;
	OutMesh = nullptr;
	OutPhysicsObject = nullptr;
	OutPhysicsComponent = nullptr;
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
	ObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	const bool bHit = World->SweepMultiByObjectType(
		Hits,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(GrabTraceRadius),
		QueryParams);
	if (bDrawGrabTraceDebug)
	{
		const FColor TraceColor = bHit ? FColor::Green : FColor::Red;
		DrawDebugLine(
			World,
			TraceStart,
			TraceEnd,
			TraceColor,
			false,
			GrabTraceDebugDrawSeconds,
			0,
			1.5f);
		DrawDebugSphere(
			World,
			TargetLocation,
			GrabTraceRadius,
			12,
			TraceColor,
			false,
			GrabTraceDebugDrawSeconds);
		DrawDebugSphere(
			World,
			TraceStart,
			GrabTraceRadius,
			12,
			FColor::Cyan,
			false,
			GrabTraceDebugDrawSeconds);
		DrawDebugSphere(
			World,
			TraceEnd,
			GrabTraceRadius,
			12,
			FColor::Yellow,
			false,
			GrabTraceDebugDrawSeconds);
	}
	if (!bHit)
	{
		return false;
	}

	for (const FHitResult& Hit : Hits)
	{
		if (bDrawGrabTraceDebug)
		{
			const FVector HitPoint =
				Hit.ImpactPoint.IsNearlyZero() ? Hit.Location : Hit.ImpactPoint;
			DrawDebugSphere(
				World,
				HitPoint,
				GrabTraceRadius * 0.5f,
				12,
				FColor::Orange,
				false,
				GrabTraceDebugDrawSeconds);
		}

		const UPrimitiveComponent* HitComponent = Hit.GetComponent();
		if (Hit.GetActor() == Character
			|| (HitComponent && HitComponent->GetOwner() == Character))
		{
			continue;
		}

		AGrabbablePhysicsObject* PhysicsObject =
			Cast<AGrabbablePhysicsObject>(Hit.GetActor());
		UPrimitiveComponent* PhysicsComponent = PhysicsObject
			? PhysicsObject->GetPhysicsComponent()
			: nullptr;
		if (PhysicsObject && PhysicsObject->CanBeGrabbed() && PhysicsComponent)
		{
			OutPhysicsObject = PhysicsObject;
			OutPhysicsComponent = PhysicsComponent;
			OutAttachedWorldLocation =
				Hit.ImpactPoint.IsNearlyZero() ? Hit.Location : Hit.ImpactPoint;
			OutAttachmentType = ESnowRumbleGrabAttachmentType::PhysicsObject;
			return true;
		}

		ASnowRumbleCharacter* HitCharacter =
			Cast<ASnowRumbleCharacter>(Hit.GetActor());
		if (!HitCharacter)
		{
			const FVector ImpactPoint =
				Hit.ImpactPoint.IsNearlyZero() ? Hit.Location : Hit.ImpactPoint;
			const float AttachHeightFromActor =
				ImpactPoint.Z - Character->GetActorLocation().Z;
			const bool bVerticalEnoughSurface =
				FMath::Abs(Hit.ImpactNormal.GetSafeNormal().Z)
					<= WorldGrabMaxSurfaceNormalZ;
			const bool bReachableHeight =
				AttachHeightFromActor >= WorldGrabMinAttachHeightFromActor
				&& AttachHeightFromActor <= WorldGrabMaxAttachHeightFromActor;
			if (bAllowWorldAttachment
				&& Hit.bBlockingHit
				&& Hit.GetActor()
				&& Hit.GetActor() != Character
				&& bVerticalEnoughSurface
				&& bReachableHeight)
			{
				OutAttachedWorldLocation = ImpactPoint;
				OutAttachmentType = ESnowRumbleGrabAttachmentType::World;
				return true;
			}
			continue;
		}
		if (HitCharacter == Character || HitCharacter->IsDead())
		{
			continue;
		}
		if (HitCharacter->IsGrabbedByCharacter())
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
	const float CurrentGrabProgress = GetGrabRemainingTimeProgress();
	GrabHoldProgress = CurrentGrabProgress;
	GrabProgressAtAttachmentStart = CurrentGrabProgress;
	GrabRecoveryStartedServerTime = 0.0f;

	GrabbedCharacter = TargetCharacter;
	GrabbedTargetBoneName = TargetBoneName;
	GrabAttachedWorldLocation = AttachedWorldLocation;
	GrabAttachmentType = ESnowRumbleGrabAttachmentType::Character;
	if (const UWorld* World = GetWorld())
	{
		const AGameStateBase* GameState = World->GetGameState();
		GrabAttachmentStartedServerTime = GameState
			? GameState->GetServerWorldTimeSeconds()
			: World->GetTimeSeconds();
	}
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
	MulticastPlayGrabSound(Character->GetActorLocation(), false);
}

void UPlayerGrabComponent::AttachWorldGrab(FVector AttachedWorldLocation)
{
	ASnowRumbleCharacter* Character = GetOwnerCharacter();
	if (!Character || !Character->HasAuthority())
	{
		return;
	}

	ClearGrabConstraint();
	const float CurrentGrabProgress = GetGrabRemainingTimeProgress();
	GrabHoldProgress = CurrentGrabProgress;
	GrabProgressAtAttachmentStart = CurrentGrabProgress;
	GrabRecoveryStartedServerTime = 0.0f;

	GrabAttachedWorldLocation = AttachedWorldLocation;
	GrabAttachmentType = ESnowRumbleGrabAttachmentType::World;
	if (const UWorld* World = GetWorld())
	{
		const AGameStateBase* GameState = World->GetGameState();
		GrabAttachmentStartedServerTime = GameState
			? GameState->GetServerWorldTimeSeconds()
			: World->GetTimeSeconds();
	}
	GrabbedActorLocationOffsetFromAttachedPoint = FVector::ZeroVector;
	Character->HandleWorldGrabChanged(true);
	Character->ForceNetUpdate();
	MulticastPlayGrabSound(Character->GetActorLocation(), false);
}

void UPlayerGrabComponent::AttachPhysicsObject(
	AGrabbablePhysicsObject* TargetObject,
	UPrimitiveComponent* TargetComponent,
	FVector AttachedWorldLocation)
{
	ASnowRumbleCharacter* Character = GetOwnerCharacter();
	if (!Character || !Character->HasAuthority() || !TargetObject || !TargetComponent)
	{
		return;
	}

	ClearGrabConstraint();
	const float CurrentGrabProgress = GetGrabRemainingTimeProgress();
	GrabHoldProgress = CurrentGrabProgress;
	GrabProgressAtAttachmentStart = CurrentGrabProgress;
	GrabRecoveryStartedServerTime = 0.0f;
	GrabbedPhysicsObject = TargetObject;
	GrabbedPhysicsComponent = TargetComponent;
	GrabbedCharacter = nullptr;
	GrabbedTargetBoneName = NAME_None;
	GrabAttachedWorldLocation = AttachedWorldLocation;
	GrabAttachmentType = ESnowRumbleGrabAttachmentType::PhysicsObject;
	GrabbedPhysicsRelativeTransform =
		TargetComponent->GetComponentTransform().GetRelativeTransform(
			Character->GetActorTransform());
	if (const UWorld* World = GetWorld())
	{
		const AGameStateBase* GameState = World->GetGameState();
		GrabAttachmentStartedServerTime = GameState
			? GameState->GetServerWorldTimeSeconds()
			: World->GetTimeSeconds();
	}
	ApplyPhysicsObjectRotationMode();

	GrabPhysicsAnchorComponent =
		NewObject<USphereComponent>(Character, TEXT("GrabPhysicsObjectAnchor"));
	if (!GrabPhysicsAnchorComponent)
	{
		return;
	}
	GrabPhysicsAnchorComponent->SetSphereRadius(6.0f);
	GrabPhysicsAnchorComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GrabPhysicsAnchorComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	GrabPhysicsAnchorComponent->SetGenerateOverlapEvents(false);
	GrabPhysicsAnchorComponent->SetSimulatePhysics(false);
	GrabPhysicsAnchorComponent->SetHiddenInGame(true);
	GrabPhysicsAnchorComponent->RegisterComponent();
	GrabPhysicsAnchorComponent->AttachToComponent(
		Character->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
	GrabPhysicsAnchorComponent->SetWorldLocation(
		TargetComponent->GetComponentLocation());

	bGrabbedPhysicsWasSimulating = TargetComponent->IsSimulatingPhysics();
	bGrabbedPhysicsGravityEnabled = TargetComponent->IsGravityEnabled();
	TargetComponent->SetPhysicsLinearVelocity(FVector::ZeroVector, false);
	TargetComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector, false);
	TargetComponent->SetSimulatePhysics(false);
	TargetComponent->SetEnableGravity(false);
	TargetComponent->IgnoreActorWhenMoving(Character, true);
	if (UCapsuleComponent* CharacterCapsule = Character->GetCapsuleComponent())
	{
		CharacterCapsule->IgnoreActorWhenMoving(TargetObject, true);
	}
	TargetComponent->SetWorldTransform(
		TargetComponent->GetComponentTransform(),
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
	LastGrabbedPhysicsLocation = TargetComponent->GetComponentLocation();
	TargetObject->HandleGrabbedByCharacter(Character);

	Character->ForceNetUpdate();
	TargetObject->ForceNetUpdate();
	MulticastPlayGrabSound(Character->GetActorLocation(), false);
}

void UPlayerGrabComponent::ClearGrabConstraint()
{
	ASnowRumbleCharacter* PreviousGrabbedCharacter = GrabbedCharacter.Get();
	const bool bWasAttached = IsGrabAttached();
	const bool bWasWorldGrab =
		GrabAttachmentType == ESnowRumbleGrabAttachmentType::World;
	ClearPhysicsObjectRotationMode();
	if (GrabConstraintComponent)
	{
		GrabConstraintComponent->BreakConstraint();
		GrabConstraintComponent->DestroyComponent();
		GrabConstraintComponent = nullptr;
	}
	if (GrabCollisionConstraintComponent)
	{
		GrabCollisionConstraintComponent->BreakConstraint();
		GrabCollisionConstraintComponent->DestroyComponent();
		GrabCollisionConstraintComponent = nullptr;
	}
	if (GrabPhysicsAnchorComponent)
	{
		GrabPhysicsAnchorComponent->DestroyComponent();
		GrabPhysicsAnchorComponent = nullptr;
	}
	if (GrabbedPhysicsObject)
	{
		GrabbedPhysicsObject->HandleReleasedByCharacter(GetOwnerCharacter());
		if (UPrimitiveComponent* PhysicsComponent =
			GrabbedPhysicsObject->GetPhysicsComponent())
		{
			if (AActor* Owner = GetOwner())
			{
				PhysicsComponent->IgnoreActorWhenMoving(Owner, false);
				if (ASnowRumbleCharacter* OwnerCharacter = GetOwnerCharacter())
				{
					if (UCapsuleComponent* CharacterCapsule =
						OwnerCharacter->GetCapsuleComponent())
					{
						CharacterCapsule->IgnoreActorWhenMoving(
							GrabbedPhysicsObject,
							false);
					}
				}
			}
			if (PhysicsComponent == GrabbedPhysicsComponent)
			{
				PhysicsComponent->SetEnableGravity(bGrabbedPhysicsGravityEnabled);
				PhysicsComponent->SetSimulatePhysics(bGrabbedPhysicsWasSimulating);
			}
		}
	}

	if (AActor* Owner = GetOwner(); Owner && Owner->HasAuthority())
	{
		if (bWasAttached)
		{
			MulticastPlayGrabSound(Owner->GetActorLocation(), true);
		}
		if (bWasAttached)
		{
			GrabHoldProgress = GetGrabRemainingTimeProgress();
			GrabProgressAtAttachmentStart = GrabHoldProgress;
			GrabRecoveryStartedServerTime = GrabHoldProgress < 1.0f
				? GetCurrentServerTimeSeconds() + GrabRecoveryDelaySeconds
				: 0.0f;
		}
		if (PreviousGrabbedCharacter)
		{
			PreviousGrabbedCharacter->ClearGrabbedByCharacter(GetOwnerCharacter());
		}
		GrabbedCharacter = nullptr;
		GrabbedPhysicsObject = nullptr;
		GrabbedPhysicsComponent = nullptr;
		bGrabbedPhysicsWasSimulating = false;
		bGrabbedPhysicsGravityEnabled = true;
		LastGrabbedPhysicsLocation = FVector::ZeroVector;
		GrabbedPhysicsRelativeTransform = FTransform::Identity;
		GrabbedTargetBoneName = NAME_None;
		GrabAttachmentType = ESnowRumbleGrabAttachmentType::None;
		GrabAttachedWorldLocation = FVector::ZeroVector;
		GrabAttachmentStartedServerTime = 0.0f;
		GrabbedActorLocationOffsetFromAttachedPoint = FVector::ZeroVector;
		if (ASnowRumbleCharacter* OwnerCharacter = GetOwnerCharacter())
		{
			OwnerCharacter->HandleWorldGrabChanged(false);
			if (bWasWorldGrab)
			{
				if (UCharacterMovementComponent* MovementComponent =
					OwnerCharacter->GetCharacterMovement())
				{
					MovementComponent->Velocity.Z =
						FMath::Min(MovementComponent->Velocity.Z, 0.0f);
				}
			}
		}
	}
}

void UPlayerGrabComponent::MulticastPlayGrabSound_Implementation(
	FVector_NetQuantize Location,
	bool bReleased)
{
	SnowRumbleAudio::PlaySoundAtLocation(
		this,
		bReleased ? ReleaseGrabSound : GrabSound,
		ESnowRumbleAudioMixChannel::Gameplay,
		Location,
		1.0f,
		1.0f,
		GrabSoundAttenuation);
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

	GrabAttachedWorldLocation = BuildHandGrabAnchorLocation(ActiveGrabHand);
	if (GrabConstraintComponent)
	{
		GrabConstraintComponent->SetWorldLocation(GrabAttachedWorldLocation);
	}
	TargetCharacter->SetGrabbedByCharacterWorldLocationFromServer(
		GrabAttachedWorldLocation);
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
	FVector CorrectionVelocity = PullDirection * PullSpeed;
	CorrectionVelocity.Z = FMath::Min(
		CorrectionVelocity.Z,
		WorldGrabTetherMaxUpwardSpeed);
	MovementComponent->Velocity =
		MovementComponent->Velocity * WorldGrabInputVelocityRetention
		+ CorrectionVelocity;
	MovementComponent->Velocity = MovementComponent->Velocity.GetClampedToMaxSize(
		WorldGrabTetherMaxPullSpeed);
	Character->ForceNetUpdate();
}

void UPlayerGrabComponent::UpdatePhysicsObjectGrabTether(float DeltaTime)
{
	ASnowRumbleCharacter* Character = GetOwnerCharacter();
	if (!Character || !Character->HasAuthority())
	{
		return;
	}
	ApplyPhysicsObjectRotationMode();

	const FTransform DesiredWorldTransform =
		GrabbedPhysicsRelativeTransform * Character->GetActorTransform();
	GrabAttachedWorldLocation = DesiredWorldTransform.GetLocation();
	if (GrabPhysicsAnchorComponent)
	{
		GrabPhysicsAnchorComponent->SetWorldLocation(GrabAttachedWorldLocation);
	}
	if (GrabConstraintComponent)
	{
		GrabConstraintComponent->SetWorldLocation(GrabAttachedWorldLocation);
	}
	if (GrabbedPhysicsObject && GrabbedPhysicsComponent)
	{
		const FVector PreviousLocation = LastGrabbedPhysicsLocation;
		GrabbedPhysicsComponent->SetWorldTransform(
			DesiredWorldTransform,
			false,
			nullptr,
			ETeleportType::TeleportPhysics);
		LastGrabbedPhysicsLocation = GrabbedPhysicsComponent->GetComponentLocation();

		const FVector HeldMotion = LastGrabbedPhysicsLocation - PreviousLocation;
		GrabbedPhysicsObject->TickGrabbedByCharacter(
			Character,
			HeldMotion,
			DeltaTime);
		UWorld* World = GetWorld();
		if (World)
		{
			TArray<FOverlapResult> OverlapResults;
			FComponentQueryParams QueryParams(
				SCENE_QUERY_STAT(PlayerGrabHeldPush),
				Character);
			QueryParams.AddIgnoredActor(Character);
			QueryParams.AddIgnoredActor(GrabbedPhysicsObject);

			FCollisionObjectQueryParams ObjectQueryParams;
			ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

			if (World->ComponentOverlapMulti(
				OverlapResults,
				GrabbedPhysicsComponent,
				GrabbedPhysicsComponent->GetComponentLocation(),
				GrabbedPhysicsComponent->GetComponentQuat(),
				QueryParams,
				ObjectQueryParams))
			{
				for (const FOverlapResult& Overlap : OverlapResults)
				{
					ACharacter* HitCharacter = Cast<ACharacter>(Overlap.GetActor());
					if (HitCharacter && HitCharacter != Character)
					{
						GrabbedPhysicsObject->PushCharacterFromGrabMotion(
							HitCharacter,
							HeldMotion);
					}
				}
			}
		}
		GrabbedPhysicsObject->ForceNetUpdate();
	}
	Character->ForceNetUpdate();
}

void UPlayerGrabComponent::ApplyPhysicsObjectRotationMode()
{
	ASnowRumbleCharacter* Character = GetOwnerCharacter();
	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent =
		Character->GetCharacterMovement();
	if (!bHasPhysicsObjectRotationOverride)
	{
		bUseControllerRotationYawBeforePhysicsObjectGrab =
			Character->bUseControllerRotationYaw;
		bOrientRotationToMovementBeforePhysicsObjectGrab =
			MovementComponent
				? MovementComponent->bOrientRotationToMovement
				: true;
		bHasPhysicsObjectRotationOverride = true;
	}

	Character->bUseControllerRotationYaw = true;
	if (MovementComponent)
	{
		MovementComponent->bOrientRotationToMovement = false;
	}
}

void UPlayerGrabComponent::ClearPhysicsObjectRotationMode()
{
	if (!bHasPhysicsObjectRotationOverride)
	{
		return;
	}

	if (ASnowRumbleCharacter* Character = GetOwnerCharacter())
	{
		Character->bUseControllerRotationYaw =
			bUseControllerRotationYawBeforePhysicsObjectGrab;
		if (UCharacterMovementComponent* MovementComponent =
			Character->GetCharacterMovement())
		{
			MovementComponent->bOrientRotationToMovement =
				bOrientRotationToMovementBeforePhysicsObjectGrab;
		}
	}

	bHasPhysicsObjectRotationOverride = false;
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

FVector UPlayerGrabComponent::BuildHandGrabAnchorLocation(
	ESnowRumbleGrabHand Hand) const
{
	const ASnowRumbleCharacter* Character = GetOwnerCharacter();
	const USkeletalMeshComponent* OwnerMesh =
		Character ? Character->GetMesh() : nullptr;
	if (!OwnerMesh)
	{
		return BuildHandGrabTargetLocation(Hand);
	}

	const FName HandBoneName = Hand == ESnowRumbleGrabHand::Right
		? RightGrabHandBoneName
		: LeftGrabHandBoneName;
	if (HandBoneName == NAME_None)
	{
		return BuildHandGrabTargetLocation(Hand);
	}

	if (OwnerMesh->DoesSocketExist(HandBoneName)
		|| OwnerMesh->GetBoneIndex(HandBoneName) != INDEX_NONE)
	{
		return OwnerMesh->GetSocketLocation(HandBoneName);
	}

	return BuildHandGrabTargetLocation(Hand);
}

ASnowRumbleCharacter* UPlayerGrabComponent::GetOwnerCharacter() const
{
	return Cast<ASnowRumbleCharacter>(GetOwner());
}

float UPlayerGrabComponent::GetCurrentServerTimeSeconds() const
{
	const UWorld* World = GetWorld();
	const AGameStateBase* GameState = World
		? World->GetGameState()
		: nullptr;
	return GameState
		? GameState->GetServerWorldTimeSeconds()
		: World
			? World->GetTimeSeconds()
			: 0.0f;
}

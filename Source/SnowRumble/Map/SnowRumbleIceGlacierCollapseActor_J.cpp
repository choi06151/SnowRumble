// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleIceGlacierCollapseActor_J.h"

#include "../Game/SnowRumbleGameState_C.h"
#include "../Item/Campfire_C.h"
#include "../Item/GiftBox_C.h"
#include "../Item/GiftBoxItemPickup_C.h"
#include "../Snowball/SnowballItem.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Net/UnrealNetwork.h"

namespace
{
constexpr float IceGlacierCarryBoundsPaddingXY = 50.0f;
constexpr float IceGlacierCarryTraceStartOffset = 10.0f;
constexpr float IceGlacierCarryTraceDownDistance = 1200.0f;
constexpr float IceGlacierCarryMaximumSurfaceDistance = 200.0f;

enum class EIceCarryCandidateRejectReason : uint8
{
	None,
	Invalid,
	OwnerSelf,
	CharacterOrPawn,
	Attached,
	NoRoot,
	StaticRoot,
	WorldStaticPrimitive,
	NoValidPrimitive,
	SnowballState,
	UnsupportedClass
};

TSet<const AActor*> GIceCarryLoggedFirstMovePieces;

const TCHAR* IceCarryBoolText(bool bValue)
{
	return bValue ? TEXT("true") : TEXT("false");
}

const TCHAR* IceCarryRejectReasonText(EIceCarryCandidateRejectReason Reason)
{
	switch (Reason)
	{
	case EIceCarryCandidateRejectReason::Invalid:
		return TEXT("Invalid");
	case EIceCarryCandidateRejectReason::OwnerSelf:
		return TEXT("OwnerSelf");
	case EIceCarryCandidateRejectReason::CharacterOrPawn:
		return TEXT("CharacterOrPawn");
	case EIceCarryCandidateRejectReason::Attached:
		return TEXT("Attached");
	case EIceCarryCandidateRejectReason::NoRoot:
		return TEXT("NoRoot");
	case EIceCarryCandidateRejectReason::StaticRoot:
		return TEXT("StaticRoot");
	case EIceCarryCandidateRejectReason::WorldStaticPrimitive:
		return TEXT("WorldStaticPrimitive");
	case EIceCarryCandidateRejectReason::NoValidPrimitive:
		return TEXT("NoValidPrimitive");
	case EIceCarryCandidateRejectReason::SnowballState:
		return TEXT("SnowballState");
	case EIceCarryCandidateRejectReason::UnsupportedClass:
		return TEXT("UnsupportedClass");
	case EIceCarryCandidateRejectReason::None:
	default:
		return TEXT("None");
	}
}

EIceCarryCandidateRejectReason DiagnoseIceCarryCandidateRejectReason(
	const AActor* CandidateActor,
	const AActor* OwnerActor)
{
	if (!IsValid(CandidateActor))
	{
		return EIceCarryCandidateRejectReason::Invalid;
	}
	if (CandidateActor == OwnerActor)
	{
		return EIceCarryCandidateRejectReason::OwnerSelf;
	}
	if (Cast<ACharacter>(CandidateActor) || Cast<APawn>(CandidateActor))
	{
		return EIceCarryCandidateRejectReason::CharacterOrPawn;
	}
	if (CandidateActor->GetAttachParentActor())
	{
		return EIceCarryCandidateRejectReason::Attached;
	}

	const USceneComponent* CandidateRootComponent =
		CandidateActor->GetRootComponent();
	if (!CandidateRootComponent)
	{
		return EIceCarryCandidateRejectReason::NoRoot;
	}
	if (CandidateRootComponent->GetAttachParent())
	{
		return EIceCarryCandidateRejectReason::Attached;
	}

	const bool bRootMobilityStatic =
		CandidateRootComponent->Mobility == EComponentMobility::Static;
	bool bHasValidPrimitive = false;
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	CandidateActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (const UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent)
		{
			continue;
		}
		if (PrimitiveComponent->Mobility != EComponentMobility::Static &&
			PrimitiveComponent->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
		{
			bHasValidPrimitive = true;
		}
	}

	if (!bHasValidPrimitive)
	{
		if (bRootMobilityStatic)
		{
			return EIceCarryCandidateRejectReason::StaticRoot;
		}
		return EIceCarryCandidateRejectReason::NoValidPrimitive;
	}

	if (const ASnowballItem* Snowball = Cast<ASnowballItem>(CandidateActor))
	{
		return Snowball->CanBePickedUp()
			? EIceCarryCandidateRejectReason::None
			: EIceCarryCandidateRejectReason::SnowballState;
	}

	if (Cast<AGiftBox>(CandidateActor) ||
		Cast<AGiftBoxItemPickup>(CandidateActor) ||
		Cast<ACampfire>(CandidateActor))
	{
		return EIceCarryCandidateRejectReason::None;
	}

	return EIceCarryCandidateRejectReason::UnsupportedClass;
}
}

ASnowRumbleIceGlacierCollapseActor::ASnowRumbleIceGlacierCollapseActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false);

	RootSceneComponent =
		CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootSceneComponent);
}

void ASnowRumbleIceGlacierCollapseActor::BeginPlay()
{
	Super::BeginPlay();

	InitializeCollapseTargets();
	ApplyCollapseState(GetRoundElapsedSeconds());
}

void ASnowRumbleIceGlacierCollapseActor::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	StopLocalWarningCameraShake();

	Super::EndPlay(EndPlayReason);
}

void ASnowRumbleIceGlacierCollapseActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float RoundElapsedSeconds = GetRoundElapsedSeconds();
	if (HasAuthority())
	{
		const ESnowRumbleIceGlacierCollapsePhase NewPhase =
			CalculateCollapsePhase(RoundElapsedSeconds);
		if (CurrentCollapsePhase != NewPhase)
		{
			CurrentCollapsePhase = NewPhase;
			ForceNetUpdate();
		}
	}

	ApplyCollapseState(RoundElapsedSeconds);
	UpdateLocalWarningCameraShake(RoundElapsedSeconds);
}

void ASnowRumbleIceGlacierCollapseActor::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASnowRumbleIceGlacierCollapseActor, CurrentCollapsePhase);
}

ESnowRumbleIceGlacierCollapsePhase
ASnowRumbleIceGlacierCollapseActor::GetCurrentCollapsePhase() const
{
	return CurrentCollapsePhase;
}

float ASnowRumbleIceGlacierCollapseActor::GetGroup1Alpha() const
{
	return CalculateGroup1Alpha(GetRoundElapsedSeconds());
}

float ASnowRumbleIceGlacierCollapseActor::GetGroup2Alpha() const
{
	return CalculateGroup2Alpha(GetRoundElapsedSeconds());
}

void ASnowRumbleIceGlacierCollapseActor::OnRep_CollapsePhase()
{
	ApplyCollapseState(GetRoundElapsedSeconds());
}

void ASnowRumbleIceGlacierCollapseActor::InitializeCollapseTargets()
{
	Group1TargetStates.Reset();
	Group2TargetStates.Reset();

	InitializeCollapseGroup(Group1Pieces, Group1TargetStates);
	InitializeCollapseGroup(Group2Pieces, Group2TargetStates);
}

void ASnowRumbleIceGlacierCollapseActor::InitializeCollapseGroup(
	const TArray<FIceGlacierCollapsePiece>& SourcePieces,
	TArray<FIceGlacierCollapseTargetState>& OutTargetStates)
{
	for (const FIceGlacierCollapsePiece& Piece : SourcePieces)
	{
		AActor* TargetActor = Piece.TargetActor.Get();
		if (!IsValid(TargetActor))
		{
			continue;
		}

		FIceGlacierCollapseTargetState TargetState;
		TargetState.Actor = TargetActor;
		TargetState.PieceSettings.StartDelaySeconds = Piece.StartDelaySeconds;
		TargetState.PieceSettings.WarningDurationSeconds =
			Piece.WarningDurationSeconds;
		TargetState.PieceSettings.ShakeAmplitude = Piece.ShakeAmplitude;
		TargetState.PieceSettings.ShakeFrequency = Piece.ShakeFrequency;
		TargetState.PieceSettings.FallDurationSeconds = Piece.FallDurationSeconds;
		TargetState.PieceSettings.SinkDistance = Piece.SinkDistance;
		TargetState.InitialTransform = TargetActor->GetActorTransform();

		TArray<UPrimitiveComponent*> PrimitiveComponents;
		TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
		for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
		{
			if (!PrimitiveComponent)
			{
				continue;
			}

			FIceGlacierCollapseComponentCollisionState CollisionState;
			CollisionState.Component = PrimitiveComponent;
			CollisionState.CollisionEnabled =
				PrimitiveComponent->GetCollisionEnabled();
			TargetState.CollisionStates.Add(CollisionState);
		}

		OutTargetStates.Add(TargetState);
	}
}

ESnowRumbleIceGlacierCollapsePhase
ASnowRumbleIceGlacierCollapseActor::CalculateCollapsePhase(
	float RoundElapsedSeconds) const
{
	if (RoundElapsedSeconds < Group1StartSeconds)
	{
		return ESnowRumbleIceGlacierCollapsePhase::Stable;
	}
	if (RoundElapsedSeconds < Group1EndSeconds)
	{
		return ESnowRumbleIceGlacierCollapsePhase::Group1Sinking;
	}
	if (RoundElapsedSeconds < Group2StartSeconds)
	{
		return ESnowRumbleIceGlacierCollapsePhase::Group1Complete;
	}
	if (RoundElapsedSeconds < Group2EndSeconds)
	{
		return ESnowRumbleIceGlacierCollapsePhase::Group2Sinking;
	}

	return ESnowRumbleIceGlacierCollapsePhase::Complete;
}

float ASnowRumbleIceGlacierCollapseActor::CalculateAlpha(
	float RoundElapsedSeconds,
	float StartSeconds,
	float EndSeconds) const
{
	const float DurationSeconds =
		FMath::Max(KINDA_SMALL_NUMBER, EndSeconds - StartSeconds);
	return FMath::Clamp(
		(RoundElapsedSeconds - StartSeconds) / DurationSeconds,
		0.0f,
		1.0f);
}

float ASnowRumbleIceGlacierCollapseActor::CalculateGroup1Alpha(
	float RoundElapsedSeconds) const
{
	if (RoundElapsedSeconds < Group1StartSeconds)
	{
		return 0.0f;
	}

	switch (CurrentCollapsePhase)
	{
	case ESnowRumbleIceGlacierCollapsePhase::Group1Sinking:
		return CalculateAlpha(
			RoundElapsedSeconds,
			Group1StartSeconds,
			Group1EndSeconds);
	case ESnowRumbleIceGlacierCollapsePhase::Group1Complete:
	case ESnowRumbleIceGlacierCollapsePhase::Group2Sinking:
	case ESnowRumbleIceGlacierCollapsePhase::Complete:
		return 1.0f;
	case ESnowRumbleIceGlacierCollapsePhase::Stable:
	default:
		return 0.0f;
	}
}

float ASnowRumbleIceGlacierCollapseActor::CalculateGroup2Alpha(
	float RoundElapsedSeconds) const
{
	if (RoundElapsedSeconds < Group2StartSeconds)
	{
		return 0.0f;
	}

	switch (CurrentCollapsePhase)
	{
	case ESnowRumbleIceGlacierCollapsePhase::Group2Sinking:
		return CalculateAlpha(
			RoundElapsedSeconds,
			Group2StartSeconds,
			Group2EndSeconds);
	case ESnowRumbleIceGlacierCollapsePhase::Complete:
		return 1.0f;
	case ESnowRumbleIceGlacierCollapsePhase::Stable:
	case ESnowRumbleIceGlacierCollapsePhase::Group1Sinking:
	case ESnowRumbleIceGlacierCollapsePhase::Group1Complete:
	default:
		return 0.0f;
	}
}

void ASnowRumbleIceGlacierCollapseActor::ApplyCollapseState(
	float RoundElapsedSeconds)
{
	float Group1TimeSeconds = Group1StartSeconds - 1.0f;
	float Group2TimeSeconds = Group2StartSeconds - 1.0f;

	switch (CurrentCollapsePhase)
	{
	case ESnowRumbleIceGlacierCollapsePhase::Group1Sinking:
		Group1TimeSeconds = RoundElapsedSeconds;
		break;
	case ESnowRumbleIceGlacierCollapsePhase::Group1Complete:
		Group1TimeSeconds = Group1EndSeconds;
		break;
	case ESnowRumbleIceGlacierCollapsePhase::Group2Sinking:
		Group1TimeSeconds = Group1EndSeconds;
		Group2TimeSeconds = RoundElapsedSeconds;
		break;
	case ESnowRumbleIceGlacierCollapsePhase::Complete:
		Group1TimeSeconds = Group1EndSeconds;
		Group2TimeSeconds = Group2EndSeconds;
		break;
	case ESnowRumbleIceGlacierCollapsePhase::Stable:
	default:
		break;
	}

	ApplyCollapseGroup(
		Group1TargetStates,
		Group1TimeSeconds,
		Group1StartSeconds,
		Group1EndSeconds);
	ApplyCollapseGroup(
		Group2TargetStates,
		Group2TimeSeconds,
		Group2StartSeconds,
		Group2EndSeconds);
}

void ASnowRumbleIceGlacierCollapseActor::ApplyCollapseGroup(
	TArray<FIceGlacierCollapseTargetState>& TargetStates,
	float RoundElapsedSeconds,
	float GroupStartSeconds,
	float GroupEndSeconds)
{
	for (FIceGlacierCollapseTargetState& TargetState : TargetStates)
	{
		float WarningAlpha = 0.0f;
		float FallAlpha = 0.0f;
		const EIceGlacierCollapsePieceState PieceState = CalculatePieceState(
			TargetState.PieceSettings,
			RoundElapsedSeconds,
			GroupStartSeconds,
			GroupEndSeconds,
			WarningAlpha,
			FallAlpha);

		switch (PieceState)
		{
		case EIceGlacierCollapsePieceState::Warning:
		{
			const float WarningStartSeconds = GroupStartSeconds +
				FMath::Max(0.0f, TargetState.PieceSettings.StartDelaySeconds);
			const float WarningElapsedSeconds =
				FMath::Max(0.0f, RoundElapsedSeconds - WarningStartSeconds);
			ApplyWarningTransform(
				TargetState,
				WarningElapsedSeconds,
				WarningAlpha);
			RestoreTargetCollision(TargetState);
			break;
		}
		case EIceGlacierCollapsePieceState::Falling:
			if (HasAuthority())
			{
				UpdateFallingCarryActors(TargetState, FallAlpha);
			}
			ApplyFallingTransform(TargetState, FallAlpha);
			RestoreTargetCollision(TargetState);
			break;
		case EIceGlacierCollapsePieceState::Done:
			ApplyFallingTransform(TargetState, 1.0f);
			if (bDisableCollisionAfterFullySunk)
			{
				DisableTargetCollision(TargetState);
			}
			else
			{
				RestoreTargetCollision(TargetState);
			}
			break;
		case EIceGlacierCollapsePieceState::Idle:
		default:
			ApplyInitialTransform(TargetState);
			RestoreTargetCollision(TargetState);
			break;
		}

		if (HasAuthority() && PieceState != EIceGlacierCollapsePieceState::Falling)
		{
			ClearCarryActors(TargetState);
		}
		TargetState.PreviousPieceState = PieceState;
	}
}

EIceGlacierCollapsePieceState
ASnowRumbleIceGlacierCollapseActor::CalculatePieceState(
	const FIceGlacierCollapsePieceRuntimeSettings& PieceSettings,
	float RoundElapsedSeconds,
	float GroupStartSeconds,
	float GroupEndSeconds,
	float& OutWarningAlpha,
	float& OutFallAlpha) const
{
	OutWarningAlpha = 0.0f;
	OutFallAlpha = 0.0f;

	if (RoundElapsedSeconds >= GroupEndSeconds)
	{
		OutFallAlpha = 1.0f;
		return EIceGlacierCollapsePieceState::Done;
	}

	const float WarningStartSeconds = GroupStartSeconds +
		FMath::Max(0.0f, PieceSettings.StartDelaySeconds);
	if (RoundElapsedSeconds < WarningStartSeconds)
	{
		return EIceGlacierCollapsePieceState::Idle;
	}

	const float WarningDurationSeconds =
		FMath::Max(0.0f, PieceSettings.WarningDurationSeconds);
	const float WarningEndSeconds =
		WarningStartSeconds + WarningDurationSeconds;
	if (WarningDurationSeconds > KINDA_SMALL_NUMBER &&
		RoundElapsedSeconds < WarningEndSeconds)
	{
		OutWarningAlpha = FMath::Clamp(
			(RoundElapsedSeconds - WarningStartSeconds) /
				WarningDurationSeconds,
			0.0f,
			1.0f);
		return EIceGlacierCollapsePieceState::Warning;
	}

	OutWarningAlpha = 1.0f;

	const float FallDurationSeconds =
		FMath::Max(0.0f, PieceSettings.FallDurationSeconds);
	if (FallDurationSeconds <= KINDA_SMALL_NUMBER)
	{
		OutFallAlpha = 1.0f;
		return EIceGlacierCollapsePieceState::Done;
	}

	const float FallEndSeconds = WarningEndSeconds + FallDurationSeconds;
	if (RoundElapsedSeconds < FallEndSeconds)
	{
		OutFallAlpha = FMath::Clamp(
			(RoundElapsedSeconds - WarningEndSeconds) / FallDurationSeconds,
			0.0f,
			1.0f);
		return EIceGlacierCollapsePieceState::Falling;
	}

	OutFallAlpha = 1.0f;
	return EIceGlacierCollapsePieceState::Done;
}

void ASnowRumbleIceGlacierCollapseActor::ApplyWarningTransform(
	FIceGlacierCollapseTargetState& TargetState,
	float WarningElapsedSeconds,
	float WarningAlpha)
{
	AActor* TargetActor = TargetState.Actor.Get();
	if (!IsValid(TargetActor))
	{
		return;
	}

	const float ShakeAmplitude = TargetState.PieceSettings.ShakeAmplitude;
	const float ShakeFrequency = TargetState.PieceSettings.ShakeFrequency;
	if (ShakeAmplitude <= KINDA_SMALL_NUMBER ||
		ShakeFrequency <= KINDA_SMALL_NUMBER)
	{
		ApplyInitialTransform(TargetState);
		return;
	}

	const float ClampedWarningAlpha = FMath::Clamp(WarningAlpha, 0.0f, 1.0f);
	const float ShakeStrength = ClampedWarningAlpha * ClampedWarningAlpha;
	const float ShakePhase = WarningElapsedSeconds * ShakeFrequency * 2.0f * PI;

	FTransform NewTransform = TargetState.InitialTransform;
	FVector NewLocation = TargetState.InitialTransform.GetLocation();
	NewLocation.X += FMath::Sin(ShakePhase) * ShakeAmplitude * 0.65f * ShakeStrength;
	NewLocation.Y += FMath::Cos(ShakePhase * 1.31f) * ShakeAmplitude * 0.45f * ShakeStrength;
	NewTransform.SetLocation(NewLocation);

	const float RotationAmplitudeDegrees =
		FMath::Min(1.5f, ShakeAmplitude * 0.08f) * ShakeStrength;
	FRotator NewRotation = TargetState.InitialTransform.GetRotation().Rotator();
	NewRotation.Roll +=
		FMath::Sin(ShakePhase * 0.87f) * RotationAmplitudeDegrees;
	NewRotation.Pitch +=
		FMath::Cos(ShakePhase * 1.11f) * RotationAmplitudeDegrees;
	NewTransform.SetRotation(NewRotation.Quaternion());

	TargetActor->SetActorTransform(
		NewTransform,
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
}

FVector ASnowRumbleIceGlacierCollapseActor::CalculateFallingPieceLocation(
	const FIceGlacierCollapseTargetState& TargetState,
	float Alpha) const
{
	FVector NewLocation = TargetState.InitialTransform.GetLocation();
	NewLocation.Z -= TargetState.PieceSettings.SinkDistance *
		FMath::Clamp(Alpha, 0.0f, 1.0f);
	return NewLocation;
}

void ASnowRumbleIceGlacierCollapseActor::ApplyFallingTransform(
	FIceGlacierCollapseTargetState& TargetState,
	float Alpha)
{
	AActor* TargetActor = TargetState.Actor.Get();
	if (!IsValid(TargetActor))
	{
		return;
	}

	FTransform NewTransform = TargetState.InitialTransform;
	NewTransform.SetLocation(CalculateFallingPieceLocation(TargetState, Alpha));
	TargetActor->SetActorTransform(
		NewTransform,
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
}

void ASnowRumbleIceGlacierCollapseActor::ApplyInitialTransform(
	FIceGlacierCollapseTargetState& TargetState)
{
	AActor* TargetActor = TargetState.Actor.Get();
	if (!IsValid(TargetActor))
	{
		return;
	}

	TargetActor->SetActorTransform(
		TargetState.InitialTransform,
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
}

void ASnowRumbleIceGlacierCollapseActor::UpdateFallingCarryActors(
	FIceGlacierCollapseTargetState& TargetState,
	float FallAlpha)
{
	const FVector CurrentPieceLocation =
		CalculateFallingPieceLocation(TargetState, FallAlpha);
	if (TargetState.PreviousPieceState != EIceGlacierCollapsePieceState::Falling ||
		!TargetState.bCarryInitializedForFall)
	{
		InitializeCarryActorsForFallingPiece(TargetState, CurrentPieceLocation);
		return;
	}

	MoveCarriedActorsWithFallingPiece(TargetState, CurrentPieceLocation);
}

void ASnowRumbleIceGlacierCollapseActor::InitializeCarryActorsForFallingPiece(
	FIceGlacierCollapseTargetState& TargetState,
	const FVector& CurrentPieceLocation)
{
	ClearCarryActors(TargetState);
	TargetState.PreviousCarryPieceLocation = CurrentPieceLocation;
	TargetState.bCarryInitializedForFall = true;

	AActor* TargetActor = TargetState.Actor.Get();
	GIceCarryLoggedFirstMovePieces.Remove(TargetActor);
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[IceCarry] FallingEnter Piece=%s Authority=%s PieceLocation=%s"),
		*GetNameSafe(TargetActor),
		IceCarryBoolText(HasAuthority()),
		*CurrentPieceLocation.ToCompactString());

	UWorld* World = GetWorld();
	if (!IsValid(TargetActor) || !World)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[IceCarry] CarryCount=0 TargetPiece=%s Reason=InvalidTargetOrWorld"),
			*GetNameSafe(TargetActor));
		return;
	}

	FBox TargetBounds(ForceInit);
	if (!CalculateTargetActorBounds(TargetActor, TargetBounds))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[IceCarry] CarryCount=0 TargetPiece=%s Reason=InvalidTargetBounds"),
			*GetNameSafe(TargetActor));
		return;
	}
	const FBox ExpandedTargetBounds = TargetBounds.ExpandBy(
		FVector(IceGlacierCarryBoundsPaddingXY, IceGlacierCarryBoundsPaddingXY, 0.0f));

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[IceCarry] CandidateScan Begin TargetPiece=%s BoundsMin=%s BoundsMax=%s"),
		*GetNameSafe(TargetActor),
		*ExpandedTargetBounds.Min.ToCompactString(),
		*ExpandedTargetBounds.Max.ToCompactString());

	int32 ScannedCandidateCount = 0;
	const auto TryAddCarryCandidate =
		[this,
		 TargetActor,
		 &TargetState,
		 &ExpandedTargetBounds,
		 &ScannedCandidateCount](AActor* CandidateActor)
	{
		++ScannedCandidateCount;
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[IceCarry] Candidate=%s Class=%s TargetPiece=%s"),
			*GetNameSafe(CandidateActor),
			*GetNameSafe(CandidateActor ? CandidateActor->GetClass() : nullptr),
			*GetNameSafe(TargetActor));

		if (!ShouldCarryCandidateActor(CandidateActor))
		{
			const EIceCarryCandidateRejectReason RejectReason =
				DiagnoseIceCarryCandidateRejectReason(CandidateActor, this);
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[IceCarry] Reject: %s Candidate=%s TargetPiece=%s"),
				IceCarryRejectReasonText(RejectReason),
				*GetNameSafe(CandidateActor),
				*GetNameSafe(TargetActor));
			return;
		}

		if (CandidateActor == TargetActor)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[IceCarry] Reject: TargetActorSelf Candidate=%s TargetPiece=%s"),
				*GetNameSafe(CandidateActor),
				*GetNameSafe(TargetActor));
			return;
		}

		const FVector CandidateLocation = CandidateActor->GetActorLocation();
		if (CandidateLocation.X < ExpandedTargetBounds.Min.X ||
			CandidateLocation.X > ExpandedTargetBounds.Max.X ||
			CandidateLocation.Y < ExpandedTargetBounds.Min.Y ||
			CandidateLocation.Y > ExpandedTargetBounds.Max.Y)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[IceCarry] Reject: XYBounds Candidate=%s TargetPiece=%s Location=%s BoundsMin=%s BoundsMax=%s"),
				*GetNameSafe(CandidateActor),
				*GetNameSafe(TargetActor),
				*CandidateLocation.ToCompactString(),
				*ExpandedTargetBounds.Min.ToCompactString(),
				*ExpandedTargetBounds.Max.ToCompactString());
			return;
		}

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[IceCarry] XYBoundsPass Candidate=%s TargetPiece=%s Location=%s"),
			*GetNameSafe(CandidateActor),
			*GetNameSafe(TargetActor),
			*CandidateLocation.ToCompactString());

		if (!IsCandidateStandingOnTargetActorByTrace(CandidateActor, TargetActor))
		{
			return;
		}

		for (const TWeakObjectPtr<AActor>& CarriedActor : TargetState.CarriedActors)
		{
			if (CarriedActor.Get() == CandidateActor)
			{
				return;
			}
		}
		TargetState.CarriedActors.Add(CandidateActor);
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[IceCarry] ACCEPT TargetPiece=%s Candidate=%s CarryCount=%d"),
			*GetNameSafe(TargetActor),
			*GetNameSafe(CandidateActor),
			TargetState.CarriedActors.Num());
	};

	for (TActorIterator<AGiftBox> Iterator(World); Iterator; ++Iterator)
	{
		TryAddCarryCandidate(*Iterator);
	}
	for (TActorIterator<AGiftBoxItemPickup> Iterator(World); Iterator; ++Iterator)
	{
		TryAddCarryCandidate(*Iterator);
	}
	for (TActorIterator<ACampfire> Iterator(World); Iterator; ++Iterator)
	{
		TryAddCarryCandidate(*Iterator);
	}
	for (TActorIterator<ASnowballItem> Iterator(World); Iterator; ++Iterator)
	{
		TryAddCarryCandidate(*Iterator);
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[IceCarry] CarryCount=%d TargetPiece=%s ScannedCandidates=%d"),
		TargetState.CarriedActors.Num(),
		*GetNameSafe(TargetActor),
		ScannedCandidateCount);
}

void ASnowRumbleIceGlacierCollapseActor::MoveCarriedActorsWithFallingPiece(
	FIceGlacierCollapseTargetState& TargetState,
	const FVector& CurrentPieceLocation)
{
	AActor* TargetActor = TargetState.Actor.Get();
	const FVector DeltaLocation =
		CurrentPieceLocation - TargetState.PreviousCarryPieceLocation;
	const bool bLogFirstMoveAttempt =
		TargetActor && !GIceCarryLoggedFirstMovePieces.Contains(TargetActor);
	if (bLogFirstMoveAttempt)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[IceCarry] FirstDeltaMoveAttempt TargetPiece=%s CarryCount=%d Delta=%s DeltaNearlyZero=%s PreviousPieceLocation=%s CurrentPieceLocation=%s"),
			*GetNameSafe(TargetActor),
			TargetState.CarriedActors.Num(),
			*DeltaLocation.ToCompactString(),
			IceCarryBoolText(DeltaLocation.IsNearlyZero()),
			*TargetState.PreviousCarryPieceLocation.ToCompactString(),
			*CurrentPieceLocation.ToCompactString());
		GIceCarryLoggedFirstMovePieces.Add(TargetActor);
	}

	for (int32 Index = TargetState.CarriedActors.Num() - 1; Index >= 0; --Index)
	{
		AActor* CarriedActor = TargetState.CarriedActors[Index].Get();
		if (!IsValid(CarriedActor))
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[IceCarry] Remove: InvalidCarriedActor TargetPiece=%s Index=%d"),
				*GetNameSafe(TargetActor),
				Index);
			TargetState.CarriedActors.RemoveAtSwap(Index);
			continue;
		}

		if (!ShouldKeepCarryingActor(CarriedActor))
		{
			const EIceCarryCandidateRejectReason RejectReason =
				DiagnoseIceCarryCandidateRejectReason(CarriedActor, this);
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[IceCarry] Remove: StateChanged Candidate=%s TargetPiece=%s Reason=%s"),
				*GetNameSafe(CarriedActor),
				*GetNameSafe(TargetActor),
				IceCarryRejectReasonText(RejectReason));
			TargetState.CarriedActors.RemoveAtSwap(Index);
			continue;
		}

		if (!DeltaLocation.IsNearlyZero())
		{
			if (bLogFirstMoveAttempt)
			{
				UE_LOG(
					LogTemp,
					Warning,
					TEXT("[IceCarry] AddActorWorldOffset Attempt Candidate=%s TargetPiece=%s Delta=%s"),
					*GetNameSafe(CarriedActor),
					*GetNameSafe(TargetActor),
					*DeltaLocation.ToCompactString());
			}
			CarriedActor->AddActorWorldOffset(
				DeltaLocation,
				false,
				nullptr,
				ETeleportType::TeleportPhysics);
			CarriedActor->ForceNetUpdate();
		}
	}

	TargetState.PreviousCarryPieceLocation = CurrentPieceLocation;
}

void ASnowRumbleIceGlacierCollapseActor::ClearCarryActors(
	FIceGlacierCollapseTargetState& TargetState)
{
	TargetState.CarriedActors.Reset();
	TargetState.PreviousCarryPieceLocation = FVector::ZeroVector;
	TargetState.bCarryInitializedForFall = false;
}

bool ASnowRumbleIceGlacierCollapseActor::CalculateTargetActorBounds(
	const AActor* TargetActor,
	FBox& OutBounds) const
{
	OutBounds = FBox(ForceInit);
	if (!IsValid(TargetActor))
	{
		return false;
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (const UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent)
		{
			continue;
		}

		const FBox ComponentBox = PrimitiveComponent->Bounds.GetBox();
		if (ComponentBox.IsValid)
		{
			OutBounds += ComponentBox;
		}
	}

	return OutBounds.IsValid != 0;
}

bool ASnowRumbleIceGlacierCollapseActor::ShouldCarryCandidateActor(
	const AActor* CandidateActor) const
{
	return DiagnoseIceCarryCandidateRejectReason(CandidateActor, this) ==
		EIceCarryCandidateRejectReason::None;
}

bool ASnowRumbleIceGlacierCollapseActor::ShouldKeepCarryingActor(
	const AActor* CandidateActor) const
{
	return ShouldCarryCandidateActor(CandidateActor);
}

FVector ASnowRumbleIceGlacierCollapseActor::GetCarryTraceStartLocation(
	const AActor* CandidateActor) const
{
	if (!CandidateActor)
	{
		return FVector::ZeroVector;
	}

	FVector TraceStart = CandidateActor->GetActorLocation();
	float BestBottomZ = -TNumericLimits<float>::Max();
	bool bFoundCollisionBounds = false;

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	CandidateActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (const UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent ||
			PrimitiveComponent->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
		{
			continue;
		}

		const FBox ComponentBox = PrimitiveComponent->Bounds.GetBox();
		if (!ComponentBox.IsValid)
		{
			continue;
		}

		const bool bActorLocationInsideComponentXY =
			TraceStart.X >= ComponentBox.Min.X - IceGlacierCarryBoundsPaddingXY &&
			TraceStart.X <= ComponentBox.Max.X + IceGlacierCarryBoundsPaddingXY &&
			TraceStart.Y >= ComponentBox.Min.Y - IceGlacierCarryBoundsPaddingXY &&
			TraceStart.Y <= ComponentBox.Max.Y + IceGlacierCarryBoundsPaddingXY;
		if (!bActorLocationInsideComponentXY)
		{
			continue;
		}

		BestBottomZ = FMath::Max(BestBottomZ, ComponentBox.Min.Z);
		bFoundCollisionBounds = true;
	}

	if (bFoundCollisionBounds)
	{
		TraceStart.Z = FMath::Max(TraceStart.Z, BestBottomZ);
	}
	TraceStart.Z += IceGlacierCarryTraceStartOffset;
	return TraceStart;
}

bool ASnowRumbleIceGlacierCollapseActor::IsCandidateStandingOnTargetActorByTrace(
	const AActor* CandidateActor,
	const AActor* TargetActor) const
{
	UWorld* World = GetWorld();
	if (!World || !IsValid(CandidateActor) || !IsValid(TargetActor))
	{
		return false;
	}

	const FVector TraceStart = GetCarryTraceStartLocation(CandidateActor);
	const FVector TraceEnd =
		TraceStart - FVector::UpVector * IceGlacierCarryTraceDownDistance;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[IceCarry] TraceStart=%s TraceEnd=%s Channel=Visibility Candidate=%s TargetPiece=%s"),
		*TraceStart.ToCompactString(),
		*TraceEnd.ToCompactString(),
		*GetNameSafe(CandidateActor),
		*GetNameSafe(TargetActor));

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(IceGlacierCarryTrace), false);
	QueryParams.AddIgnoredActor(CandidateActor);
	QueryParams.AddIgnoredActor(this);

	TArray<FHitResult> Hits;
	if (!World->LineTraceMultiByChannel(
		Hits,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[IceCarry] TraceHit=None Candidate=%s TargetPiece=%s LineTraceHit=false"),
			*GetNameSafe(CandidateActor),
			*GetNameSafe(TargetActor));
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[IceCarry] Reject: FloorHitOtherActor Candidate=%s TargetPiece=%s TargetActorMatch=false"),
			*GetNameSafe(CandidateActor),
			*GetNameSafe(TargetActor));
		return false;
	}

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!Hit.bBlockingHit || !IsValid(HitActor) ||
			HitActor == CandidateActor || HitActor == this ||
			Cast<ACharacter>(HitActor) || Cast<APawn>(HitActor))
		{
			continue;
		}

		const bool bTargetActorMatch = HitActor == TargetActor;
		const bool bWithinMaximumSurfaceDistance =
			Hit.Distance <= IceGlacierCarryMaximumSurfaceDistance;
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[IceCarry] TraceHit=%s Class=%s Distance=%.1f TargetActorMatch=%s WithinMaxDistance=%s Candidate=%s TargetPiece=%s"),
			*GetNameSafe(HitActor),
			*GetNameSafe(HitActor->GetClass()),
			Hit.Distance,
			IceCarryBoolText(bTargetActorMatch),
			IceCarryBoolText(bWithinMaximumSurfaceDistance),
			*GetNameSafe(CandidateActor),
			*GetNameSafe(TargetActor));

		if (!bTargetActorMatch || !bWithinMaximumSurfaceDistance)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[IceCarry] Reject: FloorHitOtherActor Candidate=%s TargetPiece=%s TargetActorMatch=%s WithinMaxDistance=%s"),
				*GetNameSafe(CandidateActor),
				*GetNameSafe(TargetActor),
				IceCarryBoolText(bTargetActorMatch),
				IceCarryBoolText(bWithinMaximumSurfaceDistance));
		}
		return bTargetActorMatch && bWithinMaximumSurfaceDistance;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[IceCarry] TraceHit=None Candidate=%s TargetPiece=%s ValidBlockingHit=false"),
		*GetNameSafe(CandidateActor),
		*GetNameSafe(TargetActor));
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[IceCarry] Reject: FloorHitOtherActor Candidate=%s TargetPiece=%s TargetActorMatch=false"),
		*GetNameSafe(CandidateActor),
		*GetNameSafe(TargetActor));
	return false;
}

void ASnowRumbleIceGlacierCollapseActor::UpdateLocalWarningCameraShake(
	float RoundElapsedSeconds)
{
	float WarningAlpha = 0.0f;
	AActor* WarningTargetActor = FindWarningPieceUnderLocalPlayer(
		RoundElapsedSeconds,
		WarningAlpha);
	AActor* ActiveTargetActor = ActiveWarningCameraShakeTargetActor.Get();
	if (ActiveTargetActor && ActiveTargetActor == WarningTargetActor)
	{
		if (UCameraShakeBase* CameraShakeInstance =
			ActiveWarningCameraShakeInstance.Get())
		{
			CameraShakeInstance->ShakeScale =
				CalculateWarningCameraShakeScale(WarningAlpha);
			return;
		}
	}

	if (ActiveTargetActor || ActiveWarningCameraShakeInstance.IsValid())
	{
		StopLocalWarningCameraShake();
	}

	if (WarningTargetActor)
	{
		StartLocalWarningCameraShake(WarningTargetActor, WarningAlpha);
	}
}

float ASnowRumbleIceGlacierCollapseActor::CalculateWarningCameraShakeScale(
	float WarningAlpha) const
{
	const float ClampedWarningAlpha = FMath::Clamp(WarningAlpha, 0.0f, 1.0f);
	const float CameraShakeStrength =
		FMath::Lerp(
		0.3f,
		1.0f,
		ClampedWarningAlpha * ClampedWarningAlpha
	);
	return WarningCameraShakeScale * CameraShakeStrength;
}

AActor* ASnowRumbleIceGlacierCollapseActor::FindWarningPieceUnderLocalPlayer(
	float RoundElapsedSeconds,
	float& OutWarningAlpha) const
{
	OutWarningAlpha = 0.0f;

	UWorld* World = GetWorld();
	APlayerController* PlayerController = World
		? World->GetFirstPlayerController()
		: nullptr;
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return nullptr;
	}

	const ACharacter* Character = Cast<ACharacter>(PlayerController->GetPawn());
	if (!Character || !Character->IsLocallyControlled())
	{
		return nullptr;
	}

	const auto FindWarningTargetActor =
		[this, Character, RoundElapsedSeconds, &OutWarningAlpha](
			const TArray<FIceGlacierCollapseTargetState>& TargetStates,
			float GroupStartSeconds,
			float GroupEndSeconds) -> AActor*
	{
		for (const FIceGlacierCollapseTargetState& TargetState : TargetStates)
		{
			AActor* TargetActor = TargetState.Actor.Get();
			if (!IsValid(TargetActor))
			{
				continue;
			}

			float WarningAlpha = 0.0f;
			float FallAlpha = 0.0f;
			const EIceGlacierCollapsePieceState PieceState = CalculatePieceState(
				TargetState.PieceSettings,
				RoundElapsedSeconds,
				GroupStartSeconds,
				GroupEndSeconds,
				WarningAlpha,
				FallAlpha);
			if (PieceState == EIceGlacierCollapsePieceState::Warning &&
				IsLocalCharacterStandingOnTargetActor(Character, TargetActor))
			{
				OutWarningAlpha = WarningAlpha;
				return TargetActor;
			}
		}

		return nullptr;
	};

	if (AActor* Group1TargetActor = FindWarningTargetActor(
		Group1TargetStates,
		Group1StartSeconds,
		Group1EndSeconds))
	{
		return Group1TargetActor;
	}

	return FindWarningTargetActor(
		Group2TargetStates,
		Group2StartSeconds,
		Group2EndSeconds);
}

bool ASnowRumbleIceGlacierCollapseActor::IsLocalCharacterStandingOnTargetActor(
	const ACharacter* Character,
	const AActor* TargetActor) const
{
	if (!Character || !IsValid(TargetActor))
	{
		return false;
	}

	const UCharacterMovementComponent* MovementComponent =
		Character->GetCharacterMovement();
	const UPrimitiveComponent* MovementBase = MovementComponent
		? MovementComponent->GetMovementBase()
		: nullptr;
	return MovementBase && MovementBase->GetOwner() == TargetActor;
}

void ASnowRumbleIceGlacierCollapseActor::StartLocalWarningCameraShake(
	AActor* TargetActor,
	float WarningAlpha)
{
	if (!IsValid(TargetActor) || !WarningCameraShakeClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	APlayerController* PlayerController = World
		? World->GetFirstPlayerController()
		: nullptr;
	APlayerCameraManager* PlayerCameraManager = PlayerController
		? PlayerController->PlayerCameraManager
		: nullptr;
	if (!PlayerController || !PlayerController->IsLocalController() ||
		!PlayerCameraManager)
	{
		return;
	}

	const float InitialCameraShakeScale =
		CalculateWarningCameraShakeScale(WarningAlpha);
	UCameraShakeBase* CameraShakeInstance =
		PlayerCameraManager->StartCameraShake(
			WarningCameraShakeClass,
			InitialCameraShakeScale);
	ActiveWarningCameraShakeInstance = CameraShakeInstance;
	if (CameraShakeInstance)
	{
		ActiveWarningCameraShakeTargetActor = TargetActor;
	}
	else
	{
		ActiveWarningCameraShakeTargetActor.Reset();
	}
}

void ASnowRumbleIceGlacierCollapseActor::StopLocalWarningCameraShake()
{
	UCameraShakeBase* CameraShakeInstance =
		ActiveWarningCameraShakeInstance.Get();
	if (CameraShakeInstance)
	{
		UWorld* World = GetWorld();
		APlayerController* PlayerController = World
			? World->GetFirstPlayerController()
			: nullptr;
		APlayerCameraManager* PlayerCameraManager = PlayerController
			? PlayerController->PlayerCameraManager
			: nullptr;
		if (PlayerController && PlayerController->IsLocalController() &&
			PlayerCameraManager)
		{
			PlayerCameraManager->StopCameraShake(CameraShakeInstance, true);
		}
	}

	ActiveWarningCameraShakeInstance.Reset();
	ActiveWarningCameraShakeTargetActor.Reset();
}

void ASnowRumbleIceGlacierCollapseActor::DisableTargetCollision(
	FIceGlacierCollapseTargetState& TargetState)
{
	if (TargetState.bCollisionDisabledAfterSink)
	{
		return;
	}

	for (const FIceGlacierCollapseComponentCollisionState& CollisionState :
		TargetState.CollisionStates)
	{
		if (UPrimitiveComponent* PrimitiveComponent =
			CollisionState.Component.Get())
		{
			PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	TargetState.bCollisionDisabledAfterSink = true;
}

void ASnowRumbleIceGlacierCollapseActor::RestoreTargetCollision(
	FIceGlacierCollapseTargetState& TargetState)
{
	if (!TargetState.bCollisionDisabledAfterSink)
	{
		return;
	}

	for (const FIceGlacierCollapseComponentCollisionState& CollisionState :
		TargetState.CollisionStates)
	{
		if (UPrimitiveComponent* PrimitiveComponent =
			CollisionState.Component.Get())
		{
			PrimitiveComponent->SetCollisionEnabled(
				CollisionState.CollisionEnabled);
		}
	}

	TargetState.bCollisionDisabledAfterSink = false;
}

float ASnowRumbleIceGlacierCollapseActor::GetRoundElapsedSeconds() const
{
	UWorld* World = GetWorld();
	const ASnowRumbleGameState* SnowRumbleGameState = World
		? World->GetGameState<ASnowRumbleGameState>()
		: nullptr;
	return SnowRumbleGameState
		? SnowRumbleGameState->GetRoundElapsedSeconds()
		: 0.0f;
}

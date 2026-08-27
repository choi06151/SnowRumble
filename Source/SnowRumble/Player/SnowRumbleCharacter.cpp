// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleCharacter.h"

#include "SnowRumbleHealthComponent.h"
#include "../Audio/SnowRumbleAudioHelpers.h"
#include "../Game/SnowmanModeGameState_K.h"
#include "../Game/SnowRumbleGameState_C.h"
#include "../Game/SnowRumbleLobbyGameState.h"
#include "../Game/SnowRumbleMatchSubsystem_C.h"
#include "../Game/SnowRumblePlayerState.h"
#include "../Environment/SnowTrailRenderTargetManager_C.h"
#include "../Interaction/LobbyInteractionBoard_C.h"
#include "../Interaction/PhotoInteractionActor_C.h"
#include "../Interaction/JukeboxActor_C.h"
#include "../Interaction/OutlineComponent.h"
#include "../Item/GiftBox_C.h"
#include "../Item/GiftBoxItemPickup_C.h"
#include "../Item/GiftItemEffectComponent_C.h"
#include "PlayerGrabComponent_C.h"
#include "SnowRumbleCharacterMovementComponent_C.h"
#include "../Snowball/SnowballCreationComponent.h"
#include "../Snowball/SnowballDamageTypes.h"
#include "../Snowball/SnowballEquipmentComponent.h"
#include "../Snowball/SnowballItem.h"
#include "../UI/EmoteRadialMenuWidget.h"
#include "../UI/CustomizationPlayerController_C.h"
#include "../UI/DamageTextWidget_C.h"
#include "../UI/InteractionPromptWidget_C.h"
#include "../UI/KeyGuideWidget_C.h"
#include "../UI/MainHUDWidget.h"
#include "../UI/OverheadTimedActionWidget.h"
#include "../UI/SpectatorWidget_C.h"
#include "../UI/MainMenuPlayerController.h"
#include "../Game/PodiumPlayerController.h"
#include "../UI/OverheadNameplateWidget_C.h"
#include "../UI/SnowRumblePlayerController.h"
#include "SnowRumbleCharacterAnimInstance_C.h"
#include "Animation/AnimMontage.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Components/WidgetComponent.h"
#include "DrawDebugHelpers.h"
#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Canvas.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "Engine/DamageEvents.h"
#include "EngineUtils.h"
#include "Engine/GameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/DateTime.h"
#include "UnrealClient.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"
#include "Blueprint/UserWidget.h"
#include "SnowRumbleUserSettingsSubsystem_C.h"

DEFINE_LOG_CATEGORY_STATIC(LogSnowTrailCharacter, Log, All);

namespace
{
FTransform ResolveCustomizationAccessoryTransform(
	const TArray<FTransform>& RelativeTransformOverrides,
	int32 MeshIndex,
	const FVector& DefaultLocation,
	const FRotator& DefaultRotation,
	const FVector& DefaultScale)
{
	if (RelativeTransformOverrides.IsValidIndex(MeshIndex))
	{
		return RelativeTransformOverrides[MeshIndex];
	}

	FTransform DefaultTransform;
	DefaultTransform.SetLocation(DefaultLocation);
	DefaultTransform.SetRotation(DefaultRotation.Quaternion());
	DefaultTransform.SetScale3D(DefaultScale);
	return DefaultTransform;
}
}

ASnowRumbleCharacter::ASnowRumbleCharacter(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<
		USnowRumbleCharacterMovementComponent_C>(
		ACharacter::CharacterMovementComponentName))
{
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.0f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	SetNetUpdateFrequency(60.0f);
	SetMinNetUpdateFrequency(30.0f);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = true;
	CameraBoom->ProbeSize = 12.0f;
	CameraBoom->TargetOffset = FVector(0.0f, 0.0f, CameraPivotHeight);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	LocalSnowEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LocalSnowEffect"));
	LocalSnowEffect->SetupAttachment(FollowCamera);
	LocalSnowEffect->SetAutoActivate(false);
	LocalSnowEffect->SetOnlyOwnerSee(true);
	LocalSnowEffect->SetVisibility(false);

	HealthComponent = CreateDefaultSubobject<USnowRumbleHealthComponent>(TEXT("HealthComponent"));

	GiftItemEffectComponent =
		CreateDefaultSubobject<UGiftItemEffectComponent>(
			TEXT("GiftItemEffectComponent"));

	SnowballEquipmentComponent =
		CreateDefaultSubobject<USnowballEquipmentComponent>(TEXT("SnowballEquipmentComponent"));

	SnowballCreationComponent =
		CreateDefaultSubobject<USnowballCreationComponent>(TEXT("SnowballCreationComponent"));

	PlayerGrabComponent =
		CreateDefaultSubobject<UPlayerGrabComponent>(TEXT("PlayerGrabComponent"));

	RollingSnowballCollision =
		CreateDefaultSubobject<USphereComponent>(TEXT("RollingSnowballCollision"));
	RollingSnowballCollision->SetupAttachment(RootComponent);
	RollingSnowballCollision->SetUsingAbsoluteLocation(true);
	RollingSnowballCollision->InitSphereRadius(18.0f);
	RollingSnowballCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RollingSnowballCollision->SetCollisionObjectType(ECC_WorldDynamic);
	RollingSnowballCollision->SetCollisionResponseToAllChannels(ECR_Block);
	RollingSnowballCollision->SetGenerateOverlapEvents(false);

	SnowballHoldPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SnowballHoldPoint"));
	SnowballHoldPoint->SetupAttachment(GetMesh(), TEXT("SnowballSocket"));

	LargeSnowballHoldPoint =
		CreateDefaultSubobject<USceneComponent>(TEXT("LargeSnowballHoldPoint"));
	LargeSnowballHoldPoint->SetupAttachment(GetMesh(), TEXT("LargeSnowballSocket"));

	SnowDuckBallHoldPoint =
		CreateDefaultSubobject<USceneComponent>(TEXT("SnowDuckBallHoldPoint"));
	SnowDuckBallHoldPoint->SetupAttachment(GetMesh(), TEXT("SnowDuckBallSocket"));

	OutlineComponent = CreateDefaultSubobject<UOutlineComponent>(TEXT("OutlineComponent"));

	LobbyBoardWidgetInteractionComponent =
		CreateDefaultSubobject<UWidgetInteractionComponent>(
			TEXT("LobbyBoardWidgetInteractionComponent"));
	LobbyBoardWidgetInteractionComponent->SetupAttachment(FollowCamera);
	LobbyBoardWidgetInteractionComponent->InteractionSource =
		EWidgetInteractionSource::Custom;
	LobbyBoardWidgetInteractionComponent->InteractionDistance = 3000.0f;
	LobbyBoardWidgetInteractionComponent->TraceChannel = ECC_Visibility;
	LobbyBoardWidgetInteractionComponent->bEnableHitTesting = true;
	LobbyBoardWidgetInteractionComponent->SetActive(false);

	OverheadNameplateComponent =
		CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadNameplateComponent"));
	OverheadNameplateComponent->SetupAttachment(RootComponent);
	OverheadNameplateComponent->SetWidgetSpace(EWidgetSpace::World);
	OverheadNameplateComponent->SetDrawAtDesiredSize(true);
	OverheadNameplateComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HatMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HatMeshComponent"));
	HatMeshComponent->SetupAttachment(GetMesh(), TEXT("HatSocket"));
	HatMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HatMeshComponent->SetGenerateOverlapEvents(false);
	HatMeshComponent->SetVisibility(false);

	GlassesMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GlassesMeshComponent"));
	GlassesMeshComponent->SetupAttachment(GetMesh(), TEXT("GlassesSocket"));
	GlassesMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GlassesMeshComponent->SetGenerateOverlapEvents(false);
	GlassesMeshComponent->SetVisibility(false);

	NoseMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NoseMeshComponent"));
	NoseMeshComponent->SetupAttachment(GetMesh(), TEXT("NoseSocket"));
	NoseMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NoseMeshComponent->SetGenerateOverlapEvents(false);
	NoseMeshComponent->SetVisibility(false);

	EarmuffsMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EarmuffsMeshComponent"));
	EarmuffsMeshComponent->SetupAttachment(GetMesh(), TEXT("EarmuffsSocket"));
	EarmuffsMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EarmuffsMeshComponent->SetGenerateOverlapEvents(false);
	EarmuffsMeshComponent->SetVisibility(false);

	ScarfMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScarfMeshComponent"));
	ScarfMeshComponent->SetupAttachment(GetMesh(), TEXT("ScarfSocket"));
	ScarfMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ScarfMeshComponent->SetGenerateOverlapEvents(false);
	ScarfMeshComponent->SetVisibility(false);

	LeftBootsMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(
			TEXT("LeftBootsMeshComponent"));
	LeftBootsMeshComponent->SetupAttachment(GetMesh(), TEXT("LeftBootsSocket"));
	LeftBootsMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftBootsMeshComponent->SetGenerateOverlapEvents(false);
	LeftBootsMeshComponent->SetVisibility(false);

	RightBootsMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(
			TEXT("RightBootsMeshComponent"));
	RightBootsMeshComponent->SetupAttachment(GetMesh(), TEXT("RightBootsSocket"));
	RightBootsMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightBootsMeshComponent->SetGenerateOverlapEvents(false);
	RightBootsMeshComponent->SetVisibility(false);

	LeftGlovesMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(
			TEXT("LeftGlovesMeshComponent"));
	LeftGlovesMeshComponent->SetupAttachment(GetMesh(), TEXT("LeftGlovesSocket"));
	LeftGlovesMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftGlovesMeshComponent->SetGenerateOverlapEvents(false);
	LeftGlovesMeshComponent->SetVisibility(false);

	RightGlovesMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(
			TEXT("RightGlovesMeshComponent"));
	RightGlovesMeshComponent->SetupAttachment(GetMesh(), TEXT("RightGlovesSocket"));
	RightGlovesMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightGlovesMeshComponent->SetGenerateOverlapEvents(false);
	RightGlovesMeshComponent->SetVisibility(false);

	PaddingMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PaddingMeshComponent"));
	PaddingMeshComponent->SetupAttachment(GetMesh(), TEXT("PaddingSocket"));
	PaddingMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PaddingMeshComponent->SetGenerateOverlapEvents(false);
	PaddingMeshComponent->SetVisibility(false);

	HotPackMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HotPackMeshComponent"));
	HotPackMeshComponent->SetupAttachment(GetMesh(), TEXT("HotPackSocket"));
	HotPackMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HotPackMeshComponent->SetGenerateOverlapEvents(false);
	HotPackMeshComponent->SetVisibility(false);

	ShovelMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShovelMeshComponent"));
	ShovelMeshComponent->SetupAttachment(GetMesh(), TEXT("ShovelSocket"));
	ShovelMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShovelMeshComponent->SetGenerateOverlapEvents(false);
	ShovelMeshComponent->SetVisibility(false);

	DuckMakerMeshComponent =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DuckMakerMeshComponent"));
	DuckMakerMeshComponent->SetupAttachment(GetMesh(), TEXT("DuckMakerSocket"));
	DuckMakerMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DuckMakerMeshComponent->SetGenerateOverlapEvents(false);
	DuckMakerMeshComponent->SetVisibility(false);
}

void ASnowRumbleCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateIceGlacierMovementSurface();
	EnsureOverheadTimedActionWidget();

	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		if (bIsPickingUpItem)
		{
			SetActorRotation(PickupLockedRotation);

			if (IsLocallyControlled())
			{
				if (AController* LocalController = GetController())
				{
					FRotator LockedControlRotation =
						LocalController->GetControlRotation();
					LockedControlRotation.Yaw =
						PickupLockedControlRotation.Yaw;
					LocalController->SetControlRotation(LockedControlRotation);
				}
			}
		}

		FVector TargetMeshRelativeLocation = DefaultCharacterMeshRelativeLocation;
		bool bHasRollingSurface = false;
		bool bIsRollingOnSnowSurface = false;
		if (SnowballEquipmentComponent
			&& SnowballEquipmentComponent->IsRollingSnowball())
		{
			FHitResult RollingSurfaceHit;
			bHasRollingSurface = FindFootstepSurface(
				NAME_None,
				RollingSurfaceHit);
			bIsRollingOnSnowSurface = bHasRollingSurface
				&& RollingSurfaceHit.GetActor()
				&& RollingSurfaceHit.GetActor()->ActorHasTag(
					SnowFootstepSurfaceTag);
		}
		if (bHasRollingSurface)
		{
			TargetMeshRelativeLocation.Z += bIsRollingOnSnowSurface
				? RollingPlayerMeshSnowSurfaceZOffset
				: RollingPlayerMeshGroundZOffset;
		}

		if (!CharacterMesh->GetRelativeLocation().Equals(
			TargetMeshRelativeLocation,
			KINDA_SMALL_NUMBER))
		{
			CharacterMesh->SetRelativeLocation(TargetMeshRelativeLocation);
		}
	}

	const UWorld* World = GetWorld();
	const bool bUseAimCameraPresentation =
		IsAiming()
		|| (World
			&& World->GetTimeSeconds() < PostThrowAimCameraEndTime);
	const ASnowballItem* HeldSnowball = SnowballEquipmentComponent
		? SnowballEquipmentComponent->GetHeldSnowball()
		: nullptr;
	const bool bHasLargeSnowball =
		HeldSnowball && HeldSnowball->IsFullyGrown();
	const bool bUseLargeSnowballAimCamera =
		IsAiming()
		&& bHasLargeSnowball;
	const bool bUseSprintCamera =
		IsSprinting()
		&& !bUseAimCameraPresentation
		&& !bHasLargeSnowball;
	const float CameraTransitionInterpSpeed = FMath::Max(
		0.0f,
		bUseLargeSnowballAimCamera
			? LargeSnowballAimCameraInterpSpeed
			: (bHasLargeSnowball
				? LargeSnowballHeldCameraInterpSpeed
				: (bUseAimCameraPresentation
					? AimFieldOfViewInterpSpeed
					: (bUseSprintCamera
						? SprintCameraInterpSpeed
						: CameraPositionInterpSpeed))));
	if (IsLocallyControlled() && bTiebreakerSpectator)
	{
		RefreshTiebreakerSpectatorViewTarget();
	}

	if (IsLocallyControlled() && CameraBoom && !bTiebreakerSpectator)
	{
		UpdateCameraZoomInput();

		FVector TargetCameraOffset = DefaultCameraSocketOffset;
		const float ShoulderOffset =
			bUseAimCameraPresentation
				? (bUseLargeSnowballAimCamera
					? LargeSnowballAimShoulderOffset
					: AimShoulderOffset)
				: (bHasLargeSnowball
					? LargeSnowballHeldShoulderOffset
					: (bUseSprintCamera
						? SprintShoulderOffset
						: DefaultShoulderOffset));
		TargetCameraOffset.Y += CameraShoulderSide * ShoulderOffset;

		CameraBoom->SocketOffset = FMath::VInterpTo(
			CameraBoom->SocketOffset,
			TargetCameraOffset,
			DeltaSeconds,
			CameraTransitionInterpSpeed);
		CameraBoom->SocketOffset += CalculateLocalDamageCameraShakeOffset();
		CameraBoom->TargetArmLength = FMath::FInterpTo(
			CameraBoom->TargetArmLength,
			bUseAimCameraPresentation
				? (bUseLargeSnowballAimCamera
					? LargeSnowballAimCameraArmLength
					: AimCameraArmLength)
				: (bHasLargeSnowball
					? LargeSnowballHeldCameraArmLength
					: (bUseSprintCamera
						? SprintCameraArmLength
						: DesiredCameraArmLength)),
			DeltaSeconds,
			CameraTransitionInterpSpeed);
	}

	if (IsLocallyControlled() && FollowCamera && !bTiebreakerSpectator)
	{
		const float TargetFieldOfView =
			bUseAimCameraPresentation
				? (bUseLargeSnowballAimCamera
					? LargeSnowballAimFieldOfView
					: AimFieldOfView)
				: (bHasLargeSnowball
					? LargeSnowballHeldFieldOfView
					: (bUseSprintCamera
						? SprintFieldOfView
						: DefaultFieldOfView));
		FollowCamera->SetFieldOfView(FMath::FInterpTo(
			FollowCamera->FieldOfView,
			TargetFieldOfView,
			DeltaSeconds,
			CameraTransitionInterpSpeed));
	}
	if (IsLocallyControlled())
	{
		UpdateReplicatedSpectatorCameraView();
		if (bLifeStateSpectating)
		{
			UpdateLocalSpectatorCameraView();
		}
	}

	if (OutlineComponent)
	{
		AActor* OutlinedActor = nullptr;
		if (IsLocallyControlled()
			&& CanPerformGameplayAction()
			&& !FocusedLobbyBoard
			&& !FocusedPhotoActor)
		{
			OutlinedActor = FindClosestPhotoInteractionCandidate();
			if (!OutlinedActor)
			{
				AJukeboxActor* Jukebox = FindClosestJukeboxCandidate();
				if (Jukebox && !Jukebox->IsPlaying())
				{
					OutlinedActor = Jukebox;
				}
			}
			if (!OutlinedActor)
			{
				OutlinedActor = FindClosestLobbyBoardCandidate();
			}
			if (!OutlinedActor)
			{
				OutlinedActor = FindClosestFrozenTeammateCandidate();
			}
			if (!OutlinedActor)
			{
				OutlinedActor = FindClosestGiftBoxCandidate();
			}
			if (!OutlinedActor)
			{
				OutlinedActor = FindClosestGiftBoxItemPickupCandidate();
			}
			if (!OutlinedActor
				&& SnowballEquipmentComponent
				&& !SnowballEquipmentComponent->IsRollingSnowball()
				&& !SnowballEquipmentComponent->HasHeldSnowball())
			{
				OutlinedActor =
					SnowballEquipmentComponent->FindClosestPickupCandidate();
			}
		}

		OutlineComponent->SetOutlinedActor(OutlinedActor);
	}

	RefreshInteractionPromptWidget();
	DrawRollingSnowballCollisionDebug();
	RefreshOverheadNameplateFacing();
	RefreshPvpMatchInputLock();
	ApplyMovementSpeed();
	RefreshLocalSnowEffect();
	UpdateDistanceBasedSnowTrail(DeltaSeconds);

	if (IsLocallyControlled() && FocusedLobbyBoard)
	{
		APlayerController* PlayerController =
			Cast<APlayerController>(GetController());
		if (PlayerController)
		{
			UpdateLobbyBoardWidgetHitResult();

			if (!bLobbyBoardPointerPressed
				&& PlayerController->WasInputKeyJustPressed(EKeys::LeftMouseButton))
			{
				HandleLobbyBoardPointerPressed();
				bLobbyBoardPointerPressed = true;
			}
			else if (bLobbyBoardPointerPressed
				&& PlayerController->WasInputKeyJustReleased(EKeys::LeftMouseButton))
			{
				HandleLobbyBoardPointerReleased();
				bLobbyBoardPointerPressed = false;
			}
		}
	}
}

bool ASnowRumbleCharacter::IsMoving() const
{
	return GetVelocity().SizeSquared2D() > 1.0f;
}

bool ASnowRumbleCharacter::IsInAir() const
{
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	return MovementComponent && MovementComponent->IsFalling();
}

bool ASnowRumbleCharacter::IsSprinting() const
{
	return bIsSprinting
		&& IsMoving()
		&& !IsFrozen()
		&& (!SnowballEquipmentComponent
			|| !SnowballEquipmentComponent->IsHoldingLargeSnowball());
}

bool ASnowRumbleCharacter::IsFrozen() const
{
	return HealthComponent && HealthComponent->IsFrozen();
}

bool ASnowRumbleCharacter::IsDead() const
{
	return HealthComponent && HealthComponent->IsDead();
}

float ASnowRumbleCharacter::GetFrozenSecondsRemaining() const
{
	return HealthComponent
		? HealthComponent->GetFrozenSecondsRemaining()
		: 0.0f;
}

float ASnowRumbleCharacter::GetFrozenProgress() const
{
	return HealthComponent
		? HealthComponent->GetFrozenProgress()
		: 0.0f;
}

bool ASnowRumbleCharacter::IsHoldingSnowball() const
{
	return SnowballEquipmentComponent
		&& SnowballEquipmentComponent->HasHeldSnowball()
		&& !IsPickingUpItem();
}

ESnowballCarryState ASnowRumbleCharacter::GetSnowballCarryState() const
{
	if (IsPickingUpItem()
		|| !SnowballEquipmentComponent)
	{
		return ESnowballCarryState::Normal;
	}

	const ASnowballItem* HeldSnowball =
		SnowballEquipmentComponent->GetHeldSnowball();
	if (!HeldSnowball)
	{
		return ESnowballCarryState::Normal;
	}

	return HeldSnowball->IsFullyGrown()
		? ESnowballCarryState::LargeSnowball
		: ESnowballCarryState::SmallSnowball;
}

ESnowRumbleHeldAnimationState ASnowRumbleCharacter::GetHeldAnimationState() const
{
	if (GiftItemEffectComponent)
	{
		switch (GiftItemEffectComponent->GetEquippedDuckMakerItemType())
		{
		case ESnowRumbleGiftItemType::SnowDuckMaker:
		case ESnowRumbleGiftItemType::GoldenDuckMaker:
			return ESnowRumbleHeldAnimationState::SnowDuckMaker;
		default:
			break;
		}
	}

	switch (GetSnowballCarryState())
	{
	case ESnowballCarryState::SmallSnowball:
		return ESnowRumbleHeldAnimationState::SmallSnowball;
	case ESnowballCarryState::LargeSnowball:
		return ESnowRumbleHeldAnimationState::LargeSnowball;
	default:
		break;
	}

	if (GiftItemEffectComponent)
	{
		switch (GiftItemEffectComponent->GetEquippedShovelItemType())
		{
		case ESnowRumbleGiftItemType::SnowShovel:
		case ESnowRumbleGiftItemType::GoldenShovel:
			return ESnowRumbleHeldAnimationState::SnowShovel;
		default:
			break;
		}

	}

	return ESnowRumbleHeldAnimationState::BareHands;
}

ESnowballActionState ASnowRumbleCharacter::GetSnowballActionState() const
{
	return SnowballEquipmentComponent
		&& SnowballEquipmentComponent->IsRollingSnowball()
		? ESnowballActionState::RollingSnowball
		: ESnowballActionState::None;
}

bool ASnowRumbleCharacter::IsAiming() const
{
	return SnowballEquipmentComponent
		&& SnowballEquipmentComponent->IsAiming();
}

bool ASnowRumbleCharacter::IsHoldingLargeSnowball() const
{
	return SnowballEquipmentComponent
		&& SnowballEquipmentComponent->IsHoldingLargeSnowball();
}

bool ASnowRumbleCharacter::IsChargingSnowball() const
{
	return SnowballEquipmentComponent
		&& SnowballEquipmentComponent->IsCharging();
}

float ASnowRumbleCharacter::GetSnowballChargeProgress() const
{
	return SnowballEquipmentComponent
		? SnowballEquipmentComponent->GetChargeProgress()
		: 0.0f;
}

bool ASnowRumbleCharacter::IsCreatingSnowball() const
{
	return SnowballCreationComponent
		&& SnowballCreationComponent->IsCreatingSnowball();
}

bool ASnowRumbleCharacter::IsPickingUpItem() const
{
	return bIsPickingUpItem;
}

bool ASnowRumbleCharacter::IsInteractingWithItem() const
{
	return bIsInteractingWithItem;
}

bool ASnowRumbleCharacter::IsHitReacting() const
{
	return bIsHitReacting;
}

bool ASnowRumbleCharacter::IsGrabReaching() const
{
	return PlayerGrabComponent && PlayerGrabComponent->IsGrabReaching();
}

bool ASnowRumbleCharacter::IsGrabbingCharacter() const
{
	return PlayerGrabComponent && PlayerGrabComponent->IsGrabbingCharacter();
}

bool ASnowRumbleCharacter::IsGrabAttached() const
{
	return PlayerGrabComponent && PlayerGrabComponent->IsGrabAttached();
}

bool ASnowRumbleCharacter::IsHangingFromWorldGrab() const
{
	return PlayerGrabComponent && PlayerGrabComponent->IsHangingFromWorldGrab();
}

bool ASnowRumbleCharacter::IsGrabbedByCharacter() const
{
	return bIsGrabbedByCharacter;
}

FVector ASnowRumbleCharacter::GetGrabAttachedWorldLocation() const
{
	return PlayerGrabComponent
		? PlayerGrabComponent->GetGrabAttachedWorldLocation()
		: FVector::ZeroVector;
}

FVector ASnowRumbleCharacter::GetGrabbedByCharacterWorldLocation() const
{
	return GrabbedByCharacterWorldLocation;
}

FVector ASnowRumbleCharacter::GetRightHandGrabTargetLocation() const
{
	return PlayerGrabComponent
		? PlayerGrabComponent->GetRightHandGrabTargetLocation()
		: FVector::ZeroVector;
}

FVector ASnowRumbleCharacter::GetLeftHandGrabTargetLocation() const
{
	return PlayerGrabComponent
		? PlayerGrabComponent->GetLeftHandGrabTargetLocation()
		: FVector::ZeroVector;
}

float ASnowRumbleCharacter::GetGrabReachAlpha() const
{
	return PlayerGrabComponent
		? PlayerGrabComponent->GetGrabReachAlpha()
		: 0.0f;
}

float ASnowRumbleCharacter::GetGrabRemainingTimeProgress() const
{
	return PlayerGrabComponent
		? PlayerGrabComponent->GetGrabRemainingTimeProgress()
		: 0.0f;
}

float ASnowRumbleCharacter::GetViewPitchDegrees() const
{
	if (FocusedPhotoActor && IsLocallyControlled())
	{
		return PhotoFocusViewPitchDegrees;
	}

	return FRotator::NormalizeAxis(GetBaseAimRotation().Pitch);
}

float ASnowRumbleCharacter::GetViewPitchAlpha() const
{
	const float SafeRange = FMath::Max(1.0f, ViewPitchAlphaRangeDegrees);
	return FMath::Clamp(
		0.5f + GetViewPitchDegrees() / (SafeRange * 2.0f),
		0.0f,
		1.0f);
}

float ASnowRumbleCharacter::GetViewYawDegrees() const
{
	return FRotator::NormalizeAxis(
		GetBaseAimRotation().Yaw - GetActorRotation().Yaw);
}

float ASnowRumbleCharacter::GetViewYawAlpha() const
{
	const float SafeRange = FMath::Max(1.0f, ViewYawAlphaRangeDegrees);
	return FMath::Clamp(
		GetViewYawDegrees() / (SafeRange * 2.0f),
		-0.5f,
		0.5f);
}

bool ASnowRumbleCharacter::CanStartPlayerGrabReach() const
{
	return CanPerformGameplayAction()
		&& !IsGrabbedByCharacter()
		&& !IsAiming()
		&& (!SnowballEquipmentComponent
			|| !SnowballEquipmentComponent->HasHeldSnowball())
		&& !IsChargingSnowball()
		&& !IsCreatingSnowball()
		&& !IsInteractingWithItem()
		&& !IsPickingUpItem()
		&& !IsHitReacting()
		&& !IsDead()
		&& !IsFrozen();
}

bool ASnowRumbleCharacter::ShouldPreferSnowCreationOverGrab() const
{
	return GetViewPitchAlpha() <= SnowCreationPreferredViewPitchAlpha;
}

bool ASnowRumbleCharacter::ShouldSuppressPvpWidgets() const
{
	return bPvpIntroWidgetsHidden
		|| Cast<APodiumPlayerController>(Controller) != nullptr;
}

void ASnowRumbleCharacter::SetPvpIntroWidgetsHidden(bool bShouldHide)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (bShouldHide)
	{
		if (bPvpIntroWidgetsHidden)
		{
			return;
		}

		bPvpIntroWidgetsHidden = true;
		if (EmoteRadialMenuWidget)
		{
			PvpIntroEmoteVisibility = EmoteRadialMenuWidget->GetVisibility();
			CloseEmoteRadialMenu();
			EmoteRadialMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (KeyGuideWidget)
		{
			PvpIntroKeyGuideVisibility = KeyGuideWidget->GetVisibility();
			KeyGuideWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (MainHUDWidget)
		{
			PvpIntroMainHUDVisibility = MainHUDWidget->GetVisibility();
			MainHUDWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (InteractionPromptWidget)
		{
			PvpIntroInteractionPromptVisibility =
				InteractionPromptWidget->GetVisibility();
			InteractionPromptWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (SpectatorWidget)
		{
			PvpIntroSpectatorVisibility = SpectatorWidget->GetVisibility();
			SpectatorWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		bIsKeyGuideWidgetOpen = false;
		return;
	}

	if (!bPvpIntroWidgetsHidden)
	{
		return;
	}

	bPvpIntroWidgetsHidden = false;
	if (EmoteRadialMenuWidget)
	{
		EmoteRadialMenuWidget->SetVisibility(PvpIntroEmoteVisibility);
	}
	if (KeyGuideWidget)
	{
		KeyGuideWidget->SetVisibility(PvpIntroKeyGuideVisibility);
	}
	if (MainHUDWidget)
	{
		MainHUDWidget->SetVisibility(PvpIntroMainHUDVisibility);
	}
	if (InteractionPromptWidget)
	{
		InteractionPromptWidget->SetVisibility(
			PvpIntroInteractionPromptVisibility);
	}
	if (SpectatorWidget)
	{
		SpectatorWidget->SetVisibility(PvpIntroSpectatorVisibility);
	}
}

void ASnowRumbleCharacter::ApplyGrabbedByCharacter(
	ASnowRumbleCharacter* GrabbingCharacter)
{
	if (!HasAuthority() || !GrabbingCharacter || GrabbingCharacter == this)
	{
		return;
	}

	GrabbedByCharacter = GrabbingCharacter;
	bIsGrabbedByCharacter = true;
	GrabbedByCharacterWorldLocation = GrabbingCharacter
		? GrabbingCharacter->GetGrabAttachedWorldLocation()
		: FVector::ZeroVector;
	HandleGrabbedByCharacterChanged(true);
	ForceNetUpdate();
}

void ASnowRumbleCharacter::SetGrabbedByCharacterWorldLocationFromServer(
	const FVector& NewWorldLocation)
{
	if (!HasAuthority())
	{
		return;
	}

	GrabbedByCharacterWorldLocation = NewWorldLocation;
}

void ASnowRumbleCharacter::ClearGrabbedByCharacter(
	ASnowRumbleCharacter* ExpectedGrabbingCharacter)
{
	if (!HasAuthority()
		|| !bIsGrabbedByCharacter
		|| (ExpectedGrabbingCharacter && GrabbedByCharacter != ExpectedGrabbingCharacter))
	{
		return;
	}

	bIsGrabbedByCharacter = false;
	GrabbedByCharacter = nullptr;
	GrabbedByCharacterWorldLocation = FVector::ZeroVector;
	HandleGrabbedByCharacterChanged(false);
	ForceNetUpdate();
}

void ASnowRumbleCharacter::HandleWorldGrabChanged(bool bNewWorldGrab)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	if (bNewWorldGrab)
	{
		MovementModeBeforeGrabbed = MovementComponent->MovementMode;
		CustomMovementModeBeforeGrabbed = MovementComponent->CustomMovementMode;
		bOrientRotationToMovementBeforeWorldGrab =
			MovementComponent->bOrientRotationToMovement;
		bUseControllerRotationYawBeforeWorldGrab = bUseControllerRotationYaw;
		MovementComponent->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = false;
		bIsSprinting = false;
		if (SnowballEquipmentComponent)
		{
			SnowballEquipmentComponent->SetAiming(false);
		}
		if (SnowballCreationComponent)
		{
			SnowballCreationComponent->CancelCreatingSnowball();
		}
		MovementComponent->StopMovementImmediately();
		if (MovementComponent->MovementMode == MOVE_None)
		{
			MovementComponent->SetMovementMode(MOVE_Falling);
		}
		StopJumping();
		ApplyMovementSpeed();
		return;
	}

	if (HealthComponent
		&& !HealthComponent->IsFrozen()
		&& !HealthComponent->IsDead()
		&& !bTiebreakerSpectator
		&& !bWaterSubmerged
		&& !bIsGrabbedByCharacter)
	{
		const EMovementMode RestoreMode =
			MovementModeBeforeGrabbed == MOVE_None
				? MOVE_Walking
				: MovementModeBeforeGrabbed.GetValue();
		MovementComponent->SetMovementMode(
			RestoreMode,
			CustomMovementModeBeforeGrabbed);
		MovementComponent->bOrientRotationToMovement =
			bOrientRotationToMovementBeforeWorldGrab;
		bUseControllerRotationYaw = bUseControllerRotationYawBeforeWorldGrab;
	}
}

float ASnowRumbleCharacter::GetGrabReachOriginHeight() const
{
	const UCapsuleComponent* Capsule = GetCapsuleComponent();
	return Capsule ? Capsule->GetScaledCapsuleHalfHeight() * 0.45f : 70.0f;
}

float ASnowRumbleCharacter::GetSnowballCreationProgress() const
{
	return SnowballCreationComponent
		? SnowballCreationComponent->GetCreationProgress()
		: 0.0f;
}

ESnowRumbleTimedActionState ASnowRumbleCharacter::GetTimedActionState() const
{
	if (IsFrozen())
	{
		return ESnowRumbleTimedActionState::Frozen;
	}

	if (bIsRevivingTeammate)
	{
		return ESnowRumbleTimedActionState::RevivingTeammate;
	}

	if (SnowballEquipmentComponent
		&& SnowballEquipmentComponent->IsRollingSnowball())
	{
		return ESnowRumbleTimedActionState::RollingSnowball;
	}

	return IsCreatingSnowball()
		? ESnowRumbleTimedActionState::CreatingSnowball
		: ESnowRumbleTimedActionState::None;
}

float ASnowRumbleCharacter::GetTimedActionProgress() const
{
	switch (GetTimedActionState())
	{
	case ESnowRumbleTimedActionState::CreatingSnowball:
		return FMath::Clamp(GetSnowballCreationProgress(), 0.0f, 1.0f);

	case ESnowRumbleTimedActionState::RollingSnowball:
		if (const ASnowballItem* RollingSnowball =
			SnowballEquipmentComponent
				? SnowballEquipmentComponent->GetRollingSnowball()
				: nullptr)
		{
			return FMath::Clamp(
				RollingSnowball->GetGrowthProgress(),
				0.0f,
				1.0f);
		}
		return 0.0f;

	case ESnowRumbleTimedActionState::Frozen:
		return GetFrozenProgress();

	case ESnowRumbleTimedActionState::RevivingTeammate:
		if (TeammateReviveHoldDurationSeconds <= 0.0f
			|| TeammateReviveStartTime < 0.0)
		{
			return 0.0f;
		}
		if (const UWorld* World = GetWorld())
		{
			const double ElapsedSeconds =
				World->GetTimeSeconds() - TeammateReviveStartTime;
			return FMath::Clamp(
				static_cast<float>(ElapsedSeconds)
					/ TeammateReviveHoldDurationSeconds,
				0.0f,
				1.0f);
		}
		return 0.0f;

	default:
		return 0.0f;
	}
}

void ASnowRumbleCharacter::RequestPlayEmote(int32 EmoteIndex)
{
	if (!CanPlayEmote())
	{
		return;
	}

	if (!IsValidEmoteIndex(EmoteIndex))
	{
		return;
	}

	if (HasAuthority())
	{
		ServerRequestPlayEmote_Implementation(EmoteIndex);
	}
	else
	{
		ServerRequestPlayEmote(EmoteIndex);
	}
}

void ASnowRumbleCharacter::RequestSnowFootstepEffect(FName FootSocketName)
{
	const UWorld* World = GetWorld();
	if (!World
		|| !GetMesh()
		|| IsFrozen()
		|| IsDead())
	{
		return;
	}

	const double CurrentTime = World->GetTimeSeconds();
	if (LastSnowFootstepEffectTime >= 0.0
		&& CurrentTime - LastSnowFootstepEffectTime
			< SnowFootstepEffectCooldown)
	{
		return;
	}

	if (FootSocketName.IsNone())
	{
		FootSocketName = LeftFootSocketName;
	}

	FHitResult FootstepHit;
	if (!FindFootstepSurface(FootSocketName, FootstepHit))
	{
		return;
	}

	const bool bIsSnowSurface = FootstepHit.GetActor()
		&& !SnowFootstepSurfaceTag.IsNone()
		&& FootstepHit.GetActor()->ActorHasTag(SnowFootstepSurfaceTag);
	USoundBase* FootstepSoundToPlay = bIsSnowSurface
		? FootstepSound
		: NormalFootstepSound;
	USoundAttenuation* FootstepAttenuationToUse = bIsSnowSurface
		? FootstepSoundAttenuation
		: NormalFootstepSoundAttenuation;
	LastSnowFootstepEffectTime = CurrentTime;
	SnowRumbleAudio::PlaySoundAtLocation(
		this,
		FootstepSoundToPlay,
		ESnowRumbleAudioMixChannel::Gameplay,
		FootstepHit.ImpactPoint,
		1.0f,
		1.0f,
		FootstepAttenuationToUse);
	if (bIsSnowSurface)
	{
		OnSnowFootstepEffect(
			FootSocketName,
			FootstepHit.ImpactPoint,
			FootstepHit.ImpactNormal.GetSafeNormal());
	}

	if (bIsSnowSurface
		&& bEnableSharedSnowTrailStamps
		&& IsLocallyControlled())
	{
		RequestSharedSnowTrailStamp(
			FootstepHit.ImpactPoint,
			FootstepHit.ImpactNormal.GetSafeNormal(),
			FootSocketName);

		if (bEnableDistanceBasedSnowTrailStamps)
		{
			bDistanceSnowTrailActive = true;
			LastDistanceSnowTrailStampLocation = FootstepHit.ImpactPoint;
			LastDistanceSnowTrailFootSocketName = FootSocketName;
		}
	}
}

USceneComponent* ASnowRumbleCharacter::GetSnowballHoldPoint() const
{
	return SnowballHoldPoint;
}

USceneComponent* ASnowRumbleCharacter::GetSnowballHoldPointForSnowball(
	const ASnowballItem* Snowball) const
{
	const USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (HasEquippedSnowDuckMaker()
		&& SnowDuckBallHoldPoint
		&& CharacterMesh
		&& CharacterMesh->DoesSocketExist(TEXT("SnowDuckBallSocket")))
	{
		return SnowDuckBallHoldPoint;
	}

	if (Snowball && Snowball->IsFullyGrown() && LargeSnowballHoldPoint)
	{
		if (CharacterMesh
			&& CharacterMesh->DoesSocketExist(TEXT("LargeSnowballSocket")))
		{
			return LargeSnowballHoldPoint;
		}
	}

	return SnowballHoldPoint;
}

void ASnowRumbleCharacter::EnableRollingSnowballCollision(
	const FVector& InitialLocation,
	float CollisionRadius)
{
	if (!HasAuthority()
		|| !RollingSnowballCollision
		|| InitialLocation.ContainsNaN())
	{
		return;
	}

	RollingSnowballCollision->SetSphereRadius(
		FMath::Max(CollisionRadius, 1.0f),
		false);
	RollingSnowballCollision->SetWorldLocation(
		InitialLocation,
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
	RollingSnowballCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

bool ASnowRumbleCharacter::MoveRollingSnowballCollision(
	const FVector& TargetLocation,
	float CollisionRadius,
	FHitResult& OutSweepHit)
{
	OutSweepHit = FHitResult();
	if (!HasAuthority()
		|| !RollingSnowballCollision
		|| RollingSnowballCollision->GetCollisionEnabled()
			== ECollisionEnabled::NoCollision
		|| TargetLocation.ContainsNaN())
	{
		return false;
	}

	const float NewCollisionRadius = FMath::Max(CollisionRadius, 1.0f);
	const float PreviousCollisionRadius =
		RollingSnowballCollision->GetScaledSphereRadius();
	const float RadiusIncrease =
		NewCollisionRadius - PreviousCollisionRadius;
	if (RadiusIncrease > KINDA_SMALL_NUMBER)
	{
		// 낮은 이전 중심에서 반지름부터 키우면 지면과 겹친 상태로
		// Sweep이 시작되므로, 실제 눈덩이처럼 중심 높이를 먼저 맞춘다.
		RollingSnowballCollision->AddWorldOffset(
			FVector::UpVector * RadiusIncrease,
			false,
			nullptr,
			ETeleportType::TeleportPhysics);
	}
	RollingSnowballCollision->SetSphereRadius(NewCollisionRadius, false);
	RollingSnowballCollision->SetWorldLocation(
		TargetLocation,
		true,
		&OutSweepHit,
		ETeleportType::None);
	return OutSweepHit.bBlockingHit;
}

FVector ASnowRumbleCharacter::GetRollingSnowballCollisionLocation() const
{
	return RollingSnowballCollision
		? RollingSnowballCollision->GetComponentLocation()
		: GetActorLocation();
}

void ASnowRumbleCharacter::DisableRollingSnowballCollision()
{
	if (HasAuthority() && RollingSnowballCollision)
	{
		RollingSnowballCollision->SetCollisionEnabled(
			ECollisionEnabled::NoCollision);
	}
}

void ASnowRumbleCharacter::NotifyItemPickupSucceeded()
{
	if (!HasAuthority())
	{
		return;
	}

	MulticastPlayCharacterFeedbackSound(
		ESnowRumbleCharacterFeedbackSoundType::ItemPickup);
	bIsPickingUpItem = true;
	OnRep_IsPickingUpItem();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			PickupAnimationTimerHandle,
			this,
			&ASnowRumbleCharacter::FinishPickupAnimationState,
			PickupAnimationStateDuration,
			false);
	}

	ForceNetUpdate();
}

void ASnowRumbleCharacter::NotifySnowballPickupSucceeded(bool bWasLargeSnowball)
{
	if (!HasAuthority())
	{
		return;
	}

	MulticastPlayCharacterFeedbackSound(
		ESnowRumbleCharacterFeedbackSoundType::SnowballPickup);
	bIsPickingUpItem = true;
	OnRep_IsPickingUpItem();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			PickupAnimationTimerHandle,
			this,
			&ASnowRumbleCharacter::FinishPickupAnimationState,
			PickupAnimationStateDuration,
			false);
	}

	RequestAnimationTriggerFromServer(
		bWasLargeSnowball
			? ESnowRumbleCharacterAnimTrigger::PickupLargeSnowball
			: ESnowRumbleCharacterAnimTrigger::PickupSmallSnowball);
	ForceNetUpdate();
}

void ASnowRumbleCharacter::NotifySnowballThrowSucceeded(bool bWasLargeSnowball)
{
	if (!HasAuthority())
	{
		return;
	}

	if (HasEquippedSnowDuckMaker())
	{
		RequestAnimationTriggerFromServer(
			ESnowRumbleCharacterAnimTrigger::ThrowSnowDuckMaker);
		return;
	}

	MulticastPlayCharacterFeedbackSound(
		ESnowRumbleCharacterFeedbackSoundType::SnowballThrow);
	RequestAnimationTriggerFromServer(
		bWasLargeSnowball
			? ESnowRumbleCharacterAnimTrigger::ThrowLargeSnowball
			: (IsInAir()
				? ESnowRumbleCharacterAnimTrigger::ThrowSmallSnowballInAir
				: ESnowRumbleCharacterAnimTrigger::ThrowSmallSnowball));
}

void ASnowRumbleCharacter::RequestSnowballThrowReleaseFromNotify()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	USnowballEquipmentComponent* ActiveEquipmentComponent =
		SnowballEquipmentComponent
			? SnowballEquipmentComponent.Get()
			: FindComponentByClass<USnowballEquipmentComponent>();
	if (!ActiveEquipmentComponent)
	{
		return;
	}

	ActiveEquipmentComponent->ConfirmPendingThrowFromAnimationNotify();
}

void ASnowRumbleCharacter::NotifyItemInteractionSucceeded()
{
	if (!HasAuthority())
	{
		return;
	}

	MulticastPlayCharacterFeedbackSound(
		ESnowRumbleCharacterFeedbackSoundType::ItemInteraction);
	bIsInteractingWithItem = true;
	OnRep_IsInteractingWithItem();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ItemInteractionAnimationTimerHandle,
			this,
			&ASnowRumbleCharacter::FinishItemInteractionAnimationState,
			ItemInteractionAnimationStateDuration,
			false);
	}

	RequestAnimationTriggerFromServer(
		ESnowRumbleCharacterAnimTrigger::ItemInteraction);
	ForceNetUpdate();
}

void ASnowRumbleCharacter::NotifyLobbyBoardInteractionSucceeded()
{
	if (!HasAuthority())
	{
		return;
	}

	MulticastPlayCharacterFeedbackSound(
		ESnowRumbleCharacterFeedbackSoundType::LobbyBoardInteraction);
}

bool ASnowRumbleCharacter::ApplyGiftBoxItemEffectFromServer(
	ESnowRumbleGiftItemType ItemType)
{
	return HasAuthority()
		&& GiftItemEffectComponent
		&& GiftItemEffectComponent->ApplyGiftItemFromServer(ItemType);
}

void ASnowRumbleCharacter::SetTiebreakerSpectatorFromServer(
	bool bNewTiebreakerSpectator)
{
	if (!HasAuthority() || bTiebreakerSpectator == bNewTiebreakerSpectator)
	{
		return;
	}

	bTiebreakerSpectator = bNewTiebreakerSpectator;
	OnRep_TiebreakerSpectator();
	ForceNetUpdate();
}

void ASnowRumbleCharacter::SetWaterSubmergedFromServer(
	bool bNewWaterSubmerged)
{
	if (!HasAuthority() || bWaterSubmerged == bNewWaterSubmerged)
	{
		return;
	}

	bWaterSubmerged = bNewWaterSubmerged;
	OnRep_WaterSubmerged();
	ForceNetUpdate();
}

float ASnowRumbleCharacter::GetSnowballCreationDurationMultiplier() const
{
	return GiftItemEffectComponent
		? GiftItemEffectComponent->GetSnowballCreationDurationMultiplier()
		: 1.0f;
}

float ASnowRumbleCharacter::GetSnowballDamageMultiplier() const
{
	return GiftItemEffectComponent
		? GiftItemEffectComponent->GetSnowballDamageMultiplier()
		: 1.0f;
}

bool ASnowRumbleCharacter::HasEquippedSnowDuckMaker() const
{
	if (!GiftItemEffectComponent)
	{
		return false;
	}

	switch (GiftItemEffectComponent->GetEquippedDuckMakerItemType())
	{
	case ESnowRumbleGiftItemType::SnowDuckMaker:
	case ESnowRumbleGiftItemType::GoldenDuckMaker:
		return true;
	default:
		return false;
	}
}

float ASnowRumbleCharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	// 로비는 전투 공간이 아니므로 눈덩이·환경 등 모든 피해를 받지 않는다.
	if (GetWorld()
		&& GetWorld()->GetGameState<ASnowRumbleLobbyGameState>())
	{
		return 0.0f;
	}

	const float ValidatedDamage = Super::TakeDamage(
		DamageAmount,
		DamageEvent,
		EventInstigator,
		DamageCauser);

	const UWorld* World = GetWorld();
	const double CurrentTime = World ? World->GetTimeSeconds() : 0.0;
	if (PostDamageInvulnerabilitySeconds > 0.0f
		&& CurrentTime < DamageInvulnerableUntilTime)
	{
		return 0.0f;
	}

	if (GiftItemEffectComponent && GiftItemEffectComponent->IsInvulnerable())
	{
		return 0.0f;
	}
	if (!IsDamageAllowedByTiebreaker(EventInstigator))
	{
		return 0.0f;
	}

	const float AdjustedDamage =
		ValidatedDamage
		* (GiftItemEffectComponent
			? GiftItemEffectComponent->GetIncomingDamageMultiplier()
			: 1.0f);
	const float AppliedDamage =
		HealthComponent ? HealthComponent->ApplyDamage(AdjustedDamage) : 0.0f;
	if (AppliedDamage > 0.0f)
	{
		DamageInvulnerableUntilTime =
			CurrentTime + PostDamageInvulnerabilitySeconds;

		if (SnowballEquipmentComponent)
		{
			SnowballEquipmentComponent->InterruptThrowStateFromServer();
		}
		StartHitReactAnimationState();
		const FVector DamageCauserLocation = DamageCauser
			? DamageCauser->GetActorLocation()
			: GetActorLocation();
		const UClass* DamageTypeClass = DamageEvent.DamageTypeClass
			? DamageEvent.DamageTypeClass.Get()
			: UDamageType::StaticClass();
		const bool bHeadshotDamage =
			DamageTypeClass
			&& DamageTypeClass->IsChildOf(
				USnowballHeadshotDamageType::StaticClass());
		ClientRequestLocalDamageFeedback(AppliedDamage, DamageCauserLocation);
		MulticastRequestDamageText(
			AppliedDamage,
			GetActorLocation() + DamageTextWorldOffset,
			bHeadshotDamage
				? ESnowRumbleDamageTextType::Headshot
				: ESnowRumbleDamageTextType::Normal);
		MulticastPlayDamageSound(GetActorLocation());
	}

	return AppliedDamage;
}

void ASnowRumbleCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RefreshOverheadNameplateComponentSettings();
	RefreshScarfMesh();
	RefreshGiftItemEquipmentMeshes();
}

void ASnowRumbleCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (const UWorld* World = GetWorld())
	{
		bIsIceGlacierMap = World->GetMapName().Contains(TEXT("L_IceGlacier_J"));
	}
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		DefaultGroundFriction = MovementComponent->GroundFriction;
		DefaultBrakingDecelerationWalking =
			MovementComponent->BrakingDecelerationWalking;
	}

	if (GetMesh())
	{
		DefaultCharacterMeshRelativeLocation = GetMesh()->GetRelativeLocation();
	}

	if (FollowCamera)
	{
		DefaultFieldOfView = FollowCamera->FieldOfView;
	}
	if (CameraBoom)
	{
		CameraBoom->TargetOffset.Z = CameraPivotHeight;
		DefaultCameraSocketOffset = CameraBoom->SocketOffset;
		DefaultCameraArmLength = CameraBoom->TargetArmLength;
		const float SafeMinimumCameraArmLength =
			FMath::Max(0.0f, MinimumCameraArmLength);
		const float SafeMaximumCameraArmLength =
			FMath::Max(SafeMinimumCameraArmLength, MaximumCameraArmLength);
		DesiredCameraArmLength = FMath::Clamp(
			DefaultCameraArmLength,
			SafeMinimumCameraArmLength,
			SafeMaximumCameraArmLength);
	}

	RefreshLocalSnowEffect();
	ApplyCameraPitchLimits();
	RefreshOverheadNameplateComponentSettings();

	if (HealthComponent)
	{
		HealthComponent->OnFrozenChanged.AddDynamic(this, &ASnowRumbleCharacter::HandleFrozenChanged);
		HealthComponent->OnDeathChanged.AddDynamic(this, &ASnowRumbleCharacter::HandleDeathChanged);
		HandleFrozenChanged(HealthComponent->IsFrozen());
		HandleDeathChanged(HealthComponent->IsDead());
	}

	if (SnowballEquipmentComponent)
	{
		SnowballEquipmentComponent->OnAimingChanged.AddDynamic(
			this,
			&ASnowRumbleCharacter::HandleSnowballAimingChanged);
		HandleSnowballAimingChanged(SnowballEquipmentComponent->IsAiming());
	}

	if (GiftItemEffectComponent)
	{
		GiftItemEffectComponent->OnGiftItemEffectsChanged.AddDynamic(
			this,
			&ASnowRumbleCharacter::HandleGiftItemEffectsChanged);
	}

	BindOverheadNameToPlayerState();
	BindCustomizationToPlayerState();
	RefreshOverheadPlayerName();
	RefreshCustomizationFromPlayerState();
	RefreshScarfMesh();
	RefreshGiftItemEquipmentMeshes();
	RefreshPvpMatchInputLock();
}

void ASnowRumbleCharacter::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	CancelTeammateRevive();
	if (KeyGuideWidget)
	{
		KeyGuideWidget->RemoveFromParent();
		KeyGuideWidget = nullptr;
	}

	if (InteractionPromptWidget)
	{
		InteractionPromptWidget->RemoveFromParent();
		InteractionPromptWidget = nullptr;
	}
	if (OverheadTimedActionWidget)
	{
		OverheadTimedActionWidget->RemoveFromParent();
		OverheadTimedActionWidget = nullptr;
	}
	if (SpectatorWidget)
	{
		SpectatorWidget->RemoveFromParent();
		SpectatorWidget = nullptr;
	}
	if (SpectatorCameraActor)
	{
		SpectatorCameraActor->Destroy();
		SpectatorCameraActor = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void ASnowRumbleCharacter::EnsureOverheadTimedActionWidget()
{
	if (OverheadTimedActionWidget
		|| !OverheadTimedActionWidgetClass
		|| IsLocallyControlled())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	APlayerController* LocalPlayerController = World->GetFirstPlayerController();
	if (!LocalPlayerController || !LocalPlayerController->IsLocalController())
	{
		return;
	}

	OverheadTimedActionWidget =
		CreateWidget<UOverheadTimedActionWidget>(
			LocalPlayerController,
			OverheadTimedActionWidgetClass);
	if (OverheadTimedActionWidget)
	{
		OverheadTimedActionWidget->SetObservedCharacter(this);
		OverheadTimedActionWidget->AddToViewport();
	}
}

FString ASnowRumbleCharacter::GetOverheadPlayerName() const
{
	const ASnowRumblePlayerState* SnowRumblePlayerState =
		GetPlayerState<ASnowRumblePlayerState>();
	return SnowRumblePlayerState
		? SnowRumblePlayerState->GetLobbyPlayerName()
		: FString();
}

FLinearColor ASnowRumbleCharacter::GetOverheadTeamColor() const
{
	const ASnowRumblePlayerState* SnowRumblePlayerState =
		GetPlayerState<ASnowRumblePlayerState>();
	return SnowRumblePlayerState
		? SnowRumblePlayerState->GetLobbyTeamColor()
		: FLinearColor::White;
}

void ASnowRumbleCharacter::ApplyCustomizationData(
	const FSnowRumbleCustomizationData& NewCustomizationData)
{
	AppliedCustomizationData = NewCustomizationData;

	if (CustomizationMaterialIndex >= 0
		&& !CustomizationBodyColorParameterName.IsNone())
	{
		TArray<USkeletalMeshComponent*> MeshComponents;
		GetComponents(MeshComponents);
		for (USkeletalMeshComponent* MeshComponent : MeshComponents)
		{
			if (!MeshComponent
				|| CustomizationMaterialIndex >= MeshComponent->GetNumMaterials())
			{
				continue;
			}

			UMaterialInstanceDynamic* DynamicMaterial =
				Cast<UMaterialInstanceDynamic>(
					MeshComponent->GetMaterial(CustomizationMaterialIndex));
			if (!DynamicMaterial)
			{
				DynamicMaterial =
					MeshComponent->CreateAndSetMaterialInstanceDynamic(
						CustomizationMaterialIndex);
			}
			if (DynamicMaterial)
			{
				DynamicMaterial->SetVectorParameterValue(
				CustomizationBodyColorParameterName,
				AppliedCustomizationData.BodyColor);
				if (MeshComponent == GetMesh())
				{
					CustomizationMaterialInstance = DynamicMaterial;
				}
			}
		}
	}

	RefreshCustomizationHatMesh();
	RefreshCustomizationGlassesMesh();
	RefreshCustomizationNoseMesh();
	RefreshCustomizationEarmuffsMesh();
	RefreshOverheadNameplateComponentSettings();
	RedrawCustomizationPaintTexture();
}

int32 ASnowRumbleCharacter::GetCustomizationHatOptionCount() const
{
	return CustomizationHatMeshes.Num();
}

int32 ASnowRumbleCharacter::NormalizeCustomizationHatMeshIndex(
	int32 HatMeshIndex) const
{
	if (HatMeshIndex < 0 || CustomizationHatMeshes.IsEmpty())
	{
		return INDEX_NONE;
	}

	return CustomizationHatMeshes.IsValidIndex(HatMeshIndex)
		? HatMeshIndex
		: INDEX_NONE;
}

int32 ASnowRumbleCharacter::GetCustomizationAccessoryOptionCount(
	ESnowRumbleCustomizationAccessory Accessory) const
{
	switch (Accessory)
	{
	case ESnowRumbleCustomizationAccessory::Hat:
		return CustomizationHatMeshes.Num();
	case ESnowRumbleCustomizationAccessory::Glasses:
		return CustomizationGlassesMeshes.Num();
	case ESnowRumbleCustomizationAccessory::Nose:
		return CustomizationNoseMeshes.Num();
	case ESnowRumbleCustomizationAccessory::Earmuffs:
		return CustomizationEarmuffsMeshes.Num();
	default:
		return 0;
	}
}

int32 ASnowRumbleCharacter::NormalizeCustomizationAccessoryMeshIndex(
	ESnowRumbleCustomizationAccessory Accessory,
	int32 MeshIndex) const
{
	if (MeshIndex < 0)
	{
		return INDEX_NONE;
	}

	const int32 OptionCount = GetCustomizationAccessoryOptionCount(Accessory);
	return MeshIndex < OptionCount ? MeshIndex : INDEX_NONE;
}

void ASnowRumbleCharacter::RefreshCustomizationHatMesh()
{
	if (!HatMeshComponent)
	{
		return;
	}

	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		const FName AttachSocketName = CustomizationHatAttachSocketName.IsNone()
			? NAME_None
			: CustomizationHatAttachSocketName;
		HatMeshComponent->AttachToComponent(
			CharacterMesh,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			AttachSocketName);
	}

	const int32 HatMeshIndex = NormalizeCustomizationHatMeshIndex(
		AppliedCustomizationData.HatMeshIndex);
	HatMeshComponent->SetRelativeTransform(
		ResolveCustomizationAccessoryTransform(
			CustomizationHatRelativeTransforms,
			HatMeshIndex,
			CustomizationHatRelativeLocation,
			CustomizationHatRelativeRotation,
			CustomizationHatRelativeScale));
	UStaticMesh* HatMesh = HatMeshIndex != INDEX_NONE
		? CustomizationHatMeshes[HatMeshIndex]
		: nullptr;

	HatMeshComponent->SetStaticMesh(HatMesh);
	HatMeshComponent->SetVisibility(HatMesh != nullptr, true);
}

void ASnowRumbleCharacter::RefreshCustomizationGlassesMesh()
{
	if (!GlassesMeshComponent)
	{
		return;
	}
	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		GlassesMeshComponent->AttachToComponent(
			CharacterMesh,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			CustomizationGlassesAttachSocketName);
	}
	const int32 Index = NormalizeCustomizationAccessoryMeshIndex(
		ESnowRumbleCustomizationAccessory::Glasses,
		AppliedCustomizationData.GlassesMeshIndex);
	GlassesMeshComponent->SetRelativeTransform(
		ResolveCustomizationAccessoryTransform(
			CustomizationGlassesRelativeTransforms,
			Index,
			CustomizationGlassesRelativeLocation,
			CustomizationGlassesRelativeRotation,
			CustomizationGlassesRelativeScale));
	UStaticMesh* AccessoryMesh = Index != INDEX_NONE ? CustomizationGlassesMeshes[Index] : nullptr;
	GlassesMeshComponent->SetStaticMesh(AccessoryMesh);
	GlassesMeshComponent->SetVisibility(AccessoryMesh != nullptr, true);
}

void ASnowRumbleCharacter::RefreshCustomizationNoseMesh()
{
	if (!NoseMeshComponent)
	{
		return;
	}
	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		NoseMeshComponent->AttachToComponent(
			CharacterMesh,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			CustomizationNoseAttachSocketName);
	}
	const int32 Index = NormalizeCustomizationAccessoryMeshIndex(
		ESnowRumbleCustomizationAccessory::Nose,
		AppliedCustomizationData.NoseMeshIndex);
	NoseMeshComponent->SetRelativeTransform(
		ResolveCustomizationAccessoryTransform(
			CustomizationNoseRelativeTransforms,
			Index,
			CustomizationNoseRelativeLocation,
			CustomizationNoseRelativeRotation,
			CustomizationNoseRelativeScale));
	UStaticMesh* AccessoryMesh = Index != INDEX_NONE ? CustomizationNoseMeshes[Index] : nullptr;
	NoseMeshComponent->SetStaticMesh(AccessoryMesh);
	NoseMeshComponent->SetVisibility(AccessoryMesh != nullptr, true);
}

void ASnowRumbleCharacter::RefreshCustomizationEarmuffsMesh()
{
	if (!EarmuffsMeshComponent)
	{
		return;
	}
	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		EarmuffsMeshComponent->AttachToComponent(
			CharacterMesh,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			CustomizationEarmuffsAttachSocketName);
	}
	const int32 Index = NormalizeCustomizationAccessoryMeshIndex(
		ESnowRumbleCustomizationAccessory::Earmuffs,
		AppliedCustomizationData.EarmuffsMeshIndex);
	EarmuffsMeshComponent->SetRelativeTransform(
		ResolveCustomizationAccessoryTransform(
			CustomizationEarmuffsRelativeTransforms,
			Index,
			CustomizationEarmuffsRelativeLocation,
			CustomizationEarmuffsRelativeRotation,
			CustomizationEarmuffsRelativeScale));
	UStaticMesh* AccessoryMesh = Index != INDEX_NONE ? CustomizationEarmuffsMeshes[Index] : nullptr;
	EarmuffsMeshComponent->SetStaticMesh(AccessoryMesh);
	EarmuffsMeshComponent->SetVisibility(AccessoryMesh != nullptr, true);
}

void ASnowRumbleCharacter::RefreshScarfMesh()
{
	if (!ScarfMeshComponent)
	{
		return;
	}

	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		const FName AttachSocketName = ScarfAttachSocketName.IsNone()
			? NAME_None
			: ScarfAttachSocketName;
		ScarfMeshComponent->AttachToComponent(
			CharacterMesh,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			AttachSocketName);
	}

	ScarfMeshComponent->SetRelativeLocation(ScarfRelativeLocation);
	ScarfMeshComponent->SetRelativeRotation(ScarfRelativeRotation);
	ScarfMeshComponent->SetRelativeScale3D(ScarfRelativeScale);
	ScarfMeshComponent->SetStaticMesh(ScarfMesh);
	ScarfMeshComponent->SetVisibility(ScarfMesh != nullptr, true);
	ScarfDynamicMaterial = ScarfMesh
		? ScarfMeshComponent->CreateDynamicMaterialInstance(0)
		: nullptr;
	RefreshScarfTeamColorMaterial();
}

void ASnowRumbleCharacter::RefreshScarfTeamColorMaterial()
{
	if (!ScarfMeshComponent || !ScarfMesh || ScarfTeamColorParameterName.IsNone())
	{
		return;
	}

	if (!ScarfDynamicMaterial)
	{
		ScarfDynamicMaterial =
			ScarfMeshComponent->CreateDynamicMaterialInstance(0);
	}
	if (!ScarfDynamicMaterial)
	{
		return;
	}

	ScarfDynamicMaterial->SetVectorParameterValue(
		ScarfTeamColorParameterName,
		GetOverheadTeamColor());
}

void ASnowRumbleCharacter::HandleGiftItemEffectsChanged()
{
	RefreshGiftItemEquipmentMeshes();
	ApplyMovementSpeed();
}

void ASnowRumbleCharacter::RefreshGiftItemEquipmentMeshes()
{
	const UGiftItemEffectComponent* ItemEffects = GiftItemEffectComponent;

	RefreshGiftItemEquipmentMeshSlot(
		LeftBootsMeshComponent,
		ItemEffects && ItemEffects->HasBoots()
			? LeftBootsEquipmentMesh
			: nullptr,
		LeftBootsEquipmentAttachSocketName,
		LeftBootsEquipmentRelativeLocation,
		LeftBootsEquipmentRelativeRotation,
		LeftBootsEquipmentRelativeScale);

	RefreshGiftItemEquipmentMeshSlot(
		RightBootsMeshComponent,
		ItemEffects && ItemEffects->HasBoots()
			? RightBootsEquipmentMesh
			: nullptr,
		RightBootsEquipmentAttachSocketName,
		RightBootsEquipmentRelativeLocation,
		RightBootsEquipmentRelativeRotation,
		RightBootsEquipmentRelativeScale);

	RefreshGiftItemEquipmentMeshSlot(
		LeftGlovesMeshComponent,
		ItemEffects && ItemEffects->HasGloves()
			? LeftGlovesEquipmentMesh
			: nullptr,
		LeftGlovesEquipmentAttachSocketName,
		LeftGlovesEquipmentRelativeLocation,
		LeftGlovesEquipmentRelativeRotation,
		LeftGlovesEquipmentRelativeScale);

	RefreshGiftItemEquipmentMeshSlot(
		RightGlovesMeshComponent,
		ItemEffects && ItemEffects->HasGloves()
			? RightGlovesEquipmentMesh
			: nullptr,
		RightGlovesEquipmentAttachSocketName,
		RightGlovesEquipmentRelativeLocation,
		RightGlovesEquipmentRelativeRotation,
		RightGlovesEquipmentRelativeScale);

	RefreshGiftItemEquipmentMeshSlot(
		PaddingMeshComponent,
		ItemEffects && ItemEffects->HasPadding()
			? PaddingEquipmentMesh
			: nullptr,
		PaddingEquipmentAttachSocketName,
		PaddingEquipmentRelativeLocation,
		PaddingEquipmentRelativeRotation,
		PaddingEquipmentRelativeScale);

	RefreshGiftItemEquipmentMeshSlot(
		HotPackMeshComponent,
		ItemEffects && ItemEffects->HasAnyHotPack()
			? ItemEffects->HasGoldenHotPack()
				? GoldenHotPackEquipmentMesh
				: HotPackEquipmentMesh
			: nullptr,
		HotPackEquipmentAttachSocketName,
		HotPackEquipmentRelativeLocation,
		HotPackEquipmentRelativeRotation,
		HotPackEquipmentRelativeScale);

	UStaticMesh* ShovelMesh = nullptr;
	if (ItemEffects)
	{
		switch (ItemEffects->GetEquippedShovelItemType())
		{
		case ESnowRumbleGiftItemType::SnowShovel:
			ShovelMesh = SnowShovelEquipmentMesh;
			break;

		case ESnowRumbleGiftItemType::GoldenShovel:
			ShovelMesh = GoldenShovelEquipmentMesh;
			break;

		default:
			break;
		}
	}
	RefreshGiftItemEquipmentMeshSlot(
		ShovelMeshComponent,
		ShovelMesh,
		ShovelEquipmentAttachSocketName,
		ShovelEquipmentRelativeLocation,
		ShovelEquipmentRelativeRotation,
		ShovelEquipmentRelativeScale);

	UStaticMesh* DuckMakerMesh = nullptr;
	if (ItemEffects)
	{
		switch (ItemEffects->GetEquippedDuckMakerItemType())
		{
		case ESnowRumbleGiftItemType::SnowDuckMaker:
			DuckMakerMesh = SnowDuckMakerEquipmentMesh;
			break;

		case ESnowRumbleGiftItemType::GoldenDuckMaker:
			DuckMakerMesh = GoldenDuckMakerEquipmentMesh;
			break;

		default:
			break;
		}
	}
	RefreshGiftItemEquipmentMeshSlot(
		DuckMakerMeshComponent,
		DuckMakerMesh,
		DuckMakerEquipmentAttachSocketName,
		DuckMakerEquipmentRelativeLocation,
		DuckMakerEquipmentRelativeRotation,
		DuckMakerEquipmentRelativeScale);
}

void ASnowRumbleCharacter::RefreshGiftItemEquipmentMeshSlot(
	UStaticMeshComponent* SlotComponent,
	UStaticMesh* SlotMesh,
	FName AttachSocketName,
	const FVector& RelativeLocation,
	const FRotator& RelativeRotation,
	const FVector& RelativeScale) const
{
	if (!SlotComponent)
	{
		return;
	}

	if (USkeletalMeshComponent* CharacterMesh = GetMesh())
	{
		SlotComponent->AttachToComponent(
			CharacterMesh,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			AttachSocketName.IsNone() ? NAME_None : AttachSocketName);
	}

	SlotComponent->SetRelativeLocation(RelativeLocation);
	SlotComponent->SetRelativeRotation(RelativeRotation);
	SlotComponent->SetRelativeScale3D(RelativeScale);
	SlotComponent->SetStaticMesh(SlotMesh);
	SlotComponent->SetVisibility(SlotMesh != nullptr, true);
}

void ASnowRumbleCharacter::RedrawCustomizationPaintTexture()
{
	if (CustomizationMaterialIndex < 0
		|| CustomizationPaintTextureParameterName.IsNone())
	{
		return;
	}

	TArray<USkeletalMeshComponent*> MeshComponents;
	GetComponents(MeshComponents);
	for (USkeletalMeshComponent* MeshComponent : MeshComponents)
	{
		if (!MeshComponent)
		{
			continue;
		}

		const int32 MaterialCount = MeshComponent->GetNumMaterials();
		for (int32 MaterialIndex = 0;
			MaterialIndex < MaterialCount;
			++MaterialIndex)
		{
			ActiveCustomizationPaintMeshComponentName = MeshComponent->GetFName();
			ActiveCustomizationPaintMaterialIndex = MaterialIndex;
			UCanvasRenderTarget2D* PaintRenderTarget =
				EnsureCustomizationPaintRenderTarget(
					ActiveCustomizationPaintMaterialIndex);
			if (!PaintRenderTarget)
			{
				continue;
			}

			PaintRenderTarget->UpdateResource();
			ApplyCustomizationPaintTextureToMesh(
				MeshComponent,
				MaterialIndex,
				PaintRenderTarget);
		}
	}
	ActiveCustomizationPaintMeshComponentName = NAME_None;
	ActiveCustomizationPaintMaterialIndex = INDEX_NONE;
}

UCanvasRenderTarget2D*
ASnowRumbleCharacter::EnsureCustomizationPaintRenderTarget(
	int32 TargetMaterialIndex)
{
	if (TObjectPtr<UCanvasRenderTarget2D>* ExistingRenderTarget =
		CustomizationPaintRenderTargets.Find(TargetMaterialIndex))
	{
		return ExistingRenderTarget->Get();
	}

	const int32 SafeRenderTargetSize = FMath::Clamp(
		CustomizationPaintRenderTargetSize,
		64,
		4096);
	UCanvasRenderTarget2D* NewRenderTarget =
		UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(
			this,
			UCanvasRenderTarget2D::StaticClass(),
			SafeRenderTargetSize,
			SafeRenderTargetSize);
	if (!NewRenderTarget)
	{
		return nullptr;
	}

	NewRenderTarget->ClearColor = FLinearColor::Transparent;
	NewRenderTarget->OnCanvasRenderTargetUpdate.AddUniqueDynamic(
		this,
		&ASnowRumbleCharacter::HandleCustomizationPaintCanvasUpdate);
	CustomizationPaintRenderTargets.Add(
		TargetMaterialIndex,
		NewRenderTarget);
	return NewRenderTarget;
}

void ASnowRumbleCharacter::ApplyCustomizationPaintTextureToMesh(
	USkeletalMeshComponent* MeshComponent,
	int32 TargetMaterialIndex,
	UTexture* PaintTexture)
{
	if (!MeshComponent
		|| TargetMaterialIndex < 0
		|| TargetMaterialIndex >= MeshComponent->GetNumMaterials()
		|| CustomizationPaintTextureParameterName.IsNone()
		|| !PaintTexture)
	{
		return;
	}

	UMaterialInstanceDynamic* DynamicMaterial =
		Cast<UMaterialInstanceDynamic>(
			MeshComponent->GetMaterial(TargetMaterialIndex));
	if (!DynamicMaterial)
	{
		DynamicMaterial =
			MeshComponent->CreateAndSetMaterialInstanceDynamic(
				TargetMaterialIndex);
	}
	if (DynamicMaterial)
	{
		DynamicMaterial->SetTextureParameterValue(
			CustomizationPaintTextureParameterName,
			PaintTexture);
		if (MeshComponent == GetMesh())
		{
			CustomizationMaterialInstance = DynamicMaterial;
		}
	}
}

void ASnowRumbleCharacter::HandleCustomizationPaintCanvasUpdate(
	UCanvas* Canvas,
	int32 Width,
	int32 Height)
{
	if (!Canvas)
	{
		return;
	}

	for (const FSnowRumblePaintStroke& Stroke
		: AppliedCustomizationData.PaintStrokes)
	{
		if (!Stroke.MeshComponentName.IsNone()
			&& Stroke.MeshComponentName
				!= ActiveCustomizationPaintMeshComponentName)
		{
			continue;
		}
		if (Stroke.MaterialIndex != INDEX_NONE
			&& Stroke.MaterialIndex
				!= ActiveCustomizationPaintMaterialIndex)
		{
			continue;
		}

		DrawCustomizationPaintStrokeToCanvas(Canvas, Stroke, Width, Height);
	}
}

void ASnowRumbleCharacter::DrawCustomizationPaintStrokeToCanvas(
	UCanvas* Canvas,
	const FSnowRumblePaintStroke& Stroke,
	int32 Width,
	int32 Height) const
{
	if (!Canvas || Stroke.Points.IsEmpty())
	{
		return;
	}

	auto ToCanvasPoint = [
		Width,
		Height,
		bFlipY = AppliedCustomizationData.bFlipPaintUvY](const FVector2D& PaintUv)
	{
		const float PaintY = bFlipY ? PaintUv.Y : 1.0f - PaintUv.Y;
		return FVector2D(
			PaintUv.X * static_cast<float>(Width),
			PaintY * static_cast<float>(Height));
	};

	if (Stroke.Points.Num() == 1)
	{
		const FVector2D Point = ToCanvasPoint(Stroke.Points[0]);
		Canvas->K2_DrawLine(
			Point - FVector2D(1.0f, 0.0f),
			Point + FVector2D(1.0f, 0.0f),
			Stroke.BrushThickness,
			Stroke.BrushColor);
		return;
	}

	for (int32 PointIndex = 1;
		PointIndex < Stroke.Points.Num();
		++PointIndex)
	{
		Canvas->K2_DrawLine(
			ToCanvasPoint(Stroke.Points[PointIndex - 1]),
			ToCanvasPoint(Stroke.Points[PointIndex]),
			Stroke.BrushThickness,
			Stroke.BrushColor);
	}
}

FSnowRumbleCustomizationData
ASnowRumbleCharacter::GetAppliedCustomizationData() const
{
	return AppliedCustomizationData;
}

void ASnowRumbleCharacter::SetCustomizationPaintTexture(UTexture* PaintTexture)
{
	if (CustomizationMaterialIndex < 0
		|| CustomizationPaintTextureParameterName.IsNone()
		|| !PaintTexture)
	{
		return;
	}

	TArray<USkeletalMeshComponent*> MeshComponents;
	GetComponents(MeshComponents);
	for (USkeletalMeshComponent* MeshComponent : MeshComponents)
	{
		if (!MeshComponent
			|| CustomizationMaterialIndex >= MeshComponent->GetNumMaterials())
		{
			continue;
		}

		UMaterialInstanceDynamic* DynamicMaterial =
			Cast<UMaterialInstanceDynamic>(
				MeshComponent->GetMaterial(CustomizationMaterialIndex));
		if (!DynamicMaterial)
		{
			DynamicMaterial =
				MeshComponent->CreateAndSetMaterialInstanceDynamic(
					CustomizationMaterialIndex);
		}
		if (DynamicMaterial)
		{
			DynamicMaterial->SetTextureParameterValue(
			CustomizationPaintTextureParameterName,
			PaintTexture);
			if (MeshComponent == GetMesh())
			{
				CustomizationMaterialInstance = DynamicMaterial;
			}
		}
	}
}

void ASnowRumbleCharacter::ClientFocusLobbyBoard_Implementation(
	ALobbyInteractionBoard* Board)
{
	if (!IsLocallyControlled() || !Board)
	{
		return;
	}

	APlayerController* PlayerController =
		Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	if (FocusedLobbyBoard == Board)
	{
		ClearLobbyBoardFocus();
		return;
	}

	if (FocusedLobbyBoard)
	{
		FocusedLobbyBoard->SetFocusedCharacter(nullptr);
	}

	FocusedLobbyBoard = Board;
	FocusedLobbyBoard->SetFocusedCharacter(this);
	bLobbyBoardPointerPressed = false;
	PlayerController->SetViewTargetWithBlend(Board, 0.15f);

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(true);
	PlayerController->SetIgnoreMoveInput(true);
	PlayerController->SetIgnoreLookInput(true);
	if (LobbyBoardWidgetInteractionComponent)
	{
		ConfigureLobbyBoardWidgetInteraction(PlayerController);
		LobbyBoardWidgetInteractionComponent->SetActive(true);
	}
}

void ASnowRumbleCharacter::ClientFocusPhotoActor_Implementation(
	APhotoInteractionActor* PhotoActor)
{
	if (!IsLocallyControlled() || !PhotoActor)
	{
		return;
	}

	if (FocusedLobbyBoard)
	{
		ClearLobbyBoardFocus();
	}

	APlayerController* PlayerController =
		Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	FocusedPhotoActor = PhotoActor;
	PhotoFocusViewPitchDegrees =
		FRotator::NormalizeAxis(GetBaseAimRotation().Pitch);
	if (PhotoInteractionWidget)
	{
		PhotoInteractionWidget->RemoveFromParent();
		PhotoInteractionWidget = nullptr;
	}
	if (PhotoInteractionWidgetClass)
	{
		PhotoInteractionWidget =
			CreateWidget<UUserWidget>(PlayerController, PhotoInteractionWidgetClass);
		if (PhotoInteractionWidget)
		{
			PhotoInteractionWidget->AddToViewport(100);
		}
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		bOrientRotationToMovementBeforePhotoFocus =
			MovementComponent->bOrientRotationToMovement;
		MovementComponent->bOrientRotationToMovement = false;
	}
	bUseControllerRotationYawBeforePhotoFocus = bUseControllerRotationYaw;
	bUseControllerRotationYaw = true;

	if (UCameraComponent* PhotoCamera =
		PhotoActor->GetPhotoCameraComponent())
	{
		PhotoCamera->SetActive(true);
	}

	PlayerController->SetViewTargetWithBlend(PhotoActor, 0.15f);
	PlayerController->SetInputMode(FInputModeGameOnly());
	PlayerController->SetShowMouseCursor(false);
	PlayerController->SetIgnoreMoveInput(false);
	PlayerController->SetIgnoreLookInput(false);
}

void ASnowRumbleCharacter::ClosePhotoActorFocus()
{
	if (!IsLocallyControlled() || !FocusedPhotoActor)
	{
		return;
	}

	if (PhotoInteractionWidget)
	{
		PhotoInteractionWidget->RemoveFromParent();
		PhotoInteractionWidget = nullptr;
	}

	if (UCameraComponent* PhotoCamera =
		FocusedPhotoActor->GetPhotoCameraComponent())
	{
		PhotoCamera->SetActive(false);
	}

	if (APlayerController* PlayerController =
		Cast<APlayerController>(GetController()))
	{
		PlayerController->SetViewTargetWithBlend(this, 0.15f);
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
		PlayerController->SetIgnoreMoveInput(false);
		PlayerController->SetIgnoreLookInput(false);
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement =
			bOrientRotationToMovementBeforePhotoFocus;
	}
	bUseControllerRotationYaw = bUseControllerRotationYawBeforePhotoFocus;

	FocusedPhotoActor = nullptr;
}

void ASnowRumbleCharacter::RefreshHeldEquipmentMovementState()
{
	if (SnowballEquipmentComponent
		&& SnowballEquipmentComponent->IsHoldingLargeSnowball())
	{
		bIsSprinting = false;
	}

	ApplyMovementSpeed();
}

void ASnowRumbleCharacter::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASnowRumbleCharacter, bIsSprinting);
	DOREPLIFETIME(ASnowRumbleCharacter, bIsPickingUpItem);
	DOREPLIFETIME(ASnowRumbleCharacter, bIsInteractingWithItem);
	DOREPLIFETIME(ASnowRumbleCharacter, bIsHitReacting);
	DOREPLIFETIME(ASnowRumbleCharacter, bTiebreakerSpectator);
	DOREPLIFETIME(ASnowRumbleCharacter, bWaterSubmerged);
	DOREPLIFETIME(ASnowRumbleCharacter, bIsGrabbedByCharacter);
	DOREPLIFETIME(ASnowRumbleCharacter, GrabbedByCharacter);
	DOREPLIFETIME(ASnowRumbleCharacter, GrabbedByCharacterWorldLocation);
	DOREPLIFETIME(ASnowRumbleCharacter, ReplicatedSpectatorCameraLocation);
	DOREPLIFETIME(ASnowRumbleCharacter, ReplicatedSpectatorCameraRotation);
	DOREPLIFETIME(ASnowRumbleCharacter, ReplicatedSpectatorCameraFieldOfView);
	DOREPLIFETIME(ASnowRumbleCharacter, bHasReplicatedSpectatorCameraView);
}

void ASnowRumbleCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	BindOverheadNameToPlayerState();
	BindCustomizationToPlayerState();
	RefreshOverheadPlayerName();
	RefreshCustomizationFromPlayerState();
}

void ASnowRumbleCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	BindOverheadNameToPlayerState();
	BindCustomizationToPlayerState();
	RefreshOverheadPlayerName();
	RefreshCustomizationFromPlayerState();
}

void ASnowRumbleCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	ApplyInputMappingContext();
	ApplyCameraPitchLimits();
	RefreshLocalSnowEffect();
	EnsureEmoteRadialMenuWidget();
	EnsureMainHUDWidget();
	RefreshPvpMatchInputLock();
}

void ASnowRumbleCharacter::BindOverheadNameToPlayerState()
{
	if (ASnowRumblePlayerState* SnowRumblePlayerState =
		GetPlayerState<ASnowRumblePlayerState>())
	{
		SnowRumblePlayerState->OnLobbyPlayerChanged.AddUniqueDynamic(
			this,
			&ASnowRumbleCharacter::RefreshOverheadPlayerName);
	}
}

void ASnowRumbleCharacter::BindCustomizationToPlayerState()
{
	if (ASnowRumblePlayerState* SnowRumblePlayerState =
		GetPlayerState<ASnowRumblePlayerState>())
	{
		SnowRumblePlayerState->OnCustomizationChanged.AddUniqueDynamic(
			this,
			&ASnowRumbleCharacter::RefreshCustomizationFromPlayerState);
	}
}

void ASnowRumbleCharacter::RefreshOverheadNameplateComponentSettings()
{
	if (!OverheadNameplateComponent)
	{
		return;
	}

	FVector NameplateRelativeLocation = OverheadNameRelativeLocation;
	const int32 HatMeshIndex = NormalizeCustomizationHatMeshIndex(
		AppliedCustomizationData.HatMeshIndex);
	const bool bHasHat = HatMeshIndex != INDEX_NONE
		&& CustomizationHatMeshes.IsValidIndex(HatMeshIndex)
		&& CustomizationHatMeshes[HatMeshIndex] != nullptr;
	if (bHasHat)
	{
		NameplateRelativeLocation.Z += OverheadNameplateHatZOffset;
	}
	OverheadNameplateComponent->SetRelativeLocation(NameplateRelativeLocation);
	OverheadNameplateComponent->SetWidgetSpace(EWidgetSpace::World);
	OverheadNameplateComponent->SetDrawAtDesiredSize(true);
	OverheadNameplateComponent->SetDrawSize(OverheadNameplateDrawSize);
	const float SafeWorldScale = FMath::Max(0.001f, OverheadNameplateWorldScale);
	OverheadNameplateComponent->SetRelativeScale3D(
		FVector(SafeWorldScale, SafeWorldScale, SafeWorldScale));
	if (OverheadNameplateWidgetClass)
	{
		OverheadNameplateComponent->SetWidgetClass(OverheadNameplateWidgetClass);
	}
}

void ASnowRumbleCharacter::RefreshOverheadNameplateFacing()
{
	if (!OverheadNameplateComponent)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const bool bHideLocalPvpNameplate =
		IsLocallyControlled()
		&& World
		&& World->GetGameState<ASnowRumbleGameState>() != nullptr;
	OverheadNameplateComponent->SetVisibility(!bHideLocalPvpNameplate, true);
	if (bHideLocalPvpNameplate)
	{
		return;
	}

	const APlayerCameraManager* CameraManager = World
		? UGameplayStatics::GetPlayerCameraManager(World, 0)
		: nullptr;
	if (!CameraManager)
	{
		return;
	}

	const FVector NameplateLocation =
		OverheadNameplateComponent->GetComponentLocation();
	const FVector CameraLocation = CameraManager->GetCameraLocation();
	const FRotator LookAtCameraRotation =
		(CameraLocation - NameplateLocation).Rotation();
	OverheadNameplateComponent->SetWorldRotation(LookAtCameraRotation);
}

void ASnowRumbleCharacter::RefreshOverheadPlayerName()
{
	if (!OverheadNameplateComponent)
	{
		return;
	}

	OverheadNameplateComponent->InitWidget();
	if (UOverheadNameplateWidget* NameplateWidget =
		Cast<UOverheadNameplateWidget>(OverheadNameplateComponent->GetUserWidgetObject()))
	{
		NameplateWidget->SetObservedCharacter(this);
	}

	RefreshScarfTeamColorMaterial();
}

void ASnowRumbleCharacter::RefreshCustomizationFromPlayerState()
{
	const ASnowRumblePlayerState* SnowRumblePlayerState =
		GetPlayerState<ASnowRumblePlayerState>();
	if (!SnowRumblePlayerState)
	{
		return;
	}

	ApplyCustomizationData(SnowRumblePlayerState->GetCustomizationData());
}

void ASnowRumbleCharacter::ApplyCameraPitchLimits()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	APlayerController* PlayerController =
		Cast<APlayerController>(GetController());
	if (PlayerController && PlayerController->PlayerCameraManager)
	{
		PlayerController->PlayerCameraManager->ViewPitchMin =
			CameraViewPitchMin;
		PlayerController->PlayerCameraManager->ViewPitchMax =
			CameraViewPitchMax;
	}
}

void ASnowRumbleCharacter::RefreshLocalSnowEffect()
{
	if (!LocalSnowEffect)
	{
		return;
	}

	const bool bShouldShowLocalSnow = ShouldShowLocalSnowEffect();
	if (bLocalSnowEffectActive == bShouldShowLocalSnow)
	{
		return;
	}

	bLocalSnowEffectActive = bShouldShowLocalSnow;
	LocalSnowEffect->SetVisibility(bShouldShowLocalSnow, true);

	if (bShouldShowLocalSnow)
	{
		LocalSnowEffect->Activate(true);
	}
	else
	{
		LocalSnowEffect->Deactivate();
	}
}

bool ASnowRumbleCharacter::ShouldShowLocalSnowEffect() const
{
	if (!IsLocallyControlled())
	{
		return false;
	}

	const UWorld* World = GetWorld();
	return World
		&& World->GetGameState()
		&& !World->GetGameState<ASnowRumbleLobbyGameState>();
}

void ASnowRumbleCharacter::EnsureEmoteRadialMenuWidget()
{
	if (!IsLocallyControlled()
		|| EmoteRadialMenuWidget
		|| !EmoteRadialMenuWidgetClass
		|| ShouldSuppressPvpWidgets())
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController)
	{
		return;
	}

	EmoteRadialMenuWidget =
		CreateWidget<UEmoteRadialMenuWidget>(
			PlayerController,
			EmoteRadialMenuWidgetClass);
	if (EmoteRadialMenuWidget)
	{
		EmoteRadialMenuWidget->AddToViewport();
		EmoteRadialMenuWidget->CloseEmoteMenu();
	}
}

void ASnowRumbleCharacter::EnsureKeyGuideWidget()
{
	if (!IsLocallyControlled()
		|| KeyGuideWidget
		|| !KeyGuideWidgetClass
		|| ShouldSuppressPvpWidgets())
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController)
	{
		return;
	}

	KeyGuideWidget =
		CreateWidget<UKeyGuideWidget>(
			PlayerController,
			KeyGuideWidgetClass);
	if (KeyGuideWidget)
	{
		KeyGuideWidget->AddToViewport();
		KeyGuideWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ASnowRumbleCharacter::EnsureMainHUDWidget()
{
	if (!IsLocallyControlled()
		|| MainHUDWidget
		|| ShouldSuppressPvpWidgets())
	{
		return;
	}

	if (Cast<ACustomizationPlayerController>(Controller)
		|| Cast<AMainMenuPlayerController>(Controller))
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (World && World->GetGameState<ASnowRumbleLobbyGameState>())
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController)
	{
		return;
	}

	TSubclassOf<UMainHUDWidget> HudWidgetClass = MainHUDWidgetClass;
	if (UClass* PreferredHudClass = LoadClass<UMainHUDWidget>(
		nullptr,
		TEXT("/Game/WBP/WBP_MainHUDWidget.WBP_MainHUDWidget_C")))
	{
		HudWidgetClass = PreferredHudClass;
	}
	if (!HudWidgetClass)
	{
		return;
	}

	MainHUDWidget =
		CreateWidget<UMainHUDWidget>(
			PlayerController,
			HudWidgetClass);
	if (MainHUDWidget)
	{
		MainHUDWidget->AddToViewport();
	}
}

void ASnowRumbleCharacter::EnsureInteractionPromptWidget()
{
	if (!IsLocallyControlled()
		|| InteractionPromptWidget
		|| !InteractionPromptWidgetClass
		|| ShouldSuppressPvpWidgets())
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController)
	{
		return;
	}

	InteractionPromptWidget =
		CreateWidget<UInteractionPromptWidget>(
			PlayerController,
			InteractionPromptWidgetClass);
	if (InteractionPromptWidget)
	{
		InteractionPromptWidget->AddToViewport(50);
		InteractionPromptWidget->ClearPrompt();
	}
}

void ASnowRumbleCharacter::RefreshInteractionPromptWidget()
{
	if (!IsLocallyControlled())
	{
		return;
	}
	if (bPvpIntroWidgetsHidden)
	{
		if (InteractionPromptWidget)
		{
			InteractionPromptWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		return;
	}

	EnsureInteractionPromptWidget();
	if (!InteractionPromptWidget)
	{
		return;
	}

	FText PromptText;
	AActor* PromptActor = nullptr;
	if (!GetCurrentInteractionPromptData(PromptText, PromptActor))
	{
		InteractionPromptWidget->ClearPrompt();
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController || !PromptActor)
	{
		InteractionPromptWidget->ClearPrompt();
		return;
	}

	const FVector PromptRootLocation = PromptActor->GetRootComponent()
		? PromptActor->GetRootComponent()->GetComponentLocation()
		: PromptActor->GetActorLocation();
	const FVector PromptWorldLocation = PromptRootLocation
		+ FVector(0.0f, 0.0f, InteractionPromptWorldHeightOffset);

	FVector2D PromptScreenPosition;
	if (!PlayerController->ProjectWorldLocationToScreen(
		PromptWorldLocation,
		PromptScreenPosition,
		true))
	{
		InteractionPromptWidget->ClearPrompt();
		return;
	}

	InteractionPromptWidget->SetPositionInViewport(
		PromptScreenPosition + InteractionPromptScreenOffset,
		true);
	InteractionPromptWidget->SetPromptText(PromptText);
}

bool ASnowRumbleCharacter::GetCurrentInteractionPromptData(
	FText& OutPromptText,
	AActor*& OutPromptActor) const
{
	OutPromptText = FText::GetEmpty();
	OutPromptActor = nullptr;

	if (!IsLocallyControlled()
		|| !CanPerformGameplayAction()
		|| FocusedLobbyBoard
		|| FocusedPhotoActor)
	{
		return false;
	}

	if (APhotoInteractionActor* PhotoActor =
		FindClosestPhotoInteractionCandidate())
	{
		OutPromptText = NSLOCTEXT(
			"SnowRumble",
			"InteractPromptPhoto",
			"E - 사진찍기");
		OutPromptActor = PhotoActor;
		return true;
	}

	if (AJukeboxActor* Jukebox = FindClosestJukeboxCandidate())
	{
		OutPromptText = Jukebox->IsPlaying()
			? (Jukebox->IsCharacterParticipating(this)
				? NSLOCTEXT(
					"SnowRumble",
					"InteractPromptJukeboxOptOut",
					"E - 참여 안하기")
				: NSLOCTEXT(
					"SnowRumble",
					"InteractPromptJukeboxOptIn",
					"E - 참여하기"))
			: NSLOCTEXT(
				"SnowRumble",
				"InteractPromptJukeboxStart",
				"E - 노래틀기");
		OutPromptActor = Jukebox;
		return true;
	}

	if (ALobbyInteractionBoard* Board = FindClosestLobbyBoardCandidate())
	{
		OutPromptText = NSLOCTEXT(
			"SnowRumble",
			"InteractPromptBoard",
			"E - 게시판");
		OutPromptActor = Board;
		return true;
	}

	if (ASnowRumbleCharacter* FrozenTeammate =
		FindClosestFrozenTeammateCandidate())
	{
		OutPromptText = NSLOCTEXT(
			"SnowRumble",
			"InteractPromptReviveTeammate",
			"E - 살리기");
		OutPromptActor = FrozenTeammate;
		return true;
	}

	if (AGiftBox* GiftBox = FindClosestGiftBoxCandidate())
	{
		OutPromptText = NSLOCTEXT(
			"SnowRumble",
			"InteractPromptGiftBox",
			"E - 선물상자");
		OutPromptActor = GiftBox;
		return true;
	}

	if (AGiftBoxItemPickup* Pickup = FindClosestGiftBoxItemPickupCandidate())
	{
		OutPromptText = FText::Format(
			NSLOCTEXT(
				"SnowRumble",
				"InteractPromptGiftBoxItem",
				"E - {0}"),
			Pickup->GetDisplayName());
		OutPromptActor = Pickup;
		return true;
	}

	if (!SnowballEquipmentComponent
		|| SnowballEquipmentComponent->IsRollingSnowball()
		|| SnowballEquipmentComponent->HasHeldSnowball())
	{
		return false;
	}

	ASnowballItem* Snowball =
		SnowballEquipmentComponent->FindClosestPickupCandidate();
	if (!Snowball)
	{
		return false;
	}

	OutPromptText = NSLOCTEXT(
		"SnowRumble",
		"InteractPromptSnowball",
		"E - 눈덩이");
	OutPromptActor = Snowball;
	return true;
}

void ASnowRumbleCharacter::OpenEmoteRadialMenu()
{
	if (!IsLocallyControlled() || bPvpIntroWidgetsHidden)
	{
		return;
	}
	if (bIsEmoteRadialMenuOpen)
	{
		return;
	}

	EnsureEmoteRadialMenuWidget();
	if (EmoteRadialMenuWidget)
	{
		EmoteRadialMenuWidget->OpenEmoteMenu();
	}

	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController && EmoteRadialMenuWidget)
	{
		if (ASnowRumblePlayerController* SnowRumblePlayerController =
			Cast<ASnowRumblePlayerController>(PlayerController))
		{
			SnowRumblePlayerController->EnableDefaultCursorUiInput(
				EmoteRadialMenuWidget,
				false);
		}
		else
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(EmoteRadialMenuWidget->TakeWidget());
			InputMode.SetHideCursorDuringCapture(false);
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(true);
		}
		PlayerController->SetIgnoreLookInput(true);
	}

	bIsEmoteRadialMenuOpen = true;
}

void ASnowRumbleCharacter::CloseEmoteRadialMenu()
{
	if (EmoteRadialMenuWidget)
	{
		EmoteRadialMenuWidget->CloseEmoteMenu();
	}

	if (IsLocallyControlled())
	{
		if (APlayerController* PlayerController =
			Cast<APlayerController>(Controller))
		{
			if (ASnowRumblePlayerController* SnowRumblePlayerController =
				Cast<ASnowRumblePlayerController>(PlayerController))
			{
				SnowRumblePlayerController->RestoreGameOnlyInput();
			}
			else
			{
				FInputModeGameOnly InputMode;
				PlayerController->SetInputMode(InputMode);
				PlayerController->SetShowMouseCursor(false);
			}
			PlayerController->SetIgnoreLookInput(false);
		}
	}

	bIsEmoteRadialMenuOpen = false;
}

void ASnowRumbleCharacter::OpenKeyGuideWidget()
{
	if (!IsLocallyControlled() || bPvpIntroWidgetsHidden)
	{
		return;
	}
	if (bIsKeyGuideWidgetOpen)
	{
		return;
	}

	EnsureKeyGuideWidget();
	if (!KeyGuideWidget)
	{
		return;
	}

	KeyGuideWidget->RefreshKeyGuideTexts();
	KeyGuideWidget->SetVisibility(ESlateVisibility::Visible);

	if (APlayerController* PlayerController =
		Cast<APlayerController>(Controller))
	{
		if (ASnowRumblePlayerController* SnowRumblePlayerController =
			Cast<ASnowRumblePlayerController>(PlayerController))
		{
			SnowRumblePlayerController->EnableDefaultCursorUiInput(
				KeyGuideWidget,
				false);
		}
		else
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(KeyGuideWidget->TakeWidget());
			InputMode.SetHideCursorDuringCapture(false);
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(true);
		}
		PlayerController->SetIgnoreLookInput(true);
	}

	bIsKeyGuideWidgetOpen = true;
}

void ASnowRumbleCharacter::CloseKeyGuideWidget()
{
	const bool bWasKeyGuideWidgetOpen = bIsKeyGuideWidgetOpen;

	if (KeyGuideWidget)
	{
		KeyGuideWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (IsLocallyControlled() && bWasKeyGuideWidgetOpen)
	{
		if (APlayerController* PlayerController =
			Cast<APlayerController>(Controller))
		{
			if (ASnowRumblePlayerController* SnowRumblePlayerController =
				Cast<ASnowRumblePlayerController>(PlayerController))
			{
				SnowRumblePlayerController->RestoreGameOnlyInput();
			}
			else
			{
				FInputModeGameOnly InputMode;
				PlayerController->SetInputMode(InputMode);
				PlayerController->SetShowMouseCursor(false);
			}
			PlayerController->SetIgnoreLookInput(false);
		}
	}

	bIsKeyGuideWidgetOpen = false;
}

void ASnowRumbleCharacter::DrawRollingSnowballCollisionDebug() const
{
	if (!bDrawRollingSnowballCollisionDebug
		|| !IsLocallyControlled()
		|| !RollingSnowballCollision
		|| !SnowballEquipmentComponent)
	{
		return;
	}

	const ASnowballItem* RollingSnowball =
		SnowballEquipmentComponent->GetRollingSnowball();
	if (!RollingSnowball)
	{
		return;
	}

	const bool bDrawServerProxy =
		HasAuthority()
		&& RollingSnowballCollision->GetCollisionEnabled()
			!= ECollisionEnabled::NoCollision;
	const FVector DebugLocation =
		bDrawServerProxy
			? RollingSnowballCollision->GetComponentLocation()
			: RollingSnowball->GetActorLocation();
	const float DebugRadius =
		bDrawServerProxy
			? RollingSnowballCollision->GetScaledSphereRadius()
			: RollingSnowball->GetRollingCollisionRadius();

	DrawDebugSphere(
		GetWorld(),
		DebugLocation,
		DebugRadius,
		16,
		bDrawServerProxy ? FColor::Green : FColor::Cyan,
		false,
		0.0f,
		0,
		2.0f);
}

void ASnowRumbleCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// PIE 초기화 순서와 관계없이 입력 바인딩 시점에 로컬 매핑을 보장한다.
	ApplyInputMappingContext();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!ensureMsgf(EnhancedInputComponent, TEXT("SnowRumbleCharacter에는 Enhanced Input Component가 필요합니다.")))
	{
		return;
	}

	if (MoveAction)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASnowRumbleCharacter::Move);
	}
	if (LookAction)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASnowRumbleCharacter::Look);
	}
	if (JumpAction)
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASnowRumbleCharacter::StartJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ASnowRumbleCharacter::StopJump);
	}
	if (IsLocallyControlled())
	{
		PlayerInputComponent->BindKey(
			EKeys::A,
			IE_Pressed,
			this,
			&ASnowRumbleCharacter::SelectPreviousSpectatorViewTarget);
		PlayerInputComponent->BindKey(
			EKeys::D,
			IE_Pressed,
			this,
			&ASnowRumbleCharacter::SelectNextSpectatorViewTarget);
	}
	if (MicrophonePushToTalkAction)
	{
		EnhancedInputComponent->BindAction(MicrophonePushToTalkAction, ETriggerEvent::Started, this, &ASnowRumbleCharacter::HandleMicrophonePushToTalkStarted);
		EnhancedInputComponent->BindAction(MicrophonePushToTalkAction, ETriggerEvent::Completed, this, &ASnowRumbleCharacter::HandleMicrophonePushToTalkCompleted);
		EnhancedInputComponent->BindAction(MicrophonePushToTalkAction, ETriggerEvent::Canceled, this, &ASnowRumbleCharacter::HandleMicrophonePushToTalkCompleted);
	}
	if (MicrophoneChannelToggleAction)
	{
		EnhancedInputComponent->BindAction(MicrophoneChannelToggleAction, ETriggerEvent::Started, this, &ASnowRumbleCharacter::HandleMicrophoneChannelToggle);
	}
	if (VoiceTargetMuteAction)
	{
		EnhancedInputComponent->BindAction(VoiceTargetMuteAction, ETriggerEvent::Started, this, &ASnowRumbleCharacter::HandleVoiceTargetMute);
	}
	if (SprintAction)
	{
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ASnowRumbleCharacter::HandleSprintStarted);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASnowRumbleCharacter::HandleSprintCompleted);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Canceled, this, &ASnowRumbleCharacter::HandleSprintCompleted);
	}
	if (InteractAction)
	{
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ASnowRumbleCharacter::HandleInteractStarted);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &ASnowRumbleCharacter::HandleInteractCompleted);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Canceled, this, &ASnowRumbleCharacter::HandleInteractCompleted);
	}
	if (AimAction)
	{
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ASnowRumbleCharacter::HandleAimStarted);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ASnowRumbleCharacter::HandleAimCompleted);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Canceled, this, &ASnowRumbleCharacter::HandleAimCompleted);
	}
	if (ActionAction)
	{
		EnhancedInputComponent->BindAction(ActionAction, ETriggerEvent::Started, this, &ASnowRumbleCharacter::HandleActionStarted);
		EnhancedInputComponent->BindAction(ActionAction, ETriggerEvent::Completed, this, &ASnowRumbleCharacter::HandleActionCompleted);
	}
	if (DropEquipmentAction)
	{
		EnhancedInputComponent->BindAction(DropEquipmentAction, ETriggerEvent::Started, this, &ASnowRumbleCharacter::HandleDropEquipment);
	}
	if (EmoteAction)
	{
		EnhancedInputComponent->BindAction(EmoteAction, ETriggerEvent::Started, this, &ASnowRumbleCharacter::HandleEmoteStarted);
		EnhancedInputComponent->BindAction(EmoteAction, ETriggerEvent::Completed, this, &ASnowRumbleCharacter::HandleEmoteCompleted);
	}
	if (KeyGuideAction)
	{
		EnhancedInputComponent->BindAction(KeyGuideAction, ETriggerEvent::Started, this, &ASnowRumbleCharacter::HandleKeyGuideStarted);
		EnhancedInputComponent->BindAction(KeyGuideAction, ETriggerEvent::Completed, this, &ASnowRumbleCharacter::HandleKeyGuideCompleted);
		EnhancedInputComponent->BindAction(KeyGuideAction, ETriggerEvent::Canceled, this, &ASnowRumbleCharacter::HandleKeyGuideCompleted);
	}
	PlayerInputComponent->BindKey(
		EKeys::P,
		IE_Pressed,
		this,
		&ASnowRumbleCharacter::HandlePhotoCapture);
}

void ASnowRumbleCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (!Controller
		|| (!CanPerformGameplayAction()
			&& !IsHangingFromWorldGrab()))
	{
		return;
	}
	if (const ASnowRumblePlayerController* SnowRumblePlayerController =
		Cast<ASnowRumblePlayerController>(Controller))
	{
		if (SnowRumblePlayerController->IsGameplayUiInputOpen())
		{
			return;
		}
	}
	if (Cast<ACustomizationPlayerController>(Controller)
		|| Cast<AMainMenuPlayerController>(Controller))
	{
		return;
	}

	if (bIsInteractHeld
		&& !bUsedInteractForRolling
		&& !IsHangingFromWorldGrab()
		&& !IsGrabbedByCharacter()
		&& !MovementVector.IsNearlyZero()
		&& SnowballEquipmentComponent)
	{
		bUsedInteractForRolling = true;
		SnowballEquipmentComponent->StartRollingSnowball();
	}

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ASnowRumbleCharacter::Look(const FInputActionValue& Value)
{
	if (bIsEmoteRadialMenuOpen
		|| bIsKeyGuideWidgetOpen
		|| IsPvpMatchInputLocked())
	{
		return;
	}
	if (const ASnowRumblePlayerController* SnowRumblePlayerController =
		Cast<ASnowRumblePlayerController>(Controller))
	{
		if (SnowRumblePlayerController->IsGameplayUiInputOpen())
		{
			return;
		}
	}
	if (Cast<ACustomizationPlayerController>(Controller)
		|| Cast<AMainMenuPlayerController>(Controller))
	{
		return;
	}

	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	float MouseSensitivity = 1.0f;
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
			GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>())
		{
			MouseSensitivity = UserSettingsSubsystem->GetMouseSensitivity();
		}
	}

	if (FMath::Abs(LookAxisVector.X) > KINDA_SMALL_NUMBER)
	{
		CameraShoulderSide = LookAxisVector.X < 0.0f ? 1.0f : -1.0f;
	}

	if (FocusedPhotoActor)
	{
		// Keep the photo camera fixed and apply input only to the player's gaze.
		AddControllerYawInput(LookAxisVector.X * MouseSensitivity);
		const float PitchDelta = -LookAxisVector.Y * MouseSensitivity;
		AddControllerPitchInput(PitchDelta);
		PhotoFocusViewPitchDegrees = FMath::Clamp(
			PhotoFocusViewPitchDegrees + PitchDelta,
			CameraViewPitchMin,
			CameraViewPitchMax);
		return;
	}

	AddControllerYawInput(LookAxisVector.X * MouseSensitivity);
	AddControllerPitchInput(-LookAxisVector.Y * MouseSensitivity);
}

void ASnowRumbleCharacter::UpdateCameraZoomInput()
{
	if (!IsLocallyControlled()
		|| !Controller
		|| FocusedLobbyBoard
		|| bIsEmoteRadialMenuOpen
		|| bIsKeyGuideWidgetOpen)
	{
		return;
	}
	if (const ASnowRumblePlayerController* SnowRumblePlayerController =
		Cast<ASnowRumblePlayerController>(Controller))
	{
		if (SnowRumblePlayerController->IsGameplayUiInputOpen())
		{
			return;
		}
	}

	const APlayerController* PlayerController =
		Cast<APlayerController>(Controller);
	if (!PlayerController)
	{
		return;
	}

	float ZoomDirection = 0.0f;
	if (PlayerController->WasInputKeyJustPressed(EKeys::MouseScrollUp))
	{
		ZoomDirection -= 1.0f;
	}
	if (PlayerController->WasInputKeyJustPressed(EKeys::MouseScrollDown))
	{
		ZoomDirection += 1.0f;
	}
	if (FMath::IsNearlyZero(ZoomDirection))
	{
		return;
	}

	const float SafeMinimumCameraArmLength =
		FMath::Max(0.0f, MinimumCameraArmLength);
	const float SafeMaximumCameraArmLength =
		FMath::Max(SafeMinimumCameraArmLength, MaximumCameraArmLength);
	DesiredCameraArmLength = FMath::Clamp(
		DesiredCameraArmLength + ZoomDirection * CameraZoomStep,
		SafeMinimumCameraArmLength,
		SafeMaximumCameraArmLength);
}

void ASnowRumbleCharacter::StartJump()
{
	if (CanPerformGameplayAction()
		&& !bWaterSubmerged
		&& (!SnowballEquipmentComponent
			|| !SnowballEquipmentComponent->IsRollingSnowball()))
	{
		if (CanJump())
		{
			Jump();
			SnowRumbleAudio::PlaySound2D(
				this,
				JumpSound,
				ESnowRumbleAudioMixChannel::Gameplay);
		}
	}
}

void ASnowRumbleCharacter::StopJump()
{
	StopJumping();
}

void ASnowRumbleCharacter::HandleMicrophonePushToTalkStarted()
{
	if (ASnowRumblePlayerController* SnowRumbleController =
		Cast<ASnowRumblePlayerController>(Controller))
	{
		SnowRumbleController->RequestMicrophonePushToTalkStarted();
	}
}

void ASnowRumbleCharacter::HandleMicrophonePushToTalkCompleted()
{
	if (ASnowRumblePlayerController* SnowRumbleController =
		Cast<ASnowRumblePlayerController>(Controller))
	{
		SnowRumbleController->RequestMicrophonePushToTalkCompleted();
	}
}

void ASnowRumbleCharacter::HandleMicrophoneChannelToggle()
{
	if (ASnowRumblePlayerController* SnowRumbleController =
		Cast<ASnowRumblePlayerController>(Controller))
	{
		SnowRumbleController->RequestVoiceChannelToggle();
	}
}

void ASnowRumbleCharacter::HandleVoiceTargetMute()
{
	if (ASnowRumblePlayerController* SnowRumbleController =
		Cast<ASnowRumblePlayerController>(Controller))
	{
		SnowRumbleController->RequestVoiceTargetMute();
	}
}

void ASnowRumbleCharacter::HandleSprintStarted()
{
	if (!CanPerformGameplayAction()
		|| IsAiming()
		|| (SnowballEquipmentComponent
			&& SnowballEquipmentComponent->IsHoldingLargeSnowball()))
	{
		return;
	}

	bIsSprinting = true;
	ApplyMovementSpeed();

	if (!HasAuthority())
	{
		ServerSetSprinting(true);
	}
}

void ASnowRumbleCharacter::HandleSprintCompleted()
{
	bIsSprinting = false;
	ApplyMovementSpeed();

	if (!HasAuthority())
	{
		ServerSetSprinting(false);
	}
}

void ASnowRumbleCharacter::HandleInteractStarted()
{
	if (CanPerformGameplayAction())
	{
		bIsInteractHeld = true;
		bUsedInteractForRolling = false;
		const ASnowRumbleCharacter* OutlinedTeammate = OutlineComponent
			? Cast<ASnowRumbleCharacter>(OutlineComponent->GetOutlinedActor())
			: nullptr;
		if (OutlinedTeammate
			&& OutlinedTeammate == FindClosestFrozenTeammateCandidate())
		{
			TryStartTeammateRevive();
			return;
		}
		OnInteractInput(true);
	}
}

void ASnowRumbleCharacter::HandleInteractCompleted()
{
	if (bIsRevivingTeammate)
	{
		CancelTeammateRevive();
		bIsInteractHeld = false;
		bUsedInteractForRolling = false;
		OnInteractInput(false);
		return;
	}

	if (SnowballEquipmentComponent)
	{
		if (bUsedInteractForRolling)
		{
			SnowballEquipmentComponent->StopRollingSnowball();
		}
		else if (FocusedLobbyBoard)
		{
			ClearLobbyBoardFocus();
		}
		else if (FocusedPhotoActor)
		{
			ClosePhotoActorFocus();
		}
		else if (bIsInteractHeld && CanPerformGameplayAction())
		{
			const APhotoInteractionActor* OutlinedPhotoActor = OutlineComponent
				? Cast<APhotoInteractionActor>(
					OutlineComponent->GetOutlinedActor())
				: nullptr;
			if (OutlinedPhotoActor
				&& OutlinedPhotoActor
					== FindClosestPhotoInteractionCandidate())
			{
				TryInteractWithPhotoActor();
			}
			else
			{
				AJukeboxActor* Jukebox = FindClosestJukeboxCandidate();
				if (Jukebox)
				{
					TryInteractWithJukebox();
				}
				else
				{
					const ALobbyInteractionBoard* OutlinedBoard = OutlineComponent
						? Cast<ALobbyInteractionBoard>(OutlineComponent->GetOutlinedActor())
						: nullptr;
					if (OutlinedBoard)
					{
						TryInteractWithLobbyBoard();
					}
					else if (OutlineComponent
						&& Cast<AGiftBox>(OutlineComponent->GetOutlinedActor()))
					{
						TryInteractWithGiftBox();
					}
					else if (OutlineComponent
						&& Cast<AGiftBoxItemPickup>(OutlineComponent->GetOutlinedActor()))
					{
						TryPickupGiftBoxItem();
					}
					else
					{
						SnowballEquipmentComponent->TryPickupSnowball();
					}
				}
			}
		}
	}

	bIsInteractHeld = false;
	bUsedInteractForRolling = false;
	OnInteractInput(false);
}

void ASnowRumbleCharacter::HandleAimStarted()
{
	if (CanPerformGameplayAction() && SnowballEquipmentComponent)
	{
		SnowballEquipmentComponent->SetAiming(true);
		OnAimInput(true);
	}
}

void ASnowRumbleCharacter::HandleAimCompleted()
{
	if (SnowballEquipmentComponent)
	{
		if (SnowballEquipmentComponent->IsCharging())
		{
			SnowballEquipmentComponent->HideThrowTrajectoryPreview();

			if (IsLocallyControlled() && GetWorld())
			{
				PostThrowAimCameraEndTime =
					GetWorld()->GetTimeSeconds() + PostThrowCameraHoldSeconds;
			}

			SnowballEquipmentComponent->ReleaseChargedSnowball();
			OnAimInput(false);
			return;
		}

		SnowballEquipmentComponent->SetAiming(false);
	}
	OnAimInput(false);
}

void ASnowRumbleCharacter::HandleActionStarted()
{
	const bool bCanAct = CanPerformGameplayAction();
	USnowballCreationComponent* ActiveCreationComponent =
		SnowballCreationComponent
			? SnowballCreationComponent.Get()
			: FindComponentByClass<USnowballCreationComponent>();

	if (!bCanAct)
	{
		return;
	}

	if (!IsAiming()
		&& (!SnowballEquipmentComponent
			|| !SnowballEquipmentComponent->HasHeldSnowball()))
	{
		if (!ShouldPreferSnowCreationOverGrab() && PlayerGrabComponent)
		{
			PlayerGrabComponent->StartGrabReach();
			OnActionInput(true);
			return;
		}
	}

	// Animation Blueprint용 IsHoldingSnowball()은 획득 연출 동안 의도적으로
	// 지연되므로 입력 기능 분기에 사용하지 않는다. 두 요청을 모두 전달하고
	// 서버의 실제 장비 상태가 제작 또는 충전 중 하나만 승인한다.
	if (SnowballEquipmentComponent)
	{
		SnowballEquipmentComponent->StartCharging();
	}

	if (ActiveCreationComponent)
	{
		if (!SnowballCreationComponent)
		{
			SnowballCreationComponent = ActiveCreationComponent;
		}
		ActiveCreationComponent->StartCreatingSnowball();
	}

	OnActionInput(true);
}

void ASnowRumbleCharacter::HandleActionCompleted()
{
	if (bIsEmoteRadialMenuOpen)
	{
		return;
	}

	if (PlayerGrabComponent && PlayerGrabComponent->IsGrabReaching())
	{
		PlayerGrabComponent->StopGrabReach();
		OnActionInput(false);
		return;
	}

	if (SnowballEquipmentComponent && SnowballEquipmentComponent->IsCharging())
	{
		SnowballEquipmentComponent->HideThrowTrajectoryPreview();

		if (IsLocallyControlled() && GetWorld())
		{
			PostThrowAimCameraEndTime =
				GetWorld()->GetTimeSeconds() + PostThrowCameraHoldSeconds;
		}

		SnowballEquipmentComponent->ReleaseChargedSnowball();
	}

	if (SnowballCreationComponent)
	{
		SnowballCreationComponent->CancelCreatingSnowball();
	}

	OnActionInput(false);
}

void ASnowRumbleCharacter::HandleDropEquipment()
{
	if (CanPerformGameplayAction())
	{
		if (SnowballEquipmentComponent)
		{
			SnowballEquipmentComponent->DropHeldSnowball();
		}

		OnDropEquipmentInput();
	}
}

void ASnowRumbleCharacter::HandleLobbyBoardPointerPressed()
{
	if (FocusedLobbyBoard
		&& LobbyBoardWidgetInteractionComponent
		&& UpdateLobbyBoardWidgetHitResult())
	{
		LobbyBoardWidgetInteractionComponent->PressPointerKey(
			EKeys::LeftMouseButton);
	}
}

void ASnowRumbleCharacter::HandleLobbyBoardPointerReleased()
{
	if (FocusedLobbyBoard && LobbyBoardWidgetInteractionComponent)
	{
		UpdateLobbyBoardWidgetHitResult();
		LobbyBoardWidgetInteractionComponent->ReleasePointerKey(
			EKeys::LeftMouseButton);
	}
}

bool ASnowRumbleCharacter::UpdateLobbyBoardWidgetHitResult()
{
	if (!FocusedLobbyBoard || !LobbyBoardWidgetInteractionComponent)
	{
		return false;
	}

	TArray<UWidgetComponent*> BoardWidgetComponents;
	FocusedLobbyBoard->GetBoardWidgetComponents(BoardWidgetComponents);
	if (BoardWidgetComponents.IsEmpty())
	{
		return false;
	}

	APlayerController* PlayerController =
		Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return false;
	}

	float MouseX = 0.0f;
	float MouseY = 0.0f;
	if (!PlayerController->GetMousePosition(MouseX, MouseY))
	{
		return false;
	}

	FVector WorldLocation;
	FVector WorldDirection;
	if (!PlayerController->DeprojectScreenPositionToWorld(
		MouseX,
		MouseY,
		WorldLocation,
		WorldDirection))
	{
		return false;
	}

	const FVector TraceEnd =
		WorldLocation
		+ WorldDirection * LobbyBoardWidgetInteractionComponent->InteractionDistance;

	FHitResult ClosestHitResult;
	float ClosestHitDistance = TNumericLimits<float>::Max();

	for (UWidgetComponent* BoardWidgetComponent : BoardWidgetComponents)
	{
		if (!BoardWidgetComponent)
		{
			continue;
		}

		const FVector WidgetOrigin = BoardWidgetComponent->GetComponentLocation();
		const FVector WidgetNormal = BoardWidgetComponent->GetForwardVector();
		const float PlaneDot = FVector::DotProduct(WorldDirection, WidgetNormal);
		if (FMath::IsNearlyZero(PlaneDot))
		{
			continue;
		}

		const float HitDistance =
			FVector::DotProduct(WidgetOrigin - WorldLocation, WidgetNormal)
			/ PlaneDot;
		if (HitDistance < 0.0f
			|| HitDistance > LobbyBoardWidgetInteractionComponent->InteractionDistance
			|| HitDistance >= ClosestHitDistance)
		{
			continue;
		}

		const FVector HitLocation = WorldLocation + WorldDirection * HitDistance;
		const FVector WidgetDelta = HitLocation - WidgetOrigin;
		const FVector2D DrawSize = BoardWidgetComponent->GetDrawSize();
		const FVector ComponentScale = BoardWidgetComponent->GetComponentScale();
		const float HalfWidth = DrawSize.X * FMath::Abs(ComponentScale.Y) * 0.5f;
		const float HalfHeight = DrawSize.Y * FMath::Abs(ComponentScale.Z) * 0.5f;
		const float RightDistance = FVector::DotProduct(
			WidgetDelta,
			BoardWidgetComponent->GetRightVector());
		const float UpDistance = FVector::DotProduct(
			WidgetDelta,
			BoardWidgetComponent->GetUpVector());
		if (FMath::Abs(RightDistance) > HalfWidth
			|| FMath::Abs(UpDistance) > HalfHeight)
		{
			continue;
		}

		ClosestHitDistance = HitDistance;
		ClosestHitResult = FHitResult();
		ClosestHitResult.bBlockingHit = true;
		ClosestHitResult.Time =
			HitDistance / FVector::Distance(WorldLocation, TraceEnd);
		ClosestHitResult.Distance = HitDistance;
		ClosestHitResult.Location = HitLocation;
		ClosestHitResult.ImpactPoint = HitLocation;
		ClosestHitResult.TraceStart = WorldLocation;
		ClosestHitResult.TraceEnd = TraceEnd;
		ClosestHitResult.Normal = WidgetNormal;
		ClosestHitResult.ImpactNormal = WidgetNormal;
		ClosestHitResult.HitObjectHandle =
			FActorInstanceHandle(FocusedLobbyBoard);
		ClosestHitResult.Component = BoardWidgetComponent;
	}

	if (!ClosestHitResult.bBlockingHit)
	{
		return false;
	}

	LobbyBoardWidgetInteractionComponent->SetCustomHitResult(ClosestHitResult);
	return true;
}

void ASnowRumbleCharacter::ConfigureLobbyBoardWidgetInteraction(
	APlayerController* PlayerController)
{
	if (!LobbyBoardWidgetInteractionComponent || !PlayerController)
	{
		return;
	}

	const int32 ControllerUniqueId =
		static_cast<int32>(PlayerController->GetUniqueID() % 10000);
	const int32 CharacterUniqueId =
		static_cast<int32>(GetUniqueID() % 10000);
	LobbyBoardWidgetInteractionComponent->VirtualUserIndex =
		100 + ControllerUniqueId;
	LobbyBoardWidgetInteractionComponent->PointerIndex =
		100 + CharacterUniqueId;
}

void ASnowRumbleCharacter::HandleEmoteStarted()
{
	if (CanPerformGameplayAction())
	{
		OpenEmoteRadialMenu();
		OnEmoteInput(true);
	}
}

void ASnowRumbleCharacter::HandleEmoteCompleted()
{
	if (EmoteRadialMenuWidget && bIsEmoteRadialMenuOpen)
	{
		EmoteRadialMenuWidget->SubmitHoveredEmote();
	}

	CloseEmoteRadialMenu();
	OnEmoteInput(false);
}

void ASnowRumbleCharacter::HandleKeyGuideStarted()
{
	if (CanPerformGameplayAction())
	{
		OpenKeyGuideWidget();
	}
}

void ASnowRumbleCharacter::HandleKeyGuideCompleted()
{
	CloseKeyGuideWidget();
}

void ASnowRumbleCharacter::ApplyInputMappingContext()
{
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController)
	{
		return;
	}

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (InputSubsystem && PlayerMappingContext)
	{
		RuntimePlayerMappingContext = DuplicateObject<UInputMappingContext>(
			PlayerMappingContext,
			this);
		if (RuntimePlayerMappingContext)
		{
			const TArray<FEnhancedActionKeyMapping> OriginalMappings =
				RuntimePlayerMappingContext->GetMappings();
			const UGameInstance* GameInstance = GetGameInstance();
			const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
				GameInstance
					? GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>()
					: nullptr;

			auto ApplySavedKey = [
				this,
				&OriginalMappings,
				UserSettingsSubsystem](
				const UInputAction* Action,
				FKey DefaultKey,
				FName BindingId)
			{
				if (!RuntimePlayerMappingContext
					|| !Action
					|| !UserSettingsSubsystem)
				{
					return;
				}

				const FKey SavedKey = UserSettingsSubsystem->GetKeyBinding(
					BindingId,
					DefaultKey);
				const int32 MappingCount = OriginalMappings.Num();
				for (int32 MappingIndex = 0;
					MappingIndex < MappingCount;
					++MappingIndex)
				{
					if (OriginalMappings[MappingIndex].Action != Action
						|| OriginalMappings[MappingIndex].Key != DefaultKey)
					{
						continue;
					}

					FEnhancedActionKeyMapping& Mapping =
						RuntimePlayerMappingContext->GetMapping(MappingIndex);
					Mapping.Key = SavedKey;
				}
			};

			ApplySavedKey(MoveAction, EKeys::W, TEXT("MoveForward"));
			ApplySavedKey(MoveAction, EKeys::S, TEXT("MoveBackward"));
			ApplySavedKey(MoveAction, EKeys::A, TEXT("MoveLeft"));
			ApplySavedKey(MoveAction, EKeys::D, TEXT("MoveRight"));
			ApplySavedKey(JumpAction, EKeys::SpaceBar, TEXT("Jump"));
			ApplySavedKey(SprintAction, EKeys::LeftShift, TEXT("Sprint"));
			ApplySavedKey(InteractAction, EKeys::E, TEXT("Interact"));
			ApplySavedKey(AimAction, EKeys::RightMouseButton, TEXT("Aim"));
			ApplySavedKey(ActionAction, EKeys::LeftMouseButton, TEXT("Action"));
			ApplySavedKey(DropEquipmentAction, EKeys::Q, TEXT("DropEquipment"));
			ApplySavedKey(EmoteAction, EKeys::Tab, TEXT("Emote"));
			ApplySavedKey(KeyGuideAction, EKeys::T, TEXT("KeyGuide"));
			ApplySavedKey(
				MicrophonePushToTalkAction,
				EKeys::K,
				TEXT("MicrophonePushToTalk"));
			ApplySavedKey(
				MicrophoneChannelToggleAction,
				EKeys::N,
				TEXT("MicrophoneChannelToggle"));
			ApplySavedKey(
				VoiceTargetMuteAction,
				EKeys::M,
				TEXT("VoiceTargetMute"));
		}

		InputSubsystem->ClearAllMappings();
		InputSubsystem->AddMappingContext(
			RuntimePlayerMappingContext
				? RuntimePlayerMappingContext.Get()
				: PlayerMappingContext.Get(),
			0);
	}
}

void ASnowRumbleCharacter::HandleFrozenChanged(bool bIsFrozen)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	if (bIsFrozen)
	{
		if (SnowballEquipmentComponent)
		{
			SnowballEquipmentComponent->SetAiming(false);
		}
		if (SnowballCreationComponent)
		{
			SnowballCreationComponent->CancelCreatingSnowball();
		}

		if (HasAuthority())
		{
			FinishPickupAnimationState();
			FinishItemInteractionAnimationState();
			FinishHitReactAnimationState();
		}

		bIsSprinting = false;
		ApplyMovementSpeed();

		if (HasAuthority())
		{
			ForceNetUpdate();
		}

		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
		StopJumping();
	}
	else
	{
		if (!HealthComponent || !HealthComponent->IsDead())
		{
		MovementComponent->SetMovementMode(MOVE_Walking);
		}
	}
	RefreshLifeStateSpectator();
}

void ASnowRumbleCharacter::HandleDeathChanged(bool bIsDead)
{
	if (!bIsDead)
	{
		return;
	}

	bIsSprinting = false;
	bIsPickingUpItem = false;
	bIsInteractingWithItem = false;
	bIsHitReacting = false;
	if (SnowballEquipmentComponent)
	{
		SnowballEquipmentComponent->SetAiming(false);
	}
	if (SnowballCreationComponent)
	{
		SnowballCreationComponent->CancelCreatingSnowball();
	}
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}
	StopJumping();
	ApplyMovementSpeed();
	RefreshLifeStateSpectator();
}

void ASnowRumbleCharacter::RefreshLifeStateSpectator()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	const bool bShouldSpectate = IsFrozen() || IsDead();
	if (!bShouldSpectate)
	{
		if (bLifeStateSpectating)
		{
			bLifeStateSpectating = false;
			SpectatorViewTargets.Reset();
			SpectatorViewTargetIndex = INDEX_NONE;
			if (SpectatorWidget)
			{
				SpectatorWidget->RemoveFromParent();
				SpectatorWidget = nullptr;
			}
			if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
			{
				PlayerController->SetViewTargetWithBlend(this, 0.15f);
			}
		}
		return;
	}

	if (!bLifeStateSpectating)
	{
		bLifeStateSpectating = true;
		RefreshSpectatorViewTargets();
	}
	else
	{
		ApplySpectatorViewTarget();
	}
}

bool ASnowRumbleCharacter::IsSpectatorViewTargetCandidate(
	const ASnowRumbleCharacter* Candidate) const
{
	return Candidate
		&& Candidate->GetPlayerState()
		&& !Candidate->bTiebreakerSpectator;
}

void ASnowRumbleCharacter::RefreshSpectatorViewTargets()
{
	if (!bLifeStateSpectating)
	{
		return;
	}

	ASnowRumbleCharacter* PreviousTarget = nullptr;
	if (SpectatorViewTargets.IsValidIndex(SpectatorViewTargetIndex))
	{
		PreviousTarget = SpectatorViewTargets[SpectatorViewTargetIndex].Get();
	}

	SpectatorViewTargets.Reset();
	for (TActorIterator<ASnowRumbleCharacter> It(GetWorld()); It; ++It)
	{
		if (IsSpectatorViewTargetCandidate(*It))
		{
			SpectatorViewTargets.Add(*It);
		}
	}
	SpectatorViewTargets.Sort([](
		const TWeakObjectPtr<ASnowRumbleCharacter>& Left,
		const TWeakObjectPtr<ASnowRumbleCharacter>& Right)
	{
		const APlayerState* LeftState = Left.IsValid()
			? Left->GetPlayerState()
			: nullptr;
		const APlayerState* RightState = Right.IsValid()
			? Right->GetPlayerState()
			: nullptr;
		return LeftState && RightState
			? LeftState->GetPlayerId() < RightState->GetPlayerId()
			: Left.IsValid();
	});

	SpectatorViewTargetIndex = INDEX_NONE;
	if (PreviousTarget)
	{
		for (int32 Index = 0; Index < SpectatorViewTargets.Num(); ++Index)
		{
			if (SpectatorViewTargets[Index].Get() == PreviousTarget)
			{
				SpectatorViewTargetIndex = Index;
				break;
			}
		}
	}
	if (SpectatorViewTargetIndex == INDEX_NONE && SpectatorViewTargets.Num() > 0)
	{
		for (int32 Index = 0; Index < SpectatorViewTargets.Num(); ++Index)
		{
			if (SpectatorViewTargets[Index].Get() == this)
			{
				SpectatorViewTargetIndex = Index;
				break;
			}
		}
		if (SpectatorViewTargetIndex == INDEX_NONE)
		{
			SpectatorViewTargetIndex = 0;
		}
	}
	ApplySpectatorViewTarget();
}

void ASnowRumbleCharacter::ApplySpectatorViewTarget()
{
	if (!bLifeStateSpectating)
	{
		return;
	}

	ASnowRumbleCharacter* ViewTarget = SpectatorViewTargets.IsValidIndex(SpectatorViewTargetIndex)
		? SpectatorViewTargets[SpectatorViewTargetIndex].Get()
		: nullptr;
	if (!ViewTarget)
	{
		if (SpectatorWidget)
		{
			SpectatorWidget->RemoveFromParent();
			SpectatorWidget = nullptr;
		}
		return;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (!SpectatorCameraActor)
		{
			if (UWorld* World = GetWorld())
			{
				FActorSpawnParameters SpawnParameters;
				SpawnParameters.Owner = PlayerController;
				SpawnParameters.SpawnCollisionHandlingOverride =
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpectatorCameraActor = World->SpawnActor<ACameraActor>(
					ACameraActor::StaticClass(),
					FTransform::Identity,
					SpawnParameters);
			}
		}
		if (SpectatorCameraActor)
		{
			PlayerController->SetViewTargetWithBlend(
				SpectatorCameraActor,
				0.15f);
		}
	}
	if (SpectatorWidgetClass && !SpectatorWidget)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			SpectatorWidget = CreateWidget<USpectatorWidget>(
				PlayerController,
				SpectatorWidgetClass);
			if (SpectatorWidget)
			{
				SpectatorWidget->AddToViewport(30);
			}
		}
	}
	if (SpectatorWidget)
	{
		SpectatorWidget->SetSpectatorViewTarget(ViewTarget);
	}
}

void ASnowRumbleCharacter::UpdateReplicatedSpectatorCameraView()
{
	if (!IsLocallyControlled() || !FollowCamera || !GetController())
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const APlayerController* PlayerController =
		Cast<APlayerController>(GetController());
	const APlayerCameraManager* CameraManager = PlayerController
		? PlayerController->PlayerCameraManager
		: nullptr;
	if (!CameraManager)
	{
		return;
	}

	const FVector CameraLocation = CameraManager->GetCameraLocation();
	const FRotator CameraRotation = CameraManager->GetCameraRotation();
	const float CameraFieldOfView = CameraManager->GetFOVAngle();
	const double CurrentTime = World->GetTimeSeconds();
	const bool bChanged =
		!bHasReplicatedSpectatorCameraView
		|| FVector::DistSquared(CameraLocation, LastSentSpectatorCameraLocation) > 1.0f
		|| !CameraRotation.Equals(LastSentSpectatorCameraRotation, 0.1f)
		|| !FMath::IsNearlyEqual(
			CameraFieldOfView,
			LastSentSpectatorCameraFieldOfView,
			0.1f);
	if (!bChanged)
	{
		return;
	}
	if (LastSpectatorCameraUpdateTime >= 0.0
		&& CurrentTime - LastSpectatorCameraUpdateTime < (1.0 / 60.0))
	{
		return;
	}

	LastSentSpectatorCameraLocation = CameraLocation;
	LastSentSpectatorCameraRotation = CameraRotation;
	LastSentSpectatorCameraFieldOfView = CameraFieldOfView;
	LastSpectatorCameraUpdateTime = CurrentTime;
	ServerUpdateSpectatorCameraView(
		CameraLocation,
		CameraRotation,
		CameraFieldOfView);
}

void ASnowRumbleCharacter::UpdateLocalSpectatorCameraView()
{
	if (!bLifeStateSpectating
		|| !SpectatorViewTargets.IsValidIndex(SpectatorViewTargetIndex))
	{
		return;
	}

	ASnowRumbleCharacter* ViewTarget =
		SpectatorViewTargets[SpectatorViewTargetIndex].Get();
	if (!ViewTarget || !SpectatorCameraActor)
	{
		return;
	}

	FVector CameraLocation = ViewTarget->ReplicatedSpectatorCameraLocation;
	FRotator CameraRotation = ViewTarget->ReplicatedSpectatorCameraRotation;
	float CameraFieldOfView = ViewTarget->ReplicatedSpectatorCameraFieldOfView;
	if (!ViewTarget->bHasReplicatedSpectatorCameraView
		&& ViewTarget->FollowCamera)
	{
		CameraLocation = ViewTarget->FollowCamera->GetComponentLocation();
		CameraRotation = ViewTarget->FollowCamera->GetComponentRotation();
		CameraFieldOfView = ViewTarget->FollowCamera->FieldOfView;
	}

	const float DeltaSeconds = GetWorld()
		? GetWorld()->GetDeltaSeconds()
		: 0.0f;
	if (!bHasSmoothedSpectatorCameraView)
	{
		SpectatorCameraActor->SetActorLocationAndRotation(
			CameraLocation,
			CameraRotation);
		bHasSmoothedSpectatorCameraView = true;
	}
	else
	{
		const float InterpSpeed = FMath::Max(0.0f, SpectatorCameraInterpSpeed);
		const FVector SmoothedCameraLocation = FMath::VInterpTo(
			SpectatorCameraActor->GetActorLocation(),
			CameraLocation,
			DeltaSeconds,
			InterpSpeed);
		const FRotator SmoothedCameraRotation = FMath::RInterpTo(
			SpectatorCameraActor->GetActorRotation(),
			CameraRotation,
			DeltaSeconds,
			InterpSpeed);
		SpectatorCameraActor->SetActorLocationAndRotation(
			SmoothedCameraLocation,
			SmoothedCameraRotation);
	}
	if (UCameraComponent* CameraComponent =
		SpectatorCameraActor->GetCameraComponent())
	{
		CameraComponent->SetFieldOfView(FMath::FInterpTo(
			CameraComponent->FieldOfView,
			CameraFieldOfView,
			DeltaSeconds,
			FMath::Max(0.0f, SpectatorCameraInterpSpeed)));
	}
}

void ASnowRumbleCharacter::SelectPreviousSpectatorViewTarget()
{
	if (!bLifeStateSpectating)
	{
		return;
	}
	RefreshSpectatorViewTargets();
	if (SpectatorViewTargets.Num() > 0)
	{
		SpectatorViewTargetIndex = (SpectatorViewTargetIndex - 1 + SpectatorViewTargets.Num())
			% SpectatorViewTargets.Num();
		ApplySpectatorViewTarget();
	}
}

void ASnowRumbleCharacter::SelectNextSpectatorViewTarget()
{
	if (!bLifeStateSpectating)
	{
		return;
	}
	RefreshSpectatorViewTargets();
	if (SpectatorViewTargets.Num() > 0)
	{
		SpectatorViewTargetIndex = (SpectatorViewTargetIndex + 1)
			% SpectatorViewTargets.Num();
		ApplySpectatorViewTarget();
	}
}

void ASnowRumbleCharacter::HandleGrabbedByCharacterChanged(bool bNewGrabbed)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	if (bNewGrabbed)
	{
		MovementModeBeforeGrabbed = MovementComponent->MovementMode;
		CustomMovementModeBeforeGrabbed = MovementComponent->CustomMovementMode;
		bOrientRotationToMovementBeforeGrabbedByCharacter =
			MovementComponent->bOrientRotationToMovement;
		bUseControllerRotationYawBeforeGrabbedByCharacter = bUseControllerRotationYaw;
		MovementComponent->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = false;
		bIsSprinting = false;
		if (SnowballEquipmentComponent)
		{
			SnowballEquipmentComponent->SetAiming(false);
		}
		if (SnowballCreationComponent)
		{
			SnowballCreationComponent->CancelCreatingSnowball();
		}
		MovementComponent->StopMovementImmediately();
		if (HealthComponent
			&& !HealthComponent->IsDead()
			&& !bTiebreakerSpectator
			&& !bWaterSubmerged
			&& MovementComponent->MovementMode == MOVE_None)
		{
			MovementComponent->SetMovementMode(MOVE_Walking);
		}
		StopJumping();
		ApplyMovementSpeed();
		return;
	}

	if (HealthComponent
		&& !HealthComponent->IsFrozen()
		&& !HealthComponent->IsDead()
		&& !bTiebreakerSpectator
		&& !bWaterSubmerged)
	{
		const EMovementMode RestoreMode =
			MovementModeBeforeGrabbed == MOVE_None
				? MOVE_Walking
				: MovementModeBeforeGrabbed.GetValue();
		MovementComponent->SetMovementMode(
			RestoreMode,
			CustomMovementModeBeforeGrabbed);
		MovementComponent->bOrientRotationToMovement =
			bOrientRotationToMovementBeforeGrabbedByCharacter;
		bUseControllerRotationYaw = bUseControllerRotationYawBeforeGrabbedByCharacter;
	}
	else if (HealthComponent
		&& HealthComponent->IsFrozen()
		&& !HealthComponent->IsDead())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}
}

void ASnowRumbleCharacter::HandleSnowballAimingChanged(bool bNewAiming)
{
	if (IsLocallyControlled() && MainHUDWidget)
	{
		MainHUDWidget->SetAimCrosshairVisibleImmediate(
			bNewAiming && !IsHoldingLargeSnowball());
	}

	if (IsHangingFromWorldGrab() || IsGrabbedByCharacter())
	{
		bUseControllerRotationYaw = false;

		if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
		{
			MovementComponent->bOrientRotationToMovement = false;
		}

		if (bIsSprinting)
		{
			bIsSprinting = false;

			if (!HasAuthority())
			{
				ServerSetSprinting(false);
			}
		}

		ApplyMovementSpeed();
		return;
	}

	bUseControllerRotationYaw = bNewAiming;

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = !bNewAiming;
	}

	if (bNewAiming && bIsSprinting)
	{
		bIsSprinting = false;

		if (!HasAuthority())
		{
			ServerSetSprinting(false);
		}
	}

	ApplyMovementSpeed();

	if (HasAuthority())
	{
		ForceNetUpdate();
	}
}

ALobbyInteractionBoard* ASnowRumbleCharacter::FindClosestLobbyBoardCandidate()
	const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FVector CharacterLocation = GetActorLocation();
	float ClosestDistanceSquared = TNumericLimits<float>::Max();
	ALobbyInteractionBoard* ClosestBoard = nullptr;

	for (TActorIterator<ALobbyInteractionBoard> Iterator(World); Iterator; ++Iterator)
	{
		ALobbyInteractionBoard* Candidate = *Iterator;
		if (!Candidate || !Candidate->CanInteractWith(this))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(
			CharacterLocation,
			Candidate->GetActorLocation());
		if (DistanceSquared <= ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			ClosestBoard = Candidate;
		}
	}

	return ClosestBoard;
}

APhotoInteractionActor*
ASnowRumbleCharacter::FindClosestPhotoInteractionCandidate() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FVector CharacterLocation = GetActorLocation();
	float ClosestDistanceSquared = TNumericLimits<float>::Max();
	APhotoInteractionActor* ClosestPhotoActor = nullptr;
	for (TActorIterator<APhotoInteractionActor> Iterator(World);
		Iterator;
		++Iterator)
	{
		APhotoInteractionActor* Candidate = *Iterator;
		if (!Candidate || !Candidate->CanInteractWith(this))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(
			CharacterLocation,
			Candidate->GetActorLocation());
		if (DistanceSquared <= ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			ClosestPhotoActor = Candidate;
		}
	}

	return ClosestPhotoActor;
}

AJukeboxActor* ASnowRumbleCharacter::FindClosestJukeboxCandidate() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FVector CharacterLocation = GetActorLocation();
	float ClosestDistanceSquared = TNumericLimits<float>::Max();
	AJukeboxActor* ClosestJukebox = nullptr;

	for (TActorIterator<AJukeboxActor> Iterator(World); Iterator; ++Iterator)
	{
		AJukeboxActor* Candidate = *Iterator;
		if (!Candidate || !Candidate->CanInteractWith(this))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(
			CharacterLocation,
			Candidate->GetActorLocation());
		if (DistanceSquared < ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			ClosestJukebox = Candidate;
		}
	}

	return ClosestJukebox;
}

AGiftBox* ASnowRumbleCharacter::FindClosestGiftBoxCandidate() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FVector CharacterLocation = GetActorLocation();
	float ClosestDistanceSquared = TNumericLimits<float>::Max();
	AGiftBox* ClosestGiftBox = nullptr;

	for (TActorIterator<AGiftBox> Iterator(World); Iterator; ++Iterator)
	{
		AGiftBox* Candidate = *Iterator;
		if (!Candidate || !Candidate->CanInteractWith(this))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(
			CharacterLocation,
			Candidate->GetActorLocation());
		if (DistanceSquared <= ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			ClosestGiftBox = Candidate;
		}
	}

	return ClosestGiftBox;
}

AGiftBoxItemPickup*
ASnowRumbleCharacter::FindClosestGiftBoxItemPickupCandidate() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FVector CharacterLocation = GetActorLocation();
	float ClosestDistanceSquared = TNumericLimits<float>::Max();
	AGiftBoxItemPickup* ClosestPickup = nullptr;

	for (TActorIterator<AGiftBoxItemPickup> Iterator(World); Iterator; ++Iterator)
	{
		AGiftBoxItemPickup* Candidate = *Iterator;
		if (!Candidate || !Candidate->CanInteractWith(this))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(
			CharacterLocation,
			Candidate->GetActorLocation());
		if (DistanceSquared <= ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			ClosestPickup = Candidate;
		}
	}

	return ClosestPickup;
}

ASnowRumbleCharacter*
ASnowRumbleCharacter::FindClosestFrozenTeammateCandidate() const
{
	if (!GiftItemEffectComponent
		|| !GiftItemEffectComponent->HasAnyHotPack())
	{
		return nullptr;
	}

	const ASnowRumblePlayerState* OwnerPlayerState =
		GetPlayerState<ASnowRumblePlayerState>();
	if (!OwnerPlayerState
		|| OwnerPlayerState->GetLobbyTeam() == ESnowRumbleTeam::None)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const float MaxDistanceSquared =
		FMath::Square(TeammateReviveInteractionDistance);
	const FVector CharacterLocation = GetActorLocation();
	float ClosestDistanceSquared = MaxDistanceSquared;
	ASnowRumbleCharacter* ClosestTeammate = nullptr;
	for (TActorIterator<ASnowRumbleCharacter> Iterator(World);
		Iterator;
		++Iterator)
	{
		ASnowRumbleCharacter* Candidate = *Iterator;
		const ASnowRumblePlayerState* CandidatePlayerState = Candidate
			? Candidate->GetPlayerState<ASnowRumblePlayerState>()
			: nullptr;
		if (!Candidate
			|| Candidate == this
			|| !Candidate->IsFrozen()
			|| Candidate->IsDead()
			|| !CandidatePlayerState
			|| CandidatePlayerState->GetLobbyTeam()
				!= OwnerPlayerState->GetLobbyTeam())
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(
			CharacterLocation,
			Candidate->GetActorLocation());
		if (DistanceSquared <= ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			ClosestTeammate = Candidate;
		}
	}

	return ClosestTeammate;
}

void ASnowRumbleCharacter::TryInteractWithLobbyBoard()
{
	if (!IsLocallyControlled() || !CanPerformGameplayAction())
	{
		return;
	}

	ALobbyInteractionBoard* Board = OutlineComponent
		? Cast<ALobbyInteractionBoard>(OutlineComponent->GetOutlinedActor())
		: nullptr;
	if (!Board)
	{
		return;
	}

	if (HasAuthority())
	{
		ServerTryInteractWithLobbyBoard_Implementation(Board);
	}
	else
	{
		ServerTryInteractWithLobbyBoard(Board);
	}
}

void ASnowRumbleCharacter::TryInteractWithPhotoActor()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	APhotoInteractionActor* PhotoActor =
		FindClosestPhotoInteractionCandidate();
	if (!PhotoActor)
	{
		return;
	}

	if (HasAuthority())
	{
		ServerTryInteractWithPhotoActor_Implementation(PhotoActor);
	}
	else
	{
		ServerTryInteractWithPhotoActor(PhotoActor);
	}
}

void ASnowRumbleCharacter::TryInteractWithJukebox()
{
	if (!IsLocallyControlled() || !CanPerformGameplayAction())
	{
		return;
	}

	AJukeboxActor* Jukebox = FindClosestJukeboxCandidate();
	if (!Jukebox)
	{
		return;
	}

	if (HasAuthority())
	{
		ServerTryInteractWithJukebox_Implementation(Jukebox);
	}
	else
	{
		ServerTryInteractWithJukebox(Jukebox);
	}
}

void ASnowRumbleCharacter::HandlePhotoCapture()
{
	if (!IsLocallyControlled() || !FocusedPhotoActor)
	{
		return;
	}

	const FString Timestamp =
		FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	FScreenshotRequest::RequestScreenshot(
		FString::Printf(TEXT("Photo_%s"), *Timestamp),
		false,
		false);
	SnowRumbleAudio::PlaySound2D(
		this,
		PhotoCaptureSound,
		ESnowRumbleAudioMixChannel::UserInterface);
}

void ASnowRumbleCharacter::TryInteractWithGiftBox()
{
	if (!IsLocallyControlled() || !CanPerformGameplayAction())
	{
		return;
	}

	AGiftBox* GiftBox = OutlineComponent
		? Cast<AGiftBox>(OutlineComponent->GetOutlinedActor())
		: nullptr;
	if (!GiftBox)
	{
		GiftBox = FindClosestGiftBoxCandidate();
	}
	if (!GiftBox)
	{
		return;
	}

	if (HasAuthority())
	{
		ServerTryOpenGiftBox_Implementation(GiftBox);
	}
	else
	{
		ServerTryOpenGiftBox(GiftBox);
	}
}

void ASnowRumbleCharacter::TryPickupGiftBoxItem()
{
	if (!IsLocallyControlled() || !CanPerformGameplayAction())
	{
		return;
	}

	AGiftBoxItemPickup* Pickup = OutlineComponent
		? Cast<AGiftBoxItemPickup>(OutlineComponent->GetOutlinedActor())
		: nullptr;
	if (!Pickup)
	{
		Pickup = FindClosestGiftBoxItemPickupCandidate();
	}
	if (!Pickup)
	{
		return;
	}

	if (HasAuthority())
	{
		ServerTryPickupGiftBoxItem_Implementation(Pickup);
	}
	else
	{
		ServerTryPickupGiftBoxItem(Pickup);
	}
}

void ASnowRumbleCharacter::TryStartTeammateRevive()
{
	if (!IsLocallyControlled()
		|| !CanPerformGameplayAction()
		|| bIsRevivingTeammate)
	{
		return;
	}

	ASnowRumbleCharacter* TargetCharacter = OutlineComponent
		? Cast<ASnowRumbleCharacter>(OutlineComponent->GetOutlinedActor())
		: nullptr;
	if (!TargetCharacter || TargetCharacter != FindClosestFrozenTeammateCandidate())
	{
		return;
	}

	bIsRevivingTeammate = true;
	TeammateReviveTarget = TargetCharacter;
	bUsedInteractForRolling = true;
	OnInteractInput(true);

	if (UWorld* World = GetWorld())
	{
	const float HoldSeconds = TeammateReviveHoldSeconds
			* (GiftItemEffectComponent->HasGoldenHotPack() ? 2.0f : 1.0f);
		TeammateReviveHoldDurationSeconds = FMath::Max(0.0f, HoldSeconds);
		TeammateReviveStartTime = World->GetTimeSeconds();
		World->GetTimerManager().SetTimer(
			TeammateReviveTimerHandle,
			this,
			&ASnowRumbleCharacter::CompleteTeammateRevive,
			FMath::Max(0.0f, HoldSeconds),
			false);
	}
}

void ASnowRumbleCharacter::CancelTeammateRevive()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TeammateReviveTimerHandle);
	}
	bIsRevivingTeammate = false;
	TeammateReviveTarget = nullptr;
	TeammateReviveHoldDurationSeconds = 0.0f;
	TeammateReviveStartTime = -1.0;
}

void ASnowRumbleCharacter::CompleteTeammateRevive()
{
	if (!bIsRevivingTeammate
		|| !bIsInteractHeld
		|| !TeammateReviveTarget.IsValid()
		|| !CanPerformGameplayAction())
	{
		CancelTeammateRevive();
		return;
	}

	ASnowRumbleCharacter* TargetCharacter = TeammateReviveTarget.Get();
	bIsRevivingTeammate = false;
	if (HasAuthority())
	{
		ServerReviveFrozenTeammate_Implementation(TargetCharacter);
	}
	else
	{
		ServerReviveFrozenTeammate(TargetCharacter);
	}
}

void ASnowRumbleCharacter::CloseLobbyBoardFocus()
{
	ClearLobbyBoardFocus();
}

FText ASnowRumbleCharacter::GetCurrentInteractionPromptText() const
{
	FText PromptText;
	AActor* PromptActor = nullptr;
	GetCurrentInteractionPromptData(PromptText, PromptActor);
	return PromptText;
}

void ASnowRumbleCharacter::RequestLobbyBoardAction(
	ELobbyBoardAction BoardAction)
{
	if (!IsLocallyControlled() || !FocusedLobbyBoard)
	{
		return;
	}

	if (HasAuthority())
	{
		ServerRequestLobbyBoardAction_Implementation(
			FocusedLobbyBoard,
			BoardAction);
	}
	else
	{
		ServerRequestLobbyBoardAction(FocusedLobbyBoard, BoardAction);
	}
}

void ASnowRumbleCharacter::RequestLobbyTeamSelection(ESnowRumbleTeam NewTeam)
{
	if (!IsLocallyControlled() || !FocusedLobbyBoard)
	{
		return;
	}

	if (HasAuthority())
	{
		ServerRequestLobbyTeamSelection_Implementation(
			FocusedLobbyBoard,
			NewTeam);
	}
	else
	{
		ServerRequestLobbyTeamSelection(FocusedLobbyBoard, NewTeam);
	}
}

void ASnowRumbleCharacter::ClearLobbyBoardFocus()
{
	if (!IsLocallyControlled())
	{
		FocusedLobbyBoard = nullptr;
		return;
	}

	APlayerController* PlayerController =
		Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		PlayerController->SetViewTargetWithBlend(this, 0.15f);
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
		PlayerController->SetIgnoreMoveInput(false);
		PlayerController->SetIgnoreLookInput(false);
	}
	if (LobbyBoardWidgetInteractionComponent)
	{
		if (bLobbyBoardPointerPressed)
		{
			LobbyBoardWidgetInteractionComponent->ReleasePointerKey(
				EKeys::LeftMouseButton);
		}
		LobbyBoardWidgetInteractionComponent->SetActive(false);
	}
	bLobbyBoardPointerPressed = false;

	if (FocusedLobbyBoard)
	{
		FocusedLobbyBoard->SetFocusedCharacter(nullptr);
	}
	FocusedLobbyBoard = nullptr;
}

void ASnowRumbleCharacter::ServerTryInteractWithLobbyBoard_Implementation(
	ALobbyInteractionBoard* Board)
{
	if (!CanPerformGameplayAction() || !Board || !Board->CanInteractWith(this))
	{
		return;
	}

	Board->Interact(this);
}

void ASnowRumbleCharacter::ServerTryInteractWithPhotoActor_Implementation(
	APhotoInteractionActor* PhotoActor)
{
	if (!CanPerformGameplayAction()
		|| !PhotoActor
		|| !PhotoActor->CanInteractWith(this))
	{
		return;
	}

	PhotoActor->Interact(this);
}

void ASnowRumbleCharacter::ServerTryInteractWithJukebox_Implementation(
	AJukeboxActor* Jukebox)
{
	if (!CanPerformGameplayAction()
		|| !Jukebox
		|| !Jukebox->CanInteractWith(this))
	{
		return;
	}

	Jukebox->Interact(this);
}

void ASnowRumbleCharacter::ServerTryOpenGiftBox_Implementation(
	AGiftBox* GiftBox)
{
	if (!CanPerformGameplayAction()
		|| !GiftBox
		|| !GiftBox->CanInteractWith(this))
	{
		return;
	}

	if (GiftBox->TryOpen(this))
	{
		NotifyItemInteractionSucceeded();
	}
}

void ASnowRumbleCharacter::ServerTryPickupGiftBoxItem_Implementation(
	AGiftBoxItemPickup* Pickup)
{
	if (!CanPerformGameplayAction()
		|| !Pickup
		|| !Pickup->CanInteractWith(this))
	{
		return;
	}

	Pickup->TryPickup(this);
}

void ASnowRumbleCharacter::ServerReviveFrozenTeammate_Implementation(
	ASnowRumbleCharacter* TargetCharacter)
{
	if (!CanPerformGameplayAction()
		|| !GiftItemEffectComponent
		|| !GiftItemEffectComponent->ReviveFrozenTeammate(TargetCharacter))
	{
		return;
	}

	NotifyItemInteractionSucceeded();
}

void ASnowRumbleCharacter::ServerRequestLobbyBoardAction_Implementation(
	ALobbyInteractionBoard* Board,
	ELobbyBoardAction BoardAction)
{
	if (!CanPerformGameplayAction() || !Board || !Board->CanInteractWith(this))
	{
		return;
	}

	Board->HandleBoardAction(this, BoardAction);
}

void ASnowRumbleCharacter::ServerRequestLobbyTeamSelection_Implementation(
	ALobbyInteractionBoard* Board,
	ESnowRumbleTeam NewTeam)
{
	if (!CanPerformGameplayAction() || !Board || !Board->CanInteractWith(this))
	{
		return;
	}

	if (ASnowRumblePlayerState* SnowRumblePlayerState =
		GetPlayerState<ASnowRumblePlayerState>())
	{
		SnowRumblePlayerState->RequestSetLobbyTeam(NewTeam);
	}
}

bool ASnowRumbleCharacter::CanPerformGameplayAction() const
{
	const ASnowRumblePlayerController* SnowRumblePlayerController =
		Cast<ASnowRumblePlayerController>(GetController());
	return HealthComponent
		&& !HealthComponent->IsFrozen()
		&& !HealthComponent->IsDead()
		&& !bIsPickingUpItem
		&& !bIsInteractingWithItem
		&& !bIsEmoteRadialMenuOpen
		&& !bIsKeyGuideWidgetOpen
		&& !IsGrabbedByCharacter()
		&& !bTiebreakerSpectator
		&& !IsHangingFromWorldGrab()
		&& !IsPvpMatchInputLocked()
		&& (!SnowRumblePlayerController
			|| !SnowRumblePlayerController->IsGameplayUiInputOpen());
}

bool ASnowRumbleCharacter::IsDamageAllowedByTiebreaker(
	AController* EventInstigator) const
{
	const UGameInstance* GameInstance =
		GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const USnowRumbleMatchSubsystem* MatchSubsystem = GameInstance
		? GameInstance->GetSubsystem<USnowRumbleMatchSubsystem>()
		: nullptr;
	if (!MatchSubsystem || !MatchSubsystem->IsTiebreakerActive())
	{
		return true;
	}

	const ASnowRumblePlayerState* TargetPlayerState =
		GetPlayerState<ASnowRumblePlayerState>();
	if (!TargetPlayerState
		|| !MatchSubsystem->IsTiebreakerTeam(TargetPlayerState->GetLobbyTeam()))
	{
		return false;
	}

	const ASnowRumblePlayerState* InstigatorPlayerState = EventInstigator
		? EventInstigator->GetPlayerState<ASnowRumblePlayerState>()
		: nullptr;
	return InstigatorPlayerState
		&& MatchSubsystem->IsTiebreakerTeam(
			InstigatorPlayerState->GetLobbyTeam());
}

bool ASnowRumbleCharacter::IsPvpMatchInputLocked() const
{
	const UWorld* World = GetWorld();
	const ASnowRumbleGameState* SnowRumbleGameState = World
		? World->GetGameState<ASnowRumbleGameState>()
		: nullptr;
	if (SnowRumbleGameState && SnowRumbleGameState->IsMatchInputLocked())
	{
		return true;
	}

	const ASnowmanModeGameState* SnowmanModeGameState = World
		? World->GetGameState<ASnowmanModeGameState>()
		: nullptr;
	return SnowmanModeGameState
		&& SnowmanModeGameState->IsSnowmanModeInputLocked();
}

void ASnowRumbleCharacter::RefreshTiebreakerSpectatorViewTarget()
{
	if (!IsLocallyControlled() || !bTiebreakerSpectator)
	{
		return;
	}

	APlayerController* PlayerController =
		Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	ASnowRumbleCharacter* ViewTargetCharacter =
		Cast<ASnowRumbleCharacter>(PlayerController->GetViewTarget());
	if (ViewTargetCharacter
		&& ViewTargetCharacter != this
		&& !ViewTargetCharacter->bTiebreakerSpectator
		&& !ViewTargetCharacter->IsDead())
	{
		return;
	}

	ViewTargetCharacter = FindTiebreakerSpectatorViewTarget();
	if (ViewTargetCharacter)
	{
		PlayerController->SetViewTargetWithBlend(ViewTargetCharacter, 0.25f);
	}
}

ASnowRumbleCharacter* ASnowRumbleCharacter::FindTiebreakerSpectatorViewTarget()
	const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<ASnowRumbleCharacter> It(World); It; ++It)
	{
		ASnowRumbleCharacter* Candidate = *It;
		if (Candidate
			&& Candidate != this
			&& !Candidate->bTiebreakerSpectator
			&& !Candidate->IsDead())
		{
			return Candidate;
		}
	}

	return nullptr;
}

void ASnowRumbleCharacter::RefreshPvpMatchInputLock()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	APlayerController* PlayerController =
		Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	const bool bMatchInputLocked = IsPvpMatchInputLocked();

	const UGameInstance* GameInstance =
		GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const USnowRumbleMatchSubsystem* MatchSubsystem = GameInstance
		? GameInstance->GetSubsystem<USnowRumbleMatchSubsystem>()
		: nullptr;
	const bool bTiebreakerActive = MatchSubsystem && MatchSubsystem->IsTiebreakerActive();

	const bool bShouldBlockMove = bMatchInputLocked || (bTiebreakerSpectator && bTiebreakerActive);
	const bool bShouldBlockLook = bMatchInputLocked;
	const ASnowRumblePlayerController* SnowRumblePlayerController =
		Cast<ASnowRumblePlayerController>(PlayerController);
	const bool bUiInputActive =
		FocusedLobbyBoard
		|| bIsEmoteRadialMenuOpen
		|| bIsKeyGuideWidgetOpen
		|| Cast<ACustomizationPlayerController>(PlayerController)
		|| Cast<AMainMenuPlayerController>(PlayerController)
		|| (SnowRumblePlayerController
			&& SnowRumblePlayerController->IsGameplayUiInputOpen());

	if (bShouldBlockMove && !bPvpMatchMoveInputIgnoreApplied)
	{
		PlayerController->SetIgnoreMoveInput(true);
		bPvpMatchMoveInputIgnoreApplied = true;
	}
	else if (!bShouldBlockMove
		&& bPvpMatchMoveInputIgnoreApplied
		&& !bUiInputActive)
	{
		PlayerController->ResetIgnoreMoveInput();
		bPvpMatchMoveInputIgnoreApplied = false;
	}

	if (bShouldBlockLook && !bPvpMatchLookInputIgnoreApplied)
	{
		PlayerController->SetIgnoreLookInput(true);
		bPvpMatchLookInputIgnoreApplied = true;
	}
	else if (!bShouldBlockLook
		&& bPvpMatchLookInputIgnoreApplied
		&& !bUiInputActive)
	{
		PlayerController->ResetIgnoreLookInput();
		bPvpMatchLookInputIgnoreApplied = false;
	}

	if (bShouldBlockMove)
	{
		if (UCharacterMovementComponent* MovementComponent =
			GetCharacterMovement())
		{
			MovementComponent->StopMovementImmediately();
		}
		bIsSprinting = false;
	}

	if (!bShouldBlockMove
		&& !bShouldBlockLook
		&& !bUiInputActive)
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
	}
}

bool ASnowRumbleCharacter::IsValidEmoteIndex(int32 EmoteIndex) const
{
	return EmoteIndex >= 0
		&& EmoteIndex < 8
		&& EmoteMontages.IsValidIndex(EmoteIndex)
		&& EmoteMontages[EmoteIndex];
}

bool ASnowRumbleCharacter::CanPlayEmote() const
{
	const ASnowRumblePlayerController* SnowRumblePlayerController =
		Cast<ASnowRumblePlayerController>(GetController());
	return HealthComponent
		&& !HealthComponent->IsFrozen()
		&& !HealthComponent->IsDead()
		&& !bIsPickingUpItem
		&& !bIsInteractingWithItem
		&& !bTiebreakerSpectator
		&& !IsPvpMatchInputLocked()
		&& (!SnowRumblePlayerController
			|| !SnowRumblePlayerController->IsChatInputOpen())
		&& (!SnowballEquipmentComponent
			|| !SnowballEquipmentComponent->IsRollingSnowball())
		&& (!SnowballCreationComponent
			|| !SnowballCreationComponent->IsCreatingSnowball());
}

void ASnowRumbleCharacter::PlayEmoteMontage(int32 EmoteIndex)
{
	if (!IsValidEmoteIndex(EmoteIndex))
	{
		return;
	}

	PlayAnimMontage(EmoteMontages[EmoteIndex].Get());
}

bool ASnowRumbleCharacter::FindSnowFootstepSurface(
	FName FootSocketName,
	FHitResult& OutFootstepHit) const
{
	if (!FindFootstepSurface(FootSocketName, OutFootstepHit))
	{
		return false;
	}

	return OutFootstepHit.GetActor()
		&& !SnowFootstepSurfaceTag.IsNone()
		&& OutFootstepHit.GetActor()->ActorHasTag(SnowFootstepSurfaceTag);
}

bool ASnowRumbleCharacter::FindFootstepSurface(
	FName FootSocketName,
	FHitResult& OutFootstepHit) const
{
	if (!GetMesh())
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FVector SocketLocation = FootSocketName.IsNone()
		? GetSnowTrailProbeLocation()
		: (GetMesh()->DoesSocketExist(FootSocketName)
			? GetMesh()->GetSocketLocation(FootSocketName)
			: GetActorLocation());
	const float TraceUpOffset = FMath::Max(80.0f, SnowFootstepTraceUpOffset);
	const float TraceDownDistance =
		FMath::Max(140.0f, SnowFootstepTraceDownDistance);
	const FVector TraceStart =
		SocketLocation + FVector::UpVector * TraceUpOffset;
	const FVector TraceEnd =
		SocketLocation
		- FVector::UpVector * TraceDownDistance;

	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(SnowFootstepTrace),
		false,
		this);
	QueryParams.AddIgnoredActor(this);

	const bool bHit = World->LineTraceSingleByChannel(
		OutFootstepHit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams);
	if (!bHit)
	{
		if (const ASnowTrailRenderTargetManager* SnowTrailManager =
			ASnowTrailRenderTargetManager::FindSnowTrailManager(this))
		{
			if (SnowTrailManager->ShouldLogSnowTrailDebug())
			{
				UE_LOG(
					LogSnowTrailCharacter,
					Warning,
					TEXT("[SnowTrail] Foot trace failed Character=%s Foot=%s Hit=%d HitActor=%s Start=%s End=%s Tag=%s"),
					*GetNameSafe(this),
					*FootSocketName.ToString(),
					bHit ? 1 : 0,
					*GetNameSafe(OutFootstepHit.GetActor()),
					*TraceStart.ToCompactString(),
					*TraceEnd.ToCompactString(),
					*SnowFootstepSurfaceTag.ToString());
			}
		}
	}
	return bHit;
}

bool ASnowRumbleCharacter::FindSnowFootstepSurfaceAtLocation(
	const FVector& FootstepLocation,
	FHitResult& OutFootstepHit) const
{
	if (SnowFootstepSurfaceTag.IsNone())
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const float TraceUpOffset = FMath::Max(80.0f, SnowFootstepTraceUpOffset);
	const float TraceDownDistance =
		FMath::Max(140.0f, SnowFootstepTraceDownDistance);
	const FVector TraceStart =
		FootstepLocation + FVector::UpVector * TraceUpOffset;
	const FVector TraceEnd =
		FootstepLocation
		- FVector::UpVector * TraceDownDistance;

	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(SnowFootstepLocationTrace),
		false,
		this);
	QueryParams.AddIgnoredActor(this);

	const bool bHit = World->LineTraceSingleByChannel(
		OutFootstepHit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams);
	const bool bHitSnowSurface = bHit
		&& OutFootstepHit.GetActor()
		&& OutFootstepHit.GetActor()->ActorHasTag(SnowFootstepSurfaceTag);
	if (!bHitSnowSurface)
	{
		if (const ASnowTrailRenderTargetManager* SnowTrailManager =
			ASnowTrailRenderTargetManager::FindSnowTrailManager(this))
		{
			if (SnowTrailManager->ShouldLogSnowTrailDebug())
			{
				UE_LOG(
					LogSnowTrailCharacter,
					Warning,
					TEXT("[SnowTrail] Location trace failed Character=%s Location=%s Hit=%d HitActor=%s HasTag=%d Start=%s End=%s Tag=%s"),
					*GetNameSafe(this),
					*FootstepLocation.ToCompactString(),
					bHit ? 1 : 0,
					*GetNameSafe(OutFootstepHit.GetActor()),
					OutFootstepHit.GetActor()
						&& OutFootstepHit.GetActor()->ActorHasTag(SnowFootstepSurfaceTag)
							? 1
							: 0,
					*TraceStart.ToCompactString(),
					*TraceEnd.ToCompactString(),
					*SnowFootstepSurfaceTag.ToString());
			}
		}
	}
	return bHitSnowSurface;
}

void ASnowRumbleCharacter::UpdateDistanceBasedSnowTrail(float DeltaSeconds)
{
	if (!bEnableDistanceBasedSnowTrailStamps
		|| !bDistanceSnowTrailActive
		|| !bEnableSharedSnowTrailStamps
		|| !IsLocallyControlled()
		|| IsFrozen()
		|| IsDead()
		|| IsInAir()
		|| bTiebreakerSpectator)
	{
		return;
	}

	const FVector Velocity2D(GetVelocity().X, GetVelocity().Y, 0.0f);
	if (Velocity2D.Size() < SnowTrailDistanceStampMinimumSpeed)
	{
		return;
	}

	FHitResult TrailSurfaceHit;
	if (!FindSnowFootstepSurfaceAtLocation(
		GetSnowTrailProbeLocation(),
		TrailSurfaceHit))
	{
		bDistanceSnowTrailActive = false;
		return;
	}

	const float StampInterval =
		FMath::Max(1.0f, SnowTrailDistanceStampInterval);
	if (FVector::DistSquared2D(
		LastDistanceSnowTrailStampLocation,
		TrailSurfaceHit.ImpactPoint) < FMath::Square(StampInterval))
	{
		return;
	}

	const FName FootSocketName =
		LastDistanceSnowTrailFootSocketName.IsNone()
			? LeftFootSocketName
			: LastDistanceSnowTrailFootSocketName;
	RequestSharedSnowTrailStamp(
		TrailSurfaceHit.ImpactPoint,
		TrailSurfaceHit.ImpactNormal.GetSafeNormal(),
		FootSocketName);
	LastDistanceSnowTrailStampLocation = TrailSurfaceHit.ImpactPoint;
}

void ASnowRumbleCharacter::RequestSharedSnowTrailStamp(
	const FVector& FootstepLocation,
	const FVector& FootstepNormal,
	FName FootSocketName)
{
	if (!bEnableSharedSnowTrailStamps)
	{
		return;
	}

	if (HasAuthority())
	{
		ServerRequestSnowTrailStamp_Implementation(
			FootstepLocation,
			FootstepNormal.GetSafeNormal(),
			FootSocketName);
	}
	else
	{
		ServerRequestSnowTrailStamp(
			FootstepLocation,
			FootstepNormal.GetSafeNormal(),
			FootSocketName);
	}
}

FVector ASnowRumbleCharacter::GetSnowTrailProbeLocation() const
{
	const UCapsuleComponent* Capsule = GetCapsuleComponent();
	const float CapsuleHalfHeight = Capsule
		? Capsule->GetScaledCapsuleHalfHeight()
		: 0.0f;
	return GetActorLocation()
		- FVector::UpVector * FMath::Max(0.0f, CapsuleHalfHeight - 5.0f);
}

void ASnowRumbleCharacter::ServerRequestSnowTrailStamp_Implementation(
	FVector_NetQuantize FootstepLocation,
	FVector_NetQuantizeNormal FootstepNormal,
	FName FootSocketName)
{
	if (!bEnableSharedSnowTrailStamps
		|| IsFrozen()
		|| IsDead())
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const double CurrentTime = World->GetTimeSeconds();
	if (LastSnowTrailStampServerTime >= 0.0
		&& CurrentTime - LastSnowTrailStampServerTime
			< SnowTrailStampServerCooldown)
	{
		return;
	}

	const float MaxDistance =
		FMath::Max(0.0f, SnowTrailStampMaxClientDistance);
	if (MaxDistance > 0.0f
		&& FVector::DistSquared(
			FootstepLocation,
			GetActorLocation()) > FMath::Square(MaxDistance))
	{
		if (const ASnowTrailRenderTargetManager* SnowTrailManager =
			ASnowTrailRenderTargetManager::FindSnowTrailManager(this))
		{
			if (SnowTrailManager->ShouldLogSnowTrailDebug())
			{
				UE_LOG(
					LogSnowTrailCharacter,
					Warning,
					TEXT("[SnowTrail] Server rejected stamp: client location too far Character=%s FootLocation=%s ActorLocation=%s MaxDistance=%.1f"),
					*GetNameSafe(this),
					*FootstepLocation.ToCompactString(),
					*GetActorLocation().ToCompactString(),
					MaxDistance);
			}
		}
		return;
	}

	FHitResult ServerFootstepHit;
	if (!FindSnowFootstepSurfaceAtLocation(
		FootstepLocation,
		ServerFootstepHit))
	{
		if (const ASnowTrailRenderTargetManager* SnowTrailManager =
			ASnowTrailRenderTargetManager::FindSnowTrailManager(this))
		{
			if (SnowTrailManager->ShouldLogSnowTrailDebug())
			{
				UE_LOG(
					LogSnowTrailCharacter,
					Warning,
					TEXT("[SnowTrail] Server rejected stamp: no SnowSurface Character=%s FootLocation=%s"),
					*GetNameSafe(this),
					*FootstepLocation.ToCompactString());
			}
		}
		return;
	}

	LastSnowTrailStampServerTime = CurrentTime;
	MulticastStampSnowTrail(
		ServerFootstepHit.ImpactPoint,
		ServerFootstepHit.ImpactNormal.GetSafeNormal(),
		FootSocketName,
		SnowTrailStampRadius);
}

void ASnowRumbleCharacter::MulticastStampSnowTrail_Implementation(
	FVector_NetQuantize FootstepLocation,
	FVector_NetQuantizeNormal FootstepNormal,
	FName FootSocketName,
	float RadiusWorld)
{
	ASnowTrailRenderTargetManager* SnowTrailManager =
		ASnowTrailRenderTargetManager::FindSnowTrailManager(this);
	if (!SnowTrailManager)
	{
		return;
	}

	const bool bStamped = SnowTrailManager->StampSnowTrailAtWorldLocation(
		FootstepLocation,
		FootstepNormal.GetSafeNormal(),
		RadiusWorld,
		FootSocketName,
		this);
	if (!bStamped && SnowTrailManager->ShouldLogSnowTrailDebug())
	{
		UE_LOG(
			LogSnowTrailCharacter,
			Warning,
			TEXT("[SnowTrail] Multicast stamp failed Character=%s Location=%s Radius=%.1f Foot=%s"),
			*GetNameSafe(this),
			*FootstepLocation.ToCompactString(),
			RadiusWorld,
			*FootSocketName.ToString());
	}
}

void ASnowRumbleCharacter::ClientRequestLocalDamageFeedback_Implementation(
	float AppliedDamage,
	FVector_NetQuantize DamageCauserLocation)
{
	if (APlayerController* PlayerController =
		Cast<APlayerController>(GetController()))
	{
		if (PlayerController->PlayerCameraManager)
		{
			PlayerController->PlayerCameraManager->StartCameraFade(
				FMath::Clamp(DamageFeedbackTintAlpha, 0.0f, 1.0f),
				0.0f,
				FMath::Max(0.01f, DamageFeedbackTintDuration),
				DamageFeedbackTintColor,
				false,
				false);
		}
	}

	if (const UWorld* World = GetWorld())
	{
		const double CurrentTime = World->GetTimeSeconds();
		LocalDamageCameraShakeStartTime = CurrentTime;
		LocalDamageCameraShakeEndTime =
			CurrentTime + FMath::Max(0.01f, DamageFeedbackCameraShakeDuration);
	}

	OnLocalDamageFeedbackRequested(AppliedDamage, DamageCauserLocation);
}

void ASnowRumbleCharacter::MulticastPlayDamageSound_Implementation(
	FVector_NetQuantize DamageLocation)
{
	SnowRumbleAudio::PlaySoundAtLocation(
		this,
		DamageSound,
		ESnowRumbleAudioMixChannel::Gameplay,
		DamageLocation,
		1.0f,
		1.0f,
		DamageSoundAttenuation);
}

void ASnowRumbleCharacter::MulticastRequestDamageText_Implementation(
	float AppliedDamage,
	FVector_NetQuantize DamageTextWorldLocation,
	ESnowRumbleDamageTextType DamageTextType)
{
	if (AppliedDamage <= 0.0f)
	{
		return;
	}

	TSubclassOf<UDamageTextWidget> WidgetClass = DamageTextWidgetClass;
	if (DamageTextType == ESnowRumbleDamageTextType::Headshot
		&& HeadshotDamageTextWidgetClass)
	{
		WidgetClass = HeadshotDamageTextWidgetClass;
	}
	if (WidgetClass)
	{
		UWidgetComponent* DamageTextComponent =
			NewObject<UWidgetComponent>(this);
		if (DamageTextComponent)
		{
			DamageTextComponent->SetWidgetClass(WidgetClass);
			DamageTextComponent->SetWidgetSpace(EWidgetSpace::Screen);
			DamageTextComponent->SetDrawAtDesiredSize(false);
			DamageTextComponent->SetDrawSize(DamageTextWidgetDrawSize);
			DamageTextComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			DamageTextComponent->RegisterComponent();
			DamageTextComponent->SetWorldLocation(DamageTextWorldLocation);
			DamageTextComponent->InitWidget();
			if (UDamageTextWidget* DamageTextWidget =
				Cast<UDamageTextWidget>(DamageTextComponent->GetUserWidgetObject()))
			{
				DamageTextWidget->InitializeDamageText(
					AppliedDamage,
					DamageTextType);
			}

			if (UWorld* World = GetWorld())
			{
				FTimerHandle DestroyTimerHandle;
				TWeakObjectPtr<UWidgetComponent> WeakDamageTextComponent =
					DamageTextComponent;
				World->GetTimerManager().SetTimer(
					DestroyTimerHandle,
					[WeakDamageTextComponent]()
					{
						if (UWidgetComponent* Component =
							WeakDamageTextComponent.Get())
						{
							Component->DestroyComponent();
						}
					},
					FMath::Max(0.01f, DamageTextWidgetLifeSeconds),
					false);
			}
		}
	}

	OnDamageTextRequested(
		AppliedDamage,
		DamageTextWorldLocation,
		DamageTextType);
}

FVector ASnowRumbleCharacter::CalculateLocalDamageCameraShakeOffset() const
{
	const UWorld* World = GetWorld();
	if (!World
		|| LocalDamageCameraShakeEndTime <= LocalDamageCameraShakeStartTime
		|| DamageFeedbackCameraShakeAmplitude <= 0.0f
		|| DamageFeedbackCameraShakeFrequency <= 0.0f)
	{
		return FVector::ZeroVector;
	}

	const double CurrentTime = World->GetTimeSeconds();
	if (CurrentTime >= LocalDamageCameraShakeEndTime)
	{
		return FVector::ZeroVector;
	}

	const double Duration =
		LocalDamageCameraShakeEndTime - LocalDamageCameraShakeStartTime;
	const float Elapsed = static_cast<float>(
		CurrentTime - LocalDamageCameraShakeStartTime);
	const float NormalizedTime = FMath::Clamp(
		Duration > KINDA_SMALL_NUMBER
			? Elapsed / static_cast<float>(Duration)
			: 1.0f,
		0.0f,
		1.0f);
	const float FadeOut = 1.0f - NormalizedTime;
	const float Phase =
		Elapsed * DamageFeedbackCameraShakeFrequency * UE_TWO_PI;
	const float Amplitude = DamageFeedbackCameraShakeAmplitude * FadeOut;

	return FVector(
		FMath::Sin(Phase * 1.31f) * Amplitude * 0.25f,
		FMath::Sin(Phase) * Amplitude,
		FMath::Cos(Phase * 1.73f) * Amplitude * 0.65f);
}

void ASnowRumbleCharacter::RequestAnimationTriggerFromServer(
	ESnowRumbleCharacterAnimTrigger Trigger)
{
	if (!HasAuthority()
		|| Trigger == ESnowRumbleCharacterAnimTrigger::None)
	{
		return;
	}

	MulticastRequestAnimationTrigger(Trigger);
}

void ASnowRumbleCharacter::MulticastRequestAnimationTrigger_Implementation(
	ESnowRumbleCharacterAnimTrigger Trigger)
{
	if (Trigger == ESnowRumbleCharacterAnimTrigger::None
		|| !GetMesh())
	{
		return;
	}

	USnowRumbleCharacterAnimInstance* AnimInstance =
		Cast<USnowRumbleCharacterAnimInstance>(GetMesh()->GetAnimInstance());
	if (!AnimInstance)
	{
		return;
	}

	AnimInstance->OnAnimationTriggerRequested(Trigger);
}

void ASnowRumbleCharacter::ServerRequestPlayEmote_Implementation(int32 EmoteIndex)
{
	if (!CanPlayEmote()
		|| !IsValidEmoteIndex(EmoteIndex))
	{
		return;
	}

	MulticastPlayEmote(EmoteIndex);
}

void ASnowRumbleCharacter::MulticastPlayEmote_Implementation(int32 EmoteIndex)
{
	PlayEmoteMontage(EmoteIndex);
}

void ASnowRumbleCharacter::MulticastPlayCharacterFeedbackSound_Implementation(
	ESnowRumbleCharacterFeedbackSoundType FeedbackSoundType)
{
	USoundBase* FeedbackSound = nullptr;
	switch (FeedbackSoundType)
	{
	case ESnowRumbleCharacterFeedbackSoundType::ItemPickup:
		FeedbackSound = ItemPickupSound;
		break;
	case ESnowRumbleCharacterFeedbackSoundType::SnowballPickup:
		FeedbackSound = SnowballPickupSound;
		break;
	case ESnowRumbleCharacterFeedbackSoundType::SnowballThrow:
		FeedbackSound = SnowballThrowSound;
		break;
	case ESnowRumbleCharacterFeedbackSoundType::ItemInteraction:
		FeedbackSound = ItemInteractionSound;
		break;
	case ESnowRumbleCharacterFeedbackSoundType::LobbyBoardInteraction:
		FeedbackSound = LobbyBoardInteractionSound;
		break;
	default:
		break;
	}

	SnowRumbleAudio::PlaySoundAtLocation(
		this,
		FeedbackSound,
		ESnowRumbleAudioMixChannel::Gameplay,
		GetActorLocation(),
		1.0f,
		1.0f,
		FeedbackSoundType == ESnowRumbleCharacterFeedbackSoundType::SnowballThrow
			? SnowballThrowSoundAttenuation
			: nullptr);
}

void ASnowRumbleCharacter::PlayServerDirectedEmote(int32 EmoteIndex)
{
	if (!HasAuthority() || !IsValidEmoteIndex(EmoteIndex))
	{
		return;
	}

	MulticastPlayEmote(EmoteIndex);
}

void ASnowRumbleCharacter::PlayRandomServerDirectedEmote()
{
	if (!HasAuthority())
	{
		return;
	}

	TArray<int32> ValidEmoteIndices;
	ValidEmoteIndices.Reserve(EmoteMontages.Num());
	for (int32 EmoteIndex = 0; EmoteIndex < EmoteMontages.Num(); ++EmoteIndex)
	{
		if (IsValidEmoteIndex(EmoteIndex))
		{
			ValidEmoteIndices.Add(EmoteIndex);
		}
	}

	if (ValidEmoteIndices.IsEmpty())
	{
		return;
	}

	const int32 RandomIndex = FMath::RandHelper(ValidEmoteIndices.Num());
	PlayServerDirectedEmote(ValidEmoteIndices[RandomIndex]);
}

void ASnowRumbleCharacter::ApplyMovementSpeed()
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		const float ItemMovementSpeedMultiplier =
			GiftItemEffectComponent
				? GiftItemEffectComponent->GetMovementSpeedMultiplier()
				: 1.0f;
		MovementComponent->MaxWalkSpeed =
			(Cast<ACustomizationPlayerController>(Controller)
				|| Cast<AMainMenuPlayerController>(Controller)
				? 0.0f
				: IsPvpMatchInputLocked()
				? 0.0f
				: HealthComponent && HealthComponent->IsDead()
				? 0.0f
			: bIsPickingUpItem
					|| bIsInteractingWithItem
				? 0.0f
				: PlayerGrabComponent
					&& PlayerGrabComponent->IsCarryingOpposingFrozenCharacter()
				? OpposingFrozenCarryWalkSpeed
				: SnowballEquipmentComponent
					&& SnowballEquipmentComponent->IsRollingSnowball()
					? SnowballEquipmentComponent->GetRollingWalkSpeed()
				: SnowballEquipmentComponent
					&& SnowballEquipmentComponent->IsHoldingLargeSnowball()
					? SnowballEquipmentComponent->GetLargeSnowballCarryWalkSpeed()
				: IsAiming()
				? AimWalkSpeed
				: bIsSprinting
					? SprintSpeed
					: WalkSpeed)
			* ItemMovementSpeedMultiplier;
	}
}

void ASnowRumbleCharacter::UpdateIceGlacierMovementSurface()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	bool bShouldApplySlipperyMovement = false;
	if (bEnableIceGlacierSlipperyMovement && bIsIceGlacierMap
		&& MovementComponent->IsMovingOnGround()
		&& MovementComponent->CurrentFloor.HitResult.GetActor())
	{
		const AActor* FloorActor =
			MovementComponent->CurrentFloor.HitResult.GetActor();
		bShouldApplySlipperyMovement =
			!FloorActor->ActorHasTag(SnowFootstepSurfaceTag);
	}

	if (bSlipperyMovementApplied == bShouldApplySlipperyMovement)
	{
		return;
	}

	if (bShouldApplySlipperyMovement)
	{
		MovementComponent->GroundFriction = FMath::Max(
			0.0f,
			SlipperyGroundFriction);
		MovementComponent->BrakingDecelerationWalking = FMath::Max(
			0.0f,
			SlipperyBrakingDecelerationWalking);
	}
	else
	{
		MovementComponent->GroundFriction = DefaultGroundFriction;
		MovementComponent->BrakingDecelerationWalking =
			DefaultBrakingDecelerationWalking;
	}

	bSlipperyMovementApplied = bShouldApplySlipperyMovement;
}

void ASnowRumbleCharacter::ServerSetSprinting_Implementation(bool bNewSprinting)
{
	bIsSprinting = bNewSprinting
		&& CanPerformGameplayAction()
		&& !IsAiming()
		&& (!SnowballEquipmentComponent
			|| !SnowballEquipmentComponent->IsHoldingLargeSnowball());
	ApplyMovementSpeed();
	ForceNetUpdate();
}

void ASnowRumbleCharacter::ServerUpdateSpectatorCameraView_Implementation(
	FVector_NetQuantize10 CameraLocation,
	FRotator CameraRotation,
	float CameraFieldOfView)
{
	if (!IsValid(GetController())
		|| !IsValid(FollowCamera)
		|| CameraFieldOfView <= 0.0f
		|| CameraFieldOfView > 180.0f)
	{
		return;
	}

	ReplicatedSpectatorCameraLocation = CameraLocation;
	ReplicatedSpectatorCameraRotation = CameraRotation;
	ReplicatedSpectatorCameraFieldOfView = CameraFieldOfView;
	bHasReplicatedSpectatorCameraView = true;
	ForceNetUpdate();
}

void ASnowRumbleCharacter::OnRep_IsSprinting()
{
	ApplyMovementSpeed();
}

void ASnowRumbleCharacter::FinishPickupAnimationState()
{
	if (!HasAuthority())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PickupAnimationTimerHandle);
	}

	bIsPickingUpItem = false;
	OnRep_IsPickingUpItem();
	ForceNetUpdate();
}

void ASnowRumbleCharacter::FinishItemInteractionAnimationState()
{
	if (!HasAuthority())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(
			ItemInteractionAnimationTimerHandle);
	}

	bIsInteractingWithItem = false;
	OnRep_IsInteractingWithItem();
	ForceNetUpdate();
}

void ASnowRumbleCharacter::StartHitReactAnimationState()
{
	if (!HasAuthority()
		|| IsFrozen()
		|| IsDead())
	{
		return;
	}

	bIsHitReacting = true;
	RequestAnimationTriggerFromServer(
		ESnowRumbleCharacterAnimTrigger::HitReact);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			HitReactAnimationTimerHandle,
			this,
			&ASnowRumbleCharacter::FinishHitReactAnimationState,
			HitReactAnimationStateDuration,
			false);
	}

	ForceNetUpdate();
}

void ASnowRumbleCharacter::FinishHitReactAnimationState()
{
	if (!HasAuthority())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HitReactAnimationTimerHandle);
	}

	bIsHitReacting = false;
	ForceNetUpdate();
}

void ASnowRumbleCharacter::OnRep_IsPickingUpItem()
{
	if (bIsPickingUpItem)
	{
		PickupLockedRotation = GetActorRotation();
		if (AController* LocalController = GetController())
		{
			PickupLockedControlRotation = LocalController->GetControlRotation();
		}
	}

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (bIsPickingUpItem && MovementComponent)
	{
		FVector CurrentVelocity = MovementComponent->Velocity;
		CurrentVelocity.X = 0.0f;
		CurrentVelocity.Y = 0.0f;
		MovementComponent->Velocity = CurrentVelocity;
		bIsSprinting = false;
	}

	ApplyMovementSpeed();
}

void ASnowRumbleCharacter::OnRep_IsInteractingWithItem()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (bIsInteractingWithItem && MovementComponent)
	{
		FVector CurrentVelocity = MovementComponent->Velocity;
		CurrentVelocity.X = 0.0f;
		CurrentVelocity.Y = 0.0f;
		MovementComponent->Velocity = CurrentVelocity;
		bIsSprinting = false;
	}

	ApplyMovementSpeed();
}

void ASnowRumbleCharacter::OnRep_TiebreakerSpectator()
{
	if (IsLocallyControlled() && !bTiebreakerSpectator)
	{
		if (APlayerController* PlayerController =
			Cast<APlayerController>(GetController()))
		{
			PlayerController->SetViewTargetWithBlend(this, 0.15f);
		}
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		if (bTiebreakerSpectator)
		{
			Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
			Capsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
			Capsule->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
		}
		else
		{
			Capsule->SetCollisionProfileName(TEXT("Pawn"));
		}
	}

	bIsSprinting = false;
	if (SnowballEquipmentComponent)
	{
		SnowballEquipmentComponent->SetAiming(false);
	}
	RefreshPvpMatchInputLock();
	RefreshTiebreakerSpectatorViewTarget();
	ApplyMovementSpeed();
}

void ASnowRumbleCharacter::OnRep_WaterSubmerged()
{
	if (bWaterSubmerged)
	{
		StopJumping();
	}
}

void ASnowRumbleCharacter::OnRep_GrabbedByCharacter()
{
	HandleGrabbedByCharacterChanged(bIsGrabbedByCharacter);
}

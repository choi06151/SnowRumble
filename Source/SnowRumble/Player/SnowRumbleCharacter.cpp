// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleCharacter.h"

#include "SnowRumbleHealthComponent.h"
#include "../Game/SnowRumbleGameState_C.h"
#include "../Game/SnowRumbleLobbyGameState.h"
#include "../Game/SnowRumblePlayerState.h"
#include "../Interaction/LobbyInteractionBoard_C.h"
#include "../Interaction/OutlineComponent.h"
#include "../Snowball/SnowballCreationComponent.h"
#include "../Snowball/SnowballEquipmentComponent.h"
#include "../Snowball/SnowballItem.h"
#include "../UI/EmoteRadialMenuWidget.h"
#include "../UI/MainHUDWidget.h"
#include "../UI/OverheadNameplateWidget_C.h"
#include "Animation/AnimMontage.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Components/WidgetComponent.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"

ASnowRumbleCharacter::ASnowRumbleCharacter()
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

	SnowballEquipmentComponent =
		CreateDefaultSubobject<USnowballEquipmentComponent>(TEXT("SnowballEquipmentComponent"));

	SnowballCreationComponent =
		CreateDefaultSubobject<USnowballCreationComponent>(TEXT("SnowballCreationComponent"));

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
}

void ASnowRumbleCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const UWorld* World = GetWorld();
	const bool bUseAimCameraPresentation =
		IsAiming()
		|| (World
			&& World->GetTimeSeconds() < PostThrowAimCameraEndTime);

	if (IsLocallyControlled() && CameraBoom)
	{
		FVector TargetCameraOffset = DefaultCameraSocketOffset;
		const float ShoulderOffset =
			bUseAimCameraPresentation
				? AimShoulderOffset
				: DefaultShoulderOffset;
		TargetCameraOffset.Y += CameraShoulderSide * ShoulderOffset;

		CameraBoom->SocketOffset = FMath::VInterpTo(
			CameraBoom->SocketOffset,
			TargetCameraOffset,
			DeltaSeconds,
			CameraPositionInterpSpeed);
		CameraBoom->TargetArmLength = FMath::FInterpTo(
			CameraBoom->TargetArmLength,
			bUseAimCameraPresentation
				? AimCameraArmLength
				: DefaultCameraArmLength,
			DeltaSeconds,
			CameraPositionInterpSpeed);
	}

	if (IsLocallyControlled() && FollowCamera)
	{
		const float TargetFieldOfView =
			bUseAimCameraPresentation
				? AimFieldOfView
				: DefaultFieldOfView;
		FollowCamera->SetFieldOfView(FMath::FInterpTo(
			FollowCamera->FieldOfView,
			TargetFieldOfView,
			DeltaSeconds,
			AimFieldOfViewInterpSpeed));
	}

	if (OutlineComponent)
	{
		AActor* OutlinedActor = nullptr;
		if (IsLocallyControlled()
			&& CanPerformGameplayAction()
			&& !FocusedLobbyBoard)
		{
			OutlinedActor = FindClosestLobbyBoardCandidate();
			if (!OutlinedActor
				&& SnowballEquipmentComponent
				&& !SnowballEquipmentComponent->HasHeldSnowball())
			{
				OutlinedActor = SnowballEquipmentComponent->IsRollingSnowball()
					? SnowballEquipmentComponent->GetRollingSnowball()
					: SnowballEquipmentComponent->FindClosestPickupCandidate();
			}
		}

		OutlineComponent->SetOutlinedActor(OutlinedActor);
	}

	DrawRollingSnowballCollisionDebug();
	RefreshOverheadNameplateFacing();
	RefreshPvpMatchInputLock();
	ApplyMovementSpeed();
	RefreshLocalSnowEffect();

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

float ASnowRumbleCharacter::GetSnowballCreationProgress() const
{
	return SnowballCreationComponent
		? SnowballCreationComponent->GetCreationProgress()
		: 0.0f;
}

ESnowRumbleTimedActionState ASnowRumbleCharacter::GetTimedActionState() const
{
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

USceneComponent* ASnowRumbleCharacter::GetSnowballHoldPoint() const
{
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

float ASnowRumbleCharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	const float ValidatedDamage = Super::TakeDamage(
		DamageAmount,
		DamageEvent,
		EventInstigator,
		DamageCauser);

	return HealthComponent ? HealthComponent->ApplyDamage(ValidatedDamage) : 0.0f;
}

void ASnowRumbleCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RefreshOverheadNameplateComponentSettings();
}

void ASnowRumbleCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera)
	{
		DefaultFieldOfView = FollowCamera->FieldOfView;
	}
	if (CameraBoom)
	{
		CameraBoom->TargetOffset.Z = CameraPivotHeight;
		DefaultCameraSocketOffset = CameraBoom->SocketOffset;
		DefaultCameraArmLength = CameraBoom->TargetArmLength;
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

	BindOverheadNameToPlayerState();
	RefreshOverheadPlayerName();
	RefreshPvpMatchInputLock();
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
}

void ASnowRumbleCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	BindOverheadNameToPlayerState();
	RefreshOverheadPlayerName();
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

void ASnowRumbleCharacter::RefreshOverheadNameplateComponentSettings()
{
	if (!OverheadNameplateComponent)
	{
		return;
	}

	OverheadNameplateComponent->SetRelativeLocation(OverheadNameRelativeLocation);
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
		|| !EmoteRadialMenuWidgetClass)
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

void ASnowRumbleCharacter::EnsureMainHUDWidget()
{
	if (!IsLocallyControlled()
		|| MainHUDWidget
		|| !MainHUDWidgetClass)
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

	MainHUDWidget =
		CreateWidget<UMainHUDWidget>(
			PlayerController,
			MainHUDWidgetClass);
	if (MainHUDWidget)
	{
		MainHUDWidget->AddToViewport();
	}
}

void ASnowRumbleCharacter::OpenEmoteRadialMenu()
{
	if (!IsLocallyControlled())
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
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(EmoteRadialMenuWidget->TakeWidget());
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);
		PlayerController->SetShowMouseCursor(true);
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
			FInputModeGameOnly InputMode;
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(false);
			PlayerController->SetIgnoreLookInput(false);
		}
	}

	bIsEmoteRadialMenuOpen = false;
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
}

void ASnowRumbleCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (!Controller || !CanPerformGameplayAction())
	{
		return;
	}

	if (bIsInteractHeld
		&& !bUsedInteractForRolling
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
	if (bIsEmoteRadialMenuOpen || IsPvpMatchInputLocked())
	{
		return;
	}

	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (FMath::Abs(LookAxisVector.X) > KINDA_SMALL_NUMBER)
	{
		CameraShoulderSide = LookAxisVector.X < 0.0f ? 1.0f : -1.0f;
	}

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(-LookAxisVector.Y);
}

void ASnowRumbleCharacter::StartJump()
{
	if (CanPerformGameplayAction()
		&& (!SnowballEquipmentComponent
			|| !SnowballEquipmentComponent->IsRollingSnowball()))
	{
		Jump();
	}
}

void ASnowRumbleCharacter::StopJump()
{
	StopJumping();
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
		OnInteractInput(true);
	}
}

void ASnowRumbleCharacter::HandleInteractCompleted()
{
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
		else if (bIsInteractHeld && CanPerformGameplayAction())
		{
			const ALobbyInteractionBoard* OutlinedBoard = OutlineComponent
				? Cast<ALobbyInteractionBoard>(OutlineComponent->GetOutlinedActor())
				: nullptr;
			if (OutlinedBoard)
			{
				TryInteractWithLobbyBoard();
			}
			else
			{
				SnowballEquipmentComponent->TryPickupSnowball();
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
	if (SnowballEquipmentComponent && SnowballEquipmentComponent->IsCharging())
	{
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
		InputSubsystem->ClearAllMappings();
		InputSubsystem->AddMappingContext(PlayerMappingContext, 0);
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
}

void ASnowRumbleCharacter::HandleDeathChanged(bool bIsDead)
{
	if (!bIsDead)
	{
		return;
	}

	bIsSprinting = false;
	bIsPickingUpItem = false;
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
}

void ASnowRumbleCharacter::HandleSnowballAimingChanged(bool bNewAiming)
{
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

void ASnowRumbleCharacter::CloseLobbyBoardFocus()
{
	ClearLobbyBoardFocus();
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
	return HealthComponent
		&& !HealthComponent->IsFrozen()
		&& !HealthComponent->IsDead()
		&& !bIsPickingUpItem
		&& !IsPvpMatchInputLocked();
}

bool ASnowRumbleCharacter::IsPvpMatchInputLocked() const
{
	const UWorld* World = GetWorld();
	const ASnowRumbleGameState* SnowRumbleGameState = World
		? World->GetGameState<ASnowRumbleGameState>()
		: nullptr;
	return SnowRumbleGameState
		&& SnowRumbleGameState->IsMatchInputLocked();
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

	const bool bShouldBlockInput = IsPvpMatchInputLocked();
	if (bShouldBlockInput)
	{
		if (!bPvpMatchInputIgnoreApplied)
		{
			PlayerController->SetIgnoreMoveInput(true);
			PlayerController->SetIgnoreLookInput(true);
			bPvpMatchInputIgnoreApplied = true;
		}
		if (UCharacterMovementComponent* MovementComponent =
			GetCharacterMovement())
		{
			MovementComponent->StopMovementImmediately();
		}
		bIsSprinting = false;
		return;
	}

	if (bPvpMatchInputIgnoreApplied && !FocusedLobbyBoard)
	{
		PlayerController->ResetIgnoreMoveInput();
		PlayerController->ResetIgnoreLookInput();
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
		bPvpMatchInputIgnoreApplied = false;
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
	return CanPerformGameplayAction()
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

void ASnowRumbleCharacter::ApplyMovementSpeed()
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed =
			IsPvpMatchInputLocked()
				? 0.0f
				: HealthComponent && HealthComponent->IsDead()
				? 0.0f
				: bIsPickingUpItem
				? 0.0f
				: SnowballEquipmentComponent
					&& SnowballEquipmentComponent->IsRollingSnowball()
					? SnowballEquipmentComponent->GetRollingWalkSpeed()
				: SnowballEquipmentComponent
					&& SnowballEquipmentComponent->IsHoldingLargeSnowball()
					? SnowballEquipmentComponent->GetLargeSnowballCarryWalkSpeed()
				: IsAiming()
				? AimWalkSpeed
				: bIsSprinting && CanPerformGameplayAction()
					? SprintSpeed
					: WalkSpeed;
	}
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

void ASnowRumbleCharacter::OnRep_IsPickingUpItem()
{
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

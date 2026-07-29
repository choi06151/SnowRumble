// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleCharacter.h"

#include "SnowRumbleHealthComponent.h"
#include "../Interaction/OutlineComponent.h"
#include "../Snowball/SnowballCreationComponent.h"
#include "../Snowball/SnowballEquipmentComponent.h"
#include "../Snowball/SnowballItem.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

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

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	HealthComponent = CreateDefaultSubobject<USnowRumbleHealthComponent>(TEXT("HealthComponent"));

	SnowballEquipmentComponent =
		CreateDefaultSubobject<USnowballEquipmentComponent>(TEXT("SnowballEquipmentComponent"));

	SnowballCreationComponent =
		CreateDefaultSubobject<USnowballCreationComponent>(TEXT("SnowballCreationComponent"));

	SnowballHoldPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SnowballHoldPoint"));
	SnowballHoldPoint->SetupAttachment(GetMesh(), TEXT("SnowballSocket"));

	OutlineComponent = CreateDefaultSubobject<UOutlineComponent>(TEXT("OutlineComponent"));
}

void ASnowRumbleCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsLocallyControlled() && IsChargingSnowball() && GEngine)
	{
		const int32 ChargePercent = FMath::RoundToInt(GetSnowballChargeProgress() * 100.0f);
		GEngine->AddOnScreenDebugMessage(
			static_cast<uint64>(GetUniqueID()),
			0.05f,
			FColor::Cyan,
			FString::Printf(TEXT("Snowball Charge: %d%%"), ChargePercent));
	}

	if (OutlineComponent)
	{
		AActor* OutlinedActor = nullptr;
		if (IsLocallyControlled()
			&& CanPerformGameplayAction()
			&& SnowballEquipmentComponent
			&& !SnowballEquipmentComponent->HasHeldSnowball())
		{
			OutlinedActor = SnowballEquipmentComponent->IsRollingSnowball()
				? SnowballEquipmentComponent->GetRollingSnowball()
				: SnowballEquipmentComponent->FindClosestPickupCandidate();
		}

		OutlineComponent->SetOutlinedActor(OutlinedActor);
	}

	ApplyMovementSpeed();
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

USceneComponent* ASnowRumbleCharacter::GetSnowballHoldPoint() const
{
	return SnowballHoldPoint;
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

void ASnowRumbleCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera)
	{
		DefaultFieldOfView = FollowCamera->FieldOfView;
	}

	if (HealthComponent)
	{
		HealthComponent->OnFrozenChanged.AddDynamic(this, &ASnowRumbleCharacter::HandleFrozenChanged);
		HandleFrozenChanged(HealthComponent->IsFrozen());
	}

	if (SnowballEquipmentComponent)
	{
		SnowballEquipmentComponent->OnAimingChanged.AddDynamic(
			this,
			&ASnowRumbleCharacter::HandleSnowballAimingChanged);
		HandleSnowballAimingChanged(SnowballEquipmentComponent->IsAiming());
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

void ASnowRumbleCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	ApplyInputMappingContext();
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
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(-LookAxisVector.Y);
}

void ASnowRumbleCharacter::StartJump()
{
	if (CanPerformGameplayAction())
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
		else if (bIsInteractHeld && CanPerformGameplayAction())
		{
			SnowballEquipmentComponent->TryPickupSnowball();
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

void ASnowRumbleCharacter::HandleEmoteStarted()
{
	if (CanPerformGameplayAction())
	{
		OnEmoteInput(true);
	}
}

void ASnowRumbleCharacter::HandleEmoteCompleted()
{
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
		MovementComponent->SetMovementMode(MOVE_Walking);
	}
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

	if (IsLocallyControlled() && FollowCamera)
	{
		FollowCamera->SetFieldOfView(bNewAiming ? AimFieldOfView : DefaultFieldOfView);
	}

	if (HasAuthority())
	{
		ForceNetUpdate();
	}
}

bool ASnowRumbleCharacter::CanPerformGameplayAction() const
{
	return HealthComponent
		&& !HealthComponent->IsFrozen()
		&& !bIsPickingUpItem;
}

void ASnowRumbleCharacter::ApplyMovementSpeed()
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed =
			bIsPickingUpItem
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

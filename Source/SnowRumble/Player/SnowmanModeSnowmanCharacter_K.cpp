// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowmanModeSnowmanCharacter_K.h"

#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "../UI/OverheadNameplateWidget_C.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ASnowmanModeSnowmanCharacter::ASnowmanModeSnowmanCharacter()
{
	static ConstructorHelpers::FObjectFinder<UInputMappingContext>
		DefaultPlayerMappingContext(TEXT("/Game/Input/IMC_Player.IMC_Player"));
	static ConstructorHelpers::FObjectFinder<UInputAction>
		DefaultMoveAction(TEXT("/Game/Input/IA_Move.IA_Move"));
	static ConstructorHelpers::FObjectFinder<UInputAction>
		DefaultLookAction(TEXT("/Game/Input/IA_Look.IA_Look"));
	static ConstructorHelpers::FObjectFinder<UInputAction>
		DefaultJumpAction(TEXT("/Game/Input/IA_Jump.IA_Jump"));
	static ConstructorHelpers::FClassFinder<UOverheadNameplateWidget>
		DefaultOverheadNameplateWidget(
			TEXT("/Game/WBP/WBP_OverheadNamePlate"));

	if (DefaultPlayerMappingContext.Succeeded())
	{
		PlayerMappingContext = DefaultPlayerMappingContext.Object;
	}
	if (DefaultMoveAction.Succeeded())
	{
		MoveAction = DefaultMoveAction.Object;
	}
	if (DefaultLookAction.Succeeded())
	{
		LookAction = DefaultLookAction.Object;
	}
	if (DefaultJumpAction.Succeeded())
	{
		JumpAction = DefaultJumpAction.Object;
	}
	if (DefaultOverheadNameplateWidget.Succeeded())
	{
		OverheadNameplateWidgetClass =
			DefaultOverheadNameplateWidget.Class;
	}

	WalkSpeed = SnowmanWalkSpeed;
	SprintSpeed = SnowmanWalkSpeed;
	AimWalkSpeed = SnowmanWalkSpeed;

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = SnowmanWalkSpeed;
	}
}

void ASnowmanModeSnowmanCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetSnowmanWalkSpeedFromMode(SnowmanWalkSpeed);
}

void ASnowmanModeSnowmanCharacter::SetSnowmanWalkSpeedFromMode(
	float NewSnowmanWalkSpeed)
{
	SnowmanWalkSpeed = FMath::Max(0.0f, NewSnowmanWalkSpeed);
	WalkSpeed = SnowmanWalkSpeed;
	SprintSpeed = SnowmanWalkSpeed;
	AimWalkSpeed = SnowmanWalkSpeed;

	ApplySnowballHitStunMovementState();
}

void ASnowmanModeSnowmanCharacter::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(
		ASnowmanModeSnowmanCharacter,
		bSnowballHitStunned);
}

void ASnowmanModeSnowmanCharacter::ApplySnowballHitStunFromServer()
{
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || SnowballHitStunSeconds <= 0.0f)
	{
		return;
	}

	bSnowballHitStunned = true;
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}
	OnRep_SnowballHitStunned();

	World->GetTimerManager().SetTimer(
		SnowballHitStunTimerHandle,
		this,
		&ASnowmanModeSnowmanCharacter::ClearSnowballHitStun,
		SnowballHitStunSeconds,
		false);
	ForceNetUpdate();
}

void ASnowmanModeSnowmanCharacter::ClearSnowballHitStun()
{
	if (!HasAuthority())
	{
		return;
	}

	bSnowballHitStunned = false;
	OnRep_SnowballHitStunned();
	ForceNetUpdate();
}

void ASnowmanModeSnowmanCharacter::OnRep_SnowballHitStunned()
{
	ApplySnowballHitStunMovementState();
}

void ASnowmanModeSnowmanCharacter::ApplySnowballHitStunMovementState()
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed =
			bSnowballHitStunned ? 0.0f : SnowmanWalkSpeed;
	}
}

void ASnowmanModeSnowmanCharacter::SetupPlayerInputComponent(
	UInputComponent* PlayerInputComponent)
{
	ACharacter::SetupPlayerInputComponent(PlayerInputComponent);

	ApplyInputMappingContext();

	UEnhancedInputComponent* EnhancedInputComponent =
		Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!ensureMsgf(
			EnhancedInputComponent,
			TEXT("SnowmanModeSnowmanCharacter에는 Enhanced Input Component가 필요합니다.")))
	{
		return;
	}

	if (MoveAction)
	{
		EnhancedInputComponent->BindAction(
			MoveAction,
			ETriggerEvent::Triggered,
			this,
			&ASnowmanModeSnowmanCharacter::Move);
	}
	if (LookAction)
	{
		EnhancedInputComponent->BindAction(
			LookAction,
			ETriggerEvent::Triggered,
			this,
			&ASnowmanModeSnowmanCharacter::Look);
	}
	if (JumpAction)
	{
		EnhancedInputComponent->BindAction(
			JumpAction,
			ETriggerEvent::Started,
			this,
			&ASnowmanModeSnowmanCharacter::StartJump);
		EnhancedInputComponent->BindAction(
			JumpAction,
			ETriggerEvent::Completed,
			this,
			&ASnowmanModeSnowmanCharacter::StopJump);
	}
}

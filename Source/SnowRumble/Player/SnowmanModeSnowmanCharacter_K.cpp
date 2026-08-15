// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowmanModeSnowmanCharacter_K.h"

#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
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

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = SnowmanWalkSpeed;
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

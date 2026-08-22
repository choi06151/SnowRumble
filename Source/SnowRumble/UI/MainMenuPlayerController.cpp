// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainMenuPlayerController.h"

#include "Animation/AnimationAsset.h"
#include "Blueprint/UserWidget.h"
#include "../Audio/SnowRumbleAudioHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MainMenuWidget.h"
#include "OptionsWidget_C.h"
#include "../Online/SnowRumbleSessionSubsystem.h"
#include "../Player/SnowRumbleCharacter.h"
#include "../Player/SnowRumbleCustomizationSubsystem_C.h"
#include "Sound/SoundBase.h"

void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (USnowRumbleSessionSubsystem* SessionSubsystem =
				GameInstance->GetSubsystem<USnowRumbleSessionSubsystem>())
			{
				SessionSubsystem->LeaveLanSession();
			}
		}

		ShowMainMenu();
		ApplyMainMenuInputLock();
		ApplyMainMenuPreviewAnimation();
		ApplyMainMenuPreviewCustomization();
		PlayBackgroundMusic();
	}
}

void AMainMenuPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OptionsWidget)
	{
		OptionsWidget->RemoveFromParent();
		OptionsWidget = nullptr;
	}
	DefaultMouseCursorWidget = nullptr;
	StopBackgroundMusic();
	HideMainMenu();

	Super::EndPlay(EndPlayReason);
}

void AMainMenuPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (IsLocalController())
	{
		ApplyMainMenuInputLock();
		ApplyMainMenuPreviewAnimation();
		ApplyMainMenuPreviewCustomization();
	}
}

void AMainMenuPlayerController::ShowMainMenu()
{
	if (!IsLocalController())
	{
		return;
	}

	UMainMenuWidget* Widget = EnsureMainMenuWidget();
	if (!Widget)
	{
		return;
	}

	if (!Widget->IsInViewport())
	{
		Widget->AddToViewport();
	}

	bShowMouseCursor = true;
	ApplyDefaultMouseCursorWidget();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(Widget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AMainMenuPlayerController::HideMainMenu()
{
	if (MainMenuWidget)
	{
		MainMenuWidget->RemoveFromParent();
	}

	if (IsLocalController())
	{
		bShowMouseCursor = false;
		SetInputMode(FInputModeGameOnly());
	}
}

void AMainMenuPlayerController::ShowOptionsMenu()
{
	if (!IsLocalController())
	{
		return;
	}

	UOptionsWidget* Widget = EnsureOptionsWidget();
	if (!Widget)
	{
		return;
	}

	if (!Widget->IsInViewport())
	{
		Widget->AddToViewport(200);
	}
	Widget->SetKeyboardFocus();

	bShowMouseCursor = true;
	ApplyDefaultMouseCursorWidget();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(Widget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AMainMenuPlayerController::HideOptionsMenu()
{
	if (OptionsWidget)
	{
		OptionsWidget->DiscardPendingOptionChanges();
		OptionsWidget->RemoveFromParent();
	}

	if (IsLocalController())
	{
		ShowMainMenu();
	}
}

void AMainMenuPlayerController::TravelToCustomizationLevel()
{
	if (!IsLocalController() || CustomizationLevelUrl.IsEmpty())
	{
		return;
	}

	ClientTravel(CustomizationLevelUrl, TRAVEL_Absolute);
}

UMainMenuWidget* AMainMenuPlayerController::EnsureMainMenuWidget()
{
	if (MainMenuWidget)
	{
		return MainMenuWidget;
	}

	if (!MainMenuWidgetClass)
	{
		return nullptr;
	}

	MainMenuWidget = CreateWidget<UMainMenuWidget>(this, MainMenuWidgetClass);
	return MainMenuWidget;
}

UOptionsWidget* AMainMenuPlayerController::EnsureOptionsWidget()
{
	if (OptionsWidget)
	{
		return OptionsWidget;
	}

	if (!OptionsWidgetClass)
	{
		return nullptr;
	}

	OptionsWidget = CreateWidget<UOptionsWidget>(this, OptionsWidgetClass);
	if (OptionsWidget)
	{
		OptionsWidget->OnOptionsCloseRequestedNative.AddUObject(
			this,
			&AMainMenuPlayerController::HideOptionsMenu);
	}
	return OptionsWidget;
}

void AMainMenuPlayerController::ApplyDefaultMouseCursorWidget()
{
	if (!IsLocalController() || !DefaultMouseCursorWidgetClass)
	{
		return;
	}

	if (!DefaultMouseCursorWidget)
	{
		DefaultMouseCursorWidget =
			CreateWidget<UUserWidget>(this, DefaultMouseCursorWidgetClass);
	}
	if (!DefaultMouseCursorWidget)
	{
		return;
	}

	SetMouseCursorWidget(EMouseCursor::Default, DefaultMouseCursorWidget);
	DefaultMouseCursor = EMouseCursor::Default;
	CurrentMouseCursor = EMouseCursor::Default;
}

void AMainMenuPlayerController::ApplyMainMenuInputLock()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!IsMoveInputIgnored())
	{
		SetIgnoreMoveInput(true);
	}
	if (!IsLookInputIgnored())
	{
		SetIgnoreLookInput(true);
	}
	bShowMouseCursor = true;

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	if (UCharacterMovementComponent* MovementComponent =
		ControlledPawn->FindComponentByClass<UCharacterMovementComponent>())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->Velocity = FVector::ZeroVector;
		MovementComponent->GravityScale = 0.0f;
		MovementComponent->MaxWalkSpeed = 0.0f;
		if (MovementComponent->MovementMode != MOVE_None)
		{
			MovementComponent->DisableMovement();
		}
	}
}

void AMainMenuPlayerController::ApplyMainMenuPreviewAnimation()
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		LastAnimatedPreviewPawn.Reset();
		LastCustomizedPreviewCharacter.Reset();
		bHasAppliedPreviewCustomizationData = false;
		return;
	}

	if (LastAnimatedPreviewPawn.Get() == ControlledPawn)
	{
		return;
	}

	LastAnimatedPreviewPawn = ControlledPawn;

	TArray<USkeletalMeshComponent*> MeshComponents;
	ControlledPawn->GetComponents(MeshComponents);
	for (USkeletalMeshComponent* MeshComponent : MeshComponents)
	{
		if (!MeshComponent)
		{
			continue;
		}

		if (MainMenuPreviewAnimationAsset)
		{
			MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			MeshComponent->SetAnimation(MainMenuPreviewAnimationAsset);
			MeshComponent->SetPosition(
				MainMenuPreviewAnimationPositionSeconds,
				false);
			if (!bPauseMainMenuPreviewAnimation)
			{
				MeshComponent->Play(true);
			}
		}
		MeshComponent->bPauseAnims = bPauseMainMenuPreviewAnimation;

		const float SafeMeshScale = FMath::Max(0.01f, MainMenuPreviewMeshScale);
		MeshComponent->SetRelativeScale3D(FVector(SafeMeshScale));
	}
}

void AMainMenuPlayerController::ApplyMainMenuPreviewCustomization()
{
	ASnowRumbleCharacter* PreviewCharacter =
		Cast<ASnowRumbleCharacter>(GetPawn());
	if (!PreviewCharacter)
	{
		LastCustomizedPreviewCharacter.Reset();
		bHasAppliedPreviewCustomizationData = false;
		return;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const USnowRumbleCustomizationSubsystem* CustomizationSubsystem =
		GameInstance
			? GameInstance->GetSubsystem<USnowRumbleCustomizationSubsystem>()
			: nullptr;
	const FSnowRumbleCustomizationData CustomizationData =
		CustomizationSubsystem
			? CustomizationSubsystem->GetCustomizationData()
			: USnowRumbleCustomizationSubsystem::GetDefaultCustomizationData();

	if (LastCustomizedPreviewCharacter.Get() == PreviewCharacter
		&& bHasAppliedPreviewCustomizationData
		&& LastAppliedPreviewCustomizationData == CustomizationData)
	{
		return;
	}

	PreviewCharacter->ApplyCustomizationData(CustomizationData);
	LastCustomizedPreviewCharacter = PreviewCharacter;
	LastAppliedPreviewCustomizationData = CustomizationData;
	bHasAppliedPreviewCustomizationData = true;
}

void AMainMenuPlayerController::PlayBackgroundMusic()
{
	if (!IsLocalController())
	{
		return;
	}

	if (BackgroundMusicComponent.IsValid())
	{
		BackgroundMusicComponent->Stop();
	}

	if (!BackgroundMusicSound)
	{
		BackgroundMusicComponent.Reset();
		return;
	}

	BackgroundMusicComponent = SnowRumbleAudio::SpawnSound2D(
		this,
		BackgroundMusicSound,
		ESnowRumbleAudioMixChannel::BackgroundMusic,
		1.0f,
		1.0f,
		true);
}

void AMainMenuPlayerController::StopBackgroundMusic()
{
	if (BackgroundMusicComponent.IsValid())
	{
		BackgroundMusicComponent->Stop();
	}
	BackgroundMusicComponent.Reset();
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "MainMenuPlayerController.h"

#include "Animation/AnimationAsset.h"
#include "Blueprint/UserWidget.h"
#include "../Audio/SnowRumbleBackgroundMusicSubsystem_C.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MainMenuWidget.h"
#include "MainMenuKeyGuideWidget_C.h"
#include "OptionsWidget_C.h"
#include "../Online/SnowRumbleSessionSubsystem.h"
#include "../Player/SnowRumbleCharacter.h"
#include "../Player/SnowRumbleCustomizationSubsystem_C.h"
#include "Sound/SoundBase.h"

namespace
{
void DisableMainMenuActorShadowCasting(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	Actor->GetComponents(PrimitiveComponents);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (PrimitiveComponent)
		{
			PrimitiveComponent->SetCastShadow(false);
		}
	}
}
}

void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		RemoveStaleNetworkCharacters();

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

void AMainMenuPlayerController::RemoveStaleNetworkCharacters()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APawn* LocalPreviewPawn = GetPawn();
	for (TActorIterator<ASnowRumbleCharacter> Iterator(World); Iterator; ++Iterator)
	{
		ASnowRumbleCharacter* StaleCharacter = *Iterator;
		if (StaleCharacter && StaleCharacter != LocalPreviewPawn)
		{
			StaleCharacter->Destroy();
		}
	}
}

void AMainMenuPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (KeyGuideWidget)
	{
		KeyGuideWidget->RemoveFromParent();
		KeyGuideWidget = nullptr;
	}
	bKeyGuideWidgetVisible = false;
	if (OptionsWidget)
	{
		OptionsWidget->RemoveFromParent();
		OptionsWidget = nullptr;
	}
	DefaultMouseCursorWidget = nullptr;
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

void AMainMenuPlayerController::QuitGame()
{
	if (!IsLocalController())
	{
		return;
	}

	UKismetSystemLibrary::QuitGame(
		this,
		this,
		EQuitPreference::Quit,
		false);
}

void AMainMenuPlayerController::ToggleKeyGuideWidget()
{
	if (!IsLocalController())
	{
		return;
	}

	EnsureKeyGuideWidget();
	if (!KeyGuideWidget)
	{
		return;
	}

	bKeyGuideWidgetVisible = !bKeyGuideWidgetVisible;
	if (bKeyGuideWidgetVisible)
	{
		KeyGuideWidget->RefreshKeyGuideTexts();
		KeyGuideWidget->SetVisibility(ESlateVisibility::Visible);
		bShowMouseCursor = true;
		ApplyDefaultMouseCursorWidget();

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(KeyGuideWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
	else
	{
		CloseKeyGuideWidget();
	}
}

void AMainMenuPlayerController::CloseKeyGuideWidget()
{
	if (!IsLocalController())
	{
		return;
	}

	if (KeyGuideWidget)
	{
		KeyGuideWidget->RemoveFromParent();
		KeyGuideWidget = nullptr;
	}
	bKeyGuideWidgetVisible = false;

	ShowMainMenu();
}

void AMainMenuPlayerController::SetBackgroundMusicPreviewVolume(
	float MasterVolume,
	float BgmVolume)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USnowRumbleBackgroundMusicSubsystem* BackgroundMusicSubsystem =
			GameInstance->GetSubsystem<USnowRumbleBackgroundMusicSubsystem>())
		{
			BackgroundMusicSubsystem->SetBackgroundMusicPreviewVolume(
				MasterVolume,
				BgmVolume);
		}
	}
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

void AMainMenuPlayerController::EnsureKeyGuideWidget()
{
	if (KeyGuideWidget || !KeyGuideWidgetClass)
	{
		return;
	}

	KeyGuideWidget = CreateWidget<UMainMenuKeyGuideWidget>(
		this,
		KeyGuideWidgetClass);
	if (KeyGuideWidget)
	{
		KeyGuideWidget->AddToViewport(100);
		KeyGuideWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
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

	DisableMainMenuActorShadowCasting(ControlledPawn);

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

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USnowRumbleBackgroundMusicSubsystem* BackgroundMusicSubsystem =
			GameInstance->GetSubsystem<USnowRumbleBackgroundMusicSubsystem>())
		{
			BackgroundMusicSubsystem->PlayBackgroundMusic(BackgroundMusicSound);
		}
	}
}

void AMainMenuPlayerController::StopBackgroundMusic()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USnowRumbleBackgroundMusicSubsystem* BackgroundMusicSubsystem =
			GameInstance->GetSubsystem<USnowRumbleBackgroundMusicSubsystem>())
		{
			BackgroundMusicSubsystem->StopBackgroundMusic();
		}
	}
}

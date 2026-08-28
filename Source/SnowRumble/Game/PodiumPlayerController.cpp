// Copyright Epic Games, Inc. All Rights Reserved.

#include "PodiumPlayerController.h"

#include "../Audio/SnowRumbleBackgroundMusicSubsystem_C.h"
#include "../UI/LoadingScreenSubsystem.h"
#include "../UI/PodiumWinnerWidget.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Sound/SoundBase.h"

namespace
{
void DisableActorShadowCasting(AActor* Actor)
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

APodiumPlayerController::APodiumPlayerController()
{
}

void APodiumPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() && GetLocalPlayer())
	{
		UGameInstance* GameInstance = GetGameInstance();
		if (ULoadingScreenSubsystem* LoadingScreenSubsystem = GameInstance
			? GameInstance->GetSubsystem<ULoadingScreenSubsystem>()
			: nullptr)
		{
			LoadingScreenSubsystem->HideLoadingScreen();
		}
	}

	UWorld* World = GetWorld();
	ACameraActor* PodiumCamActor = nullptr;
	if (World)
	{
		for (TActorIterator<ACameraActor> It(World); It; ++It)
		{
			ACameraActor* Cam = *It;
			if (Cam && Cam->GetName().Contains(TEXT("Podium_Camera")))
			{
				PodiumCamActor = Cam;
				break;
			}
		}
	}

	if (PodiumCamActor)
	{
		SetViewTargetWithBlend(PodiumCamActor, 0.5f);
	}

	ApplyPodiumPawnPresentation(GetPawn());

	if (IsLocalController() && GetLocalPlayer() && PodiumWinnerWidgetClass)
	{
		PodiumWinnerWidget = CreateWidget<UPodiumWinnerWidget>(
			this,
			PodiumWinnerWidgetClass);
		if (PodiumWinnerWidget && !PodiumWinnerWidget->IsInViewport())
		{
			PodiumWinnerWidget->AddToViewport(100);
		}
	}

}

void APodiumPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PodiumWinnerWidget)
	{
		PodiumWinnerWidget->RemoveFromParent();
		PodiumWinnerWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void APodiumPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ApplyPodiumPawnPresentation(InPawn);
}

void APodiumPlayerController::ClientSetPodiumWinner_Implementation(
	ESnowRumbleTeam WinningTeam,
	const FText& Subtitle)
{
	if (!IsLocalController())
	{
		return;
	}

	if (!GetLocalPlayer())
	{
		return;
	}

	if (!PodiumWinnerWidget && PodiumWinnerWidgetClass)
	{
		PodiumWinnerWidget = CreateWidget<UPodiumWinnerWidget>(
			this,
			PodiumWinnerWidgetClass);
	}

	if (!PodiumWinnerWidget)
	{
		return;
	}

	if (!PodiumWinnerWidget->IsInViewport())
	{
		PodiumWinnerWidget->AddToViewport(100);
	}

	PodiumWinnerWidget->SetWinnerPresentation(
		WinningTeam,
		Subtitle);
}

void APodiumPlayerController::ClientUpdatePodiumReturnSubtitle_Implementation(
	const FText& Subtitle)
{
	if (!IsLocalController())
	{
		return;
	}

	if (!PodiumWinnerWidget)
	{
		return;
	}

	PodiumWinnerWidget->SetSubtitleText(Subtitle);
}

void APodiumPlayerController::ClientPlayBackgroundMusic_Implementation(
	const FSoftObjectPath& BackgroundMusicPath)
{
	if (!IsLocalController())
	{
		return;
	}

	if (!BackgroundMusicPath.IsValid())
	{
		return;
	}

	USoundBase* LoadedBackgroundMusic =
		Cast<USoundBase>(BackgroundMusicPath.TryLoad());
	if (LoadedBackgroundMusic)
	{
		PlayBackgroundMusic(LoadedBackgroundMusic);
	}
}

void APodiumPlayerController::ClientStopBackgroundMusic_Implementation()
{
	if (!IsLocalController())
	{
		return;
	}

	StopBackgroundMusic();
}

void APodiumPlayerController::ClientRefreshPodiumPawnPresentation_Implementation()
{
	if (!IsLocalController())
	{
		return;
	}

	ApplyPodiumPawnPresentation(GetPawn());
}

void APodiumPlayerController::SetBackgroundMusicPreviewVolume(
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

void APodiumPlayerController::PlayBackgroundMusic(USoundBase* Music)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USnowRumbleBackgroundMusicSubsystem* BackgroundMusicSubsystem =
			GameInstance->GetSubsystem<USnowRumbleBackgroundMusicSubsystem>())
		{
			BackgroundMusicSubsystem->PlayBackgroundMusic(Music, false);
		}
	}
}

void APodiumPlayerController::StopBackgroundMusic()
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

void APodiumPlayerController::ApplyPodiumPawnPresentation(APawn* InPawn)
{
	SetCinematicMode(true, false, true, true, true);
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);

	if (!InPawn)
	{
		return;
	}

	DisableActorShadowCasting(InPawn);
	InPawn->DisableInput(this);

	if (ACharacter* PodiumCharacter = Cast<ACharacter>(InPawn))
	{
		if (UCharacterMovementComponent* MovementComponent =
			PodiumCharacter->GetCharacterMovement())
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->SetMovementMode(MOVE_None);
			MovementComponent->GravityScale = 0.0f;
		}
	}
}

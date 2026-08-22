// Copyright Epic Games, Inc. All Rights Reserved.

#include "PodiumPlayerController.h"

#include "../Audio/SnowRumbleAudioHelpers.h"
#include "../UI/LoadingScreenSubsystem.h"
#include "../UI/PodiumWidget.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

APodiumPlayerController::APodiumPlayerController()
{
}

void APodiumPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
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

	SetCinematicMode(true, false, true, true, true);

	if (APawn* P = GetPawn())
	{
		P->DisableInput(this);
	}

	if (IsLocalController() && PodiumWidgetClass)
	{
		PodiumWidget = CreateWidget<UPodiumWidget>(this, PodiumWidgetClass);
		if (PodiumWidget && !PodiumWidget->IsInViewport())
		{
			PodiumWidget->AddToViewport(100);
		}
	}

	if (IsLocalController())
	{
		PlayBackgroundMusic(BackgroundMusicSound);
	}
}

void APodiumPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PodiumWidget)
	{
		PodiumWidget->RemoveFromParent();
		PodiumWidget = nullptr;
	}

	if (BackgroundMusicComponent.IsValid())
	{
		BackgroundMusicComponent->Stop();
	}
	BackgroundMusicComponent.Reset();

	Super::EndPlay(EndPlayReason);
}

void APodiumPlayerController::ClientSetPodiumResults_Implementation(
	const FText& FirstPlace,
	const FText& SecondPlace,
	const FText& ThirdPlace,
	const FText& Subtitle)
{
	if (!IsLocalController())
	{
		return;
	}

	if (!PodiumWidget && PodiumWidgetClass)
	{
		PodiumWidget = CreateWidget<UPodiumWidget>(this, PodiumWidgetClass);
	}

	if (!PodiumWidget)
	{
		return;
	}

	if (!PodiumWidget->IsInViewport())
	{
		PodiumWidget->AddToViewport(100);
	}

	PodiumWidget->SetPodiumNames(FirstPlace, SecondPlace, ThirdPlace);
	PodiumWidget->SetSubtitle(Subtitle);
}

void APodiumPlayerController::ClientPlayBackgroundMusic_Implementation(
	USoundBase* NewBackgroundMusicSound)
{
	if (!IsLocalController())
	{
		return;
	}

	PlayBackgroundMusic(NewBackgroundMusicSound);
}

void APodiumPlayerController::ClientStopBackgroundMusic_Implementation()
{
	if (!IsLocalController())
	{
		return;
	}

	if (BackgroundMusicComponent.IsValid())
	{
		BackgroundMusicComponent->Stop();
	}
	BackgroundMusicComponent.Reset();
}

void APodiumPlayerController::PlayBackgroundMusic(USoundBase* Music)
{
	if (BackgroundMusicComponent.IsValid())
	{
		BackgroundMusicComponent->Stop();
	}

	if (!Music)
	{
		BackgroundMusicComponent.Reset();
		return;
	}

	BackgroundMusicComponent = SnowRumbleAudio::SpawnSound2D(
		this,
		Music,
		ESnowRumbleAudioMixChannel::BackgroundMusic,
		1.0f,
		1.0f,
		true);
}

void APodiumPlayerController::StopBackgroundMusic()
{
	if (BackgroundMusicComponent.IsValid())
	{
		BackgroundMusicComponent->Stop();
	}
	BackgroundMusicComponent.Reset();
}

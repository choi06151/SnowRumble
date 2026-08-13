// Copyright Epic Games, Inc. All Rights Reserved.

#include "PodiumPlayerController.h"

#include "../UI/PodiumWidget.h"
#include "EngineUtils.h"
#include "Camera/CameraActor.h"

APodiumPlayerController::APodiumPlayerController()
{
}

void APodiumPlayerController::BeginPlay()
{
	Super::BeginPlay();

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
}

void APodiumPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PodiumWidget)
	{
		PodiumWidget->RemoveFromParent();
		PodiumWidget = nullptr;
	}

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

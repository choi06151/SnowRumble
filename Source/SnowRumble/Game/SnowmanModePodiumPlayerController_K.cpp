// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowmanModePodiumPlayerController_K.h"

#include "Camera/CameraActor.h"
#include "EngineUtils.h"
#include "../UI/PodiumWinnerWidget.h"

ASnowmanModePodiumPlayerController::ASnowmanModePodiumPlayerController()
{
}

void ASnowmanModePodiumPlayerController::BeginPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("=== SNOWMAN PODIUM CONTROLLER BEGIN PLAY ACTIVE ==="));
    
	// 부모의 BeginPlay가 돌면서 기존 설정된 위젯(WBP_PodiumWinnerWidget_Snowman_K)을 자동으로 생성합니다.
	Super::BeginPlay();
}

void ASnowmanModePodiumPlayerController
	::ClientSetSnowmanPodiumResult_Implementation(
	   const FText& ResultText,
	   const FText& SubtitleText)
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

	// 부모가 기본으로 생성해 둔 위젯(PodiumWinnerWidget)을 캐스팅해서 사용합니다.
	if (UPodiumWinnerWidget* WinnerWidget = Cast<UPodiumWinnerWidget>(PodiumWinnerWidget))
	{
		WinnerWidget->SetWinnerPresentationText(ResultText, SubtitleText);
       
		if (!WinnerWidget->IsInViewport())
		{
			WinnerWidget->AddToViewport(120);
		}
	}
}

void ASnowmanModePodiumPlayerController::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	if (SnowmanPodiumResultWidget)
	{
		SnowmanPodiumResultWidget->RemoveFromParent();
		SnowmanPodiumResultWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void ASnowmanModePodiumPlayerController
	::ClientUpdateSnowmanPodiumSubtitle_Implementation(
	   const FText& SubtitleText)
{
	if (!IsLocalController())
	{
		return;
	}

	if (UPodiumWinnerWidget* WinnerWidget = Cast<UPodiumWinnerWidget>(PodiumWinnerWidget))
	{
		WinnerWidget->SetSubtitleText(SubtitleText);
	}
}

void ASnowmanModePodiumPlayerController
	::ClientSetSnowmanPodiumCamera_Implementation()
{
	if (!IsLocalController())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ACameraActor> It(World); It; ++It)
	{
		ACameraActor* Camera = *It;
		if (Camera
			&& (Camera->ActorHasTag(TEXT("Podium_Camera"))
				|| Camera->GetName().Contains(TEXT("Podium_Camera"))))
		{
			SetViewTargetWithBlend(Camera, 0.5f);
			return;
		}
	}
}

void ASnowmanModePodiumPlayerController::EnsureSnowmanPodiumResultWidget()
{
	if (!GetLocalPlayer())
	{
		return;
	}

	if (!SnowmanPodiumResultWidget)
	{
		if (!SnowmanPodiumWinnerWidgetClass)
		{
			return;
		}

		SnowmanPodiumResultWidget =
			CreateWidget<UPodiumWinnerWidget>(
				this,
				SnowmanPodiumWinnerWidgetClass);
	}

	if (SnowmanPodiumResultWidget
		&& !SnowmanPodiumResultWidget->IsInViewport())
	{
		SnowmanPodiumResultWidget->AddToViewport(120);
	}
}

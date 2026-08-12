// Copyright Epic Games, Inc. All Rights Reserved.

#include "LoadingScreenSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "LoadingScreenWidget.h"

void ULoadingScreenSubsystem::ShowLoadingScreen(
	TSubclassOf<ULoadingScreenWidget> WidgetClass)
{
	if (!WidgetClass)
	{
		WidgetClass = ULoadingScreenWidget::StaticClass();
	}

	if (!LoadingScreenWidget
		|| LoadingScreenWidget->GetClass() != WidgetClass)
	{
		if (LoadingScreenWidget)
		{
			LoadingScreenWidget->RemoveFromParent();
		}

		LoadingScreenWidget =
			CreateWidget<ULoadingScreenWidget>(GetGameInstance(), WidgetClass);
	}

	if (LoadingScreenWidget && !LoadingScreenWidget->IsInViewport())
	{
		LoadingScreenWidget->AddToViewport(10000);
	}
}

void ULoadingScreenSubsystem::HideLoadingScreen()
{
	if (LoadingScreenWidget)
	{
		LoadingScreenWidget->RemoveFromParent();
	}
}

bool ULoadingScreenSubsystem::IsLoadingScreenVisible() const
{
	return LoadingScreenWidget && LoadingScreenWidget->IsInViewport();
}

void ULoadingScreenSubsystem::SetLoadingProgress(
	int32 LoadedPlayers,
	int32 ExpectedPlayers)
{
	ExpectedPlayerCount = FMath::Max(0, ExpectedPlayers);
	LoadedPlayerCount = FMath::Clamp(
		LoadedPlayers,
		0,
		ExpectedPlayerCount > 0 ? ExpectedPlayerCount : LoadedPlayers);
}

float ULoadingScreenSubsystem::GetLoadingProgress() const
{
	return ExpectedPlayerCount > 0
		? static_cast<float>(LoadedPlayerCount)
			/ static_cast<float>(ExpectedPlayerCount)
		: 0.0f;
}

int32 ULoadingScreenSubsystem::GetLoadedPlayerCount() const
{
	return LoadedPlayerCount;
}

int32 ULoadingScreenSubsystem::GetExpectedPlayerCount() const
{
	return ExpectedPlayerCount;
}

FText ULoadingScreenSubsystem::GetLoadingStatusText() const
{
	return FText::Format(
		NSLOCTEXT("SnowRumble", "LoadingPlayerCountFormat", "{0} / {1}"),
		FText::AsNumber(LoadedPlayerCount),
		FText::AsNumber(ExpectedPlayerCount));
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "LoadingScreenSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "LoadingScreenWidget.h"
#include "MoviePlayer.h"
#include "SlateOptMacros.h"
#include "Subsystems/SubsystemCollection.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
class STravelLoadingScreen final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STravelLoadingScreen)
	{
	}
	SLATE_END_ARGS()

	BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
	void Construct(const FArguments& InArgs)
	{
		StartTimeSeconds = FPlatformTime::Seconds();

		ChildSlot
		[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0.015f, 0.02f, 0.028f, 1.0f))
			.Padding(56.0f)
			[
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.ColorAndOpacity(FSlateColor(FLinearColor::White))
						.Text(NSLOCTEXT("SnowRumble", "TravelLoadingTitle", "Loading PvP Match"))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 18.0f, 0.0f, 0.0f)
					[
						SNew(SBox)
						.WidthOverride(520.0f)
						.HeightOverride(18.0f)
						[
							SNew(SProgressBar)
							.Percent(this, &STravelLoadingScreen::GetProgressPercent)
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 12.0f, 0.0f, 0.0f)
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.ColorAndOpacity(FSlateColor(FLinearColor(0.78f, 0.84f, 0.92f, 1.0f)))
						.Text(NSLOCTEXT("SnowRumble", "TravelLoadingMessage", "Preparing arena..."))
					]
				]
			]
		];

		RegisterActiveTimer(
			0.0f,
			FWidgetActiveTimerDelegate::CreateSP(
				this,
				&STravelLoadingScreen::TickLoadingProgress));
	}
	END_SLATE_FUNCTION_BUILD_OPTIMIZATION

private:
	EActiveTimerReturnType TickLoadingProgress(
		double InCurrentTime,
		float InDeltaTime)
	{
		return EActiveTimerReturnType::Continue;
	}

	TOptional<float> GetProgressPercent() const
	{
		const double ElapsedSeconds = FPlatformTime::Seconds() - StartTimeSeconds;
		const float FastFill = FMath::Clamp(
			static_cast<float>(ElapsedSeconds / 1.8),
			0.0f,
			1.0f);
		const float SlowFill = FMath::Clamp(
			static_cast<float>((ElapsedSeconds - 1.8) / 4.0),
			0.0f,
			1.0f);

		return FMath::Clamp(
			(FastFill * 0.72f) + (SlowFill * 0.18f),
			0.0f,
			0.9f);
	}

	double StartTimeSeconds = 0.0;
};
}

void ULoadingScreenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this,
		&ULoadingScreenSubsystem::HandlePostLoadMapWithWorld);
}

void ULoadingScreenSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	StopMoviePlayerLoadingScreen();

	if (LoadingScreenWidget)
	{
		LoadingScreenWidget->RemoveFromParent();
		LoadingScreenWidget = nullptr;
	}

	Super::Deinitialize();
}

void ULoadingScreenSubsystem::ShowLoadingScreen(
	TSubclassOf<ULoadingScreenWidget> WidgetClass)
{
	if (!WidgetClass)
	{
		WidgetClass = ULoadingScreenWidget::StaticClass();
	}

	bLoadingScreenRequested = true;
	LoadingScreenWidgetClass = WidgetClass;

	StartMoviePlayerLoadingScreen();
	EnsureLoadingScreenWidget();
	AddLoadingScreenWidgetToViewport();
}

void ULoadingScreenSubsystem::HideLoadingScreen()
{
	bLoadingScreenRequested = false;
	StopMoviePlayerLoadingScreen();

	if (LoadingScreenWidget)
	{
		LoadingScreenWidget->RemoveFromParent();
	}
}

bool ULoadingScreenSubsystem::IsLoadingScreenVisible() const
{
	return bMoviePlayerLoadingScreenActive
		|| (LoadingScreenWidget && LoadingScreenWidget->IsInViewport());
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

void ULoadingScreenSubsystem::EnsureLoadingScreenWidget()
{
	if (!LoadingScreenWidgetClass)
	{
		LoadingScreenWidgetClass = ULoadingScreenWidget::StaticClass();
	}

	if (!LoadingScreenWidget
		|| LoadingScreenWidget->GetClass() != LoadingScreenWidgetClass)
	{
		if (LoadingScreenWidget)
		{
			LoadingScreenWidget->RemoveFromParent();
		}

		LoadingScreenWidget = CreateWidget<ULoadingScreenWidget>(
			GetGameInstance(),
			LoadingScreenWidgetClass);
	}
}

void ULoadingScreenSubsystem::AddLoadingScreenWidgetToViewport()
{
	if (!bLoadingScreenRequested)
	{
		return;
	}

	EnsureLoadingScreenWidget();

	if (LoadingScreenWidget && !LoadingScreenWidget->IsInViewport())
	{
		LoadingScreenWidget->AddToViewport(10000);
	}
}

void ULoadingScreenSubsystem::StartMoviePlayerLoadingScreen()
{
	if (bMoviePlayerLoadingScreenActive || !IsMoviePlayerEnabled())
	{
		return;
	}

	FLoadingScreenAttributes LoadingScreenAttributes;
	LoadingScreenAttributes.bAutoCompleteWhenLoadingCompletes = false;
	LoadingScreenAttributes.bMoviesAreSkippable = false;
	LoadingScreenAttributes.bWaitForManualStop = true;
	LoadingScreenAttributes.MinimumLoadingScreenDisplayTime = 0.0f;
	LoadingScreenAttributes.WidgetLoadingScreen = SNew(STravelLoadingScreen);

	GetMoviePlayer()->SetupLoadingScreen(LoadingScreenAttributes);
	GetMoviePlayer()->PlayMovie();
	bMoviePlayerLoadingScreenActive = true;
}

void ULoadingScreenSubsystem::StopMoviePlayerLoadingScreen()
{
	if (!bMoviePlayerLoadingScreenActive || !IsMoviePlayerEnabled())
	{
		bMoviePlayerLoadingScreenActive = false;
		return;
	}

	GetMoviePlayer()->StopMovie();
	bMoviePlayerLoadingScreenActive = false;
}

void ULoadingScreenSubsystem::HandlePostLoadMapWithWorld(UWorld* LoadedWorld)
{
	if (!bLoadingScreenRequested)
	{
		return;
	}

	if (LoadingScreenWidget)
	{
		LoadingScreenWidget->RemoveFromParent();
	}

	AddLoadingScreenWidgetToViewport();
}

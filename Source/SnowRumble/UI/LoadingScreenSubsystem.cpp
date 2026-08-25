// Copyright Epic Games, Inc. All Rights Reserved.

#include "LoadingScreenSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "LoadingScreenWidget.h"
#include "MoviePlayer.h"
#include "SlateOptMacros.h"
#include "Subsystems/SubsystemCollection.h"
#include "Engine/Texture2D.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
class STravelLoadingScreen final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STravelLoadingScreen)
		: _MapDisplayName(FText::GetEmpty())
		, _MapLoadingImage(nullptr)
	{
	}
		SLATE_ARGUMENT(FText, MapDisplayName)
		SLATE_ARGUMENT(UTexture2D*, MapLoadingImage)
		SLATE_ARGUMENT(TArray<FString>, TeamPlayerNames)
	SLATE_END_ARGS()

	BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
	void Construct(const FArguments& InArgs)
	{
		MapDisplayName = InArgs._MapDisplayName;
		TeamPlayerNames = InArgs._TeamPlayerNames;
		if (UTexture2D* MapLoadingImage = InArgs._MapLoadingImage)
		{
			MapImageBrush.SetResourceObject(MapLoadingImage);
			MapImageBrush.ImageSize = FVector2D(420.0f, 236.0f);
		}

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
					SNew(SOverlay)
					+ SOverlay::Slot()
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
							.Text(this, &STravelLoadingScreen::GetTitleText)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 18.0f, 0.0f, 0.0f)
						.HAlign(HAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(420.0f)
							.HeightOverride(236.0f)
							.Visibility(this, &STravelLoadingScreen::GetMapImageVisibility)
							[
								SNew(SImage)
								.Image(&MapImageBrush)
							]
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
					+ SOverlay::Slot()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Bottom)
					[
						SNew(SBox)
						.WidthOverride(260.0f)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(STextBlock)
								.ColorAndOpacity(FSlateColor(FLinearColor(0.78f, 0.84f, 0.92f, 1.0f)))
								.Text(NSLOCTEXT("SnowRumble", "TravelLoadingTeamTitle", "Team"))
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.0f, 8.0f, 0.0f, 0.0f)
							[
								BuildTeamPlayerList()
							]
						]
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
	TSharedRef<SWidget> BuildTeamPlayerList() const
	{
		TSharedRef<SVerticalBox> TeamList = SNew(SVerticalBox);
		if (TeamPlayerNames.IsEmpty())
		{
			TeamList->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
				.Text(NSLOCTEXT("SnowRumble", "TravelLoadingTeamFallback", "Player"))
			];
			return TeamList;
		}

		for (const FString& TeamPlayerName : TeamPlayerNames)
		{
			TeamList->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 3.0f)
			[
				SNew(STextBlock)
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
				.Text(FText::FromString(TeamPlayerName))
			];
		}
		return TeamList;
	}

	FText GetTitleText() const
	{
		if (MapDisplayName.IsEmpty())
		{
			return NSLOCTEXT(
				"SnowRumble",
				"TravelLoadingTitle",
				"Loading PvP Match");
		}

		return FText::Format(
			NSLOCTEXT("SnowRumble", "TravelLoadingMapTitle", "{0}"),
			MapDisplayName);
	}

	EVisibility GetMapImageVisibility() const
	{
		return MapImageBrush.GetResourceObject()
			? EVisibility::Visible
			: EVisibility::Collapsed;
	}

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
	FSlateBrush MapImageBrush;
	FText MapDisplayName;
	TArray<FString> TeamPlayerNames;
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

void ULoadingScreenSubsystem::SetLoadingPresentation(
	const FString& MapPackageName,
	const FText& MapDisplayName,
	TSoftObjectPtr<UTexture2D> MapLoadingImage,
	const TArray<FString>& TeamPlayerNames)
{
	LoadingMapPackageName = MapPackageName;
	LoadingMapDisplayName = MapDisplayName;
	LoadingTeamPlayerNames = TeamPlayerNames;
	LoadedMapLoadingImage = MapLoadingImage.IsNull()
		? nullptr
		: MapLoadingImage.LoadSynchronous();
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

FString ULoadingScreenSubsystem::GetLoadingMapPackageName() const
{
	return LoadingMapPackageName;
}

FText ULoadingScreenSubsystem::GetLoadingMapDisplayName() const
{
	return LoadingMapDisplayName;
}

UTexture2D* ULoadingScreenSubsystem::GetLoadingMapImage() const
{
	return LoadedMapLoadingImage;
}

TArray<FString> ULoadingScreenSubsystem::GetLoadingTeamPlayerNames() const
{
	return LoadingTeamPlayerNames;
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
	LoadingScreenAttributes.WidgetLoadingScreen =
		SNew(STravelLoadingScreen)
		.MapDisplayName(LoadingMapDisplayName)
		.MapLoadingImage(LoadedMapLoadingImage)
		.TeamPlayerNames(LoadingTeamPlayerNames);

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

	// 이전 전환에서 남아 있을 수 있는 MoviePlayer를 정리하고 WBP만 유지한다.
	StopMoviePlayerLoadingScreen();

	if (LoadingScreenWidget)
	{
		LoadingScreenWidget->RemoveFromParent();
	}

	AddLoadingScreenWidgetToViewport();
}

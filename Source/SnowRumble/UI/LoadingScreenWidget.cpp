// Copyright Epic Games, Inc. All Rights Reserved.

#include "LoadingScreenWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Math/UnrealMathUtility.h"
#include "LoadingScreenSubsystem.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void ULoadingScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	DisplayedLoadingProgress = GetTargetLoadingProgress();
	RefreshLoadingPresentation();
}

void ULoadingScreenWidget::NativeTick(
	const FGeometry& MyGeometry,
	float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	DisplayedLoadingProgress = FMath::FInterpTo(
		DisplayedLoadingProgress,
		GetTargetLoadingProgress(),
		InDeltaTime,
		LoadingProgressInterpSpeed);
	RefreshLoadingPresentation();
}

float ULoadingScreenWidget::GetLoadingProgress() const
{
	return DisplayedLoadingProgress;
}

float ULoadingScreenWidget::GetTargetLoadingProgress() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const ULoadingScreenSubsystem* LoadingScreenSubsystem = GameInstance
		? GameInstance->GetSubsystem<ULoadingScreenSubsystem>()
		: nullptr;
	return LoadingScreenSubsystem
		? LoadingScreenSubsystem->GetLoadingProgress()
		: 0.0f;
}

FText ULoadingScreenWidget::GetLoadingStatusText() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const ULoadingScreenSubsystem* LoadingScreenSubsystem = GameInstance
		? GameInstance->GetSubsystem<ULoadingScreenSubsystem>()
		: nullptr;
	return LoadingScreenSubsystem
		? LoadingScreenSubsystem->GetLoadingStatusText()
		: FText::FromString(TEXT("0 / 0"));
}

FText ULoadingScreenWidget::GetLoadingMessageText() const
{
	return NSLOCTEXT(
		"SnowRumble",
		"LoadingPlayerJoinMessage",
		"플레이어 접속 중...");
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
TSharedRef<SWidget> ULoadingScreenWidget::RebuildWidget()
{
	if (WidgetTree && WidgetTree->RootWidget)
	{
		return Super::RebuildWidget();
	}

	return SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.02f, 0.03f, 0.04f, 1.0f))
		.Padding(48.0f)
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
					.Text(NSLOCTEXT("SnowRumble", "DefaultLoadingScreenText", "Loading..."))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 12.0f, 0.0f, 0.0f)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
					.ColorAndOpacity(FSlateColor(FLinearColor::White))
					.Text(GetLoadingStatusText())
				]
			]
		];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void ULoadingScreenWidget::RefreshLoadingPresentation()
{
	if (LoadingProgressBar)
	{
		LoadingProgressBar->SetPercent(GetLoadingProgress());
	}
	if (LoadingStatusText)
	{
		LoadingStatusText->SetText(GetLoadingStatusText());
	}
	if (LoadingMessageText)
	{
		LoadingMessageText->SetText(GetLoadingMessageText());
	}
}

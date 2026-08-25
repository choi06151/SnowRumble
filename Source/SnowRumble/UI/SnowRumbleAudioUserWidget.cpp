// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleAudioUserWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/TextLocalizationManager.h"
#include "SnowRumble/Audio/SnowRumbleAudioHelpers.h"
#include "Sound/SoundBase.h"

void USnowRumbleAudioUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!WidgetTree)
	{
		return;
	}

	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		if (UButton* Button = Cast<UButton>(Widget))
		{
			Button->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleButtonClicked);
			Button->OnHovered.AddUniqueDynamic(this, &ThisClass::HandleButtonHovered);
		}
	});

	if (!TextRevisionChangedHandle.IsValid())
	{
		TextRevisionChangedHandle = FTextLocalizationManager::Get().OnTextRevisionChangedEvent.AddUObject(
			this,
			&ThisClass::HandleTextRevisionChanged);
	}

	ApplyLocalizedFontSize();
}

void USnowRumbleAudioUserWidget::NativeDestruct()
{
	if (TextRevisionChangedHandle.IsValid())
	{
		FTextLocalizationManager::Get().OnTextRevisionChangedEvent.Remove(TextRevisionChangedHandle);
		TextRevisionChangedHandle.Reset();
	}

	Super::NativeDestruct();
}

void USnowRumbleAudioUserWidget::HandleButtonClicked()
{
	SnowRumbleAudio::PlaySound2D(
		this,
		ButtonClickSound,
		ESnowRumbleAudioMixChannel::UserInterface);
}

void USnowRumbleAudioUserWidget::HandleButtonHovered()
{
	SnowRumbleAudio::PlaySound2D(
		this,
		ButtonHoverSound,
		ESnowRumbleAudioMixChannel::UserInterface);
}

void USnowRumbleAudioUserWidget::ApplyLocalizedFontSize()
{
	if (!WidgetTree)
	{
		return;
	}

	const bool bUseReducedEnglishSize = IsEnglishLanguage() && EnglishFontSizeReduction > 0;

	WidgetTree->ForEachWidget([this, bUseReducedEnglishSize](UWidget* Widget)
	{
		UTextBlock* TextBlock = Cast<UTextBlock>(Widget);
		if (!TextBlock)
		{
			return;
		}

		FSlateFontInfo FontInfo = TextBlock->GetFont();
		const TWeakObjectPtr<UTextBlock> TextBlockKey(TextBlock);

		int32* OriginalSize = OriginalTextBlockFontSizes.Find(TextBlockKey);
		if (!OriginalSize)
		{
			OriginalSize = &OriginalTextBlockFontSizes.Add(TextBlockKey, FontInfo.Size);
		}

		const int32 TargetSize = bUseReducedEnglishSize
			? FMath::Max(1, *OriginalSize - EnglishFontSizeReduction)
			: *OriginalSize;

		if (FontInfo.Size != TargetSize)
		{
			FontInfo.Size = TargetSize;
			TextBlock->SetFont(FontInfo);
		}
	});

	InvalidateLayoutAndVolatility();
	ForceLayoutPrepass();
}

void USnowRumbleAudioUserWidget::HandleTextRevisionChanged()
{
	ApplyLocalizedFontSize();
}

bool USnowRumbleAudioUserWidget::IsEnglishLanguage() const
{
	const TSharedPtr<FCulture> CurrentLanguage = FInternationalization::Get().GetCurrentLanguage();
	return CurrentLanguage.IsValid() && CurrentLanguage->GetName().StartsWith(TEXT("en"));
}

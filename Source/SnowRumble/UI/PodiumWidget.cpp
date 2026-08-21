// Copyright Epic Games, Inc. All Rights Reserved.

#include "PodiumWidget.h"
#include "Components/TextBlock.h"

void UPodiumWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 위젯 생성 전에 들어온 결과 문구를 바인딩된 TextBlock에 다시 적용한다.
	if (FirstPlaceText && !CachedFirst.IsEmpty())
	{
		FirstPlaceText->SetText(CachedFirst);
	}
	if (SecondPlaceText && !CachedSecond.IsEmpty())
	{
		SecondPlaceText->SetText(CachedSecond);
	}
	if (ThirdPlaceText && !CachedThird.IsEmpty())
	{
		ThirdPlaceText->SetText(CachedThird);
	}
	if (SubtitleText && !CachedSubtitle.IsEmpty())
	{
		SubtitleText->SetText(CachedSubtitle);
	}
}

void UPodiumWidget::SetPodiumNames(const FText& FirstPlace, const FText& SecondPlace, const FText& ThirdPlace)
{
	CachedFirst = FirstPlace;
	CachedSecond = SecondPlace;
	CachedThird = ThirdPlace;

	if (FirstPlaceText)
	{
		FirstPlaceText->SetText(CachedFirst);
	}
	if (SecondPlaceText)
	{
		SecondPlaceText->SetText(CachedSecond);
	}
	if (ThirdPlaceText)
	{
		ThirdPlaceText->SetText(CachedThird);
	}

	OnPodiumUpdated();
}

void UPodiumWidget::SetSubtitle(const FText& Subtitle)
{
	CachedSubtitle = Subtitle;
	if (SubtitleText)
	{
		SubtitleText->SetText(CachedSubtitle);
	}
	OnPodiumUpdated();
}

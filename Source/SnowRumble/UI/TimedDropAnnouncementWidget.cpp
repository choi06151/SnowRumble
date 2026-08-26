// Copyright Epic Games, Inc. All Rights Reserved.

#include "TimedDropAnnouncementWidget.h"

void UTimedDropAnnouncementWidget::StartAnnouncementAnimation()
{
	if (FadeInOutAnimation)
	{
		PlayAnimation(
			FadeInOutAnimation,
			0.0f,
			1,
			EUMGSequencePlayMode::Forward,
			1.0f);
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "DamageTextWidget_C.h"

#include "Components/TextBlock.h"

void UDamageTextWidget::InitializeDamageText(
	float AppliedDamage,
	ESnowRumbleDamageTextType DamageTextType)
{
	if (DamageText)
	{
		const int32 RoundedDamage = FMath::RoundToInt(AppliedDamage);
		DamageText->SetText(FText::AsNumber(RoundedDamage));
	}

	OnDamageTextInitialized(AppliedDamage, DamageTextType);
}

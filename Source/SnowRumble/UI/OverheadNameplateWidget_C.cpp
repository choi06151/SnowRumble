// Copyright Epic Games, Inc. All Rights Reserved.

#include "OverheadNameplateWidget_C.h"

#include "../Player/SnowRumbleCharacter.h"
#include "Components/TextBlock.h"

void UOverheadNameplateWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	RefreshNameText();
}

void UOverheadNameplateWidget::NativeTick(
	const FGeometry& MyGeometry,
	float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const FString CurrentDisplayName = GetDisplayName();
	if (CurrentDisplayName != LastDisplayName)
	{
		RefreshNameText();
	}
}

void UOverheadNameplateWidget::SetObservedCharacter(
	ASnowRumbleCharacter* NewCharacter)
{
	ObservedCharacter = NewCharacter;
	RefreshNameText();
}

ASnowRumbleCharacter* UOverheadNameplateWidget::GetObservedCharacter() const
{
	return ObservedCharacter;
}

FString UOverheadNameplateWidget::GetDisplayName() const
{
	return ObservedCharacter
		? ObservedCharacter->GetOverheadPlayerName()
		: FString();
}

void UOverheadNameplateWidget::RefreshNameText()
{
	LastDisplayName = GetDisplayName();
	if (PlayerNameTextBlock)
	{
		PlayerNameTextBlock->SetText(FText::FromString(LastDisplayName));
	}
}

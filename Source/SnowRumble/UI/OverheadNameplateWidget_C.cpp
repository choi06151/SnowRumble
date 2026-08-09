// Copyright Epic Games, Inc. All Rights Reserved.

#include "OverheadNameplateWidget_C.h"

#include "../Game/SnowRumbleLobbyGameState.h"
#include "../Game/SnowRumblePlayerState.h"
#include "../Player/SnowRumbleCharacter.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"

void UOverheadNameplateWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	RefreshNameplatePresentation();
}

void UOverheadNameplateWidget::NativeTick(
	const FGeometry& MyGeometry,
	float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const FString CurrentDisplayName = GetDisplayName();
	const FLinearColor CurrentTeamColor = GetTeamColor();
	const bool bCurrentShowReadyImage = ShouldShowReadyImage();
	const bool bCurrentShowHostImage = ShouldShowHostImage();
	if (CurrentDisplayName != LastDisplayName
		|| CurrentTeamColor != LastTeamColor
		|| bCurrentShowReadyImage != bLastShowReadyImage
		|| bCurrentShowHostImage != bLastShowHostImage)
	{
		RefreshNameplatePresentation();
	}
}

void UOverheadNameplateWidget::SetObservedCharacter(
	ASnowRumbleCharacter* NewCharacter)
{
	ObservedCharacter = NewCharacter;
	RefreshNameplatePresentation();
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

FLinearColor UOverheadNameplateWidget::GetTeamColor() const
{
	return ObservedCharacter
		? ObservedCharacter->GetOverheadTeamColor()
		: FLinearColor::White;
}

bool UOverheadNameplateWidget::ShouldShowReadyImage() const
{
	if (!GetWorld() || !GetWorld()->GetGameState<ASnowRumbleLobbyGameState>())
	{
		return false;
	}

	const ASnowRumblePlayerState* PlayerState = ObservedCharacter
		? ObservedCharacter->GetPlayerState<ASnowRumblePlayerState>()
		: nullptr;
	return PlayerState && !PlayerState->IsLobbyHost()
		&& PlayerState->IsLobbyReady();
}

bool UOverheadNameplateWidget::ShouldShowHostImage() const
{
	if (!GetWorld() || !GetWorld()->GetGameState<ASnowRumbleLobbyGameState>())
	{
		return false;
	}

	const ASnowRumblePlayerState* PlayerState = ObservedCharacter
		? ObservedCharacter->GetPlayerState<ASnowRumblePlayerState>()
		: nullptr;
	return PlayerState && PlayerState->IsLobbyHost();
}

void UOverheadNameplateWidget::RefreshNameplatePresentation()
{
	LastDisplayName = GetDisplayName();
	LastTeamColor = GetTeamColor();
	bLastShowReadyImage = ShouldShowReadyImage();
	bLastShowHostImage = ShouldShowHostImage();
	if (PlayerNameTextBlock)
	{
		PlayerNameTextBlock->SetText(FText::FromString(LastDisplayName));
		PlayerNameTextBlock->SetColorAndOpacity(FSlateColor(LastTeamColor));
	}

	if (PlayerNameBackgroundBorder)
	{
		FLinearColor BackgroundColor = LastTeamColor;
		BackgroundColor.A = 0.35f;
		PlayerNameBackgroundBorder->SetBrushColor(BackgroundColor);
	}

	if (ReadyStateImage)
	{
		ReadyStateImage->SetVisibility(
			bLastShowReadyImage
				? ESlateVisibility::SelfHitTestInvisible
				: ESlateVisibility::Collapsed);
	}

	if (HostStateImage)
	{
		HostStateImage->SetVisibility(
			bLastShowHostImage
				? ESlateVisibility::SelfHitTestInvisible
				: ESlateVisibility::Collapsed);
	}
}

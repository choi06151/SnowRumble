// Copyright Epic Games, Inc. All Rights Reserved.

#include "HealthBarWidget.h"

#include "../Game/SnowmanModeGameState_K.h"
#include "../Game/SnowRumblePlayerState.h"
#include "../Player/SnowRumbleCharacter.h"
#include "../Player/SnowRumbleHealthComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/Actor.h"

void UHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ShouldHideForSnowmanMode())
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (ObservedHealthComponent)
	{
		UpdateHealthPresentation(
			ObservedHealthComponent->GetCurrentHealth(),
			ObservedHealthComponent->GetMaxHealth());
	}
	else
	{
		UpdateHealthPresentation(0.0f, 1.0f);
	}
	UpdateObservedPlayerName();
}

void UHealthBarWidget::NativeDestruct()
{
	UnbindObservedHealthComponent();

	Super::NativeDestruct();
}

void UHealthBarWidget::NativeTick(
	const FGeometry& MyGeometry,
	float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (ShouldHideForSnowmanMode())
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (ObservedHealthComponent)
	{
		UpdateHealthPresentation(
			ObservedHealthComponent->GetCurrentHealth(),
			ObservedHealthComponent->GetMaxHealth());
	}
	UpdateObservedPlayerName();
}

void UHealthBarWidget::SetObservedActor(AActor* NewObservedActor)
{
	ObservedActor = NewObservedActor;
	if (ShouldHideForSnowmanMode())
	{
		SetVisibility(ESlateVisibility::Collapsed);
		SetObservedHealthComponent(nullptr);
		return;
	}

	SetObservedHealthComponent(
		NewObservedActor
			? NewObservedActor->FindComponentByClass<USnowRumbleHealthComponent>()
			: nullptr);
	UpdateObservedPlayerName();
}

void UHealthBarWidget::SetObservedHealthComponent(
	USnowRumbleHealthComponent* NewHealthComponent)
{
	if (ObservedHealthComponent == NewHealthComponent)
	{
		return;
	}

	UnbindObservedHealthComponent();
	ObservedHealthComponent = NewHealthComponent;

	if (!ObservedHealthComponent)
	{
		UpdateHealthPresentation(0.0f, 1.0f);
		return;
	}

	ObservedHealthComponent->OnHealthChanged.AddUniqueDynamic(
		this,
		&UHealthBarWidget::HandleHealthChanged);
	UpdateHealthPresentation(
		ObservedHealthComponent->GetCurrentHealth(),
		ObservedHealthComponent->GetMaxHealth());
}

void UHealthBarWidget::UnbindObservedHealthComponent()
{
	if (ObservedHealthComponent)
	{
		ObservedHealthComponent->OnHealthChanged.RemoveAll(this);
	}
}

bool UHealthBarWidget::ShouldHideForSnowmanMode() const
{
	const UWorld* World = GetWorld();
	return World && World->GetGameState<ASnowmanModeGameState>();
}

void UHealthBarWidget::UpdateHealthPresentation(
	float CurrentHealth,
	float MaxHealth)
{
	if (ShouldHideForSnowmanMode())
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const float SafeMaxHealth = FMath::Max(MaxHealth, 1.0f);
	const float HealthRatio =
		FMath::Clamp(CurrentHealth / SafeMaxHealth, 0.0f, 1.0f);

	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(HealthRatio);
	}

	if (HealthTextBlock)
	{
		HealthTextBlock->SetText(FText::Format(
			NSLOCTEXT("SnowRumble", "HealthBarText", "{0} / {1}"),
			FText::AsNumber(FMath::RoundToInt(CurrentHealth)),
			FText::AsNumber(FMath::RoundToInt(SafeMaxHealth))));
	}
}

void UHealthBarWidget::UpdateObservedPlayerName()
{
	if (!PlayerNameText)
	{
		return;
	}

	const ASnowRumbleCharacter* Character =
		Cast<ASnowRumbleCharacter>(ObservedActor);
	const ASnowRumblePlayerState* PlayerState = Character
		? Character->GetPlayerState<ASnowRumblePlayerState>()
		: nullptr;
	PlayerNameText->SetText(
		PlayerState
			? FText::FromString(PlayerState->GetLobbyPlayerName())
			: FText::GetEmpty());
}

void UHealthBarWidget::HandleHealthChanged(
	float CurrentHealth,
	float MaxHealth)
{
	UpdateHealthPresentation(CurrentHealth, MaxHealth);
}

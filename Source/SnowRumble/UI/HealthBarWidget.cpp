// Copyright Epic Games, Inc. All Rights Reserved.

#include "HealthBarWidget.h"

#include "../Player/SnowRumbleHealthComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/Actor.h"

void UHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

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

	if (ObservedHealthComponent)
	{
		UpdateHealthPresentation(
			ObservedHealthComponent->GetCurrentHealth(),
			ObservedHealthComponent->GetMaxHealth());
	}
}

void UHealthBarWidget::SetObservedActor(AActor* NewObservedActor)
{
	SetObservedHealthComponent(
		NewObservedActor
			? NewObservedActor->FindComponentByClass<USnowRumbleHealthComponent>()
			: nullptr);
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

void UHealthBarWidget::UpdateHealthPresentation(
	float CurrentHealth,
	float MaxHealth)
{
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

void UHealthBarWidget::HandleHealthChanged(
	float CurrentHealth,
	float MaxHealth)
{
	UpdateHealthPresentation(CurrentHealth, MaxHealth);
}

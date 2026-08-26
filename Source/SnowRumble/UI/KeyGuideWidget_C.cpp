// Copyright Epic Games, Inc. All Rights Reserved.

#include "KeyGuideWidget_C.h"

#include "../Player/SnowRumbleUserSettingsSubsystem_C.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"

void UKeyGuideWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
			GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>())
		{
			UserSettingsSubsystem->OnKeyBindingsChanged.AddUObject(
				this,
				&UKeyGuideWidget::RefreshKeyGuideTexts);
		}
	}

	RefreshKeyGuideTexts();
}

void UKeyGuideWidget::NativeDestruct()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
			GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>())
		{
			UserSettingsSubsystem->OnKeyBindingsChanged.RemoveAll(this);
		}
	}

	Super::NativeDestruct();
}

void UKeyGuideWidget::RefreshKeyGuideTexts()
{
	const FText MoveText = FText::Format(
		NSLOCTEXT("SnowRumble", "KeyGuideMoveFormat", "{0}{1}{2}{3}"),
		GetKeyDisplayText(TEXT("MoveForward"), EKeys::W),
		GetKeyDisplayText(TEXT("MoveLeft"), EKeys::A),
		GetKeyDisplayText(TEXT("MoveBackward"), EKeys::S),
		GetKeyDisplayText(TEXT("MoveRight"), EKeys::D));

	SetOptionalText(MoveKeyText, MoveText);
	SetOptionalText(
		SnowCreateKeyText,
		GetKeyDisplayText(TEXT("Action"), EKeys::LeftMouseButton));
	SetOptionalText(
		SnowPickupKeyText,
		GetKeyDisplayText(TEXT("Interact"), EKeys::E));
	SetOptionalText(
		SnowRollKeyText,
		GetKeyDisplayText(TEXT("Interact"), EKeys::E));
	SetOptionalText(
		AimKeyText,
		GetKeyDisplayText(TEXT("Aim"), EKeys::RightMouseButton));
	SetOptionalText(
		ThrowKeyText,
		GetKeyDisplayText(TEXT("Action"), EKeys::LeftMouseButton));
	SetOptionalText(
		EmoteKeyText,
		GetKeyDisplayText(TEXT("Emote"), EKeys::Tab));
	SetOptionalText(
		VoiceMuteKeyText,
		GetKeyDisplayText(TEXT("VoiceTargetMute"), EKeys::M));
	SetOptionalText(
		VoiceChannelKeyText,
		GetKeyDisplayText(TEXT("MicrophoneChannelToggle"), EKeys::N));
	SetOptionalText(
		KeyGuideKeyText,
		GetKeyDisplayText(TEXT("KeyGuide"), EKeys::T));
	SetOptionalText(
		ChatKeyText,
		GetKeyDisplayText(TEXT("Chat"), EKeys::Enter));

	OnKeyGuideTextsRefreshed();
}

FText UKeyGuideWidget::GetKeyDisplayText(
	FName BindingId,
	FKey DefaultKey) const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const USnowRumbleUserSettingsSubsystem* UserSettingsSubsystem =
		GameInstance
			? GameInstance->GetSubsystem<USnowRumbleUserSettingsSubsystem>()
			: nullptr;
	const FKey Key = UserSettingsSubsystem
		? UserSettingsSubsystem->GetKeyBinding(BindingId, DefaultKey)
		: DefaultKey;

	return FormatKeyDisplayText(Key);
}

FText UKeyGuideWidget::FormatKeyDisplayText(FKey Key)
{
	if (!Key.IsValid())
	{
		return NSLOCTEXT("SnowRumble", "KeyGuideUnassigned", "미할당");
	}
	if (Key == EKeys::LeftMouseButton)
	{
		return NSLOCTEXT("SnowRumble", "KeyGuideLeftMouse", "좌클릭");
	}
	if (Key == EKeys::RightMouseButton)
	{
		return NSLOCTEXT("SnowRumble", "KeyGuideRightMouse", "우클릭");
	}
	if (Key == EKeys::MiddleMouseButton)
	{
		return NSLOCTEXT("SnowRumble", "KeyGuideMiddleMouse", "휠클릭");
	}
	if (Key == EKeys::SpaceBar)
	{
		return NSLOCTEXT("SnowRumble", "KeyGuideSpace", "Space");
	}
	if (Key == EKeys::LeftShift || Key == EKeys::RightShift)
	{
		return NSLOCTEXT("SnowRumble", "KeyGuideShift", "Shift");
	}
	if (Key == EKeys::LeftControl || Key == EKeys::RightControl)
	{
		return NSLOCTEXT("SnowRumble", "KeyGuideCtrl", "Ctrl");
	}
	if (Key == EKeys::LeftAlt || Key == EKeys::RightAlt)
	{
		return NSLOCTEXT("SnowRumble", "KeyGuideAlt", "Alt");
	}

	return Key.GetDisplayName();
}

void UKeyGuideWidget::SetOptionalText(
	UTextBlock* TextBlock,
	const FText& Text) const
{
	if (TextBlock)
	{
		TextBlock->SetText(Text);
	}
}

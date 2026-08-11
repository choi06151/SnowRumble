// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleCustomizationSubsystem_C.h"

void USnowRumbleCustomizationSubsystem::SetCustomizationData(
	const FSnowRumbleCustomizationData& NewData)
{
	CustomizationData = SanitizeCustomizationData(NewData);
}

FSnowRumbleCustomizationData
USnowRumbleCustomizationSubsystem::GetCustomizationData() const
{
	return CustomizationData;
}

void USnowRumbleCustomizationSubsystem::SetBodyColor(FLinearColor NewBodyColor)
{
	FSnowRumbleCustomizationData NewData = CustomizationData;
	NewData.BodyColor = NewBodyColor;
	CustomizationData = SanitizeCustomizationData(NewData);
}

FLinearColor USnowRumbleCustomizationSubsystem::GetBodyColor() const
{
	return CustomizationData.BodyColor;
}

void USnowRumbleCustomizationSubsystem::ResetCustomizationData()
{
	CustomizationData = GetDefaultCustomizationData();
}

FSnowRumbleCustomizationData
USnowRumbleCustomizationSubsystem::GetDefaultCustomizationData()
{
	FSnowRumbleCustomizationData DefaultData;
	DefaultData.BodyColor = FLinearColor::White;
	DefaultData.PaintStrokes.Reset();
	DefaultData.bFlipPaintUvY = false;
	return DefaultData;
}

FSnowRumbleCustomizationData
USnowRumbleCustomizationSubsystem::SanitizeCustomizationData(
	const FSnowRumbleCustomizationData& Data)
{
	FSnowRumbleCustomizationData SanitizedData = Data;
	SanitizedData.BodyColor.R = FMath::Clamp(SanitizedData.BodyColor.R, 0.0f, 1.0f);
	SanitizedData.BodyColor.G = FMath::Clamp(SanitizedData.BodyColor.G, 0.0f, 1.0f);
	SanitizedData.BodyColor.B = FMath::Clamp(SanitizedData.BodyColor.B, 0.0f, 1.0f);
	SanitizedData.BodyColor.A = 1.0f;

	constexpr int32 MaxPaintStrokeCount = 64;
	constexpr int32 MaxPaintPointCountPerStroke = 256;
	if (SanitizedData.PaintStrokes.Num() > MaxPaintStrokeCount)
	{
		SanitizedData.PaintStrokes.RemoveAt(
			0,
			SanitizedData.PaintStrokes.Num() - MaxPaintStrokeCount);
	}

	for (FSnowRumblePaintStroke& Stroke : SanitizedData.PaintStrokes)
	{
		if (Stroke.Points.Num() > MaxPaintPointCountPerStroke)
		{
			Stroke.Points.SetNum(MaxPaintPointCountPerStroke);
		}

		for (FVector2D& Point : Stroke.Points)
		{
			Point.X = FMath::Clamp(Point.X, 0.0f, 1.0f);
			Point.Y = FMath::Clamp(Point.Y, 0.0f, 1.0f);
		}
	}
	return SanitizedData;
}

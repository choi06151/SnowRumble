// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SnowRumbleCustomizationData_C.generated.h"

USTRUCT(BlueprintType)
struct SNOWRUMBLE_API FSnowRumblePaintStroke
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnowRumble|Customization")
	FName MeshComponentName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnowRumble|Customization")
	int32 MaterialIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnowRumble|Customization")
	FLinearColor BrushColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnowRumble|Customization")
	float BrushThickness = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnowRumble|Customization")
	TArray<FVector2D> Points;

	bool operator==(const FSnowRumblePaintStroke& Other) const
	{
		return MeshComponentName == Other.MeshComponentName
			&& MaterialIndex == Other.MaterialIndex
			&& BrushColor == Other.BrushColor
			&& BrushThickness == Other.BrushThickness
			&& Points == Other.Points;
	}

	bool operator!=(const FSnowRumblePaintStroke& Other) const
	{
		return !(*this == Other);
	}
};

USTRUCT(BlueprintType)
struct SNOWRUMBLE_API FSnowRumbleCustomizationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnowRumble|Customization")
	FLinearColor BodyColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnowRumble|Customization")
	TArray<FSnowRumblePaintStroke> PaintStrokes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnowRumble|Customization")
	bool bFlipPaintUvY = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnowRumble|Customization")
	int32 HatMeshIndex = INDEX_NONE;

	bool operator==(const FSnowRumbleCustomizationData& Other) const
	{
		return BodyColor == Other.BodyColor
			&& PaintStrokes == Other.PaintStrokes
			&& bFlipPaintUvY == Other.bFlipPaintUvY
			&& HatMeshIndex == Other.HatMeshIndex;
	}

	bool operator!=(const FSnowRumbleCustomizationData& Other) const
	{
		return !(*this == Other);
	}
};

// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowTrailRenderTargetManager_C.h"

#include "../Player/SnowRumbleCharacter.h"
#include "Engine/Canvas.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Components/PrimitiveComponent.h"

ASnowTrailRenderTargetManager::ASnowTrailRenderTargetManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASnowTrailRenderTargetManager::BeginPlay()
{
	Super::BeginPlay();

	InitializeSnowTrailRenderTargetForPlay();
}

ASnowTrailRenderTargetManager*
ASnowTrailRenderTargetManager::FindSnowTrailManager(
	const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject
		? WorldContextObject->GetWorld()
		: nullptr;
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<ASnowTrailRenderTargetManager> It(World); It; ++It)
	{
		return *It;
	}

	return nullptr;
}

UCanvasRenderTarget2D*
ASnowTrailRenderTargetManager::EnsureSnowTrailRenderTarget()
{
	if (SnowTrailRenderTarget)
	{
		BindSnowTrailRenderTargetUpdate();
		return SnowTrailRenderTarget;
	}

	const int32 SafeRenderTargetSize = FMath::Clamp(
		RenderTargetSize,
		64,
		4096);
	SnowTrailRenderTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(
		this,
		UCanvasRenderTarget2D::StaticClass(),
		SafeRenderTargetSize,
		SafeRenderTargetSize);
	if (SnowTrailRenderTarget)
	{
		SnowTrailRenderTarget->ClearColor = FLinearColor::Transparent;
		BindSnowTrailRenderTargetUpdate();
		SnowTrailRenderTarget->UpdateResource();
	}

	return SnowTrailRenderTarget;
}

void ASnowTrailRenderTargetManager::InitializeSnowTrailRenderTargetForPlay()
{
	if (!bAutoCreateRenderTarget && !SnowTrailRenderTarget)
	{
		return;
	}

	UCanvasRenderTarget2D* TargetRenderTarget = EnsureSnowTrailRenderTarget();
	if (!TargetRenderTarget)
	{
		return;
	}

	SnowTrailStamps.Reset();
	LastStampLocationByCharacter.Reset();
	TargetRenderTarget->ClearColor = FLinearColor::Transparent;
	if (bClearRenderTargetOnBeginPlay)
	{
		UKismetRenderingLibrary::ClearRenderTarget2D(
			this,
			TargetRenderTarget,
			TargetRenderTarget->ClearColor);
	}
	TargetRenderTarget->UpdateResource();

	ApplySnowTrailMaterialParameters(TargetRenderTarget);
	OnSnowTrailRenderTargetReady(
		TargetRenderTarget,
		TrailWorldCenter,
		TrailWorldSize);
}

void ASnowTrailRenderTargetManager::BindSnowTrailRenderTargetUpdate()
{
	if (!SnowTrailRenderTarget)
	{
		return;
	}

	SnowTrailRenderTarget->OnCanvasRenderTargetUpdate.AddUniqueDynamic(
		this,
		&ASnowTrailRenderTargetManager::HandleSnowTrailCanvasUpdate);
}

void ASnowTrailRenderTargetManager::ApplySnowTrailMaterialParameters(
	UCanvasRenderTarget2D* TargetRenderTarget)
{
	if (!bApplySnowTrailParametersToMaterials || !TargetRenderTarget)
	{
		return;
	}

	AppliedSnowTrailMaterials.Reset();

	const FLinearColor WorldCenterParameter(
		TrailWorldCenter.X,
		TrailWorldCenter.Y,
		0.0f,
		0.0f);

	auto ApplyToComponent =
		[this, TargetRenderTarget, WorldCenterParameter](
			UPrimitiveComponent* TargetComponent)
	{
		if (!TargetComponent)
		{
			return;
		}

		const int32 MaterialCount = TargetComponent->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
		{
			if (!TargetComponent->GetMaterial(MaterialIndex))
			{
				continue;
			}

			UMaterialInstanceDynamic* DynamicMaterial =
				TargetComponent->CreateDynamicMaterialInstance(MaterialIndex);
			if (!DynamicMaterial)
			{
				continue;
			}

			if (!SnowTrailRenderTargetParameterName.IsNone())
			{
				DynamicMaterial->SetTextureParameterValue(
					SnowTrailRenderTargetParameterName,
					TargetRenderTarget);
			}
			if (!SnowTrailWorldCenterParameterName.IsNone())
			{
				DynamicMaterial->SetVectorParameterValue(
					SnowTrailWorldCenterParameterName,
					WorldCenterParameter);
			}
			if (!SnowTrailWorldSizeParameterName.IsNone())
			{
				DynamicMaterial->SetScalarParameterValue(
					SnowTrailWorldSizeParameterName,
					TrailWorldSize);
			}

			AppliedSnowTrailMaterials.Add(DynamicMaterial);
		}
	};

	for (UPrimitiveComponent* PrimitiveComponent : SnowTrailMaterialComponents)
	{
		ApplyToComponent(PrimitiveComponent);
	}

	for (AActor* MaterialActor : SnowTrailMaterialActors)
	{
		if (!MaterialActor)
		{
			continue;
		}

		TArray<UPrimitiveComponent*> PrimitiveComponents;
		MaterialActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
		for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
		{
			ApplyToComponent(PrimitiveComponent);
		}
	}
}

UCanvasRenderTarget2D*
ASnowTrailRenderTargetManager::GetSnowTrailRenderTarget() const
{
	return SnowTrailRenderTarget;
}

FVector2D ASnowTrailRenderTargetManager::GetTrailWorldCenter() const
{
	return TrailWorldCenter;
}

float ASnowTrailRenderTargetManager::GetTrailWorldSize() const
{
	return TrailWorldSize;
}

bool ASnowTrailRenderTargetManager::ConvertWorldLocationToTrailUV(
	FVector WorldLocation,
	FVector2D& OutTrailUV) const
{
	if (TrailWorldSize <= 0.0f)
	{
		OutTrailUV = FVector2D::ZeroVector;
		return false;
	}

	const FVector2D HalfWorldSize(TrailWorldSize * 0.5f, TrailWorldSize * 0.5f);
	const FVector2D MinimumWorld = TrailWorldCenter - HalfWorldSize;
	OutTrailUV = FVector2D(
		(WorldLocation.X - MinimumWorld.X) / TrailWorldSize,
		(WorldLocation.Y - MinimumWorld.Y) / TrailWorldSize);

	return OutTrailUV.X >= 0.0f
		&& OutTrailUV.X <= 1.0f
		&& OutTrailUV.Y >= 0.0f
		&& OutTrailUV.Y <= 1.0f;
}

bool ASnowTrailRenderTargetManager::StampSnowTrailAtWorldLocation(
	FVector WorldLocation,
	FVector WorldNormal,
	float RadiusWorld,
	FName FootSocketName,
	ASnowRumbleCharacter* SourceCharacter)
{
	UCanvasRenderTarget2D* TargetRenderTarget = EnsureSnowTrailRenderTarget();
	if (!TargetRenderTarget)
	{
		return false;
	}

	FSnowTrailStampData FinalStampData;
	bool bAddedAnyStamp = false;
	const FVector SafeWorldNormal = WorldNormal.GetSafeNormal();

	if (bInterpolateTrailBetweenStamps && SourceCharacter)
	{
		if (const FVector* PreviousLocation =
			LastStampLocationByCharacter.Find(SourceCharacter))
		{
			const float Distance =
				FVector::Dist(*PreviousLocation, WorldLocation);
			if (Distance > InterpolatedStampSpacingWorld
				&& Distance <= MaxInterpolatedStampDistanceWorld)
			{
				const int32 SegmentCount = FMath::Clamp(
					FMath::FloorToInt(
						Distance
						/ FMath::Max(1.0f, InterpolatedStampSpacingWorld)),
					1,
					16);
				for (int32 SegmentIndex = 1;
					SegmentIndex < SegmentCount;
					++SegmentIndex)
				{
					const float Alpha =
						static_cast<float>(SegmentIndex)
						/ static_cast<float>(SegmentCount);
					FSnowTrailStampData InterpolatedStampData;
					bAddedAnyStamp |= AddSnowTrailStamp(
						FMath::Lerp(*PreviousLocation, WorldLocation, Alpha),
						SafeWorldNormal,
						RadiusWorld,
						FootSocketName,
						SourceCharacter,
						InterpolatedStampData);
				}
			}
		}
	}

	if (!AddSnowTrailStamp(
		WorldLocation,
		SafeWorldNormal,
		RadiusWorld,
		FootSocketName,
		SourceCharacter,
		FinalStampData))
	{
		return bAddedAnyStamp;
	}

	bAddedAnyStamp = true;
	if (SourceCharacter)
	{
		LastStampLocationByCharacter.Add(SourceCharacter, WorldLocation);
	}

	OnSnowTrailStampRequested(
		TargetRenderTarget,
		FinalStampData.WorldLocation,
		FinalStampData.WorldNormal,
		FinalStampData.TrailUV,
		FinalStampData.RadiusWorld,
		FinalStampData.RadiusPixels,
		FinalStampData.FootSocketName,
		FinalStampData.SourceCharacter);
	TargetRenderTarget->UpdateResource();
	return bAddedAnyStamp;
}

bool ASnowTrailRenderTargetManager::AddSnowTrailStamp(
	FVector WorldLocation,
	FVector WorldNormal,
	float RadiusWorld,
	FName FootSocketName,
	ASnowRumbleCharacter* SourceCharacter,
	FSnowTrailStampData& OutStampData)
{
	FVector2D TrailUV;
	if (!ConvertWorldLocationToTrailUV(WorldLocation, TrailUV))
	{
		return false;
	}
	if (!SnowTrailRenderTarget)
	{
		return false;
	}

	const float SafeRadiusWorld =
		RadiusWorld > 0.0f ? RadiusWorld : DefaultStampRadiusWorld;
	const float RadiusPixels =
		TrailWorldSize > 0.0f
			? SafeRadiusWorld / TrailWorldSize
				* static_cast<float>(SnowTrailRenderTarget->SizeX)
			: 0.0f;

	FSnowTrailStampData StampData;
	StampData.WorldLocation = WorldLocation;
	StampData.WorldNormal = WorldNormal.GetSafeNormal();
	StampData.TrailUV = TrailUV;
	StampData.RadiusWorld = SafeRadiusWorld;
	StampData.RadiusPixels = RadiusPixels;
	StampData.FootSocketName = FootSocketName;
	StampData.SourceCharacter = SourceCharacter;

	SnowTrailStamps.Add(StampData);
	const int32 SafeMaxStoredStampCount = FMath::Max(1, MaxStoredStampCount);
	if (SnowTrailStamps.Num() > SafeMaxStoredStampCount)
	{
		SnowTrailStamps.RemoveAt(
			0,
			SnowTrailStamps.Num() - SafeMaxStoredStampCount,
			EAllowShrinking::No);
	}

	OutStampData = StampData;
	return true;
}

TArray<FSnowTrailStampData>
ASnowTrailRenderTargetManager::GetSnowTrailStamps() const
{
	return SnowTrailStamps;
}

void ASnowTrailRenderTargetManager::RefreshSnowTrailMaterialParameters()
{
	ApplySnowTrailMaterialParameters(EnsureSnowTrailRenderTarget());
}

void ASnowTrailRenderTargetManager::HandleSnowTrailCanvasUpdate(
	UCanvas* Canvas,
	int32 Width,
	int32 Height)
{
	if (!Canvas)
	{
		return;
	}

	for (const FSnowTrailStampData& StampData : SnowTrailStamps)
	{
		if (bDrawStampsInCpp)
		{
			DrawSnowTrailStampInCpp(Canvas, Width, Height, StampData);
		}

		OnDrawSnowTrailStamp(Canvas, Width, Height, StampData);
	}
}

void ASnowTrailRenderTargetManager::DrawSnowTrailStampInCpp(
	UCanvas* Canvas,
	int32 Width,
	int32 Height,
	const FSnowTrailStampData& StampData) const
{
	if (!Canvas
		|| !SnowTrailStampMaterial
		|| Width <= 0
		|| Height <= 0
		|| StampData.RadiusPixels <= 0.0f)
	{
		return;
	}

	const float DiameterPixels = StampData.RadiusPixels * 2.0f;
	const FVector2D StampSize(DiameterPixels, DiameterPixels);
	const FVector2D StampCenter(
		StampData.TrailUV.X * static_cast<float>(Width),
		StampData.TrailUV.Y * static_cast<float>(Height));
	const FVector2D StampPosition = StampCenter - StampSize * 0.5f;

	Canvas->K2_DrawMaterial(
		SnowTrailStampMaterial,
		StampPosition,
		StampSize,
		FVector2D::ZeroVector,
		FVector2D::UnitVector,
		0.0f,
		FVector2D(0.5f, 0.5f));
}

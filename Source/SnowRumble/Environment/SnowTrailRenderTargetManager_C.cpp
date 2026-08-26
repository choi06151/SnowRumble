// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowTrailRenderTargetManager_C.h"

#include "../Player/SnowRumbleCharacter.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Components/PrimitiveComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogSnowTrail, Log, All);

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
		ASnowTrailRenderTargetManager* Manager = *It;
		if (Manager && Manager->ShouldLogSnowTrailDebug())
		{
			UE_LOG(
				LogSnowTrail,
				Log,
				TEXT("[SnowTrail] Found manager=%s World=%s Center=(%.1f, %.1f) Size=%.1f RT=%s"),
				*GetNameSafe(Manager),
				*GetNameSafe(World),
				Manager->TrailWorldCenter.X,
				Manager->TrailWorldCenter.Y,
				Manager->TrailWorldSize,
				*GetNameSafe(Manager->SnowTrailRenderTarget));
		}
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
	UpdateTrailWorldAreaFromMaterialBounds();

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

bool ASnowTrailRenderTargetManager::UpdateTrailWorldAreaFromMaterialBounds()
{
	if (!bAutoFitTrailWorldAreaFromMaterialBounds)
	{
		return false;
	}

	FBox CombinedBounds(ForceInit);
	bool bHasValidBounds = false;
	auto AccumulateActorBounds =
		[this, &CombinedBounds, &bHasValidBounds](AActor* TargetActor)
	{
		if (!TargetActor)
		{
			return;
		}

		TArray<UPrimitiveComponent*> PrimitiveComponents;
		TargetActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
		for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
		{
			bHasValidBounds |= AccumulateTrailMaterialComponentBounds(
				PrimitiveComponent,
				CombinedBounds);
		}
	};

	for (UPrimitiveComponent* PrimitiveComponent : SnowTrailMaterialComponents)
	{
		bHasValidBounds |= AccumulateTrailMaterialComponentBounds(
			PrimitiveComponent,
			CombinedBounds);
	}

	for (AActor* MaterialActor : SnowTrailMaterialActors)
	{
		AccumulateActorBounds(MaterialActor);
	}

	if (bAutoApplyToSnowSurfaceTaggedActors
		&& !SnowTrailMaterialAutoApplyActorTag.IsNone())
	{
		UWorld* World = GetWorld();
		if (World)
		{
			for (TActorIterator<AActor> It(World); It; ++It)
			{
				AActor* TaggedActor = *It;
				if (TaggedActor
					&& TaggedActor->ActorHasTag(
						SnowTrailMaterialAutoApplyActorTag))
				{
					AccumulateActorBounds(TaggedActor);
				}
			}
		}
	}

	if (!bHasValidBounds || !CombinedBounds.IsValid)
	{
		if (bLogSnowTrailDebug)
		{
			UE_LOG(
				LogSnowTrail,
				Warning,
				TEXT("[SnowTrail] Auto-fit skipped: no valid material bounds Manager=%s"),
				*GetNameSafe(this));
		}
		return false;
	}

	const FVector BoundsCenter = CombinedBounds.GetCenter();
	const FVector BoundsExtent = CombinedBounds.GetExtent();
	const float SafePadding = FMath::Max(0.0f, TrailWorldBoundsPadding);
	const float FittedWorldSize = FMath::Max(
		FMath::Max(BoundsExtent.X * 2.0f, BoundsExtent.Y * 2.0f)
			+ SafePadding * 2.0f,
		100.0f);

	TrailWorldCenter = FVector2D(BoundsCenter.X, BoundsCenter.Y);
	TrailWorldSize = FittedWorldSize;

	if (bLogSnowTrailDebug)
	{
		UE_LOG(
			LogSnowTrail,
			Log,
			TEXT("[SnowTrail] Auto-fit bounds Manager=%s Center=(%.1f, %.1f) Size=%.1f Padding=%.1f BoundsMin=%s BoundsMax=%s"),
			*GetNameSafe(this),
			TrailWorldCenter.X,
			TrailWorldCenter.Y,
			TrailWorldSize,
			SafePadding,
			*CombinedBounds.Min.ToCompactString(),
			*CombinedBounds.Max.ToCompactString());
	}
	return true;
}

bool ASnowTrailRenderTargetManager::AccumulateTrailMaterialComponentBounds(
	UPrimitiveComponent* TargetComponent,
	FBox& InOutBounds) const
{
	if (!TargetComponent)
	{
		return false;
	}

	const FBoxSphereBounds& ComponentBounds = TargetComponent->Bounds;
	const FVector BoundsExtent = ComponentBounds.BoxExtent;
	if (BoundsExtent.X <= 0.0f || BoundsExtent.Y <= 0.0f)
	{
		return false;
	}

	const FBox ComponentBox = ComponentBounds.GetBox();
	if (!ComponentBox.IsValid)
	{
		return false;
	}

	InOutBounds += ComponentBox;
	return true;
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

	const FLinearColor UVScaleParameter(
		SnowTrailUVScale.X,
		SnowTrailUVScale.Y,
		0.0f,
		0.0f);
	const FLinearColor UVOffsetParameter(
		SnowTrailUVOffset.X,
		SnowTrailUVOffset.Y,
		0.0f,
		0.0f);
	TSet<UPrimitiveComponent*> AppliedComponents;

	auto ApplyToComponent =
		[
			this,
			TargetRenderTarget,
			UVScaleParameter,
			UVOffsetParameter,
			&AppliedComponents](
			UPrimitiveComponent* TargetComponent)
	{
		if (!TargetComponent || AppliedComponents.Contains(TargetComponent))
		{
			return;
		}
		AppliedComponents.Add(TargetComponent);

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
			if (!SnowTrailWorldCenterXParameterName.IsNone())
			{
				DynamicMaterial->SetScalarParameterValue(
					SnowTrailWorldCenterXParameterName,
					TrailWorldCenter.X);
			}
			if (!SnowTrailWorldCenterYParameterName.IsNone())
			{
				DynamicMaterial->SetScalarParameterValue(
					SnowTrailWorldCenterYParameterName,
					TrailWorldCenter.Y);
			}
			if (!SnowTrailWorldSizeParameterName.IsNone())
			{
				DynamicMaterial->SetScalarParameterValue(
					SnowTrailWorldSizeParameterName,
					TrailWorldSize);
			}
			DynamicMaterial->SetScalarParameterValue(
				TEXT("SnowTrailWorldSize"),
				TrailWorldSize);
			DynamicMaterial->SetScalarParameterValue(
				TEXT("TrailWorldSize"),
				TrailWorldSize);
			DynamicMaterial->SetScalarParameterValue(
				TEXT("Size"),
				TrailWorldSize);
			DynamicMaterial->SetScalarParameterValue(
				TEXT("WorldSize"),
				TrailWorldSize);
			DynamicMaterial->SetScalarParameterValue(
				TEXT("TrailSize"),
				TrailWorldSize);
			DynamicMaterial->SetVectorParameterValue(
				TEXT("SnowTrailUVScale"),
				UVScaleParameter);
			DynamicMaterial->SetVectorParameterValue(
				TEXT("TrailUVScale"),
				UVScaleParameter);
			DynamicMaterial->SetVectorParameterValue(
				TEXT("SnowTrailUVOffset"),
				UVOffsetParameter);
			DynamicMaterial->SetVectorParameterValue(
				TEXT("TrailUVOffset"),
				UVOffsetParameter);
			DynamicMaterial->SetScalarParameterValue(
				TEXT("SnowTrailFlipU"),
				bSnowTrailFlipU ? 1.0f : 0.0f);
			DynamicMaterial->SetScalarParameterValue(
				TEXT("TrailFlipU"),
				bSnowTrailFlipU ? 1.0f : 0.0f);
			DynamicMaterial->SetScalarParameterValue(
				TEXT("SnowTrailFlipV"),
				bSnowTrailFlipV ? 1.0f : 0.0f);
			DynamicMaterial->SetScalarParameterValue(
				TEXT("TrailFlipV"),
				bSnowTrailFlipV ? 1.0f : 0.0f);

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

	if (bAutoApplyToSnowSurfaceTaggedActors
		&& !SnowTrailMaterialAutoApplyActorTag.IsNone())
	{
		UWorld* World = GetWorld();
		if (World)
		{
			for (TActorIterator<AActor> It(World); It; ++It)
			{
				AActor* TaggedActor = *It;
				if (!TaggedActor
					|| !TaggedActor->ActorHasTag(
						SnowTrailMaterialAutoApplyActorTag))
				{
					continue;
				}

				TArray<UPrimitiveComponent*> PrimitiveComponents;
				TaggedActor->GetComponents<UPrimitiveComponent>(
					PrimitiveComponents);
				for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
				{
					ApplyToComponent(PrimitiveComponent);
				}
			}
		}
	}

	if (bLogSnowTrailDebug)
	{
		UE_LOG(
			LogSnowTrail,
			Log,
			TEXT("[SnowTrail] Applied material params RT=%s Components=%d Materials=%d AutoTag=%s"),
			*GetNameSafe(TargetRenderTarget),
			AppliedComponents.Num(),
			AppliedSnowTrailMaterials.Num(),
			*SnowTrailMaterialAutoApplyActorTag.ToString());
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

	const bool bInsideBounds = OutTrailUV.X >= 0.0f
		&& OutTrailUV.X <= 1.0f
		&& OutTrailUV.Y >= 0.0f
		&& OutTrailUV.Y <= 1.0f;
	if (!bInsideBounds && bLogSnowTrailDebug)
	{
		UE_LOG(
			LogSnowTrail,
			Warning,
			TEXT("[SnowTrail] UV outside bounds Location=%s UV=(%.3f, %.3f) Center=(%.1f, %.1f) Size=%.1f"),
			*WorldLocation.ToCompactString(),
			OutTrailUV.X,
			OutTrailUV.Y,
			TrailWorldCenter.X,
			TrailWorldCenter.Y,
			TrailWorldSize);
	}
	return bInsideBounds;
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
		if (bLogSnowTrailDebug)
		{
			UE_LOG(
				LogSnowTrail,
				Warning,
				TEXT("[SnowTrail] Stamp failed: render target missing Manager=%s"),
				*GetNameSafe(this));
		}
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
		if (bLogSnowTrailDebug)
		{
			UE_LOG(
				LogSnowTrail,
				Warning,
				TEXT("[SnowTrail] Stamp rejected Location=%s Normal=%s Radius=%.1f Foot=%s Source=%s"),
				*WorldLocation.ToCompactString(),
				*SafeWorldNormal.ToCompactString(),
				RadiusWorld,
				*FootSocketName.ToString(),
				*GetNameSafe(SourceCharacter));
		}
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
	if (bLogSnowTrailDebug)
	{
		UE_LOG(
			LogSnowTrail,
			Log,
			TEXT("[SnowTrail] Stamp accepted Location=%s UV=(%.3f, %.3f) RadiusWorld=%.1f RadiusPixels=%.1f Count=%d RT=%s"),
			*FinalStampData.WorldLocation.ToCompactString(),
			FinalStampData.TrailUV.X,
			FinalStampData.TrailUV.Y,
			FinalStampData.RadiusWorld,
			FinalStampData.RadiusPixels,
			SnowTrailStamps.Num(),
			*GetNameSafe(TargetRenderTarget));
	}
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

bool ASnowTrailRenderTargetManager::ShouldLogSnowTrailDebug() const
{
	return bLogSnowTrailDebug;
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

	if (bLogSnowTrailDebug)
	{
		UE_LOG(
			LogSnowTrail,
			Verbose,
			TEXT("[SnowTrail] Canvas update RT=%s Width=%d Height=%d Count=%d StampMaterial=%s"),
			*GetNameSafe(SnowTrailRenderTarget),
			Width,
			Height,
			SnowTrailStamps.Num(),
			*GetNameSafe(SnowTrailStampMaterial));
	}

	for (const FSnowTrailStampData& StampData : SnowTrailStamps)
	{
		if (bDrawStampsInCpp || !SnowTrailStampMaterial)
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
		|| Width <= 0
		|| Height <= 0
		|| StampData.RadiusPixels <= 0.0f)
	{
		return;
	}

	const float SafeRadiusPixels =
		FMath::Max(StampData.RadiusPixels, MinimumStampRadiusPixels);
	const float DiameterPixels = SafeRadiusPixels * 2.0f;
	const FVector2D StampSize(DiameterPixels, DiameterPixels);
	const FVector2D StampCenter(
		StampData.TrailUV.X * static_cast<float>(Width),
		StampData.TrailUV.Y * static_cast<float>(Height));
	const FVector2D StampPosition = StampCenter - StampSize * 0.5f;

	if (SnowTrailStampMaterial)
	{
		Canvas->K2_DrawMaterial(
			SnowTrailStampMaterial,
			StampPosition,
			StampSize,
			FVector2D::ZeroVector,
			FVector2D::UnitVector,
			0.0f,
			FVector2D(0.5f, 0.5f));
		return;
	}

	FCanvasTileItem TileItem(StampPosition, StampSize, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TileItem);
}

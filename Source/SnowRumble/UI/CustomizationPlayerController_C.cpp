// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomizationPlayerController_C.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Border.h"
#include "Components/CapsuleComponent.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "CustomizationWidget_C.h"
#include "Animation/AnimationAsset.h"
#include "Engine/Canvas.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "EngineUtils.h"
#include "Engine/GameInstance.h"
#include "Engine/SkinnedAsset.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Colors/SColorPicker.h"
#include "../Player/SnowRumbleCharacter.h"
#include "../Player/SnowRumbleCustomizationSubsystem_C.h"

void ACustomizationPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		EnsurePreviewCharacter();
		ApplyPreviewAnimationSettings();
		LoadSavedCustomizationForPreview();
		ConfigurePreviewCharacterForPainting();
		EnsurePaintRenderTarget();
		ApplyCustomizationCameraView();
		ShowCustomizationMenu();
	}
}

void ACustomizationPlayerController::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{
	if (CustomizationWidget)
	{
		CustomizationWidget->RemoveFromParent();
		CustomizationWidget = nullptr;
	}
	DefaultMouseCursorWidget = nullptr;
	PaintMouseCursorWidget = nullptr;

	Super::EndPlay(EndPlayReason);
}

void ACustomizationPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (IsLocalController())
	{
		UpdatePreviewRotation(DeltaTime);
		UpdatePaintUndoInput();
		UpdatePaintInput();
		UpdatePaintMouseCursorPresentation();
	}
}

void ACustomizationPlayerController::ShowCustomizationMenu()
{
	if (!IsLocalController())
	{
		return;
	}

	UCustomizationWidget* Widget = EnsureCustomizationWidget();
	if (!Widget)
	{
		return;
	}

	if (!Widget->IsInViewport())
	{
		Widget->AddToViewport(100);
	}
	Widget->SetKeyboardFocus();

	bShowMouseCursor = true;
	EnsureMouseCursorWidgets();
	ApplyCurrentMouseCursorWidget();
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(Widget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
}

void ACustomizationPlayerController::ReturnToMainMenu()
{
	if (!IsLocalController() || MainMenuTravelUrl.IsEmpty())
	{
		return;
	}

	ClientTravel(MainMenuTravelUrl, TRAVEL_Absolute);
}

void ACustomizationPlayerController::SetPreviewBodyColor(
	FLinearColor NewBodyColor)
{
	FSnowRumbleCustomizationData NewData = PreviewCustomizationData;
	NewData.BodyColor = NewBodyColor;
	PreviewCustomizationData =
		USnowRumbleCustomizationSubsystem::SanitizeCustomizationData(NewData);
	ApplyPreviewDataToCharacter();
}

void ACustomizationPlayerController::OpenPaintBrushColorPicker()
{
	if (!IsLocalController())
	{
		return;
	}

	FColorPickerArgs PickerArgs;
	PickerArgs.DisplayGamma =
		TAttribute<float>::Create(TAttribute<float>::FGetter::CreateUObject(
			GEngine,
			&UEngine::GetDisplayGamma));
	PickerArgs.InitialColor = PaintBrushColor;
	PickerArgs.bUseAlpha = false;
	PickerArgs.bOnlyRefreshOnMouseUp = false;
	PickerArgs.bOnlyRefreshOnOk = false;
	PickerArgs.bClampValue = true;
	PickerArgs.OnColorCommitted =
		FOnLinearColorValueChanged::CreateUObject(
			this,
			&ACustomizationPlayerController::HandlePaintBrushColorPicked);
	OpenColorPicker(PickerArgs);
}

void ACustomizationPlayerController::OpenPaintBrushColorPickerOnLeft(
	const FVector2D& AnchorScreenPosition)
{
	if (!IsLocalController())
	{
		return;
	}

	FSlateApplication& SlateApplication = FSlateApplication::Get();
	const FVector2D OriginalCursorPosition = SlateApplication.GetCursorPos();
	const FVector2D ColorPickerWindowSizeEstimate =
		SColorPicker::DEFAULT_WINDOW_SIZE + FVector2D(0.0f, 130.0f);
	const FVector2D PickerAnchorPosition(
		AnchorScreenPosition.X
			- ColorPickerWindowSizeEstimate.X
			- PaintBrushColorPickerLeftPadding,
		AnchorScreenPosition.Y);

	SlateApplication.SetCursorPos(PickerAnchorPosition);
	OpenPaintBrushColorPicker();
	SlateApplication.SetCursorPos(OriginalCursorPosition);
}

void ACustomizationPlayerController::SetPaintBrushColor(
	FLinearColor NewBrushColor)
{
	NewBrushColor.R = FMath::Clamp(NewBrushColor.R, 0.0f, 1.0f);
	NewBrushColor.G = FMath::Clamp(NewBrushColor.G, 0.0f, 1.0f);
	NewBrushColor.B = FMath::Clamp(NewBrushColor.B, 0.0f, 1.0f);
	NewBrushColor.A = 1.0f;
	PaintBrushColor = NewBrushColor;
	UpdatePaintMouseCursorPresentation();
}

FLinearColor ACustomizationPlayerController::GetPaintBrushColor() const
{
	return PaintBrushColor;
}

void ACustomizationPlayerController::FillPreviewBodyWithBrushColor()
{
	SetPreviewBodyColor(PaintBrushColor);
	SavePreviewCustomizationData();
}

void ACustomizationPlayerController::StartAdjustPaintBrushSize()
{
	bIsAdjustingPaintBrushSize = true;
}

void ACustomizationPlayerController::StopAdjustPaintBrushSize()
{
	bIsAdjustingPaintBrushSize = false;
}

void ACustomizationPlayerController::AdjustPaintBrushSizeFromWheel(
	float WheelDelta)
{
	if (!bIsAdjustingPaintBrushSize || FMath::IsNearlyZero(WheelDelta))
	{
		return;
	}

	const float SafeMinSize = FMath::Max(1.0f, MinPaintBrushSize);
	const float SafeMaxSize = FMath::Max(SafeMinSize, MaxPaintBrushSize);
	PaintStrokeThickness = FMath::Clamp(
		PaintStrokeThickness + WheelDelta * PaintBrushWheelStep,
		SafeMinSize,
		SafeMaxSize);
	UpdatePaintMouseCursorPresentation();
}

float ACustomizationPlayerController::GetPaintBrushSize() const
{
	return PaintStrokeThickness;
}

void ACustomizationPlayerController::ApplyPreviewCustomization()
{
	if (!IsLocalController())
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	USnowRumbleCustomizationSubsystem* CustomizationSubsystem = GameInstance
		? GameInstance->GetSubsystem<USnowRumbleCustomizationSubsystem>()
		: nullptr;
	if (CustomizationSubsystem)
	{
		CustomizationSubsystem->SetCustomizationData(PreviewCustomizationData);
	}
}

void ACustomizationPlayerController::ResetPreviewCustomization()
{
	PreviewCustomizationData =
		USnowRumbleCustomizationSubsystem::GetDefaultCustomizationData();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USnowRumbleCustomizationSubsystem* CustomizationSubsystem =
			GameInstance->GetSubsystem<USnowRumbleCustomizationSubsystem>())
		{
			CustomizationSubsystem->ResetCustomizationData();
		}
	}

	PaintStrokes = PreviewCustomizationData.PaintStrokes;
	ActivePaintStroke.Points.Reset();
	bIsPaintingStroke = false;
	bWasPaintMouseDown = false;
	RedrawPaintRenderTarget();
	ApplyPreviewDataToCharacter();
}

FSnowRumbleCustomizationData
ACustomizationPlayerController::GetPreviewCustomizationData() const
{
	return PreviewCustomizationData;
}

void ACustomizationPlayerController::UndoLastPaintStroke()
{
	if (bIsPaintingStroke || !ActivePaintStroke.Points.IsEmpty())
	{
		ActivePaintStroke.Points.Reset();
		bIsPaintingStroke = false;
		bWasPaintMouseDown = false;
		SyncPaintStrokesToPreviewData();
		SavePreviewCustomizationData();
		RedrawPaintRenderTarget();
		return;
	}

	if (!PaintStrokes.IsEmpty())
	{
		PaintStrokes.Pop();
		SyncPaintStrokesToPreviewData();
		SavePreviewCustomizationData();
		RedrawPaintRenderTarget();
	}
}

void ACustomizationPlayerController::ResetPaintStrokes()
{
	PaintStrokes.Reset();
	ActivePaintStroke.Points.Reset();
	bIsPaintingStroke = false;
	bWasPaintMouseDown = false;
	SyncPaintStrokesToPreviewData();
	SavePreviewCustomizationData();
	RedrawPaintRenderTarget();
}

UCanvasRenderTarget2D*
ACustomizationPlayerController::GetPaintRenderTarget() const
{
	return PaintRenderTarget;
}

void ACustomizationPlayerController::StartRotatePreviewLeft()
{
	PreviewRotationInput = 1.0f;
}

void ACustomizationPlayerController::StartRotatePreviewRight()
{
	PreviewRotationInput = -1.0f;
}

void ACustomizationPlayerController::StopRotatePreview()
{
	PreviewRotationInput = 0.0f;
}

void ACustomizationPlayerController::SetPaintCursorActive(
	bool bNewPaintCursorActive)
{
	if (bIsPaintCursorActive == bNewPaintCursorActive)
	{
		return;
	}

	bIsPaintCursorActive = bNewPaintCursorActive;
	ApplyCurrentMouseCursorWidget();
}

void ACustomizationPlayerController::ApplyCustomizationCameraView()
{
	if (CustomizationCameraTag.IsNone())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Candidate = *It;
		if (Candidate && Candidate->ActorHasTag(CustomizationCameraTag))
		{
			SetViewTargetWithBlend(Candidate, 0.0f);
			return;
		}
	}
}

ASnowRumbleCharacter*
ACustomizationPlayerController::GetPreviewCharacter() const
{
	if (ASnowRumbleCharacter* PossessedPreviewCharacter =
		GetPawn<ASnowRumbleCharacter>())
	{
		return PossessedPreviewCharacter;
	}

	return CachedPreviewCharacter;
}

ASnowRumbleCharacter*
ACustomizationPlayerController::EnsurePreviewCharacter()
{
	if (ASnowRumbleCharacter* ExistingPreviewCharacter = GetPreviewCharacter())
	{
		CachedPreviewCharacter = ExistingPreviewCharacter;
		return ExistingPreviewCharacter;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	if (!PreviewCharacterTag.IsNone())
	{
		for (TActorIterator<ASnowRumbleCharacter> It(World); It; ++It)
		{
			ASnowRumbleCharacter* Candidate = *It;
			if (Candidate && Candidate->ActorHasTag(PreviewCharacterTag))
			{
				CachedPreviewCharacter = Candidate;
				if (HasAuthority() && !Candidate->GetController())
				{
					Possess(Candidate);
				}
				return Candidate;
			}
		}
	}

	for (TActorIterator<ASnowRumbleCharacter> It(World); It; ++It)
	{
		ASnowRumbleCharacter* Candidate = *It;
		if (Candidate)
		{
			CachedPreviewCharacter = Candidate;
			if (HasAuthority() && !Candidate->GetController())
			{
				Possess(Candidate);
			}
			return Candidate;
		}
	}

	if (!HasAuthority())
	{
		return nullptr;
	}

	TSubclassOf<ASnowRumbleCharacter> CharacterClass = PreviewCharacterClass;
	if (!CharacterClass)
	{
		CharacterClass = ASnowRumbleCharacter::StaticClass();
	}
	ASnowRumbleCharacter* SpawnedPreviewCharacter =
		World->SpawnActorDeferred<ASnowRumbleCharacter>(
			CharacterClass,
			GetPreviewCharacterSpawnTransform(),
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (!SpawnedPreviewCharacter)
	{
		return nullptr;
	}

	if (!PreviewCharacterTag.IsNone())
	{
		SpawnedPreviewCharacter->Tags.AddUnique(PreviewCharacterTag);
	}

	UGameplayStatics::FinishSpawningActor(
		SpawnedPreviewCharacter,
		GetPreviewCharacterSpawnTransform());
	CachedPreviewCharacter = SpawnedPreviewCharacter;
	Possess(SpawnedPreviewCharacter);
	return SpawnedPreviewCharacter;
}

FTransform ACustomizationPlayerController::GetPreviewCharacterSpawnTransform()
	const
{
	UWorld* World = GetWorld();
	if (World)
	{
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			const APlayerStart* PlayerStart = *It;
			if (PlayerStart)
			{
				return PlayerStart->GetActorTransform();
			}
		}
	}

	return FTransform(
		FRotator::ZeroRotator,
		FVector(0.0f, 0.0f, 100.0f),
		FVector::OneVector);
}

void ACustomizationPlayerController::ApplyPreviewAnimationSettings()
{
	ASnowRumbleCharacter* PreviewCharacter = GetPreviewCharacter();
	if (!PreviewCharacter)
	{
		return;
	}

	TArray<USkeletalMeshComponent*> MeshComponents;
	PreviewCharacter->GetComponents(MeshComponents);
	for (USkeletalMeshComponent* MeshComponent : MeshComponents)
	{
		if (!MeshComponent || !MeshComponent->GetSkinnedAsset())
		{
			continue;
		}

		if (PreviewAnimationAsset)
		{
			MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			MeshComponent->SetAnimation(PreviewAnimationAsset);
			MeshComponent->SetPosition(PreviewAnimationPositionSeconds, false);
		}

		MeshComponent->bPauseAnims = bPausePreviewAnimation;
	}
}

void ACustomizationPlayerController::LoadSavedCustomizationForPreview()
{
	UGameInstance* GameInstance = GetGameInstance();
	const USnowRumbleCustomizationSubsystem* CustomizationSubsystem = GameInstance
		? GameInstance->GetSubsystem<USnowRumbleCustomizationSubsystem>()
		: nullptr;
	PreviewCustomizationData = CustomizationSubsystem
		? CustomizationSubsystem->GetCustomizationData()
		: USnowRumbleCustomizationSubsystem::GetDefaultCustomizationData();
	PaintStrokes = PreviewCustomizationData.PaintStrokes;
	ActivePaintStroke.Points.Reset();
	bIsPaintingStroke = false;
	bWasPaintMouseDown = false;
	RedrawPaintRenderTarget();

	ApplyPreviewDataToCharacter();
}

void ACustomizationPlayerController::ApplyPreviewDataToCharacter()
{
	if (ASnowRumbleCharacter* PreviewCharacter = GetPreviewCharacter())
	{
		PreviewCharacter->ApplyCustomizationData(PreviewCustomizationData);
		if (PaintRenderTarget)
		{
			PreviewCharacter->SetCustomizationPaintTexture(PaintRenderTarget);
		}
	}
}

void ACustomizationPlayerController::ConfigurePreviewCharacterForPainting()
{
	ASnowRumbleCharacter* PreviewCharacter = GetPreviewCharacter();
	if (!PreviewCharacter)
	{
		return;
	}

	TArray<USkeletalMeshComponent*> MeshComponents;
	PreviewCharacter->GetComponents(MeshComponents);
	for (USkeletalMeshComponent* MeshComponent : MeshComponents)
	{
		if (!MeshComponent || !MeshComponent->GetSkinnedAsset())
		{
			continue;
		}

		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		MeshComponent->bReturnMaterialOnMove = true;
		MeshComponent->SetEnablePerPolyCollision(true);
		MeshComponent->RecreatePhysicsState();
	}

	if (UCapsuleComponent* CapsuleComponent =
		PreviewCharacter->GetCapsuleComponent())
	{
		CapsuleComponent->SetCollisionResponseToChannel(
			ECC_Visibility,
			ECR_Ignore);
	}

	TArray<UWidgetComponent*> WidgetComponents;
	PreviewCharacter->GetComponents(WidgetComponents);
	for (UWidgetComponent* WidgetComponent : WidgetComponents)
	{
		if (WidgetComponent
			&& WidgetComponent->GetFName()
				== TEXT("OverheadNameplateComponent"))
		{
			WidgetComponent->SetVisibility(false, true);
			WidgetComponent->SetHiddenInGame(true, true);
			WidgetComponent->SetComponentTickEnabled(false);
		}
	}
}

void ACustomizationPlayerController::EnsurePaintRenderTarget()
{
	if (PaintRenderTarget)
	{
		ApplyPreviewDataToCharacter();
		return;
	}

	const int32 SafeRenderTargetSize =
		FMath::Clamp(PaintRenderTargetSize, 64, 4096);
	PaintRenderTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(
		this,
		UCanvasRenderTarget2D::StaticClass(),
		SafeRenderTargetSize,
		SafeRenderTargetSize);
	if (!PaintRenderTarget)
	{
		return;
	}

	PaintRenderTarget->ClearColor = FLinearColor::Transparent;
	PaintRenderTarget->OnCanvasRenderTargetUpdate.AddUniqueDynamic(
		this,
		&ACustomizationPlayerController::HandlePaintCanvasUpdate);
	RedrawPaintRenderTarget();
	ApplyPreviewDataToCharacter();
}

bool ACustomizationPlayerController::GetPaintUvUnderCursor(
	FVector2D& OutPaintUv,
	FName& OutMeshComponentName,
	int32& OutMaterialIndex)
{
	const ASnowRumbleCharacter* PreviewCharacter = GetPreviewCharacter();
	if (!PreviewCharacter)
	{
		ShowPaintDebugMessage(TEXT("Paint failed: preview character not found"));
		return false;
	}

	float MouseX = 0.0f;
	float MouseY = 0.0f;
	if (!GetPaintCursorScreenPosition(MouseX, MouseY))
	{
		ShowPaintDebugMessage(TEXT("Paint failed: mouse position not found"));
		return false;
	}

	FVector WorldLocation;
	FVector WorldDirection;
	if (!DeprojectScreenPositionToWorld(
		MouseX,
		MouseY,
		WorldLocation,
		WorldDirection))
	{
		ShowPaintDebugMessage(TEXT("Paint failed: mouse deproject failed"));
		return false;
	}

	const FVector TraceEnd = WorldLocation + WorldDirection * 100000.0f;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CustomizationPaintTrace), true);
	QueryParams.bReturnFaceIndex = true;
	QueryParams.bReturnPhysicalMaterial = false;

	TArray<USkeletalMeshComponent*> MeshComponents;
	PreviewCharacter->GetComponents(MeshComponents);
	bool bHitAnyPreviewMesh = false;
	for (USkeletalMeshComponent* MeshComponent : MeshComponents)
	{
		if (!MeshComponent || !MeshComponent->GetSkinnedAsset())
		{
			continue;
		}

		FHitResult HitResult;
		if (!MeshComponent->LineTraceComponent(
			HitResult,
			WorldLocation,
			TraceEnd,
			QueryParams))
		{
			continue;
		}

		bHitAnyPreviewMesh = true;
		if (FindPaintUvOnSkinnedRenderData(
			MeshComponent,
			WorldLocation,
			TraceEnd,
			OutPaintUv,
			OutMaterialIndex))
		{
			OutMeshComponentName = MeshComponent->GetFName();
			return true;
		}

		if (!UGameplayStatics::FindCollisionUV(HitResult, 0, OutPaintUv))
		{
			ShowPaintDebugMessage(FString::Printf(
				TEXT("Paint failed: mesh hit but UV unavailable, FaceIndex=%d"),
				HitResult.FaceIndex));
			return false;
		}

		OutPaintUv.X = FMath::Clamp(OutPaintUv.X, 0.0f, 1.0f);
		OutPaintUv.Y = FMath::Clamp(OutPaintUv.Y, 0.0f, 1.0f);
		OutMeshComponentName = MeshComponent->GetFName();
		OutMaterialIndex = INDEX_NONE;
		return true;
	}

	if (bHitAnyPreviewMesh)
	{
		ShowPaintDebugMessage(TEXT("Paint failed: preview mesh hit rejected"));
	}
	else
	{
		for (USkeletalMeshComponent* MeshComponent : MeshComponents)
		{
			if (MeshComponent
				&& MeshComponent->GetSkinnedAsset()
				&& FindPaintUvOnSkinnedRenderData(
					MeshComponent,
					WorldLocation,
					TraceEnd,
					OutPaintUv,
					OutMaterialIndex))
			{
				OutMeshComponentName = MeshComponent->GetFName();
				return true;
			}
		}

		ShowPaintDebugMessage(TEXT("Paint failed: render triangle trace missed preview meshes"));
	}
	return false;
}

bool ACustomizationPlayerController::GetPaintCursorScreenPosition(
	float& OutMouseX,
	float& OutMouseY)
{
	if (!GetMousePosition(OutMouseX, OutMouseY))
	{
		return false;
	}

	const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(this);
	if (ViewportScale > UE_KINDA_SMALL_NUMBER)
	{
		const FVector2D SlateMousePosition =
			UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
		OutMouseX = SlateMousePosition.X * ViewportScale;
		OutMouseY = SlateMousePosition.Y * ViewportScale;
	}

	OutMouseX += PaintCursorScreenOffset.X;
	OutMouseY += PaintCursorScreenOffset.Y;
	return true;
}

bool ACustomizationPlayerController::FindPaintUvOnSkinnedRenderData(
	const USkeletalMeshComponent* MeshComponent,
	const FVector& TraceStart,
	const FVector& TraceEnd,
	FVector2D& OutPaintUv,
	int32& OutMaterialIndex) const
{
	const USkinnedAsset* SkinnedAsset = MeshComponent
		? MeshComponent->GetSkinnedAsset()
		: nullptr;
	const FSkeletalMeshRenderData* RenderData = SkinnedAsset
		? SkinnedAsset->GetResourceForRendering()
		: nullptr;
	if (!MeshComponent
		|| !RenderData
		|| !RenderData->LODRenderData.IsValidIndex(0))
	{
		return false;
	}

	const FSkeletalMeshLODRenderData& LODData = RenderData->LODRenderData[0];
	if (!LODData.IsDataReady()
		|| LODData.StaticVertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords()
			<= 0)
	{
		return false;
	}

	TArray<uint32> IndexBuffer;
	LODData.MultiSizeIndexContainer.GetIndexBuffer(IndexBuffer);
	if (IndexBuffer.IsEmpty())
	{
		return false;
	}

	const FTransform& ComponentTransform = MeshComponent->GetComponentTransform();
	float ClosestTraceDistance = TNumericLimits<float>::Max();
	FVector2D ClosestUv = FVector2D::ZeroVector;
	int32 ClosestMaterialIndex = INDEX_NONE;
	bool bFoundHit = false;

	for (const FSkelMeshRenderSection& Section : LODData.RenderSections)
	{
		if (!Section.IsValid())
		{
			continue;
		}

		const int32 LastSectionIndex =
			Section.BaseIndex + Section.NumTriangles * 3;
		for (int32 Index = Section.BaseIndex;
			Index + 2 < LastSectionIndex && Index + 2 < IndexBuffer.Num();
			Index += 3)
		{
			const int32 Index0 = static_cast<int32>(IndexBuffer[Index]);
			const int32 Index1 = static_cast<int32>(IndexBuffer[Index + 1]);
			const int32 Index2 = static_cast<int32>(IndexBuffer[Index + 2]);
			if (Index0 < 0 || Index1 < 0 || Index2 < 0
				|| Index0 >= static_cast<int32>(LODData.GetNumVertices())
				|| Index1 >= static_cast<int32>(LODData.GetNumVertices())
				|| Index2 >= static_cast<int32>(LODData.GetNumVertices()))
			{
				continue;
			}

			const FVector Vertex0 = ComponentTransform.TransformPosition(
				FVector(LODData.StaticVertexBuffers.PositionVertexBuffer
					.VertexPosition(Index0)));
			const FVector Vertex1 = ComponentTransform.TransformPosition(
				FVector(LODData.StaticVertexBuffers.PositionVertexBuffer
					.VertexPosition(Index1)));
			const FVector Vertex2 = ComponentTransform.TransformPosition(
				FVector(LODData.StaticVertexBuffers.PositionVertexBuffer
					.VertexPosition(Index2)));

			FVector HitLocation;
			FVector HitNormal;
			if (!FMath::SegmentTriangleIntersection(
				TraceStart,
				TraceEnd,
				Vertex0,
				Vertex1,
				Vertex2,
				HitLocation,
				HitNormal))
			{
				continue;
			}

			const float TraceDistance =
				FVector::DistSquared(TraceStart, HitLocation);
			if (TraceDistance >= ClosestTraceDistance)
			{
				continue;
			}

			const FVector Barycentric =
				FMath::ComputeBaryCentric2D(
					HitLocation,
					Vertex0,
					Vertex1,
					Vertex2);
			const FVector2D Uv0(
				LODData.StaticVertexBuffers.StaticMeshVertexBuffer.GetVertexUV(
					Index0,
					0));
			const FVector2D Uv1(
				LODData.StaticVertexBuffers.StaticMeshVertexBuffer.GetVertexUV(
					Index1,
					0));
			const FVector2D Uv2(
				LODData.StaticVertexBuffers.StaticMeshVertexBuffer.GetVertexUV(
					Index2,
					0));

			ClosestTraceDistance = TraceDistance;
			ClosestUv = Uv0 * Barycentric.X
				+ Uv1 * Barycentric.Y
				+ Uv2 * Barycentric.Z;
			ClosestMaterialIndex = Section.MaterialIndex;
			bFoundHit = true;
		}
	}

	if (!bFoundHit)
	{
		return false;
	}

	OutPaintUv.X = FMath::Clamp(ClosestUv.X, 0.0f, 1.0f);
	OutPaintUv.Y = FMath::Clamp(ClosestUv.Y, 0.0f, 1.0f);
	OutMaterialIndex = ClosestMaterialIndex;
	return true;
}

void ACustomizationPlayerController::ShowPaintDebugMessage(
	const FString& Message)
{
	UWorld* World = GetWorld();
	const double CurrentTime = World ? World->GetTimeSeconds() : 0.0;
	if (CurrentTime - LastPaintDebugMessageTime < 0.5)
	{
		return;
	}

	LastPaintDebugMessageTime = CurrentTime;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			static_cast<uint64>(GetUniqueID()),
			0.6f,
			FColor::Yellow,
			Message);
	}
}

void ACustomizationPlayerController::HandlePaintBrushColorPicked(
	FLinearColor NewBrushColor)
{
	SetPaintBrushColor(NewBrushColor);
}

void ACustomizationPlayerController::UpdatePaintInput()
{
	if (!CustomizationWidget
		|| CustomizationWidget->GetCurrentCustomizationPage()
			!= ESnowRumbleCustomizationPage::PaintMode)
	{
		FinishPaintStroke();
		bWasPaintMouseDown = false;
		return;
	}

	const bool bPaintMouseDown = IsInputKeyDown(EKeys::LeftMouseButton);
	FVector2D PaintUv;
	FName MeshComponentName;
	int32 MaterialIndex = INDEX_NONE;
	const bool bHasPaintUv =
		GetPaintUvUnderCursor(PaintUv, MeshComponentName, MaterialIndex);

	if (bPaintMouseDown && bHasPaintUv)
	{
		if (!bIsPaintingStroke)
		{
			BeginPaintStroke(PaintUv, MeshComponentName, MaterialIndex);
		}
		else
		{
			AddPaintPoint(PaintUv, MeshComponentName, MaterialIndex);
		}
	}
	else if (!bPaintMouseDown && bWasPaintMouseDown)
	{
		FinishPaintStroke();
	}

	bWasPaintMouseDown = bPaintMouseDown;
}

void ACustomizationPlayerController::UpdatePaintUndoInput()
{
	if (!CustomizationWidget
		|| CustomizationWidget->GetCurrentCustomizationPage()
			!= ESnowRumbleCustomizationPage::PaintMode)
	{
		return;
	}

	const bool bControlDown =
		IsInputKeyDown(EKeys::LeftControl)
		|| IsInputKeyDown(EKeys::RightControl);
	if (bControlDown && WasInputKeyJustPressed(EKeys::Z))
	{
		UndoLastPaintStroke();
	}
}

void ACustomizationPlayerController::UpdatePreviewRotation(float DeltaTime)
{
	if (FMath::IsNearlyZero(PreviewRotationInput))
	{
		return;
	}

	ASnowRumbleCharacter* PreviewCharacter = GetPreviewCharacter();
	if (!PreviewCharacter)
	{
		return;
	}

	const FRotator CurrentRotation = PreviewCharacter->GetActorRotation();
	const float DeltaYaw =
		PreviewRotationInput * PreviewRotationSpeedDegrees * DeltaTime;
	PreviewCharacter->SetActorRotation(
		FRotator(
			CurrentRotation.Pitch,
			CurrentRotation.Yaw + DeltaYaw,
			CurrentRotation.Roll));
}

void ACustomizationPlayerController::BeginPaintStroke(
	const FVector2D& PaintUv,
	FName MeshComponentName,
	int32 MaterialIndex)
{
	ActivePaintStroke.Points.Reset();
	ActivePaintStroke.MeshComponentName = MeshComponentName;
	ActivePaintStroke.MaterialIndex = MaterialIndex;
	ActivePaintStroke.BrushColor = PaintBrushColor;
	ActivePaintStroke.BrushThickness = PaintStrokeThickness;
	bIsPaintingStroke = true;
	AddPaintPoint(PaintUv, MeshComponentName, MaterialIndex);
}

void ACustomizationPlayerController::AddPaintPoint(
	const FVector2D& PaintUv,
	FName MeshComponentName,
	int32 MaterialIndex)
{
	if (!bIsPaintingStroke)
	{
		return;
	}
	if (ActivePaintStroke.MeshComponentName != MeshComponentName)
	{
		return;
	}
	if (ActivePaintStroke.MaterialIndex != MaterialIndex)
	{
		return;
	}

	if (!ActivePaintStroke.Points.IsEmpty()
		&& FVector2D::Distance(ActivePaintStroke.Points.Last(), PaintUv)
			< PaintPointMinDistance)
	{
		return;
	}

	ActivePaintStroke.Points.Add(PaintUv);
	SyncPaintStrokesToPreviewData();
	SavePreviewCustomizationData();
	RedrawPaintRenderTarget();
}

void ACustomizationPlayerController::FinishPaintStroke()
{
	if (!bIsPaintingStroke)
	{
		return;
	}

	bIsPaintingStroke = false;
	if (!ActivePaintStroke.Points.IsEmpty())
	{
		PaintStrokes.Add(ActivePaintStroke);
	}
	ActivePaintStroke.Points.Reset();
	SyncPaintStrokesToPreviewData();
	SavePreviewCustomizationData();
	RedrawPaintRenderTarget();
}

void ACustomizationPlayerController::SyncPaintStrokesToPreviewData()
{
	FSnowRumbleCustomizationData NewData = PreviewCustomizationData;
	NewData.PaintStrokes = PaintStrokes;
	NewData.bFlipPaintUvY = bFlipPaintUvY;
	if (bIsPaintingStroke && !ActivePaintStroke.Points.IsEmpty())
	{
		NewData.PaintStrokes.Add(ActivePaintStroke);
	}

	PreviewCustomizationData =
		USnowRumbleCustomizationSubsystem::SanitizeCustomizationData(NewData);
	PaintStrokes = PreviewCustomizationData.PaintStrokes;
	if (bIsPaintingStroke && !PaintStrokes.IsEmpty())
	{
		ActivePaintStroke = PaintStrokes.Pop();
	}
}

void ACustomizationPlayerController::SavePreviewCustomizationData()
{
	if (!IsLocalController())
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	USnowRumbleCustomizationSubsystem* CustomizationSubsystem = GameInstance
		? GameInstance->GetSubsystem<USnowRumbleCustomizationSubsystem>()
		: nullptr;
	if (CustomizationSubsystem)
	{
		CustomizationSubsystem->SetCustomizationData(PreviewCustomizationData);
	}
}

void ACustomizationPlayerController::RedrawPaintRenderTarget()
{
	EnsurePaintRenderTarget();
	if (PaintRenderTarget)
	{
		PaintRenderTarget->UpdateResource();
	}
}

void ACustomizationPlayerController::HandlePaintCanvasUpdate(
	UCanvas* Canvas,
	int32 Width,
	int32 Height)
{
	if (!Canvas)
	{
		return;
	}

	for (const FSnowRumblePaintStroke& Stroke : PaintStrokes)
	{
		DrawStrokeToCanvas(Canvas, Stroke, Width, Height);
	}
	if (bIsPaintingStroke)
	{
		DrawStrokeToCanvas(Canvas, ActivePaintStroke, Width, Height);
	}
}

void ACustomizationPlayerController::DrawStrokeToCanvas(
	UCanvas* Canvas,
	const FSnowRumblePaintStroke& Stroke,
	int32 Width,
	int32 Height) const
{
	if (!Canvas || Stroke.Points.IsEmpty())
	{
		return;
	}

	auto ToCanvasPoint = [
		Width,
		Height,
		bFlipY = bFlipPaintUvY](const FVector2D& PaintUv)
	{
		const float PaintY = bFlipY ? PaintUv.Y : 1.0f - PaintUv.Y;
		return FVector2D(
			PaintUv.X * static_cast<float>(Width),
			PaintY * static_cast<float>(Height));
	};

	if (Stroke.Points.Num() == 1)
	{
		const FVector2D Point = ToCanvasPoint(Stroke.Points[0]);
		Canvas->K2_DrawLine(
			Point - FVector2D(1.0f, 0.0f),
			Point + FVector2D(1.0f, 0.0f),
			Stroke.BrushThickness,
			Stroke.BrushColor);
		return;
	}

	for (int32 PointIndex = 1;
		PointIndex < Stroke.Points.Num();
		++PointIndex)
	{
		Canvas->K2_DrawLine(
			ToCanvasPoint(Stroke.Points[PointIndex - 1]),
			ToCanvasPoint(Stroke.Points[PointIndex]),
			Stroke.BrushThickness,
			Stroke.BrushColor);
	}
}

UCustomizationWidget*
ACustomizationPlayerController::EnsureCustomizationWidget()
{
	if (CustomizationWidget)
	{
		return CustomizationWidget;
	}

	if (!CustomizationWidgetClass)
	{
		return nullptr;
	}

	CustomizationWidget =
		CreateWidget<UCustomizationWidget>(this, CustomizationWidgetClass);
	if (CustomizationWidget)
	{
		CustomizationWidget->SetCustomizationPlayerController(this);
	}
	return CustomizationWidget;
}

void ACustomizationPlayerController::EnsureMouseCursorWidgets()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!DefaultMouseCursorWidget && DefaultMouseCursorWidgetClass)
	{
		DefaultMouseCursorWidget =
			CreateWidget<UUserWidget>(this, DefaultMouseCursorWidgetClass);
	}
	if (!PaintMouseCursorWidget && PaintMouseCursorWidgetClass)
	{
		PaintMouseCursorWidget =
			CreateWidget<UUserWidget>(this, PaintMouseCursorWidgetClass);
	}
}

void ACustomizationPlayerController::ApplyCurrentMouseCursorWidget()
{
	if (!IsLocalController())
	{
		return;
	}

	EnsureMouseCursorWidgets();
	UUserWidget* TargetCursorWidget = bIsPaintCursorActive
		? PaintMouseCursorWidget
		: DefaultMouseCursorWidget;
	if (!TargetCursorWidget)
	{
		return;
	}

	SetMouseCursorWidget(EMouseCursor::Default, TargetCursorWidget);
	DefaultMouseCursor = EMouseCursor::Default;
	CurrentMouseCursor = EMouseCursor::Default;
	UpdatePaintMouseCursorPresentation();
}

void ACustomizationPlayerController::UpdatePaintMouseCursorPresentation()
{
	if (!bIsPaintCursorActive || !PaintMouseCursorWidget)
	{
		return;
	}

	USizeBox* BrushCursorSizeBox = FindPaintCursorSizeBox();
	if (BrushCursorSizeBox)
	{
		const float SafeMinDiameter = FMath::Max(1.0f, MinPaintCursorDiameter);
		const float SafeMaxDiameter =
			FMath::Max(SafeMinDiameter, MaxPaintCursorDiameter);
		const float CursorDiameter = FMath::Clamp(
			PaintStrokeThickness * PaintCursorBrushSizeScale,
			SafeMinDiameter,
			SafeMaxDiameter);
		BrushCursorSizeBox->SetWidthOverride(CursorDiameter);
		BrushCursorSizeBox->SetHeightOverride(CursorDiameter);
	}

	if (UBorder* BrushCursorColorBorder = FindPaintCursorColorBorder())
	{
		BrushCursorColorBorder->SetBrushColor(PaintBrushColor);
	}
	if (UImage* BrushCursorColorImage = FindPaintCursorColorImage())
	{
		BrushCursorColorImage->SetColorAndOpacity(PaintBrushColor);
	}
}

USizeBox* ACustomizationPlayerController::FindPaintCursorSizeBox() const
{
	return PaintMouseCursorWidget
		? Cast<USizeBox>(
			PaintMouseCursorWidget->GetWidgetFromName(
				TEXT("BrushCursorSizeBox")))
		: nullptr;
}

UBorder* ACustomizationPlayerController::FindPaintCursorColorBorder() const
{
	return PaintMouseCursorWidget
		? Cast<UBorder>(
			PaintMouseCursorWidget->GetWidgetFromName(
				TEXT("BrushCursorColorBorder")))
		: nullptr;
}

UImage* ACustomizationPlayerController::FindPaintCursorColorImage() const
{
	return PaintMouseCursorWidget
		? Cast<UImage>(
			PaintMouseCursorWidget->GetWidgetFromName(
				TEXT("BrushCursorColorImage")))
		: nullptr;
}

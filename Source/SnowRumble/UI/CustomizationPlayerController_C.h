// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../Player/SnowRumbleCustomizationData_C.h"
#include "GameFramework/PlayerController.h"
#include "CustomizationPlayerController_C.generated.h"

class AActor;
class ASnowRumbleCharacter;
class APlayerStart;
class UAnimationAsset;
class UCanvas;
class UCanvasRenderTarget2D;
class UCustomizationWidget;

UCLASS(Blueprintable)
class SNOWRUMBLE_API ACustomizationPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** 커스터마이징 UI를 표시하고 입력을 UI/마우스 중심으로 설정한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void ShowCustomizationMenu();

	/** 메인메뉴 맵으로 돌아간다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void ReturnToMainMenu();

	/** 프리뷰 캐릭터의 몸 색상을 즉시 변경한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void SetPreviewBodyColor(FLinearColor NewBodyColor);

	/** 현재 프리뷰 커스터마이징 데이터를 로컬 저장소에 적용한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void ApplyPreviewCustomization();

	/** 프리뷰 커스터마이징 데이터를 기본값으로 되돌리고 로컬 저장소도 초기화한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void ResetPreviewCustomization();

	/** 현재 프리뷰 중인 커스터마이징 데이터를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Customization")
	FSnowRumbleCustomizationData GetPreviewCustomizationData() const;

	/** 마지막으로 완료한 드로잉 선 하나를 제거한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void UndoLastPaintStroke();

	/** 모든 드로잉 선을 지우고 빈 메쉬 상태로 되돌린다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void ResetPaintStrokes();

	/** 현재 프리뷰에 쓰는 드로잉 RenderTarget을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Customization")
	UCanvasRenderTarget2D* GetPaintRenderTarget() const;

	/** 프리뷰 캐릭터를 왼쪽으로 계속 회전하기 시작한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void StartRotatePreviewLeft();

	/** 프리뷰 캐릭터를 오른쪽으로 계속 회전하기 시작한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void StartRotatePreviewRight();

	/** 프리뷰 캐릭터 회전을 멈춘다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void StopRotatePreview();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PlayerTick(float DeltaTime) override;

	/** 커스터마이징 레벨에서 자동 생성할 WBP 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization")
	TSubclassOf<UCustomizationWidget> CustomizationWidgetClass;

	/** 레벨에 배치한 커스터마이징 카메라를 찾는 태그다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization")
	FName CustomizationCameraTag = TEXT("CustomizationCamera");

	/** 돌아가기 버튼으로 이동할 메인메뉴 URL이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization")
	FString MainMenuTravelUrl = TEXT("/Game/Maps/L_MainMenu");

	/** 커스터마이징 레벨에 배치한 프리뷰 캐릭터를 찾는 태그다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization")
	FName PreviewCharacterTag = TEXT("CustomizationPreviewCharacter");

	/** 자동 스폰이 필요할 때 사용할 프리뷰 캐릭터 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization")
	TSubclassOf<ASnowRumbleCharacter> PreviewCharacterClass;

	/** 커스터마이징 방에서 프리뷰 캐릭터에 사용할 단일 애니메이션 에셋이다. 비워두면 현재 애니메이션을 그대로 멈춘다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Preview")
	TObjectPtr<UAnimationAsset> PreviewAnimationAsset;

	/** 커스터마이징 방에서 프리뷰 캐릭터 애니메이션을 정지 상태로 둘지 정한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Preview")
	bool bPausePreviewAnimation = true;

	/** PreviewAnimationAsset을 적용할 때 고정할 재생 위치다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Preview", meta = (ClampMin = "0.0"))
	float PreviewAnimationPositionSeconds = 0.0f;

	/** 드로잉 RenderTarget 한 변의 픽셀 크기다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Paint", meta = (ClampMin = "64", ClampMax = "4096"))
	int32 PaintRenderTargetSize = 1024;

	/** UV 좌표 변화가 이 값보다 작으면 같은 선의 다음 점으로 추가하지 않는다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Paint", meta = (ClampMin = "0.0001", ClampMax = "1.0"))
	float PaintPointMinDistance = 0.0025f;

	/** RenderTarget에 그릴 선 두께다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Paint", meta = (ClampMin = "1.0", ClampMax = "256.0"))
	float PaintStrokeThickness = 12.0f;

	/** 임시 1차 브러쉬 색상이다. 이후 UI 색상 선택과 연결한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Paint")
	FLinearColor PaintBrushColor = FLinearColor::Black;

	/** 머티리얼 UV 방향에 맞춰 드로잉 RenderTarget Y축을 뒤집을지 정한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Paint")
	bool bFlipPaintUvY = false;

	/** 페인트 trace 화면 좌표에 더할 픽셀 단위 보정값이다. X가 양수면 오른쪽, Y가 양수면 아래쪽으로 trace가 이동한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Paint")
	FVector2D PaintCursorScreenOffset = FVector2D::ZeroVector;

	/** 회전 버튼을 누르고 있을 때 프리뷰 캐릭터가 초당 회전하는 각도다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|View", meta = (ClampMin = "1.0", ClampMax = "720.0"))
	float PreviewRotationSpeedDegrees = 90.0f;

private:
	/** 커스터마이징 카메라 태그를 가진 액터를 찾아 ViewTarget으로 설정한다. */
	void ApplyCustomizationCameraView();

	/** 커스터마이징 위젯 인스턴스가 없으면 생성한다. */
	UCustomizationWidget* EnsureCustomizationWidget();

	/** 현재 조종 중인 커마용 캐릭터를 반환한다. */
	ASnowRumbleCharacter* GetPreviewCharacter() const;

	/** 프리뷰 캐릭터를 찾거나 없으면 스폰한다. */
	ASnowRumbleCharacter* EnsurePreviewCharacter();

	/** 프리뷰 캐릭터 자동 스폰 위치를 찾는다. */
	FTransform GetPreviewCharacterSpawnTransform() const;

	/** 커스터마이징 방 전용 애니메이션 에셋과 정지 상태를 프리뷰 캐릭터에 적용한다. */
	void ApplyPreviewAnimationSettings();

	/** 로컬 저장소에서 커스터마이징 데이터를 읽어 프리뷰에 적용한다. */
	void LoadSavedCustomizationForPreview();

	/** 현재 프리뷰 데이터를 캐릭터 머티리얼에 적용한다. */
	void ApplyPreviewDataToCharacter();

	/** 프리뷰 캐릭터 Mesh를 마우스 trace와 UV hit가 가능한 상태로 맞춘다. */
	void ConfigurePreviewCharacterForPainting();

	/** 드로잉 RenderTarget이 없으면 생성하고 캐릭터 머티리얼에 연결한다. */
	void EnsurePaintRenderTarget();

	/** 현재 마우스 위치가 프리뷰 캐릭터 Mesh 위라면 UV를 반환한다. */
	bool GetPaintUvUnderCursor(
		FVector2D& OutPaintUv,
		FName& OutMeshComponentName,
		int32& OutMaterialIndex);

	/** UI DPI 스케일을 반영해 페인트 trace에 사용할 뷰포트 픽셀 좌표를 구한다. */
	bool GetPaintCursorScreenPosition(float& OutMouseX, float& OutMouseY);

	/** 충돌 UV를 쓸 수 없을 때 렌더 삼각형을 직접 검사해 UV를 계산한다. */
	bool FindPaintUvOnSkinnedRenderData(
		const USkeletalMeshComponent* MeshComponent,
		const FVector& TraceStart,
		const FVector& TraceEnd,
		FVector2D& OutPaintUv,
		int32& OutMaterialIndex) const;

	void ShowPaintDebugMessage(const FString& Message);

	/** 좌클릭 상태에 따라 Stroke 시작, 점 추가, 완료를 처리한다. */
	void UpdatePaintInput();

	/** 페인트 화면에서 Ctrl+Z 입력으로 마지막 완료 Stroke를 제거한다. */
	void UpdatePaintUndoInput();

	/** 누르고 있는 버튼 방향으로 프리뷰 캐릭터를 회전한다. */
	void UpdatePreviewRotation(float DeltaTime);

	void BeginPaintStroke(
		const FVector2D& PaintUv,
		FName MeshComponentName,
		int32 MaterialIndex);
	void AddPaintPoint(
		const FVector2D& PaintUv,
		FName MeshComponentName,
		int32 MaterialIndex);
	void FinishPaintStroke();
	void SyncPaintStrokesToPreviewData();
	void SavePreviewCustomizationData();
	void RedrawPaintRenderTarget();

	UFUNCTION()
	void HandlePaintCanvasUpdate(
		UCanvas* Canvas,
		int32 Width,
		int32 Height);

	void DrawStrokeToCanvas(
		UCanvas* Canvas,
		const FSnowRumblePaintStroke& Stroke,
		int32 Width,
		int32 Height) const;

	UPROPERTY(Transient)
	TObjectPtr<UCustomizationWidget> CustomizationWidget;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasRenderTarget2D> PaintRenderTarget;

	UPROPERTY(Transient)
	TObjectPtr<ASnowRumbleCharacter> CachedPreviewCharacter;

	TArray<FSnowRumblePaintStroke> PaintStrokes;
	FSnowRumblePaintStroke ActivePaintStroke;

	FSnowRumbleCustomizationData PreviewCustomizationData;

	bool bIsPaintingStroke = false;
	bool bWasPaintMouseDown = false;
	float PreviewRotationInput = 0.0f;
	double LastPaintDebugMessageTime = -1.0;
};

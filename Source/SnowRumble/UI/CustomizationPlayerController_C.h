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
class UBorder;
class UImage;
class USizeBox;
class UUserWidget;

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

	/** 언리얼 기본 컬러 피커를 열어 현재 페인트 브러시 색을 변경한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void OpenPaintBrushColorPicker();

	/** 지정한 화면 위치 왼쪽에 언리얼 기본 컬러 피커를 연다. */
	void OpenPaintBrushColorPickerOnLeft(const FVector2D& AnchorScreenPosition);

	/** 현재 페인트 브러시 색을 직접 지정한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void SetPaintBrushColor(FLinearColor NewBrushColor);

	/** 현재 페인트 브러시 색을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Customization")
	FLinearColor GetPaintBrushColor() const;

	/** 현재 브러시 색으로 BodyColor를 채운다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void FillPreviewBodyWithBrushColor();

	/** 브러시 크기 버튼을 누른 상태를 시작한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void StartAdjustPaintBrushSize();

	/** 브러시 크기 버튼을 누른 상태를 끝낸다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void StopAdjustPaintBrushSize();

	/** 마우스 휠 값으로 브러시 크기를 작게 또는 크게 조정한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void AdjustPaintBrushSizeFromWheel(float WheelDelta);

	/** 현재 페인트 브러시 크기를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Customization")
	float GetPaintBrushSize() const;

	/** 현재 프리뷰 커스터마이징 데이터를 로컬 저장소에 적용한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void ApplyPreviewCustomization();

	/** 프리뷰 커스터마이징 데이터를 기본값으로 되돌리고 로컬 저장소도 초기화한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void ResetPreviewCustomization();

	/** 현재 프리뷰 중인 커스터마이징 데이터를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Customization")
	FSnowRumbleCustomizationData GetPreviewCustomizationData() const;

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization|Hat")
	void SetPreviewHatMeshIndex(int32 NewHatMeshIndex);

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization|Hat")
	void SelectPreviousPreviewHat();

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization|Hat")
	void SelectNextPreviewHat();

	UFUNCTION(BlueprintPure, Category = "SnowRumble|Customization|Hat")
	int32 GetPreviewHatMeshIndex() const;

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

	/** 색칠하기 페이지 진입 여부에 따라 마우스 커서를 전환한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization|Cursor")
	void SetPaintCursorActive(bool bNewPaintCursorActive);

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

	/** 커스터마이징 화면에서 평소에 사용할 전역 기본 마우스 커서 위젯이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Cursor")
	TSubclassOf<UUserWidget> DefaultMouseCursorWidgetClass;

	/** 색칠하기 화면에서 사용할 원형 마우스 커서 위젯이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Cursor")
	TSubclassOf<UUserWidget> PaintMouseCursorWidgetClass;

	/** PaintMouseCursorWidget 안의 BrushCursorSizeBox 크기에 곱할 배율이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Cursor", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float PaintCursorBrushSizeScale = 1.0f;

	/** 원형 페인트 커서의 최소 지름이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Cursor", meta = (ClampMin = "1.0", ClampMax = "512.0"))
	float MinPaintCursorDiameter = 8.0f;

	/** 원형 페인트 커서의 최대 지름이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Cursor", meta = (ClampMin = "1.0", ClampMax = "1024.0"))
	float MaxPaintCursorDiameter = 256.0f;

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

	/** UV 좌표 변화가 이 값보다 크면 UV seam으로 보고 현재 선을 끊는다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Paint", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PaintPointMaxDistance = 0.08f;

	/** RenderTarget에 그릴 선 두께다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Paint", meta = (ClampMin = "1.0", ClampMax = "256.0"))
	float PaintStrokeThickness = 12.0f;

	/** 현재 브러쉬 색상이다. 컬러 피커와 전체 칠하기 버튼이 이 값을 사용한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Paint")
	FLinearColor PaintBrushColor = FLinearColor::Black;

	/** 브러시 색상 창을 버튼 왼쪽에 띄울 때 버튼과 창 사이에 둘 화면 픽셀 간격이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Paint", meta = (ClampMin = "0.0", ClampMax = "128.0"))
	float PaintBrushColorPickerLeftPadding = 12.0f;

	/** 브러시 크기 버튼을 누른 상태에서 휠 한 칸마다 바뀌는 크기다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Paint", meta = (ClampMin = "0.1", ClampMax = "128.0"))
	float PaintBrushWheelStep = 2.0f;

	/** 브러시 크기의 최소값이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Paint", meta = (ClampMin = "1.0", ClampMax = "256.0"))
	float MinPaintBrushSize = 1.0f;

	/** 브러시 크기의 최대값이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Paint", meta = (ClampMin = "1.0", ClampMax = "256.0"))
	float MaxPaintBrushSize = 96.0f;

	/** 머티리얼 UV 방향에 맞춰 드로잉 RenderTarget Y축을 뒤집을지 정한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Paint")
	bool bFlipPaintUvY = false;

	/** 페인트 trace 화면 좌표에 더할 픽셀 단위 보정값이다. X가 양수면 오른쪽, Y가 양수면 아래쪽으로 trace가 이동한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Paint")
	FVector2D PaintCursorScreenOffset = FVector2D::ZeroVector;

	/** 소프트웨어 페인트 커서 hotspot이 고정으로 어긋난 경우 아래 고정 보정값을 trace에 반영할지 정한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Paint")
	bool bUsePaintCursorCenterTraceOffset = true;

	/** 브러시 크기와 무관하게 trace에 더할 고정 커서 중심 보정값이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Paint")
	FVector2D PaintCursorCenterTraceOffset = FVector2D::ZeroVector;

	/** 페인트를 허용할 머티리얼 슬롯이다. -1이면 모든 슬롯을 허용한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Paint")
	int32 PaintAllowedMaterialIndex = 0;

	/** 페인트 hit 컴포넌트, 머티리얼 슬롯, UV를 화면 디버그로 표시할지 정한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Paint")
	bool bShowPaintHitDebug = false;

	/** 회전 버튼을 누르고 있을 때 프리뷰 캐릭터가 초당 회전하는 각도다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|View", meta = (ClampMin = "1.0", ClampMax = "720.0"))
	float PreviewRotationSpeedDegrees = 90.0f;

private:
	/** 커스터마이징 카메라 태그를 가진 액터를 찾아 ViewTarget으로 설정한다. */
	void ApplyCustomizationCameraView();

	/** 커스터마이징 위젯 인스턴스가 없으면 생성한다. */
	UCustomizationWidget* EnsureCustomizationWidget();

	/** BP에 지정된 기본/페인트 커서 위젯을 생성하고 소프트웨어 커서로 등록한다. */
	void EnsureMouseCursorWidgets();

	/** 현재 페이지와 커서 슬롯에 맞춰 표시할 소프트웨어 커서를 적용한다. */
	void ApplyCurrentMouseCursorWidget();

	/** 커스터마이징 화면에서는 마우스/UI 입력만 쓰도록 Pawn 이동과 시점 입력을 잠근다. */
	void ApplyCustomizationInputLock();

	/** 원형 페인트 커서 위젯의 표시 크기와 색을 현재 브러시에 맞춘다. */
	void UpdatePaintMouseCursorPresentation();

	USizeBox* FindPaintCursorSizeBox() const;
	UBorder* FindPaintCursorColorBorder() const;
	UImage* FindPaintCursorColorImage() const;

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

	/** 현재 브러시 크기 기준으로 원형 커서 지름을 계산한다. */
	float GetPaintCursorDiameter() const;

	/** 충돌 UV를 쓸 수 없을 때 렌더 삼각형을 직접 검사해 UV를 계산한다. */
	bool FindPaintUvOnSkinnedRenderData(
		const USkeletalMeshComponent* MeshComponent,
		const FVector& TraceStart,
		const FVector& TraceEnd,
		FVector2D& OutPaintUv,
		int32& OutMaterialIndex) const;

	bool IsPaintMaterialIndexAllowed(int32 MaterialIndex) const;
	void ShowPaintDebugMessage(const FString& Message);
	void HandlePaintBrushColorPicked(FLinearColor NewBrushColor);

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
	TObjectPtr<UUserWidget> DefaultMouseCursorWidget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PaintMouseCursorWidget;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasRenderTarget2D> PaintRenderTarget;

	UPROPERTY(Transient)
	TObjectPtr<ASnowRumbleCharacter> CachedPreviewCharacter;

	TArray<FSnowRumblePaintStroke> PaintStrokes;
	FSnowRumblePaintStroke ActivePaintStroke;

	FSnowRumbleCustomizationData PreviewCustomizationData;

	bool bIsPaintingStroke = false;
	bool bWasPaintMouseDown = false;
	bool bIsAdjustingPaintBrushSize = false;
	bool bIsPaintCursorActive = false;
	float PreviewRotationInput = 0.0f;
	double LastPaintDebugMessageTime = -1.0;
};

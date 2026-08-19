// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SnowTrailRenderTargetManager_C.generated.h"

class ASnowRumbleCharacter;
class UCanvas;
class UCanvasRenderTarget2D;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UPrimitiveComponent;

USTRUCT(BlueprintType)
struct FSnowTrailStampData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Snow Trail")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Snow Trail")
	FVector WorldNormal = FVector::UpVector;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Snow Trail")
	FVector2D TrailUV = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Snow Trail")
	float RadiusWorld = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Snow Trail")
	float RadiusPixels = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Snow Trail")
	FName FootSocketName = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Snow Trail")
	TObjectPtr<ASnowRumbleCharacter> SourceCharacter;
};

UCLASS()
class SNOWRUMBLE_API ASnowTrailRenderTargetManager : public AActor
{
	GENERATED_BODY()

public:
	ASnowTrailRenderTargetManager();

	virtual void BeginPlay() override;

	/** 월드에서 사용할 첫 번째 눈길 RenderTarget 매니저를 찾는다. */
	static ASnowTrailRenderTargetManager* FindSnowTrailManager(
		const UObject* WorldContextObject);

	/** 눈길 마스크 RenderTarget이 없으면 생성하고 반환한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Snow Trail")
	UCanvasRenderTarget2D* EnsureSnowTrailRenderTarget();

	/** 지형 머티리얼이 읽을 눈길 마스크 RenderTarget을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snow Trail")
	UCanvasRenderTarget2D* GetSnowTrailRenderTarget() const;

	/** 눈길 RenderTarget이 표현하는 월드 중심 좌표를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snow Trail")
	FVector2D GetTrailWorldCenter() const;

	/** 눈길 RenderTarget 한 변이 덮는 월드 거리(cm)를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snow Trail")
	float GetTrailWorldSize() const;

	/** 월드 좌표를 눈길 RenderTarget UV로 변환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snow Trail")
	bool ConvertWorldLocationToTrailUV(
		FVector WorldLocation,
		FVector2D& OutTrailUV) const;

	/** 서버에서 복제된 발 위치를 현재 클라이언트의 눈길 RenderTarget stamp로 전달한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Snow Trail")
	bool StampSnowTrailAtWorldLocation(
		FVector WorldLocation,
		FVector WorldNormal,
		float RadiusWorld,
		FName FootSocketName,
		ASnowRumbleCharacter* SourceCharacter);

	/** 현재까지 누적된 눈길 stamp 목록을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snow Trail")
	TArray<FSnowTrailStampData> GetSnowTrailStamps() const;

	/** 현재 RenderTarget/월드 범위 값을 지정된 지형 머티리얼에 다시 적용한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Snow Trail|Material")
	void RefreshSnowTrailMaterialParameters();

	/** 눈길 stamp 실패 원인을 Output Log에 출력할지 정한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snow Trail|Debug")
	bool ShouldLogSnowTrailDebug() const;

protected:
	/** 눈길 마스크 RenderTarget을 에디터 지정 없이 런타임에 만들지 여부다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail")
	bool bAutoCreateRenderTarget = true;

	/** 게임 시작 시 사전 지정 RenderTarget의 이전 픽셀을 지운다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail")
	bool bClearRenderTargetOnBeginPlay = true;

	/** 런타임 생성 RenderTarget 한 변의 픽셀 크기다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail", meta = (ClampMin = "64", ClampMax = "4096"))
	int32 RenderTargetSize = 2048;

	/** 눈길 RenderTarget이 덮는 월드 중심 XY 좌표다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail")
	FVector2D TrailWorldCenter = FVector2D::ZeroVector;

	/** 눈길 RenderTarget 한 변이 덮는 월드 거리(cm)다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail", meta = (ClampMin = "100.0"))
	float TrailWorldSize = 20000.0f;

	/** 캐릭터에서 반지름을 따로 넘기지 않았을 때 사용할 stamp 반지름(cm)이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail", meta = (ClampMin = "1.0"))
	float DefaultStampRadiusWorld = 38.0f;

	/** 같은 캐릭터의 이전 stamp와 현재 stamp 사이를 채워 눈길처럼 연결할지 정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail|Interpolation")
	bool bInterpolateTrailBetweenStamps = true;

	/** 보간 stamp 사이의 목표 간격(cm)이다. 작을수록 촘촘하지만 비용이 늘어난다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail|Interpolation", meta = (ClampMin = "1.0"))
	float InterpolatedStampSpacingWorld = 42.0f;

	/** 이전 stamp와 현재 stamp 거리가 이 값보다 크면 순간이동으로 보고 연결하지 않는다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail|Interpolation", meta = (ClampMin = "1.0"))
	float MaxInterpolatedStampDistanceWorld = 220.0f;

	/** RenderTarget 갱신 중 C++가 stamp material을 직접 그릴지 정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail|Draw")
	bool bDrawStampsInCpp = true;

	/** C++가 Canvas에 직접 그릴 발자국/눈길 stamp material이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail|Draw")
	TObjectPtr<UMaterialInterface> SnowTrailStampMaterial;

	/** 월드 반지름 변환값이 너무 작을 때 RT 확인과 머티리얼 마스크용으로 보장할 최소 stamp 반지름(px)이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail|Draw", meta = (ClampMin = "1.0"))
	float MinimumStampRadiusPixels = 10.0f;

	/** RenderTarget 갱신 때 다시 그릴 최대 stamp 개수다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail", meta = (ClampMin = "1"))
	int32 MaxStoredStampCount = 4096;

	/** 에디터에서 미리 만든 RenderTarget을 지정하면 런타임 생성 대신 사용한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail")
	TObjectPtr<UCanvasRenderTarget2D> SnowTrailRenderTarget;

	/** BeginPlay 때 눈길 RenderTarget 파라미터를 자동으로 넣을 액터 목록이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail|Material")
	TArray<TObjectPtr<AActor>> SnowTrailMaterialActors;

	/** BeginPlay 때 눈길 RenderTarget 파라미터를 자동으로 넣을 지형/메쉬 컴포넌트 목록이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail|Material")
	TArray<TObjectPtr<UPrimitiveComponent>> SnowTrailMaterialComponents;

	/** 지정한 컴포넌트 머티리얼에 RenderTarget/월드 범위 파라미터를 자동 적용할지 정한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail|Material")
	bool bApplySnowTrailParametersToMaterials = true;

	/** SnowSurface 태그가 붙은 바닥 액터에도 RenderTarget/월드 범위 파라미터를 자동 적용한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail|Material")
	bool bAutoApplyToSnowSurfaceTaggedActors = true;

	/** 자동 머티리얼 적용 대상으로 찾을 액터 태그다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail|Material")
	FName SnowTrailMaterialAutoApplyActorTag = TEXT("SnowSurface");

	/** 눈길 마스크 RenderTarget을 받는 texture parameter 이름이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail|Material")
	FName SnowTrailRenderTargetParameterName = TEXT("SnowTrailMask");

	/** 눈길 RenderTarget이 덮는 월드 중심 XY를 받는 vector parameter 이름이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail|Material")
	FName SnowTrailWorldCenterParameterName = TEXT("Position");

	/** 눈길 RenderTarget 한 변의 월드 거리(cm)를 받는 scalar parameter 이름이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail|Material")
	FName SnowTrailWorldSizeParameterName = TEXT("TrailWorldSize");

	/** 현재 RenderTarget에 다시 그릴 누적 stamp 목록이다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SnowRumble|Snow Trail")
	TArray<FSnowTrailStampData> SnowTrailStamps;

	/** PIE 진단용 눈길 stamp 로그를 출력한다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snow Trail|Debug")
	bool bLogSnowTrailDebug = false;

	/** Blueprint가 stamp 요청 시 지형 머티리얼 파라미터 갱신 같은 부가 작업을 수행한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Snow Trail")
	void OnSnowTrailStampRequested(
		UCanvasRenderTarget2D* TargetRenderTarget,
		FVector WorldLocation,
		FVector WorldNormal,
		FVector2D TrailUV,
		float RadiusWorld,
		float RadiusPixels,
		FName FootSocketName,
		ASnowRumbleCharacter* SourceCharacter);

	/** BeginPlay 이후 지형 머티리얼 인스턴스에 RenderTarget과 월드 범위 파라미터를 다시 연결한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Snow Trail")
	void OnSnowTrailRenderTargetReady(
		UCanvasRenderTarget2D* TargetRenderTarget,
		FVector2D WorldCenter,
		float WorldSize);

	/** RenderTarget 갱신 중 누적 stamp 하나를 Canvas에 그린다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Snow Trail")
	void OnDrawSnowTrailStamp(
		UCanvas* Canvas,
		int32 Width,
		int32 Height,
		const FSnowTrailStampData& StampData);

private:
	/** 실제 stamp 하나를 누적 배열에 추가한다. */
	bool AddSnowTrailStamp(
		FVector WorldLocation,
		FVector WorldNormal,
		float RadiusWorld,
		FName FootSocketName,
		ASnowRumbleCharacter* SourceCharacter,
		FSnowTrailStampData& OutStampData);

	/** 게임 시작 시 RenderTarget을 런타임 상태에 맞게 준비한다. */
	void InitializeSnowTrailRenderTargetForPlay();

	/** 에디터 지정 또는 런타임 생성 RenderTarget에 Canvas 갱신 콜백을 연결한다. */
	void BindSnowTrailRenderTargetUpdate();

	/** 지정한 지형/메쉬 머티리얼에 RenderTarget, 중심, 크기 파라미터를 반영한다. */
	void ApplySnowTrailMaterialParameters(
		UCanvasRenderTarget2D* TargetRenderTarget);

	UFUNCTION()
	void HandleSnowTrailCanvasUpdate(
		UCanvas* Canvas,
		int32 Width,
		int32 Height);

	/** C++ 기본 경로로 stamp material을 Canvas에 그린다. */
	void DrawSnowTrailStampInCpp(
		UCanvas* Canvas,
		int32 Width,
		int32 Height,
		const FSnowTrailStampData& StampData) const;

	TMap<TWeakObjectPtr<ASnowRumbleCharacter>, FVector> LastStampLocationByCharacter;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> AppliedSnowTrailMaterials;
};

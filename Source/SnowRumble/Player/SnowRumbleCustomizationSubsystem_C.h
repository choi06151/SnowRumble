// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SnowRumbleCustomizationData_C.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SnowRumbleCustomizationSubsystem_C.generated.h"

UCLASS()
class SNOWRUMBLE_API USnowRumbleCustomizationSubsystem
	: public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 현재 로컬 플레이어가 선택한 커스터마이징 데이터를 저장한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void SetCustomizationData(const FSnowRumbleCustomizationData& NewData);

	/** 현재 로컬 플레이어가 선택한 커스터마이징 데이터를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Customization")
	FSnowRumbleCustomizationData GetCustomizationData() const;

	/** 몸 색상만 변경한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void SetBodyColor(FLinearColor NewBodyColor);

	/** 현재 몸 색상을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Customization")
	FLinearColor GetBodyColor() const;

	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void SetHatMeshIndex(int32 NewHatMeshIndex);

	UFUNCTION(BlueprintPure, Category = "SnowRumble|Customization")
	int32 GetHatMeshIndex() const;

	/** 커스터마이징 데이터를 기본값으로 되돌린다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void ResetCustomizationData();

	/** 프로젝트 기본 커스터마이징 값을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Customization")
	static FSnowRumbleCustomizationData GetDefaultCustomizationData();

	/** 서버나 머티리얼에 넘기기 전에 값 범위를 정리한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Customization")
	static FSnowRumbleCustomizationData SanitizeCustomizationData(
		const FSnowRumbleCustomizationData& Data);

private:
	FSnowRumbleCustomizationData CustomizationData;
};

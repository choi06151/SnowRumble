// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LocalPlayerIdentitySubsystem_C.generated.h"

UCLASS()
class SNOWRUMBLE_API ULocalPlayerIdentitySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** 메인메뉴에서 입력한 로컬 플레이어 닉네임을 저장한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Identity")
	void SetDesiredPlayerName(const FString& NewName);

	/** 닉네임이 적합하면 저장하고 true를 반환한다. 부적합하면 기존 이름을 유지한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Identity")
	bool TrySetDesiredPlayerName(const FString& NewName);

	/** 현재 로컬 플레이어가 사용하려는 닉네임을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Identity")
	FString GetDesiredPlayerName() const;

	/** 비어 있지 않은 닉네임을 저장하고 있는지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Identity")
	bool HasDesiredPlayerName() const;

	/** 닉네임 필터를 통과하는지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Identity")
	static bool IsPlayerNameAllowed(const FString& NewName);

	/** 닉네임 길이와 공백을 정리한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Identity")
	static FString SanitizePlayerName(const FString& NewName);

private:
	/** 처음 들어온 플레이어에게 보여줄 귀엽고 가벼운 기본 닉네임을 만든다. */
	FString GenerateDefaultPlayerName() const;

	/** 기본 닉네임 후보를 한 곳에서 관리한다. */
	static const TArray<FString>& GetDefaultPlayerNameCandidates();

	/** 금칙어 검사를 위해 닉네임을 비교용 문자열로 정규화한다. */
	static FString NormalizePlayerNameForFilter(const FString& NewName);

	/** 부적합한 닉네임 조각 목록이다. */
	static const TArray<FString>& GetDisallowedPlayerNameFragments();

	FString DesiredPlayerName;
};

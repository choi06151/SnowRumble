// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SnowballThrowRelease_C.generated.h"

UCLASS(meta = (DisplayName = "Snow Rumble Snowball Throw Release"))
class SNOWRUMBLE_API UAnimNotify_SnowballThrowRelease : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_SnowballThrowRelease();

	/** 던지기 몽타주 타임라인에서 실제 눈덩이가 손을 떠나는 시점에 호출된다. */
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	/** AnimNotify 목록에 표시할 이름을 반환한다. */
	virtual FString GetNotifyName_Implementation() const override;
};

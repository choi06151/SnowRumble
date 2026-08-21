// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SnowFootstep_C.generated.h"

UCLASS(meta = (DisplayName = "Snow Rumble Snow Footstep"))
class SNOWRUMBLE_API UAnimNotify_SnowFootstep : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_SnowFootstep();

	/** 애니메이션 타임라인에서 발 착지 시점에 눈 밟힘 효과를 캐릭터에 요청한다. */
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	/** AnimNotify 목록에 표시할 이름을 반환한다. */
	virtual FString GetNotifyName_Implementation() const override;

protected:
	/** 눈 밟힘 위치로 사용할 발 socket 이름이다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SnowRumble|Footstep")
	FName FootSocketName = TEXT("foot_l");
};

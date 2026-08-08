// Copyright Epic Games, Inc. All Rights Reserved.

#include "LocalPlayerIdentitySubsystem_C.h"

void ULocalPlayerIdentitySubsystem::Initialize(
	FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	DesiredPlayerName = GenerateDefaultPlayerName();
}

void ULocalPlayerIdentitySubsystem::SetDesiredPlayerName(
	const FString& NewName)
{
	DesiredPlayerName = SanitizePlayerName(NewName);
}

FString ULocalPlayerIdentitySubsystem::GetDesiredPlayerName() const
{
	return DesiredPlayerName;
}

bool ULocalPlayerIdentitySubsystem::HasDesiredPlayerName() const
{
	return !DesiredPlayerName.IsEmpty();
}

FString ULocalPlayerIdentitySubsystem::GenerateDefaultPlayerName() const
{
	const TArray<FString>& Candidates = GetDefaultPlayerNameCandidates();
	if (Candidates.IsEmpty())
	{
		return TEXT("귀여운 눈사람");
	}

	return Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
}

const TArray<FString>&
ULocalPlayerIdentitySubsystem::GetDefaultPlayerNameCandidates()
{
	static const TArray<FString> Candidates = {
		TEXT("열정적 난투가"),
		TEXT("귀여운 눈사람"),
		TEXT("말랑한 눈덩이"),
		TEXT("소심한 눈삽"),
		TEXT("씩씩한 핫초코"),
		TEXT("하찮은 장갑"),
		TEXT("졸린 눈오리"),
		TEXT("뽀송한 패딩"),
		TEXT("당당한 붕어빵"),
		TEXT("얼렁뚱땅 챔피언"),
		TEXT("눈밭의 말썽꾼"),
		TEXT("작고 강한 눈송이"),
		TEXT("우당탕 스노볼"),
		TEXT("따끈한 핫팩"),
		TEXT("느긋한 눈사람"),
		TEXT("반짝이는 얼음콩")
	};

	return Candidates;
}

FString ULocalPlayerIdentitySubsystem::SanitizePlayerName(
	const FString& NewName) const
{
	FString SanitizedName = NewName.TrimStartAndEnd();
	constexpr int32 MaximumNameLength = 16;
	if (SanitizedName.Len() > MaximumNameLength)
	{
		SanitizedName.LeftInline(MaximumNameLength);
	}

	return SanitizedName;
}

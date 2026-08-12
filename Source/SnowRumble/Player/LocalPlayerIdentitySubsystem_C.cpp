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
	TrySetDesiredPlayerName(NewName);
}

bool ULocalPlayerIdentitySubsystem::TrySetDesiredPlayerName(
	const FString& NewName)
{
	if (!IsPlayerNameAllowed(NewName))
	{
		return false;
	}

	DesiredPlayerName = SanitizePlayerName(NewName);
	return true;
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

bool ULocalPlayerIdentitySubsystem::IsPlayerNameAllowed(
	const FString& NewName)
{
	FString SanitizedName = NewName.TrimStartAndEnd();
	if (SanitizedName.IsEmpty())
	{
		return false;
	}

	const FString NormalizedName =
		NormalizePlayerNameForFilter(SanitizedName);
	for (const FString& DisallowedFragment :
		GetDisallowedPlayerNameFragments())
	{
		if (!DisallowedFragment.IsEmpty()
			&& NormalizedName.Contains(DisallowedFragment))
		{
			return false;
		}
	}

	return true;
}

FString ULocalPlayerIdentitySubsystem::SanitizePlayerName(
	const FString& NewName)
{
	FString SanitizedName = NewName.TrimStartAndEnd();
	constexpr int32 MaximumNameLength = 16;
	if (SanitizedName.Len() > MaximumNameLength)
	{
		SanitizedName.LeftInline(MaximumNameLength);
	}

	return SanitizedName;
}

FString ULocalPlayerIdentitySubsystem::NormalizePlayerNameForFilter(
	const FString& NewName)
{
	FString NormalizedName;
	for (const TCHAR Character : NewName.ToLower())
	{
		if (!FChar::IsWhitespace(Character)
			&& Character != TEXT('_')
			&& Character != TEXT('-')
			&& Character != TEXT('.'))
		{
			NormalizedName.AppendChar(Character);
		}
	}
	return NormalizedName;
}

const TArray<FString>&
ULocalPlayerIdentitySubsystem::GetDisallowedPlayerNameFragments()
{
	static const TArray<FString> DisallowedFragments = {
		TEXT("시발"),
		TEXT("씨발"),
		TEXT("ㅅㅂ"),
		TEXT("ㅆㅂ"),
		TEXT("병신"),
		TEXT("ㅂㅅ"),
		TEXT("개새"),
		TEXT("좆"),
		TEXT("존나"),
		TEXT("꺼져"),
		TEXT("죽어"),
		TEXT("fuck"),
		TEXT("shit"),
		TEXT("bitch"),
		TEXT("asshole"),
		TEXT("cunt"),
		TEXT("nigger"),
		TEXT("nigga")
	};

	return DisallowedFragments;
}

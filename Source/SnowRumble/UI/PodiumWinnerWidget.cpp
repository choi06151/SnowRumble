// Copyright Epic Games, Inc. All Rights Reserved.

#include "PodiumWinnerWidget.h"

#include "Components/TextBlock.h"

namespace
{
FText BuildWinningTeamText(ESnowRumbleTeam WinningTeam)
{
	switch (WinningTeam)
	{
	case ESnowRumbleTeam::Red:
		return NSLOCTEXT("SnowRumble", "PodiumWinnerRedTeam", "빨강 팀 우승");
	case ESnowRumbleTeam::Sky:
		return NSLOCTEXT("SnowRumble", "PodiumWinnerSkyTeam", "하늘 팀 우승");
	case ESnowRumbleTeam::Green:
		return NSLOCTEXT("SnowRumble", "PodiumWinnerGreenTeam", "초록 팀 우승");
	case ESnowRumbleTeam::Yellow:
		return NSLOCTEXT("SnowRumble", "PodiumWinnerYellowTeam", "노랑 팀 우승");
	case ESnowRumbleTeam::Purple:
		return NSLOCTEXT("SnowRumble", "PodiumWinnerPurpleTeam", "보라 팀 우승");
	case ESnowRumbleTeam::Pink:
		return NSLOCTEXT("SnowRumble", "PodiumWinnerPinkTeam", "분홍 팀 우승");
	case ESnowRumbleTeam::Blue:
		return NSLOCTEXT("SnowRumble", "PodiumWinnerBlueTeam", "파랑 팀 우승");
	case ESnowRumbleTeam::White:
		return NSLOCTEXT("SnowRumble", "PodiumWinnerWhiteTeam", "하양 팀 우승");
	case ESnowRumbleTeam::None:
	default:
		return NSLOCTEXT("SnowRumble", "PodiumWinnerNoTeam", "우승 팀 없음");
	}
}

}

void UPodiumWinnerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (WinningTeamText && !CachedWinningTeamText.IsEmpty())
	{
		WinningTeamText->SetText(CachedWinningTeamText);
	}
	if (SubtitleText && !CachedSubtitleText.IsEmpty())
	{
		SubtitleText->SetText(CachedSubtitleText);
	}
}

void UPodiumWinnerWidget::SetWinnerPresentation(
	ESnowRumbleTeam WinningTeam,
	const FText& Subtitle)
{
	CachedWinningTeam = WinningTeam;
	CachedWinningTeamText = BuildWinningTeamText(WinningTeam);
	CachedSubtitleText = Subtitle;

	if (WinningTeamText)
	{
		WinningTeamText->SetText(CachedWinningTeamText);
	}
	if (SubtitleText)
	{
		SubtitleText->SetText(CachedSubtitleText);
	}

	OnWinnerPresentationUpdated();
}

void UPodiumWinnerWidget::SetSubtitleText(const FText& NewSubtitleText)
{
	CachedSubtitleText = NewSubtitleText;

	if (SubtitleText)
	{
		SubtitleText->SetText(CachedSubtitleText);
	}
}

ESnowRumbleTeam UPodiumWinnerWidget::GetWinningTeam() const
{
	return CachedWinningTeam;
}

FText UPodiumWinnerWidget::GetWinningTeamText() const
{
	return CachedWinningTeamText;
}

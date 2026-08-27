// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowmanModeGameState_K.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "SnowRumblePlayerState.h"
#include "Net/UnrealNetwork.h"

void USnowmanModeResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ResultText)
	{
		ResultText->SetText(CurrentResultText);
		return;
	}

	if (!WidgetTree)
	{
		return;
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(
		UCanvasPanel::StaticClass(),
		TEXT("SnowmanModeResultRoot"));
	UBorder* ResultBorder = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(),
		TEXT("SnowmanModeResultBorder"));
	ResultText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(),
		TEXT("ResultText"));
	if (!RootCanvas || !ResultBorder || !ResultText)
	{
		return;
	}

	ResultBorder->SetPadding(FMargin(32.0f));
	ResultBorder->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.78f));
	ResultText->SetJustification(ETextJustify::Center);
	ResultText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	FSlateFontInfo ResultFont = ResultText->GetFont();
	ResultFont.Size = 36;
	ResultText->SetFont(ResultFont);
	ResultText->SetText(CurrentResultText);
	ResultBorder->SetContent(ResultText);

	UCanvasPanelSlot* BorderSlot = RootCanvas->AddChildToCanvas(ResultBorder);
	if (BorderSlot)
	{
		BorderSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		BorderSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		BorderSlot->SetAutoSize(true);
		BorderSlot->SetPosition(FVector2D::ZeroVector);
	}
	WidgetTree->RootWidget = RootCanvas;
}

void USnowmanModeResultWidget::SetResultText(const FText& InResultText)
{
	CurrentResultText = InResultText;
	if (ResultText)
	{
		ResultText->SetText(InResultText);
	}
}

ASnowmanModeGameState::ASnowmanModeGameState()
{
	ResultWidgetClass = USnowmanModeResultWidget::StaticClass();
}

void ASnowmanModeGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveSnowmanModeResultWidget();

	Super::EndPlay(EndPlayReason);
}

void ASnowmanModeGameState::StartSnowmanModeCountdownFromServer(
	float CountdownSeconds)
{
	if (!HasAuthority())
	{
		return;
	}

	SnowmanModeStartCountdownSeconds = FMath::Max(0.0f, CountdownSeconds);
	SnowmanModeMatchStartServerTime =
		GetServerWorldTimeSeconds() + SnowmanModeStartCountdownSeconds;
	bSnowmanModeCountdownActive = true;
	bSnowmanModeTimerActive = false;
	ForceNetUpdate();
}

void ASnowmanModeGameState::StartSnowmanModeTimerFromServer(
	float InTimeLimitSeconds)
{
	if (!HasAuthority())
	{
		return;
	}

	SnowmanModeTimeLimitSeconds = FMath::Max(0.0f, InTimeLimitSeconds);
	SnowmanModeStartServerTime = GetServerWorldTimeSeconds();
	bSnowmanModeTimerActive = true;
	bSnowmanModeEnded = false;
	SnowmanModeResult = ESnowmanModeResult::None;
	RemoveSnowmanModeResultWidget();
	ForceNetUpdate();
}

void ASnowmanModeGameState::ResetSnowmanModePlayersFromServer(
	const TArray<ASnowRumblePlayerState*>& PlayerStates)
{
	if (!HasAuthority())
	{
		return;
	}

	SnowmanModePlayerEntries.Reset();
	for (ASnowRumblePlayerState* PlayerState : PlayerStates)
	{
		if (!PlayerState)
		{
			continue;
		}

		FSnowmanModePlayerEntry NewEntry;
		NewEntry.PlayerState = PlayerState;
		SnowmanModePlayerEntries.Add(NewEntry);
	}

	ForceNetUpdate();
}

void ASnowmanModeGameState::SetSnowmanPlayerFromServer(
	ASnowRumblePlayerState* PlayerState)
{
	if (!HasAuthority() || !PlayerState)
	{
		return;
	}

	const int32 EntryIndex = FindSnowmanModePlayerEntryIndex(PlayerState);
	if (EntryIndex == INDEX_NONE)
	{
		FSnowmanModePlayerEntry NewEntry;
		NewEntry.PlayerState = PlayerState;
		NewEntry.Role = ESnowmanModePlayerRole::Snowman;
		SnowmanModePlayerEntries.Add(NewEntry);
	}
	else
	{
		SnowmanModePlayerEntries[EntryIndex].Role =
			ESnowmanModePlayerRole::Snowman;
		SnowmanModePlayerEntries[EntryIndex].InfectionCompleteServerTime = 0.0f;
	}

	ForceNetUpdate();
}

bool ASnowmanModeGameState::StartInfectionPendingFromServer(
	ASnowRumblePlayerState* PlayerState,
	float InfectionDelaySeconds)
{
	if (!HasAuthority() || !PlayerState)
	{
		return false;
	}

	const int32 EntryIndex = FindSnowmanModePlayerEntryIndex(PlayerState);
	if (EntryIndex == INDEX_NONE)
	{
		return false;
	}

	FSnowmanModePlayerEntry& Entry = SnowmanModePlayerEntries[EntryIndex];
	if (Entry.Role != ESnowmanModePlayerRole::Normal)
	{
		return false;
	}

	Entry.Role = ESnowmanModePlayerRole::InfectionPending;
	Entry.InfectionCompleteServerTime =
		GetServerWorldTimeSeconds() + FMath::Max(0.0f, InfectionDelaySeconds);
	ForceNetUpdate();
	return true;
}

void ASnowmanModeGameState::CompleteInfectionFromServer(
	ASnowRumblePlayerState* PlayerState)
{
	SetSnowmanPlayerFromServer(PlayerState);
}

bool ASnowmanModeGameState::CancelControllerlessPendingFromServer(
	ASnowRumblePlayerState* PlayerState)
{
	if (!HasAuthority() || !PlayerState)
	{
		return false;
	}

	const int32 EntryIndex = FindSnowmanModePlayerEntryIndex(PlayerState);
	if (EntryIndex == INDEX_NONE
		|| SnowmanModePlayerEntries[EntryIndex].Role
			!= ESnowmanModePlayerRole::InfectionPending)
	{
		return false;
	}

	SnowmanModePlayerEntries[EntryIndex].Role = ESnowmanModePlayerRole::Normal;
	SnowmanModePlayerEntries[EntryIndex].InfectionCompleteServerTime = 0.0f;
	ForceNetUpdate();
	return true;
}

void ASnowmanModeGameState::EndSnowmanModeFromServer(
	ESnowmanModeResult Result)
{
	if (!HasAuthority()
		|| bSnowmanModeEnded
		|| Result == ESnowmanModeResult::None)
	{
		return;
	}

	bSnowmanModeEnded = true;
	SnowmanModeResult = Result;
	bSnowmanModeTimerActive = false;
	bSnowmanModeCountdownActive = false;
	ShowSnowmanModeResultWidget();
	ForceNetUpdate();
}

bool ASnowmanModeGameState::IsSnowmanModeInputLocked() const
{
	return bSnowmanModeEnded
		|| (!bSnowmanModeTimerActive
		&& (!bSnowmanModeCountdownActive
			|| GetSecondsUntilSnowmanModeStart() > 0.0f));
}

bool ASnowmanModeGameState::IsSnowmanModeEnded() const
{
	return bSnowmanModeEnded;
}

ESnowmanModeResult ASnowmanModeGameState::GetSnowmanModeResult() const
{
	return SnowmanModeResult;
}

FText ASnowmanModeGameState::GetSnowmanModeResultText() const
{
	switch (SnowmanModeResult)
	{
	case ESnowmanModeResult::SnowmanVictory:
		return NSLOCTEXT(
			"SnowRumble",
			"SnowmanModeSnowmanVictory",
			"눈사람 승리! 모두 눈사람이 되었습니다");
	case ESnowmanModeResult::SurvivorVictory:
		return NSLOCTEXT(
			"SnowRumble",
			"SnowmanModeSurvivorVictory",
			"생존자 승리! 제한시간을 버텼습니다");
	default:
		return FText::GetEmpty();
	}
}

bool ASnowmanModeGameState::ShouldShowSnowmanModeStartCountdown() const
{
	if (!bSnowmanModeCountdownActive)
	{
		return false;
	}

	const float SecondsUntilStart = GetSecondsUntilSnowmanModeStart();
	return SecondsUntilStart > -1.0f;
}

FText ASnowmanModeGameState::GetSnowmanModeStartCountdownText() const
{
	const float SecondsUntilStart = GetSecondsUntilSnowmanModeStart();
	if (SecondsUntilStart <= 0.0f)
	{
		return NSLOCTEXT("SnowRumble", "SnowmanModeStartCountdownGo", "시작!");
	}

	const int32 DisplaySeconds =
		FMath::Clamp(FMath::CeilToInt(SecondsUntilStart), 1, 3);
	return FText::AsNumber(DisplaySeconds);
}

bool ASnowmanModeGameState::IsSnowmanModeTimerActive() const
{
	return bSnowmanModeTimerActive;
}

float ASnowmanModeGameState::GetSnowmanModeTimeLimitSeconds() const
{
	return SnowmanModeTimeLimitSeconds;
}

float ASnowmanModeGameState::GetSnowmanModeRemainingSeconds() const
{
	if (!bSnowmanModeTimerActive)
	{
		return SnowmanModeTimeLimitSeconds;
	}

	const float ElapsedSeconds =
		GetServerWorldTimeSeconds() - SnowmanModeStartServerTime;
	return FMath::Max(0.0f, SnowmanModeTimeLimitSeconds - ElapsedSeconds);
}

float ASnowmanModeGameState::GetSnowmanModeElapsedSeconds() const
{
	if (!bSnowmanModeTimerActive)
	{
		return 0.0f;
	}

	return FMath::Max(
		0.0f,
		GetServerWorldTimeSeconds() - SnowmanModeStartServerTime);
}

FText ASnowmanModeGameState::GetSnowmanModeRemainingTimeText() const
{
	return FormatSecondsAsClock(GetSnowmanModeRemainingSeconds());
}

FText ASnowmanModeGameState::GetSnowmanModeElapsedTimeText() const
{
	return FText::Format(
		NSLOCTEXT("SnowRumble", "SnowmanModeElapsedTimeFormat", "경기 시간 {0}"),
		FormatSecondsAsClock(GetSnowmanModeElapsedSeconds()));
}

ESnowmanModePlayerRole ASnowmanModeGameState::GetSnowmanModePlayerRole(
	const APlayerState* PlayerState) const
{
	const int32 EntryIndex = FindSnowmanModePlayerEntryIndex(PlayerState);
	return EntryIndex == INDEX_NONE
		? ESnowmanModePlayerRole::Normal
		: SnowmanModePlayerEntries[EntryIndex].Role;
}

bool ASnowmanModeGameState::IsSnowmanModePlayerSnowman(
	const APlayerState* PlayerState) const
{
	return GetSnowmanModePlayerRole(PlayerState)
		== ESnowmanModePlayerRole::Snowman;
}

bool ASnowmanModeGameState::IsSnowmanModePlayerInfectionPending(
	const APlayerState* PlayerState) const
{
	return GetSnowmanModePlayerRole(PlayerState)
		== ESnowmanModePlayerRole::InfectionPending;
}

float ASnowmanModeGameState::GetSnowmanModeInfectionRemainingSeconds(
	const APlayerState* PlayerState) const
{
	const int32 EntryIndex = FindSnowmanModePlayerEntryIndex(PlayerState);
	if (EntryIndex == INDEX_NONE
		|| SnowmanModePlayerEntries[EntryIndex].Role
			!= ESnowmanModePlayerRole::InfectionPending)
	{
		return 0.0f;
	}

	return FMath::Max(
		0.0f,
		SnowmanModePlayerEntries[EntryIndex].InfectionCompleteServerTime
			- GetServerWorldTimeSeconds());
}

const TArray<FSnowmanModePlayerEntry>&
ASnowmanModeGameState::GetSnowmanModePlayerEntries() const
{
	return SnowmanModePlayerEntries;
}

void ASnowmanModeGameState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASnowmanModeGameState, bSnowmanModeTimerActive);
	DOREPLIFETIME(ASnowmanModeGameState, bSnowmanModeCountdownActive);
	DOREPLIFETIME(ASnowmanModeGameState, SnowmanModeMatchStartServerTime);
	DOREPLIFETIME(ASnowmanModeGameState, SnowmanModeStartCountdownSeconds);
	DOREPLIFETIME(ASnowmanModeGameState, SnowmanModeStartServerTime);
	DOREPLIFETIME(ASnowmanModeGameState, SnowmanModeTimeLimitSeconds);
	DOREPLIFETIME(ASnowmanModeGameState, SnowmanModePlayerEntries);
	DOREPLIFETIME(ASnowmanModeGameState, bSnowmanModeEnded);
	DOREPLIFETIME(ASnowmanModeGameState, SnowmanModeResult);
}

void ASnowmanModeGameState::OnRep_SnowmanModeEnded()
{
	if (bSnowmanModeEnded)
	{
		ShowSnowmanModeResultWidget();
	}
	else
	{
		RemoveSnowmanModeResultWidget();
	}
}

int32 ASnowmanModeGameState::FindSnowmanModePlayerEntryIndex(
	const APlayerState* PlayerState) const
{
	if (!PlayerState)
	{
		return INDEX_NONE;
	}

	for (int32 EntryIndex = 0;
		EntryIndex < SnowmanModePlayerEntries.Num();
		++EntryIndex)
	{
		if (SnowmanModePlayerEntries[EntryIndex].PlayerState == PlayerState)
		{
			return EntryIndex;
		}
	}

	const int32 PlayerId = PlayerState->GetPlayerId();
	if (PlayerId != INDEX_NONE)
	{
		for (int32 EntryIndex = 0;
			EntryIndex < SnowmanModePlayerEntries.Num();
			++EntryIndex)
		{
			const APlayerState* EntryPlayerState =
				SnowmanModePlayerEntries[EntryIndex].PlayerState;
			if (EntryPlayerState
				&& EntryPlayerState->GetPlayerId() == PlayerId)
			{
				return EntryIndex;
			}
		}
	}

	const FUniqueNetIdRepl& UniqueId = PlayerState->GetUniqueId();
	if (UniqueId.IsValid())
	{
		for (int32 EntryIndex = 0;
			EntryIndex < SnowmanModePlayerEntries.Num();
			++EntryIndex)
		{
			const APlayerState* EntryPlayerState =
				SnowmanModePlayerEntries[EntryIndex].PlayerState;
			if (EntryPlayerState
				&& EntryPlayerState->GetUniqueId().IsValid()
				&& EntryPlayerState->GetUniqueId() == UniqueId)
			{
				return EntryIndex;
			}
		}
	}

	return INDEX_NONE;
}

void ASnowmanModeGameState::ShowSnowmanModeResultWidget()
{
	if (!bSnowmanModeEnded
		|| SnowmanModeResult == ESnowmanModeResult::None
		|| !ResultWidgetClass
		|| GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	APlayerController* LocalPlayerController = GetWorld()
		? GetWorld()->GetFirstPlayerController()
		: nullptr;
	if (!LocalPlayerController || !LocalPlayerController->IsLocalController())
	{
		return;
	}

	if (!SnowmanModeResultWidget)
	{
		SnowmanModeResultWidget =
			CreateWidget<USnowmanModeResultWidget>(
				LocalPlayerController,
				ResultWidgetClass);
	}

	if (!SnowmanModeResultWidget)
	{
		return;
	}

	SnowmanModeResultWidget->SetResultText(GetSnowmanModeResultText());
	if (!SnowmanModeResultWidget->IsInViewport())
	{
		SnowmanModeResultWidget->AddToViewport(100);
	}
}

void ASnowmanModeGameState::RemoveSnowmanModeResultWidget()
{
	if (SnowmanModeResultWidget)
	{
		SnowmanModeResultWidget->RemoveFromParent();
		SnowmanModeResultWidget = nullptr;
	}
}

float ASnowmanModeGameState::GetSecondsUntilSnowmanModeStart() const
{
	return SnowmanModeMatchStartServerTime - GetServerWorldTimeSeconds();
}

FText ASnowmanModeGameState::FormatSecondsAsClock(float Seconds) const
{
	const int32 TotalSeconds = FMath::Max(0, FMath::CeilToInt(Seconds));
	const int32 Minutes = TotalSeconds / 60;
	const int32 RemainingSeconds = TotalSeconds % 60;
	return FText::FromString(FString::Printf(
		TEXT("%d:%02d"),
		Minutes,
		RemainingSeconds));
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowmanModePodiumGameMode_K.h"

#include "../Player/SnowRumbleCharacter.h"
#include "Camera/CameraActor.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "SnowmanModePodiumPlayerController_K.h"
#include "SnowRumblePlayerState.h"
#include "UObject/ConstructorHelpers.h"

// 네임스페이스를 제거하고 전역 함수 형태로 안전하게 배치합니다.
namespace
{
    constexpr const TCHAR* SnowmanPodiumLobbyGameModeTravelPath =
        TEXT("/Game/Game/BP_LobbyGameMode.BP_LobbyGameMode_C");
}

static void EnsureSnowmanModePodiumTravelOption(FString& TravelUrl, const TCHAR* Option)
{
    if (!TravelUrl.Contains(Option, ESearchCase::IgnoreCase))
    {
       TravelUrl += Option;
    }
}

static void EnsureSnowmanModePodiumTravelOptionValue(
    FString& TravelUrl,
    const TCHAR* OptionName,
    const FString& OptionValue)
{
    if (OptionValue.IsEmpty())
    {
       return;
    }

    const FString OptionPrefix = FString::Printf(TEXT("%s="), OptionName);
    if (!TravelUrl.Contains(OptionPrefix, ESearchCase::IgnoreCase))
    {
       TravelUrl += FString::Printf(
          TEXT("?%s=%s"),
          OptionName,
          *OptionValue);
    }
}

static ESnowmanModeResult ParseSnowmanPodiumResult(const FString& ResultOption)
{
    if (ResultOption.Equals(TEXT("SnowmanVictory"), ESearchCase::IgnoreCase))
    {
       return ESnowmanModeResult::SnowmanVictory;
    }
    if (ResultOption.Equals(TEXT("SurvivorVictory"), ESearchCase::IgnoreCase))
    {
       return ESnowmanModeResult::SurvivorVictory;
    }
    return ESnowmanModeResult::None;
}

ASnowmanModePodiumGameMode::ASnowmanModePodiumGameMode()
{
    PlayerControllerClass = ASnowmanModePodiumPlayerController::StaticClass();
    static ConstructorHelpers::FClassFinder<ASnowmanModePodiumPlayerController>
       SnowmanPodiumControllerFinder(
         TEXT("/Game/Controller/BP_SnowmanModePodiumPlayerController_K"));
    if (SnowmanPodiumControllerFinder.Succeeded())
    {
       PlayerControllerClass = SnowmanPodiumControllerFinder.Class;
    }

    DefaultPawnClass = ASnowRumbleCharacter::StaticClass();
    PlayerStateClass = ASnowRumblePlayerState::StaticClass();
    bStartPlayersAsSpectators = true;
}

void ASnowmanModePodiumGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("=== SNOWMAN PODIUM GAMEMODE BEGIN PLAY SUCCESS ==="));

    ScheduleSnowmanPodiumSetup();
    BroadcastBackgroundMusic();
}

void ASnowmanModePodiumGameMode::InitGame(
    const FString& MapName,
    const FString& Options,
    FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    
    UE_LOG(LogTemp, Warning, TEXT("=== PODIUM INIT GAME OPTIONS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Full Options: %s"), *Options);

    const FString ExpectedPlayersOption =
       UGameplayStatics::ParseOption(Options, TEXT("ExpectedPlayers"));
    ExpectedPlayerCount = ExpectedPlayersOption.IsEmpty()
       ? 0
       : FMath::Max(0, FCString::Atoi(*ExpectedPlayersOption));
    SnowmanPodiumResult = ParseSnowmanPodiumResult(
       UGameplayStatics::ParseOption(Options, TEXT("SnowmanResult")));
    ParseWinnerPlayerIds(
       UGameplayStatics::ParseOption(Options, TEXT("WinnerPlayerIds")));

    bPodiumSetupComplete = false;
    GetWorldTimerManager().ClearTimer(PodiumSetupTimerHandle);
    GetWorldTimerManager().ClearTimer(PodiumReturnCountdownTimerHandle);
}

void ASnowmanModePodiumGameMode::PostLogin(APlayerController* NewPlayer)
{
    AGameModeBase::PostLogin(NewPlayer);
    ScheduleSnowmanPodiumSetup();
    BroadcastBackgroundMusic();
}

void ASnowmanModePodiumGameMode::HandleStartingNewPlayer_Implementation(
    APlayerController* NewPlayer)
{
    AGameModeBase::HandleStartingNewPlayer_Implementation(NewPlayer);
    ScheduleSnowmanPodiumSetup();
    BroadcastBackgroundMusic();
}

void ASnowmanModePodiumGameMode::ScheduleSnowmanPodiumSetup()
{
    if (bPodiumSetupComplete || !HasAuthority())
    {
       return;
    }

    GetWorldTimerManager().ClearTimer(PodiumSetupTimerHandle);
    GetWorldTimerManager().SetTimer(
       PodiumSetupTimerHandle,
       this,
       &ASnowmanModePodiumGameMode::SetupSnowmanPodiumFromServer,
       PodiumSetupRetryDelaySeconds,
       false);
}

void ASnowmanModePodiumGameMode::BroadcastBackgroundMusic() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
       return;
    }

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator();
       It;
       ++It)
    {
       if (ASnowmanModePodiumPlayerController* PodiumController =
          Cast<ASnowmanModePodiumPlayerController>(It->Get()))
       {
          PodiumController->ClientPlayBackgroundMusic(BackgroundMusicSound);
       }
    }
}

void ASnowmanModePodiumGameMode::SetupSnowmanPodiumFromServer()
{
    UWorld* World = GetWorld();
    if (!HasAuthority() || !World)
    {
       return;
    }

    const int32 RequiredPlayerCount =
       ExpectedPlayerCount > 0 ? ExpectedPlayerCount : GetNumPlayers();
    if (RequiredPlayerCount > 0 && GetNumPlayers() < RequiredPlayerCount)
    {
       ScheduleSnowmanPodiumSetup();
       return;
    }

    TArray<APlayerController*> WinnerControllers;
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator();
       It;
       ++It)
    {
       APlayerController* PlayerController = It->Get();
       const ASnowRumblePlayerState* SnowPlayerState = PlayerController
          ? PlayerController->GetPlayerState<ASnowRumblePlayerState>()
          : nullptr;
       if (!PlayerController || !SnowPlayerState)
       {
          continue;
       }

       if (IsWinnerPlayerState(SnowPlayerState))
       {
          WinnerControllers.Add(PlayerController);
       }
    }

    if (WinnerControllers.IsEmpty())
    {
       ScheduleSnowmanPodiumSetup();
       return;
    }

    ACameraActor* PodiumCamera = FindSnowmanPodiumCamera();
    TArray<APlayerStart*> WinnerStarts;
    CollectSnowmanPodiumPlayerStarts(WinnerStarts);
    if (!WinnerStarts.IsEmpty())
    {
       if (WinnerControllers.Num() >= 4 && WinnerStarts.Num() >= 4)
       {
          for (int32 Index = WinnerStarts.Num() - 1; Index > 0; --Index)
          {
             const int32 SwapIndex = FMath::RandRange(0, Index);
             WinnerStarts.Swap(Index, SwapIndex);
          }
       }

       for (int32 Index = 0; Index < WinnerControllers.Num(); ++Index)
       {
          APlayerController* PlayerController = WinnerControllers[Index];
          APlayerStart* SelectedStart =
             WinnerStarts[Index % WinnerStarts.Num()];
          RestartPlayerAtPlayerStart(PlayerController, SelectedStart);
          if (ASnowRumbleCharacter* PodiumCharacter =
             Cast<ASnowRumbleCharacter>(PlayerController->GetPawn()))
          {
             PodiumCharacter->PlayRandomServerDirectedEmote();
          }
       }
    }

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator();
       It;
       ++It)
    {
       if (APlayerController* PlayerController = It->Get())
       {
          if (PodiumCamera)
          {
             PlayerController->SetViewTargetWithBlend(PodiumCamera, 0.5f);
          }
       }
    }

    StartSnowmanPodiumReturnCountdown();
    bPodiumSetupComplete = true;
}

void ASnowmanModePodiumGameMode::ReturnToLobbyAfterSnowmanPodium()
{
   if (!HasAuthority())
   {
      return;
   }

   GetWorldTimerManager().ClearTimer(PodiumReturnCountdownTimerHandle);

   // 1. 실제 로비 맵 경로 적용 (Content\Maps\L_Lobby.umap -> /Game/Maps/L_Lobby)
   FString TargetMapPath = TEXT("/Game/Maps/L_Lobby"); 
    
   if (!LobbyReturnTravelUrl.IsEmpty())
   {
      TargetMapPath = LobbyReturnTravelUrl;
   }

   // 혹시 모를 기존 옵션 제거 (순수 맵 경로만 추출)
   int32 OptionIndex;
   if (TargetMapPath.FindChar('?', OptionIndex))
   {
      TargetMapPath = TargetMapPath.Left(OptionIndex);
   }

   // 2. 멀티플레이 트래블 URL 조립
   FString FinalTravelUrl = TargetMapPath;

   // ★ [수정 포인트]: TCHAR* 경로가 유효한지 확인하는 안전한 방식으로 변경
   if (SnowmanPodiumLobbyGameModeTravelPath != nullptr && SnowmanPodiumLobbyGameModeTravelPath[0] != TEXT('\0'))
   {
      FinalTravelUrl += FString::Printf(TEXT("?game=%s"), SnowmanPodiumLobbyGameModeTravelPath);
   }

   FinalTravelUrl += TEXT("?listen");

   UE_LOG(LogTemp, Warning, TEXT("=== [SERVER TRAVEL] Executing ServerTravel to: %s ==="), *FinalTravelUrl);

   // 3. 심리스 트래블 활성화로 클라이언트 동기화 안정성 확보
   bUseSeamlessTravel = true;

   if (UWorld* World = GetWorld())
   {
      World->ServerTravel(FinalTravelUrl);
   }
}

void ASnowmanModePodiumGameMode::StartSnowmanPodiumReturnCountdown()
{
    PodiumReturnCountdownRemainingSeconds =
       FMath::Max(0, FMath::CeilToInt(PodiumReturnDelaySeconds));
    BroadcastSnowmanPodiumReturnCountdown();

    if (PodiumReturnCountdownRemainingSeconds <= 0)
    {
       ReturnToLobbyAfterSnowmanPodium();
       return;
    }

    GetWorldTimerManager().ClearTimer(PodiumReturnCountdownTimerHandle);
    GetWorldTimerManager().SetTimer(
       PodiumReturnCountdownTimerHandle,
       this,
       &ASnowmanModePodiumGameMode::TickSnowmanPodiumReturnCountdown,
       1.0f,
       true);
}

void ASnowmanModePodiumGameMode::TickSnowmanPodiumReturnCountdown()
{
    if (!HasAuthority())
    {
       return;
    }

    if (PodiumReturnCountdownRemainingSeconds <= 0)
    {
       GetWorldTimerManager().ClearTimer(PodiumReturnCountdownTimerHandle);
       ReturnToLobbyAfterSnowmanPodium();
       return;
    }

    --PodiumReturnCountdownRemainingSeconds;
    if (PodiumReturnCountdownRemainingSeconds <= 0)
    {
       GetWorldTimerManager().ClearTimer(PodiumReturnCountdownTimerHandle);
       ReturnToLobbyAfterSnowmanPodium();
       return;
    }

    BroadcastSnowmanPodiumReturnCountdown();
}

void ASnowmanModePodiumGameMode::BroadcastSnowmanPodiumReturnCountdown()
{
    UWorld* World = GetWorld();
    if (!World)
    {
       return;
    }

    const FText ResultText = BuildSnowmanPodiumResultText();
    const FText SubtitleText = BuildSnowmanPodiumReturnSubtitle();
    
    UE_LOG(LogTemp, Warning, TEXT("=== SNOWMAN PODIUM BROADCAST RESULT TEXT: %s ==="), *ResultText.ToString());
    UE_LOG(LogTemp, Warning, TEXT("=== SNOWMAN PODIUM BROADCAST SUBTITLE: %s ==="), *SubtitleText.ToString());
    
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator();
       It;
       ++It)
    {
       if (ASnowmanModePodiumPlayerController* PodiumController =
          Cast<ASnowmanModePodiumPlayerController>(It->Get()))
       {
          if (PodiumReturnCountdownRemainingSeconds
             == FMath::CeilToInt(PodiumReturnDelaySeconds))
          {
             PodiumController->ClientSetSnowmanPodiumResult(
                ResultText,
                SubtitleText);
          }
          else
          {
             PodiumController->ClientUpdateSnowmanPodiumSubtitle(
                SubtitleText);
          }
       }
    }
}

void ASnowmanModePodiumGameMode::CollectSnowmanPodiumPlayerStarts(
    TArray<APlayerStart*>& OutWinnerStarts) const
{
    OutWinnerStarts.Reset();
    OutWinnerStarts.SetNumZeroed(4);

    if (const UWorld* World = GetWorld())
    {
       for (TActorIterator<APlayerStart> It(World); It; ++It)
       {
          APlayerStart* PlayerStart = *It;
          if (!PlayerStart)
          {
             continue;
          }

          if (PlayerStart->ActorHasTag(TEXT("Podium_Team1")))
          {
             OutWinnerStarts[0] = PlayerStart;
          }
          else if (PlayerStart->ActorHasTag(TEXT("Podium_Team2")))
          {
             OutWinnerStarts[1] = PlayerStart;
          }
          else if (PlayerStart->ActorHasTag(TEXT("Podium_Team3")))
          {
             OutWinnerStarts[2] = PlayerStart;
          }
          else if (PlayerStart->ActorHasTag(TEXT("Podium_Team4")))
          {
             OutWinnerStarts[3] = PlayerStart;
          }
       }
    }

    OutWinnerStarts.RemoveAll(
       [](const APlayerStart* PlayerStart)
       {
          return PlayerStart == nullptr;
       });
}

ACameraActor* ASnowmanModePodiumGameMode::FindSnowmanPodiumCamera() const
{
    if (const UWorld* World = GetWorld())
    {
       for (TActorIterator<ACameraActor> It(World); It; ++It)
       {
          ACameraActor* Camera = *It;
          if (Camera && Camera->ActorHasTag(TEXT("Podium_Camera")))
          {
             return Camera;
          }
       }
    }

    return nullptr;
}

void ASnowmanModePodiumGameMode::ParseWinnerPlayerIds(
    const FString& WinnerPlayerIdsOption)
{
    WinnerPlayerIds.Reset();

    TArray<FString> PlayerIdStrings;
    WinnerPlayerIdsOption.ParseIntoArray(
       PlayerIdStrings,
       TEXT(","),
       true);
    for (const FString& PlayerIdString : PlayerIdStrings)
    {
       const FString TrimmedPlayerId = PlayerIdString.TrimStartAndEnd();
       if (!TrimmedPlayerId.IsNumeric())
       {
          continue;
       }

       WinnerPlayerIds.Add(FCString::Atoi(*TrimmedPlayerId));
    }
}

bool ASnowmanModePodiumGameMode::IsWinnerPlayerState(
    const ASnowRumblePlayerState* PlayerState) const
{
    if (!PlayerState)
    {
       return false;
    }

    if (SnowmanPodiumResult == ESnowmanModeResult::SnowmanVictory)
    {
       return true;
    }

    return SnowmanPodiumResult == ESnowmanModeResult::SurvivorVictory
       && WinnerPlayerIds.Contains(PlayerState->GetPlayerId());
}

FText ASnowmanModePodiumGameMode::BuildSnowmanPodiumResultText() const
{
    switch (SnowmanPodiumResult)
    {
    case ESnowmanModeResult::SnowmanVictory:
       return NSLOCTEXT(
          "SnowRumble",
          "SnowmanPodiumSnowmanVictory",
          "눈사람팀 우승");
    case ESnowmanModeResult::SurvivorVictory:
       return NSLOCTEXT(
          "SnowRumble",
          "SnowmanPodiumSurvivorVictory",
          "사람팀 우승");
    case ESnowmanModeResult::None:
    default:
       return NSLOCTEXT(
          "SnowRumble",
          "SnowmanPodiumUnknownResult",
          "눈사람 모드 결과 없음");
    }
}

FText ASnowmanModePodiumGameMode::BuildSnowmanPodiumReturnSubtitle() const
{
    const int32 SafeSecondsRemaining =
       FMath::Max(0, PodiumReturnCountdownRemainingSeconds);
    return FText::Format(
       NSLOCTEXT(
          "SnowRumble",
          "SnowmanPodiumReturnSubtitleCountdown",
          "{0}초 후 로비로 돌아갑니다."),
       FText::AsNumber(SafeSecondsRemaining));
}
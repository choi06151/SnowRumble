// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleSessionSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogSnowRumbleSession, Log, All);

namespace SnowRumbleSession
{
	constexpr int32 MinimumPlayers = 2;
	constexpr int32 MaximumPlayers = 8;
	constexpr int32 MaximumSearchResults = 100;
	constexpr int32 RoomCodeDigits = 6;
	const TCHAR* LobbyGameModeTravelPath =
		TEXT("/Game/Game/BP_LobbyGameMode.BP_LobbyGameMode_C");
	const TCHAR* MainMenuGameModeTravelPath =
		TEXT("/Game/Game/BP_MainMenuGameMode.BP_MainMenuGameMode_C");
	const TCHAR* HostTravelUrl =
		TEXT("/Game/Maps/L_Lobby?listen?game=/Game/Game/BP_LobbyGameMode.BP_LobbyGameMode_C");
	const TCHAR* MainMenuTravelUrl =
		TEXT("/Game/Maps/L_MainMenu?game=/Game/Game/BP_MainMenuGameMode.BP_MainMenuGameMode_C");
	const FName RoomNameSettingKey(TEXT("SNOWRUMBLE_ROOM_NAME"));
	const FName RoomCodeSettingKey(TEXT("SNOWRUMBLE_ROOM_CODE"));
	const FName GameModeSettingKey(TEXT("SNOWRUMBLE_GAME_MODE"));
	const TCHAR* LobbyMapName = TEXT("L_Lobby");
	const TCHAR* TeamPvpModeName = TEXT("TeamPvP");

	FString GetSessionMapName(UWorld* World)
	{
		return World ? UGameplayStatics::GetCurrentLevelName(World, true) : FString();
	}

	bool IsLobbyMapName(const FString& MapName)
	{
		return MapName.Equals(LobbyMapName, ESearchCase::IgnoreCase)
			|| MapName.EndsWith(TEXT("/L_Lobby"), ESearchCase::IgnoreCase);
	}

	bool IsJoinableLobbySession(const FSnowRumbleSessionInfo& SessionInfo)
	{
		return SessionInfo.CurrentPlayers < SessionInfo.MaxPlayers
			&& IsLobbyMapName(SessionInfo.MapName);
	}
}

void USnowRumbleSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LocalSessionName = FName(
		*FString::Printf(
			TEXT("SnowRumbleSession_%s"),
			*FGuid::NewGuid().ToString(EGuidFormats::Digits)));

	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this,
		&USnowRumbleSessionSubsystem::HandlePostLoadMap);

	if (GEngine)
	{
		NetworkFailureHandle = GEngine->OnNetworkFailure().AddUObject(
			this,
			&USnowRumbleSessionSubsystem::HandleNetworkFailure);
		TravelFailureHandle = GEngine->OnTravelFailure().AddUObject(
			this,
			&USnowRumbleSessionSubsystem::HandleTravelFailure);
	}

	if (IOnlineSessionPtr SessionInterface = GetSessionInterface();
		SessionInterface.IsValid())
	{
		SessionUserInviteAcceptedHandle =
			SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(
				FOnSessionUserInviteAcceptedDelegate::CreateUObject(
					this,
					&USnowRumbleSessionSubsystem::HandleSessionUserInviteAccepted));
	}
}

void USnowRumbleSessionSubsystem::Deinitialize()
{
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}
	if (GEngine && NetworkFailureHandle.IsValid())
	{
		GEngine->OnNetworkFailure().Remove(NetworkFailureHandle);
		NetworkFailureHandle.Reset();
	}
	if (GEngine && TravelFailureHandle.IsValid())
	{
		GEngine->OnTravelFailure().Remove(TravelFailureHandle);
		TravelFailureHandle.Reset();
	}

	ClearCreateSessionDelegate();
	ClearFindSessionsDelegate();
	ClearJoinSessionDelegate();
	if (IOnlineSessionPtr SessionInterface = GetSessionInterface();
		SessionInterface.IsValid()
		&& SessionUserInviteAcceptedHandle.IsValid())
	{
		SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(
			SessionUserInviteAcceptedHandle);
	}
	SessionUserInviteAcceptedHandle.Reset();
	ActiveSessionSearch.Reset();

	if (IOnlineSessionPtr SessionInterface = GetSessionInterface();
		SessionInterface.IsValid()
		&& SessionInterface->GetNamedSession(LocalSessionName))
	{
		SessionInterface->DestroySession(LocalSessionName);
	}

	Super::Deinitialize();
}

void USnowRumbleSessionSubsystem::HostLanSession(
	int32 MaxPlayers,
	const FString& RoomName)
{
	bReturningToMainMenuAfterSessionFailure = false;
	if (IsOperationInProgress())
	{
		SetOperationState(
			ESnowRumbleSessionOperation::Host,
			ESnowRumbleSessionState::Failed,
			TEXT("다른 세션 요청이 진행 중입니다."));
		return;
	}

	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		SetOperationState(
			ESnowRumbleSessionOperation::Host,
			ESnowRumbleSessionState::Failed,
			TEXT("세션 인터페이스를 사용할 수 없습니다."));
		return;
	}

	if (SessionInterface->GetNamedSession(LocalSessionName))
	{
		SetOperationState(
			ESnowRumbleSessionOperation::Host,
			ESnowRumbleSessionState::Failed,
			TEXT("이미 생성된 게임 세션이 있습니다."));
		return;
	}

	PendingHostMaxPlayers = FMath::Clamp(
		MaxPlayers,
		SnowRumbleSession::MinimumPlayers,
		SnowRumbleSession::MaximumPlayers);
	PendingHostRoomName = RoomName.TrimStartAndEnd();
	PendingHostRoomCode = GenerateRoomCode();
	CurrentRoomCode = PendingHostRoomCode;

	UWorld* World = GetWorld();
	if (!World)
	{
		SetOperationState(
			ESnowRumbleSessionOperation::Host,
			ESnowRumbleSessionState::Failed,
			TEXT("Host 월드를 찾지 못했습니다."));
		return;
	}

	if (World->GetNetMode() == NM_Client)
	{
		SetOperationState(
			ESnowRumbleSessionOperation::Host,
			ESnowRumbleSessionState::Failed,
			TEXT("이미 서버에 연결된 클라이언트에서는 방을 만들 수 없습니다."));
		return;
	}

	bHostTravelPending = true;
	UE_LOG(
		LogSnowRumbleSession,
		Log,
		TEXT("Host requested. MaxPlayers=%d RoomName='%s' RoomCode=%s"),
		PendingHostMaxPlayers,
		*PendingHostRoomName,
		*PendingHostRoomCode);
	SetOperationState(
		ESnowRumbleSessionOperation::Host,
		ESnowRumbleSessionState::InProgress,
		TEXT("Listen Server를 준비하고 있습니다."));

	if (World->GetNetMode() == NM_Standalone)
	{
		UGameplayStatics::OpenLevel(
			World,
			FName(TEXT("/Game/Maps/L_Lobby")),
			true,
			FString::Printf(
				TEXT("listen?game=%s"),
				SnowRumbleSession::LobbyGameModeTravelPath));
		return;
	}

	if (World->ServerTravel(SnowRumbleSession::HostTravelUrl))
	{
		return;
	}

	bHostTravelPending = false;
	SetOperationState(
		ESnowRumbleSessionOperation::Host,
		ESnowRumbleSessionState::Failed,
		TEXT("Listen Server 맵 이동을 시작하지 못했습니다."));
}

void USnowRumbleSessionSubsystem::CreateLanSession(int32 MaxPlayers)
{
	if (PendingHostRoomCode.IsEmpty())
	{
		PendingHostRoomCode = GenerateRoomCode();
	}
	CurrentRoomCode = PendingHostRoomCode;

	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		bHostTravelPending = false;
		SetOperationState(
			ESnowRumbleSessionOperation::Host,
			ESnowRumbleSessionState::Failed,
			TEXT("세션 인터페이스를 사용할 수 없습니다."));
		return;
	}

	const bool bUseSteam = IsSteamSubsystem();
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = !bUseSteam;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowInvites = bUseSteam;
	SessionSettings.bUsesPresence = bUseSteam;
	SessionSettings.bUseLobbiesIfAvailable = bUseSteam;
	SessionSettings.bAllowJoinViaPresence = bUseSteam;
	SessionSettings.NumPublicConnections = MaxPlayers;
	SessionSettings.Set(
		SnowRumbleSession::RoomNameSettingKey,
		PendingHostRoomName,
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(
		SnowRumbleSession::RoomCodeSettingKey,
		PendingHostRoomCode,
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(
		SnowRumbleSession::GameModeSettingKey,
		FString(SnowRumbleSession::TeamPvpModeName),
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(
		SETTING_MAPNAME,
		FString(SnowRumbleSession::LobbyMapName),
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	UE_LOG(
		LogSnowRumbleSession,
		Log,
		TEXT("Creating %s session. MaxPlayers=%d RoomName='%s' RoomCode=%s"),
		bUseSteam ? TEXT("Steam") : TEXT("LAN"),
		MaxPlayers,
		*PendingHostRoomName,
		*PendingHostRoomCode);

	CreateSessionCompleteHandle =
		SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
			FOnCreateSessionCompleteDelegate::CreateUObject(
				this,
				&USnowRumbleSessionSubsystem::HandleCreateSessionComplete));

	SetOperationState(
		ESnowRumbleSessionOperation::Host,
		ESnowRumbleSessionState::InProgress,
		bUseSteam
			? TEXT("Steam 세션을 생성하고 있습니다.")
			: TEXT("LAN 세션을 생성하고 있습니다."));

	if (!SessionInterface->CreateSession(
		0,
		LocalSessionName,
		SessionSettings))
	{
		ClearCreateSessionDelegate();
		SetOperationState(
			ESnowRumbleSessionOperation::Host,
			ESnowRumbleSessionState::Failed,
			TEXT("LAN 세션 생성 요청을 시작하지 못했습니다."));
	}
}

void USnowRumbleSessionSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	if (!LoadedWorld || LoadedWorld->GetGameInstance() != GetGameInstance())
	{
		return;
	}

	const FString MapName = SnowRumbleSession::GetSessionMapName(LoadedWorld);
	if (MapName.Equals(TEXT("L_MainMenu"), ESearchCase::IgnoreCase))
	{
		bReturningToMainMenuAfterSessionFailure = false;
		return;
	}

	if (LoadedWorld->GetNetMode() != NM_ListenServer)
	{
		return;
	}

	if (bHostTravelPending)
	{
		bHostTravelPending = false;
		CreateLanSession(PendingHostMaxPlayers);
		return;
	}

	UpdateAdvertisedSessionMap(LoadedWorld);
}

void USnowRumbleSessionSubsystem::UpdateAdvertisedSessionMap(UWorld* LoadedWorld)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		return;
	}

	FNamedOnlineSession* NamedSession =
		SessionInterface->GetNamedSession(LocalSessionName);
	if (!NamedSession)
	{
		return;
	}

	const FString MapName = SnowRumbleSession::GetSessionMapName(LoadedWorld);
	if (MapName.IsEmpty())
	{
		return;
	}

	FOnlineSessionSettings UpdatedSettings = NamedSession->SessionSettings;
	UpdatedSettings.bAllowJoinInProgress =
		SnowRumbleSession::IsLobbyMapName(MapName);
	UpdatedSettings.Set(
		SETTING_MAPNAME,
		MapName,
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	UE_LOG(
		LogSnowRumbleSession,
		Log,
		TEXT("Updating advertised session map. Backend=%s Map=%s AllowJoinInProgress=%d"),
		IsSteamSubsystem() ? TEXT("Steam") : TEXT("LAN"),
		*MapName,
		UpdatedSettings.bAllowJoinInProgress ? 1 : 0);

	SessionInterface->UpdateSession(LocalSessionName, UpdatedSettings, true);
}

void USnowRumbleSessionSubsystem::FindLanSessions()
{
	UE_LOG(LogSnowRumbleSession, Log, TEXT("Find LAN sessions requested."));
	BeginFindLanSessions(
		ESnowRumbleSessionOperation::Search,
		TEXT("LAN 세션을 검색하고 있습니다."));
}

void USnowRumbleSessionSubsystem::QuickJoinLanSession()
{
	UE_LOG(LogSnowRumbleSession, Log, TEXT("Quick join requested."));
	BeginFindLanSessions(
		ESnowRumbleSessionOperation::QuickJoin,
		TEXT("참가 가능한 LAN 세션을 찾고 있습니다."));
}

void USnowRumbleSessionSubsystem::JoinLanSessionByRoomCode(
	const FString& RoomCode)
{
	PendingJoinRoomCode = NormalizeRoomCode(RoomCode);
	UE_LOG(
		LogSnowRumbleSession,
		Log,
		TEXT("Join by room code requested. Input='%s' Normalized='%s'"),
		*RoomCode,
		*PendingJoinRoomCode);
	if (PendingJoinRoomCode.IsEmpty())
	{
		SetOperationState(
			ESnowRumbleSessionOperation::JoinByCode,
			ESnowRumbleSessionState::Failed,
			TEXT("방 코드를 입력해 주세요."));
		return;
	}

	BeginFindLanSessions(
		ESnowRumbleSessionOperation::JoinByCode,
		TEXT("방 코드와 일치하는 LAN 세션을 찾고 있습니다."));
}

void USnowRumbleSessionSubsystem::LeaveLanSession()
{
	ClearCreateSessionDelegate();
	ClearFindSessionsDelegate();
	ClearJoinSessionDelegate();
	ActiveSessionSearch.Reset();
	SearchResults.Reset();
	PendingJoinRoomCode.Empty();
	PendingHostRoomName.Empty();
	PendingHostRoomCode.Empty();
	CurrentRoomCode.Empty();
	bHostTravelPending = false;
	bWasInOnlineSession = false;

	if (IOnlineSessionPtr SessionInterface = GetSessionInterface();
		SessionInterface.IsValid()
		&& SessionInterface->GetNamedSession(LocalSessionName))
	{
		UE_LOG(
			LogSnowRumbleSession,
			Log,
			TEXT("Leaving LAN session. SessionName=%s"),
			*LocalSessionName.ToString());
		SessionInterface->DestroySession(LocalSessionName);
	}

	SetOperationState(
		ESnowRumbleSessionOperation::None,
		ESnowRumbleSessionState::Idle,
		TEXT("LAN 세션을 정리했습니다."));
	OnSessionSearchCompleted.Broadcast(SearchResults);
}

FString USnowRumbleSessionSubsystem::ConsumePendingMainMenuAlarmMessage()
{
	FString Message = PendingMainMenuAlarmMessage;
	PendingMainMenuAlarmMessage.Empty();
	return Message;
}

void USnowRumbleSessionSubsystem::SetPendingMainMenuAlarmMessage(
	const FString& Message)
{
	PendingMainMenuAlarmMessage = Message;
}

void USnowRumbleSessionSubsystem::BeginFindLanSessions(
	ESnowRumbleSessionOperation Operation,
	const FString& Message)
{
	bReturningToMainMenuAfterSessionFailure = false;
	if (IsOperationInProgress())
	{
		SetOperationState(
			Operation,
			ESnowRumbleSessionState::Failed,
			TEXT("다른 세션 요청이 진행 중입니다."));
		return;
	}

	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		SetOperationState(
			Operation,
			ESnowRumbleSessionState::Failed,
			TEXT("세션 인터페이스를 사용할 수 없습니다."));
		return;
	}

	SearchResults.Reset();
	ActiveSessionSearch = MakeShared<FOnlineSessionSearch>();
	const bool bUseSteam = IsSteamSubsystem();
	ActiveSessionSearch->bIsLanQuery = !bUseSteam;
	if (bUseSteam)
	{
		ActiveSessionSearch->QuerySettings.Set(
			SEARCH_LOBBIES,
			true,
			EOnlineComparisonOp::Equals);
	}
	ActiveSessionSearch->MaxSearchResults = SnowRumbleSession::MaximumSearchResults;
	UE_LOG(
		LogSnowRumbleSession,
		Log,
		TEXT("Begin %s search. Operation=%d"),
		bUseSteam ? TEXT("Steam") : TEXT("LAN"),
		static_cast<int32>(Operation));

	FindSessionsCompleteHandle =
		SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
			FOnFindSessionsCompleteDelegate::CreateUObject(
				this,
				&USnowRumbleSessionSubsystem::HandleFindSessionsComplete));

	SetOperationState(
		Operation,
		ESnowRumbleSessionState::InProgress,
		Message);

	if (!SessionInterface->FindSessions(0, ActiveSessionSearch.ToSharedRef()))
	{
		ClearFindSessionsDelegate();
		ActiveSessionSearch.Reset();
		SetOperationState(
			Operation,
			ESnowRumbleSessionState::Failed,
			TEXT("LAN 세션 검색 요청을 시작하지 못했습니다."));
	}
}

void USnowRumbleSessionSubsystem::JoinLanSession(int32 ResultIndex)
{
	if (IsOperationInProgress())
	{
		SetOperationState(
			ESnowRumbleSessionOperation::Join,
			ESnowRumbleSessionState::Failed,
			TEXT("다른 세션 요청이 진행 중입니다."));
		return;
	}

	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		SetOperationState(
			ESnowRumbleSessionOperation::Join,
			ESnowRumbleSessionState::Failed,
			TEXT("세션 인터페이스를 사용할 수 없습니다."));
		return;
	}

	if (!ActiveSessionSearch.IsValid()
		|| !ActiveSessionSearch->SearchResults.IsValidIndex(ResultIndex))
	{
		SetOperationState(
			ESnowRumbleSessionOperation::Join,
			ESnowRumbleSessionState::Failed,
			TEXT("선택한 검색 결과가 유효하지 않습니다."));
		return;
	}

	JoinSearchResult(ResultIndex, ESnowRumbleSessionOperation::Join);
}

void USnowRumbleSessionSubsystem::JoinSearchResult(
	int32 ResultIndex,
	ESnowRumbleSessionOperation Operation)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		SetOperationState(
			Operation,
			ESnowRumbleSessionState::Failed,
			TEXT("세션 인터페이스를 사용할 수 없습니다."));
		return;
	}

	if (!ActiveSessionSearch.IsValid()
		|| !ActiveSessionSearch->SearchResults.IsValidIndex(ResultIndex))
	{
		SetOperationState(
			Operation,
			ESnowRumbleSessionState::Failed,
			TEXT("선택한 검색 결과가 유효하지 않습니다."));
		return;
	}

	if (SessionInterface->GetNamedSession(LocalSessionName))
	{
		DestroyLocalSessionIfPresent(TEXT("before join retry"));
		SetOperationState(
			Operation,
			ESnowRumbleSessionState::Failed,
			TEXT("이전 LAN 세션 상태를 정리했습니다. 다시 참가해 주세요."));
		return;
	}

	const FOnlineSessionSearchResult& SearchResult =
		ActiveSessionSearch->SearchResults[ResultIndex];
	FString SearchResultMapName;
	SearchResult.Session.SessionSettings.Get(
		SETTING_MAPNAME,
		SearchResultMapName);
	if (!SnowRumbleSession::IsLobbyMapName(SearchResultMapName))
	{
		SetOperationState(
			Operation,
			ESnowRumbleSessionState::Failed,
			TEXT("대기방 상태인 세션만 참가할 수 있습니다."));
		SetPendingMainMenuAlarmMessage(TEXT("이미 시작된 방에는 참가할 수 없습니다."));
		return;
	}

	FString SearchResultRoomCode;
	SearchResult.Session.SessionSettings.Get(
		SnowRumbleSession::RoomCodeSettingKey,
		SearchResultRoomCode);
	CurrentRoomCode = SearchResultRoomCode;
	UE_LOG(
		LogSnowRumbleSession,
		Log,
		TEXT("Joining search result. Operation=%d ResultIndex=%d RoomCode=%s"),
		static_cast<int32>(Operation),
		ResultIndex,
		*CurrentRoomCode);

	JoinSessionCompleteHandle =
		SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
			FOnJoinSessionCompleteDelegate::CreateUObject(
				this,
				&USnowRumbleSessionSubsystem::HandleJoinSessionComplete));

	SetOperationState(
		Operation,
		ESnowRumbleSessionState::InProgress,
		IsSteamSubsystem()
			? TEXT("Steam 세션에 참가하고 있습니다.")
			: TEXT("LAN 세션에 참가하고 있습니다."));

	if (!SessionInterface->JoinSession(
		0,
		LocalSessionName,
		ActiveSessionSearch->SearchResults[ResultIndex]))
	{
		ClearJoinSessionDelegate();
		DestroyLocalSessionIfPresent(TEXT("join request failed"));
		SetOperationState(
			Operation,
			ESnowRumbleSessionState::Failed,
			TEXT("LAN 세션 참가 요청을 시작하지 못했습니다."));
	}
}

const TArray<FSnowRumbleSessionInfo>&
USnowRumbleSessionSubsystem::GetSearchResults() const
{
	return SearchResults;
}

FString USnowRumbleSessionSubsystem::GetCurrentRoomCode() const
{
	if (!CurrentRoomCode.IsEmpty())
	{
		return CurrentRoomCode;
	}

	if (CurrentOperation == ESnowRumbleSessionOperation::Host
		&& !PendingHostRoomCode.IsEmpty())
	{
		return PendingHostRoomCode;
	}

	return CurrentRoomCode;
}

ESnowRumbleSessionOperation
USnowRumbleSessionSubsystem::GetCurrentOperation() const
{
	return CurrentOperation;
}

ESnowRumbleSessionState
USnowRumbleSessionSubsystem::GetCurrentState() const
{
	return CurrentState;
}

IOnlineSessionPtr USnowRumbleSessionSubsystem::GetSessionInterface() const
{
	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	return OnlineSubsystem ? OnlineSubsystem->GetSessionInterface() : nullptr;
}

bool USnowRumbleSessionSubsystem::IsSteamSubsystem() const
{
	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	return OnlineSubsystem
		&& OnlineSubsystem->GetSubsystemName() == FName(TEXT("STEAM"));
}

bool USnowRumbleSessionSubsystem::ShowSessionInviteUI()
{
	if (!IsSteamSubsystem())
	{
		UE_LOG(
			LogSnowRumbleSession,
			Warning,
			TEXT("Steam invite UI requested while the active subsystem is not Steam."));
		return false;
	}

	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	IOnlineExternalUIPtr ExternalUI = OnlineSubsystem
		? OnlineSubsystem->GetExternalUIInterface()
		: nullptr;
	if (!ExternalUI.IsValid())
	{
		UE_LOG(
			LogSnowRumbleSession,
			Warning,
			TEXT("Steam invite UI is unavailable."));
		return false;
	}

	const bool bShown = ExternalUI->ShowInviteUI(0, LocalSessionName);
	UE_LOG(
		LogSnowRumbleSession,
		Log,
		TEXT("Steam session invite UI requested. Shown=%d SessionName=%s"),
		bShown ? 1 : 0,
		*LocalSessionName.ToString());
	return bShown;
}

void USnowRumbleSessionSubsystem::HandleSessionUserInviteAccepted(
	const bool bWasSuccessful,
	const int32 ControllerId,
	FUniqueNetIdPtr UserId,
	const FOnlineSessionSearchResult& InviteResult)
{
	UE_LOG(
		LogSnowRumbleSession,
		Log,
		TEXT("Session invite accepted. Success=%d ControllerId=%d ValidResult=%d"),
		bWasSuccessful ? 1 : 0,
		ControllerId,
		InviteResult.IsValid() ? 1 : 0);

	if (!IsSteamSubsystem() || !bWasSuccessful || !InviteResult.IsValid())
	{
		SetPendingMainMenuAlarmMessage(TEXT("Steam 초대를 처리하지 못했습니다."));
		return;
	}

	if (IsOperationInProgress())
	{
		SetPendingMainMenuAlarmMessage(TEXT("다른 세션 요청이 진행 중입니다."));
		return;
	}

	ActiveSessionSearch = MakeShared<FOnlineSessionSearch>();
	ActiveSessionSearch->SearchResults.Add(InviteResult);
	SearchResults.Reset();
	JoinSearchResult(0, ESnowRumbleSessionOperation::Join);
}

void USnowRumbleSessionSubsystem::SetOperationState(
	ESnowRumbleSessionOperation Operation,
	ESnowRumbleSessionState State,
	const FString& Message)
{
	CurrentOperation = Operation;
	CurrentState = State;
	UE_LOG(
		LogSnowRumbleSession,
		Log,
		TEXT("Session state changed. Operation=%d State=%d Message='%s'"),
		static_cast<int32>(Operation),
		static_cast<int32>(State),
		*Message);
	OnSessionStateChanged.Broadcast(Operation, State, Message);
}

bool USnowRumbleSessionSubsystem::IsOperationInProgress() const
{
	return CurrentState == ESnowRumbleSessionState::InProgress;
}

void USnowRumbleSessionSubsystem::HandleCreateSessionComplete(
	FName SessionName,
	bool bWasSuccessful)
{
	ClearCreateSessionDelegate();

	if (!bWasSuccessful)
	{
		SetOperationState(
			ESnowRumbleSessionOperation::Host,
			ESnowRumbleSessionState::Failed,
			TEXT("LAN 세션 생성에 실패했습니다."));
		return;
	}

	bWasInOnlineSession = true;
	if (CurrentRoomCode.IsEmpty() && !PendingHostRoomCode.IsEmpty())
	{
		CurrentRoomCode = PendingHostRoomCode;
	}
	SetOperationState(
		ESnowRumbleSessionOperation::Host,
		ESnowRumbleSessionState::Succeeded,
		TEXT("LAN 세션을 생성했습니다."));
}

void USnowRumbleSessionSubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
	ClearFindSessionsDelegate();
	SearchResults.Reset();

	if (!bWasSuccessful || !ActiveSessionSearch.IsValid())
	{
		ActiveSessionSearch.Reset();
		OnSessionSearchCompleted.Broadcast(SearchResults);
		SetOperationState(
			CurrentOperation,
			ESnowRumbleSessionState::Failed,
			TEXT("LAN 세션 검색에 실패했습니다."));
		return;
	}

	UE_LOG(
		LogSnowRumbleSession,
		Log,
		TEXT("LAN search completed. RawResults=%d"),
		ActiveSessionSearch->SearchResults.Num());

	for (int32 Index = 0; Index < ActiveSessionSearch->SearchResults.Num(); ++Index)
	{
		const FOnlineSessionSearchResult& SearchResult =
			ActiveSessionSearch->SearchResults[Index];
		const FOnlineSession& Session = SearchResult.Session;

		FSnowRumbleSessionInfo SessionInfo;
		SessionInfo.ResultIndex = Index;
		SessionInfo.HostName = Session.OwningUserName;
		Session.SessionSettings.Get(
			SnowRumbleSession::RoomNameSettingKey,
			SessionInfo.RoomName);
		Session.SessionSettings.Get(
			SnowRumbleSession::RoomCodeSettingKey,
			SessionInfo.RoomCode);
		Session.SessionSettings.Get(
			SnowRumbleSession::GameModeSettingKey,
			SessionInfo.GameModeName);
		Session.SessionSettings.Get(
			SETTING_MAPNAME,
			SessionInfo.MapName);
		SessionInfo.MaxPlayers =
			Session.SessionSettings.NumPublicConnections;
		SessionInfo.CurrentPlayers =
			SessionInfo.MaxPlayers - Session.NumOpenPublicConnections;
		SessionInfo.PingMilliseconds = SearchResult.PingInMs;
		UE_LOG(
			LogSnowRumbleSession,
			Log,
			TEXT("Search result %d: Host='%s' RoomName='%s' RoomCode=%s GameMode=%s Map=%s Players=%d/%d Ping=%d"),
			Index,
			*SessionInfo.HostName,
			*SessionInfo.RoomName,
			*SessionInfo.RoomCode,
			*SessionInfo.GameModeName,
			*SessionInfo.MapName,
			SessionInfo.CurrentPlayers,
			SessionInfo.MaxPlayers,
			SessionInfo.PingMilliseconds);
		SearchResults.Add(MoveTemp(SessionInfo));
	}

	OnSessionSearchCompleted.Broadcast(SearchResults);

	if (CurrentOperation == ESnowRumbleSessionOperation::QuickJoin)
	{
		for (const FSnowRumbleSessionInfo& SessionInfo : SearchResults)
		{
			if (SnowRumbleSession::IsJoinableLobbySession(SessionInfo))
			{
				JoinSearchResult(
					SessionInfo.ResultIndex,
					ESnowRumbleSessionOperation::QuickJoin);
				return;
			}
		}

		SetOperationState(
			ESnowRumbleSessionOperation::QuickJoin,
			ESnowRumbleSessionState::Failed,
			TEXT("참가 가능한 LAN 세션이 없습니다."));
		SetPendingMainMenuAlarmMessage(TEXT("방이 존재하지 않습니다."));
		return;
	}

	if (CurrentOperation == ESnowRumbleSessionOperation::JoinByCode)
	{
		for (const FSnowRumbleSessionInfo& SessionInfo : SearchResults)
		{
			if (NormalizeRoomCode(SessionInfo.RoomCode) == PendingJoinRoomCode)
			{
				if (!SnowRumbleSession::IsLobbyMapName(SessionInfo.MapName))
				{
					SetOperationState(
						ESnowRumbleSessionOperation::JoinByCode,
						ESnowRumbleSessionState::Failed,
						TEXT("이미 시작된 방에는 참가할 수 없습니다."));
					SetPendingMainMenuAlarmMessage(TEXT("이미 시작된 방에는 참가할 수 없습니다."));
					return;
				}

				if (SessionInfo.CurrentPlayers >= SessionInfo.MaxPlayers)
				{
					SetOperationState(
						ESnowRumbleSessionOperation::JoinByCode,
						ESnowRumbleSessionState::Failed,
						TEXT("방 인원이 가득 찼습니다."));
					return;
				}

				JoinSearchResult(
					SessionInfo.ResultIndex,
					ESnowRumbleSessionOperation::JoinByCode);
				return;
			}
		}

		SetOperationState(
			ESnowRumbleSessionOperation::JoinByCode,
			ESnowRumbleSessionState::Failed,
			TEXT("입력한 방 코드와 일치하는 LAN 세션이 없습니다."));
		SetPendingMainMenuAlarmMessage(TEXT("방이 존재하지 않습니다."));
		return;
	}

	SetOperationState(
		ESnowRumbleSessionOperation::Search,
		ESnowRumbleSessionState::Succeeded,
		SearchResults.IsEmpty()
			? TEXT("검색된 LAN 세션이 없습니다.")
			: TEXT("LAN 세션 검색을 완료했습니다."));
}

void USnowRumbleSessionSubsystem::HandleJoinSessionComplete(
	FName SessionName,
	EOnJoinSessionCompleteResult::Type Result)
{
	ClearJoinSessionDelegate();
	const ESnowRumbleSessionOperation JoinOperation = CurrentOperation;
	UE_LOG(
		LogSnowRumbleSession,
		Log,
		TEXT("Join session completed. Result=%d"),
		static_cast<int32>(Result));

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		DestroyLocalSessionIfPresent(TEXT("join completed with failure"));

		FString FailureMessage;
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::SessionIsFull:
			FailureMessage = TEXT("LAN 세션의 참가 인원이 가득 찼습니다.");
			break;
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:
			FailureMessage = TEXT("선택한 LAN 세션이 더 이상 존재하지 않습니다.");
			break;
		case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
			FailureMessage = TEXT("LAN 세션의 연결 주소를 가져오지 못했습니다.");
			break;
		case EOnJoinSessionCompleteResult::AlreadyInSession:
			FailureMessage = TEXT("이미 같은 이름의 LAN 세션에 참가해 있습니다.");
			break;
		default:
			FailureMessage = TEXT("LAN 세션 참가에 실패했습니다.");
			break;
		}

		SetOperationState(
			JoinOperation,
			ESnowRumbleSessionState::Failed,
			FailureMessage);
		return;
	}

	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	FString ConnectString;
	if (!SessionInterface.IsValid()
		|| !SessionInterface->GetResolvedConnectString(
			SessionName,
			ConnectString))
	{
		SetOperationState(
			JoinOperation,
			ESnowRumbleSessionState::Failed,
			TEXT("서버 연결 주소를 확인하지 못했습니다."));
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	APlayerController* PlayerController =
		GameInstance ? GameInstance->GetFirstLocalPlayerController() : nullptr;
	if (!PlayerController)
	{
		SetOperationState(
			JoinOperation,
			ESnowRumbleSessionState::Failed,
			TEXT("로컬 PlayerController를 찾지 못했습니다."));
		return;
	}

	SetOperationState(
		JoinOperation,
		ESnowRumbleSessionState::Succeeded,
		TEXT("LAN 세션 참가에 성공했습니다."));
	bWasInOnlineSession = true;
	PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
}

void USnowRumbleSessionSubsystem::HandleNetworkFailure(
	UWorld* World,
	UNetDriver* NetDriver,
	ENetworkFailure::Type FailureType,
	const FString& ErrorString)
{
	if (!World
		|| World->GetGameInstance() != GetGameInstance())
	{
		return;
	}
	if (World->GetNetMode() != NM_Client
		|| bReturningToMainMenuAfterSessionFailure)
	{
		return;
	}

	const IOnlineSessionPtr SessionInterface = GetSessionInterface();
	const bool bHasNamedSession =
		SessionInterface.IsValid()
		&& SessionInterface->GetNamedSession(LocalSessionName);
	const bool bWasJoiningOrJoined =
		bWasInOnlineSession
		|| bHasNamedSession
		|| CurrentOperation == ESnowRumbleSessionOperation::Join
		|| CurrentOperation == ESnowRumbleSessionOperation::QuickJoin
		|| CurrentOperation == ESnowRumbleSessionOperation::JoinByCode;
	if (!bWasJoiningOrJoined)
	{
		return;
	}

	UE_LOG(
		LogSnowRumbleSession,
		Warning,
		TEXT("Network failure detected. FailureType=%s Error='%s'"),
		ENetworkFailure::ToString(FailureType),
		*ErrorString);

	ReturnToMainMenuAfterSessionFailure(
		TEXT("호스트의 연결이 해제되었습니다."));
}

void USnowRumbleSessionSubsystem::HandleTravelFailure(
	UWorld* World,
	ETravelFailure::Type FailureType,
	const FString& ErrorString)
{
	if (!World
		|| World->GetGameInstance() != GetGameInstance()
		|| World->GetNetMode() != NM_Client
		|| bReturningToMainMenuAfterSessionFailure)
	{
		return;
	}

	const bool bWasJoiningOrJoined =
		bWasInOnlineSession
		|| CurrentOperation == ESnowRumbleSessionOperation::Join
		|| CurrentOperation == ESnowRumbleSessionOperation::QuickJoin
		|| CurrentOperation == ESnowRumbleSessionOperation::JoinByCode;
	if (!bWasJoiningOrJoined)
	{
		return;
	}

	UE_LOG(
		LogSnowRumbleSession,
		Warning,
		TEXT("Travel failure detected. FailureType=%s Error='%s'"),
		ETravelFailure::ToString(FailureType),
		*ErrorString);

	ReturnToMainMenuAfterSessionFailure(
		TEXT("호스트의 연결이 해제되었습니다."));
}

void USnowRumbleSessionSubsystem::ReturnToMainMenuAfterSessionFailure(
	const FString& AlarmMessage)
{
	if (bReturningToMainMenuAfterSessionFailure)
	{
		return;
	}
	bReturningToMainMenuAfterSessionFailure = true;

	UWorld* World = GetWorld();
	LeaveLanSession();
	PendingMainMenuAlarmMessage = AlarmMessage;

	if (World)
	{
		// ClientTravel은 네트워크 월드의 Pawn이 메뉴 월드까지 남는 경우가
		// 있어, 호스트 이탈 복귀에서는 로컬 월드를 완전히 다시 연다.
		UGameplayStatics::OpenLevel(
			World,
			FName(TEXT("/Game/Maps/L_MainMenu")),
			true,
			FString::Printf(
				TEXT("game=%s"),
				SnowRumbleSession::MainMenuGameModeTravelPath));
		return;
	}

	UGameplayStatics::OpenLevel(
		World ? World : GetWorld(),
		FName(TEXT("/Game/Maps/L_MainMenu")),
		true,
		FString::Printf(
			TEXT("game=%s"),
			SnowRumbleSession::MainMenuGameModeTravelPath));
}

FString USnowRumbleSessionSubsystem::NormalizeRoomCode(
	const FString& RoomCode) const
{
	FString Result;
	for (const TCHAR Character : RoomCode)
	{
		if (!FChar::IsWhitespace(Character))
		{
			Result.AppendChar(FChar::ToUpper(Character));
		}
	}
	return Result;
}

FString USnowRumbleSessionSubsystem::GenerateRoomCode() const
{
	FString RoomCode;
	for (int32 Index = 0; Index < SnowRumbleSession::RoomCodeDigits; ++Index)
	{
		RoomCode.AppendChar(
			static_cast<TCHAR>(TEXT('0') + FMath::RandRange(0, 9)));
	}
	return RoomCode;
}

void USnowRumbleSessionSubsystem::ClearCreateSessionDelegate()
{
	if (!CreateSessionCompleteHandle.IsValid())
	{
		return;
	}

	if (IOnlineSessionPtr SessionInterface = GetSessionInterface();
		SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(
			CreateSessionCompleteHandle);
	}
	CreateSessionCompleteHandle.Reset();
}

void USnowRumbleSessionSubsystem::ClearFindSessionsDelegate()
{
	if (!FindSessionsCompleteHandle.IsValid())
	{
		return;
	}

	if (IOnlineSessionPtr SessionInterface = GetSessionInterface();
		SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(
			FindSessionsCompleteHandle);
	}
	FindSessionsCompleteHandle.Reset();
}

void USnowRumbleSessionSubsystem::ClearJoinSessionDelegate()
{
	if (!JoinSessionCompleteHandle.IsValid())
	{
		return;
	}

	if (IOnlineSessionPtr SessionInterface = GetSessionInterface();
		SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(
			JoinSessionCompleteHandle);
	}
	JoinSessionCompleteHandle.Reset();
}

void USnowRumbleSessionSubsystem::DestroyLocalSessionIfPresent(
	const TCHAR* Reason)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid()
		|| !SessionInterface->GetNamedSession(LocalSessionName))
	{
		return;
	}

	UE_LOG(
		LogSnowRumbleSession,
		Log,
		TEXT("Destroying local LAN session. Reason=%s SessionName=%s"),
		Reason ? Reason : TEXT("unknown"),
		*LocalSessionName.ToString());
	SessionInterface->DestroySession(LocalSessionName);
	bWasInOnlineSession = false;
	CurrentRoomCode.Empty();
}

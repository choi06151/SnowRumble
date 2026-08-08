// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnowRumbleSessionSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogSnowRumbleSession, Log, All);

namespace SnowRumbleSession
{
	constexpr int32 MinimumPlayers = 2;
	constexpr int32 MaximumPlayers = 8;
	constexpr int32 MaximumSearchResults = 100;
	constexpr int32 RoomCodeDigits = 6;
	const TCHAR* HostTravelUrl = TEXT("/Game/Maps/L_Lobby?listen");
	const FName RoomNameSettingKey(TEXT("SNOWRUMBLE_ROOM_NAME"));
	const FName RoomCodeSettingKey(TEXT("SNOWRUMBLE_ROOM_CODE"));
	const FName GameModeSettingKey(TEXT("SNOWRUMBLE_GAME_MODE"));
	const TCHAR* TeamPvpModeName = TEXT("TeamPvP");
}

void USnowRumbleSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LocalSessionName = FName(
		*FString::Printf(
			TEXT("SnowRumbleLanSession_%s"),
			*FGuid::NewGuid().ToString(EGuidFormats::Digits)));

	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this,
		&USnowRumbleSessionSubsystem::HandlePostLoadMap);
}

void USnowRumbleSessionSubsystem::Deinitialize()
{
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}

	ClearCreateSessionDelegate();
	ClearFindSessionsDelegate();
	ClearJoinSessionDelegate();
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
			TEXT("listen"));
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

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowInvites = false;
	SessionSettings.bUsesPresence = false;
	SessionSettings.bUseLobbiesIfAvailable = false;
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
		FString(TEXT("L_Lobby")),
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	UE_LOG(
		LogSnowRumbleSession,
		Log,
		TEXT("Creating LAN session. MaxPlayers=%d RoomName='%s' RoomCode=%s"),
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
		TEXT("LAN 세션을 생성하고 있습니다."));

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
	if (!bHostTravelPending
		|| !LoadedWorld
		|| LoadedWorld->GetGameInstance() != GetGameInstance()
		|| LoadedWorld->GetNetMode() != NM_ListenServer)
	{
		return;
	}

	bHostTravelPending = false;
	CreateLanSession(PendingHostMaxPlayers);
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

void USnowRumbleSessionSubsystem::BeginFindLanSessions(
	ESnowRumbleSessionOperation Operation,
	const FString& Message)
{
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
	ActiveSessionSearch->bIsLanQuery = true;
	ActiveSessionSearch->MaxSearchResults = SnowRumbleSession::MaximumSearchResults;
	UE_LOG(
		LogSnowRumbleSession,
		Log,
		TEXT("Begin LAN search. Operation=%d"),
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

	const FOnlineSessionSearchResult& SearchResult =
		ActiveSessionSearch->SearchResults[ResultIndex];
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
		TEXT("LAN 세션에 참가하고 있습니다."));

	if (!SessionInterface->JoinSession(
		0,
		LocalSessionName,
		ActiveSessionSearch->SearchResults[ResultIndex]))
	{
		ClearJoinSessionDelegate();
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
		SessionInfo.MaxPlayers =
			Session.SessionSettings.NumPublicConnections;
		SessionInfo.CurrentPlayers =
			SessionInfo.MaxPlayers - Session.NumOpenPublicConnections;
		SessionInfo.PingMilliseconds = SearchResult.PingInMs;
		UE_LOG(
			LogSnowRumbleSession,
			Log,
			TEXT("Search result %d: Host='%s' RoomName='%s' RoomCode=%s GameMode=%s Players=%d/%d Ping=%d"),
			Index,
			*SessionInfo.HostName,
			*SessionInfo.RoomName,
			*SessionInfo.RoomCode,
			*SessionInfo.GameModeName,
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
			if (SessionInfo.CurrentPlayers < SessionInfo.MaxPlayers)
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
		return;
	}

	if (CurrentOperation == ESnowRumbleSessionOperation::JoinByCode)
	{
		for (const FSnowRumbleSessionInfo& SessionInfo : SearchResults)
		{
			if (NormalizeRoomCode(SessionInfo.RoomCode) == PendingJoinRoomCode)
			{
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
	PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
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

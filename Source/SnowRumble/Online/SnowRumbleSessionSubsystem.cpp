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

namespace SnowRumbleSession
{
	constexpr int32 MinimumPlayers = 2;
	constexpr int32 MaximumPlayers = 16;
	constexpr int32 MaximumSearchResults = 100;
	const TCHAR* HostTravelUrl = TEXT("/Game/Maps/L_Lobby?listen");
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

void USnowRumbleSessionSubsystem::HostLanSession(int32 MaxPlayers)
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

	UWorld* World = GetWorld();
	if (!World)
	{
		SetOperationState(
			ESnowRumbleSessionOperation::Host,
			ESnowRumbleSessionState::Failed,
			TEXT("Host 월드를 찾지 못했습니다."));
		return;
	}

	bHostTravelPending = true;
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
		SETTING_MAPNAME,
		FString(TEXT("L_Lobby")),
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

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
	if (IsOperationInProgress())
	{
		SetOperationState(
			ESnowRumbleSessionOperation::Search,
			ESnowRumbleSessionState::Failed,
			TEXT("다른 세션 요청이 진행 중입니다."));
		return;
	}

	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		SetOperationState(
			ESnowRumbleSessionOperation::Search,
			ESnowRumbleSessionState::Failed,
			TEXT("세션 인터페이스를 사용할 수 없습니다."));
		return;
	}

	SearchResults.Reset();
	ActiveSessionSearch = MakeShared<FOnlineSessionSearch>();
	ActiveSessionSearch->bIsLanQuery = true;
	ActiveSessionSearch->MaxSearchResults = SnowRumbleSession::MaximumSearchResults;

	FindSessionsCompleteHandle =
		SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
			FOnFindSessionsCompleteDelegate::CreateUObject(
				this,
				&USnowRumbleSessionSubsystem::HandleFindSessionsComplete));

	SetOperationState(
		ESnowRumbleSessionOperation::Search,
		ESnowRumbleSessionState::InProgress,
		TEXT("LAN 세션을 검색하고 있습니다."));

	if (!SessionInterface->FindSessions(0, ActiveSessionSearch.ToSharedRef()))
	{
		ClearFindSessionsDelegate();
		ActiveSessionSearch.Reset();
		SetOperationState(
			ESnowRumbleSessionOperation::Search,
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

	JoinSessionCompleteHandle =
		SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
			FOnJoinSessionCompleteDelegate::CreateUObject(
				this,
				&USnowRumbleSessionSubsystem::HandleJoinSessionComplete));

	SetOperationState(
		ESnowRumbleSessionOperation::Join,
		ESnowRumbleSessionState::InProgress,
		TEXT("LAN 세션에 참가하고 있습니다."));

	if (!SessionInterface->JoinSession(
		0,
		LocalSessionName,
		ActiveSessionSearch->SearchResults[ResultIndex]))
	{
		ClearJoinSessionDelegate();
		SetOperationState(
			ESnowRumbleSessionOperation::Join,
			ESnowRumbleSessionState::Failed,
			TEXT("LAN 세션 참가 요청을 시작하지 못했습니다."));
	}
}

const TArray<FSnowRumbleSessionInfo>&
USnowRumbleSessionSubsystem::GetSearchResults() const
{
	return SearchResults;
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
			ESnowRumbleSessionOperation::Search,
			ESnowRumbleSessionState::Failed,
			TEXT("LAN 세션 검색에 실패했습니다."));
		return;
	}

	for (int32 Index = 0; Index < ActiveSessionSearch->SearchResults.Num(); ++Index)
	{
		const FOnlineSessionSearchResult& SearchResult =
			ActiveSessionSearch->SearchResults[Index];
		const FOnlineSession& Session = SearchResult.Session;

		FSnowRumbleSessionInfo SessionInfo;
		SessionInfo.ResultIndex = Index;
		SessionInfo.HostName = Session.OwningUserName;
		SessionInfo.MaxPlayers =
			Session.SessionSettings.NumPublicConnections;
		SessionInfo.CurrentPlayers =
			SessionInfo.MaxPlayers - Session.NumOpenPublicConnections;
		SessionInfo.PingMilliseconds = SearchResult.PingInMs;
		SearchResults.Add(MoveTemp(SessionInfo));
	}

	OnSessionSearchCompleted.Broadcast(SearchResults);
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
			ESnowRumbleSessionOperation::Join,
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
			ESnowRumbleSessionOperation::Join,
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
			ESnowRumbleSessionOperation::Join,
			ESnowRumbleSessionState::Failed,
			TEXT("로컬 PlayerController를 찾지 못했습니다."));
		return;
	}

	SetOperationState(
		ESnowRumbleSessionOperation::Join,
		ESnowRumbleSessionState::Succeeded,
		TEXT("LAN 세션 참가에 성공했습니다."));
	PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
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

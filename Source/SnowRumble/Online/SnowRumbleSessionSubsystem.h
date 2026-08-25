// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SnowRumbleSessionSubsystem.generated.h"

class UWorld;
class UNetDriver;

UENUM(BlueprintType)
enum class ESnowRumbleSessionOperation : uint8
{
	None,
	Host,
	Search,
	Join,
	QuickJoin,
	JoinByCode
};

UENUM(BlueprintType)
enum class ESnowRumbleSessionState : uint8
{
	Idle,
	InProgress,
	Succeeded,
	Failed
};

USTRUCT(BlueprintType)
struct FSnowRumbleSessionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Session")
	int32 ResultIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Session")
	FString HostName;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Session")
	FString RoomName;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Session")
	FString RoomCode;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Session")
	FString GameModeName;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Session")
	FString MapName;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Session")
	int32 CurrentPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Session")
	int32 MaxPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "SnowRumble|Session")
	int32 PingMilliseconds = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnSnowRumbleSessionStateChanged,
	ESnowRumbleSessionOperation,
	Operation,
	ESnowRumbleSessionState,
	State,
	const FString&,
	Message);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnSnowRumbleSessionSearchCompleted,
	const TArray<FSnowRumbleSessionInfo>&,
	Results);

UCLASS(BlueprintType)
class SNOWRUMBLE_API USnowRumbleSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 최대 인원으로 광고되는 NULL LAN Listen Server 세션을 생성한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Session")
	void HostLanSession(int32 MaxPlayers = 8, const FString& RoomName = FString());

	/** 현재 LAN에서 참가 가능한 NULL 세션을 검색한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Session")
	void FindLanSessions();

	/** 빈자리가 있는 첫 번째 LAN 세션을 찾아 자동 참가한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Session")
	void QuickJoinLanSession();

	/** 마지막 검색 결과의 인덱스를 사용해 세션 참가를 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Session")
	void JoinLanSession(int32 ResultIndex);

	/** 방 코드와 일치하는 LAN 세션 참가를 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Session")
	void JoinLanSessionByRoomCode(const FString& RoomCode);

	/** 현재 로컬 호스트 또는 참가 세션을 정리해 메인메뉴에서 다시 검색·참가할 수 있게 한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Session")
	void LeaveLanSession();

	/** 메인메뉴 진입 시 한 번 표시할 세션 종료 알림을 꺼낸다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Session")
	FString ConsumePendingMainMenuAlarmMessage();

	/** 메인메뉴에서 한 번 표시할 알림 메시지를 저장한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Session")
	void SetPendingMainMenuAlarmMessage(const FString& Message);

	/** 마지막으로 변환된 Blueprint용 검색 결과를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Session")
	const TArray<FSnowRumbleSessionInfo>& GetSearchResults() const;

	/** 현재 호스트 또는 참가 요청이 들고 있는 방 코드를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Session")
	FString GetCurrentRoomCode() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|Session")
	ESnowRumbleSessionOperation GetCurrentOperation() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|Session")
	ESnowRumbleSessionState GetCurrentState() const;

	/** Steam 세션 초대 UI를 열고 현재 세션을 초대 대상으로 지정한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Session")
	bool ShowSessionInviteUI();

	UPROPERTY(BlueprintAssignable, Category = "SnowRumble|Session")
	FOnSnowRumbleSessionStateChanged OnSessionStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "SnowRumble|Session")
	FOnSnowRumbleSessionSearchCompleted OnSessionSearchCompleted;

private:
	/** 현재 프로젝트에 설정된 OnlineSubsystem의 세션 인터페이스를 가져온다. */
	IOnlineSessionPtr GetSessionInterface() const;

	/** 현재 OnlineSubsystem이 Steam인지 확인한다. */
	bool IsSteamSubsystem() const;

	/** Listen Server NetDriver가 준비된 뒤 실제 LAN 세션을 생성한다. */
	void CreateLanSession(int32 MaxPlayers);

	/** 요청 목적에 맞는 LAN 세션 검색을 시작한다. */
	void BeginFindLanSessions(
		ESnowRumbleSessionOperation Operation,
		const FString& Message);

	/** 검색 결과 인덱스로 실제 세션 참가를 시작한다. */
	void JoinSearchResult(
		int32 ResultIndex,
		ESnowRumbleSessionOperation Operation);

	/** Host 맵 로드 완료 시 열린 포트를 사용해 LAN 세션 생성을 계속한다. */
	void HandlePostLoadMap(UWorld* LoadedWorld);

	/** 현재 호스트 맵을 세션 검색 광고에 반영해 메인메뉴 참가 대상을 제한한다. */
	void UpdateAdvertisedSessionMap(UWorld* LoadedWorld);

	/** 요청 상태를 저장하고 Blueprint 구독자에게 전달한다. */
	void SetOperationState(
		ESnowRumbleSessionOperation Operation,
		ESnowRumbleSessionState State,
		const FString& Message);

	/** 다른 비동기 세션 요청이 진행 중인지 확인한다. */
	bool IsOperationInProgress() const;

	/** 세션 생성 완료 결과를 처리하고 성공 시 Listen Server 맵으로 이동한다. */
	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	/** 검색 결과를 Blueprint 구조체로 변환한다. */
	void HandleFindSessionsComplete(bool bWasSuccessful);

	/** 참가 결과를 처리하고 성공 시 해석된 서버 주소로 이동한다. */
	void HandleJoinSessionComplete(
		FName SessionName,
		EOnJoinSessionCompleteResult::Type Result);

	/** 호스트 연결 끊김 같은 네트워크 실패를 메인메뉴 알림으로 변환한다. */
	void HandleNetworkFailure(
		UWorld* World,
		UNetDriver* NetDriver,
		ENetworkFailure::Type FailureType,
		const FString& ErrorString);

	/** 접속 URL 해석 실패 같은 travel 실패를 메인메뉴 알림으로 변환한다. */
	void HandleTravelFailure(
		UWorld* World,
		ETravelFailure::Type FailureType,
		const FString& ErrorString);

	/** 세션 연결 실패 또는 호스트 이탈 후 메인메뉴로 복귀한다. */
	void ReturnToMainMenuAfterSessionFailure(const FString& AlarmMessage);

	/** 등록된 세션 생성 완료 델리게이트를 해제한다. */
	void ClearCreateSessionDelegate();

	/** 등록된 세션 검색 완료 델리게이트를 해제한다. */
	void ClearFindSessionsDelegate();

	/** 등록된 세션 참가 완료 델리게이트를 해제한다. */
	void ClearJoinSessionDelegate();

	/** 실패한 참가 시도 뒤 남은 로컬 named session을 정리한다. */
	void DestroyLocalSessionIfPresent(const TCHAR* Reason);

	/** UI 입력 또는 세션 광고용 방 코드를 정규화한다. */
	FString NormalizeRoomCode(const FString& RoomCode) const;

	/** 새 방 생성에 사용할 짧은 숫자 코드를 만든다. */
	FString GenerateRoomCode() const;

	TSharedPtr<FOnlineSessionSearch> ActiveSessionSearch;
	TArray<FSnowRumbleSessionInfo> SearchResults;

	FDelegateHandle CreateSessionCompleteHandle;
	FDelegateHandle FindSessionsCompleteHandle;
	FDelegateHandle JoinSessionCompleteHandle;
	FDelegateHandle PostLoadMapHandle;
	FDelegateHandle NetworkFailureHandle;
	FDelegateHandle TravelFailureHandle;

	FName LocalSessionName;
	int32 PendingHostMaxPlayers = 8;
	FString PendingHostRoomName;
	FString PendingHostRoomCode;
	FString PendingJoinRoomCode;
	FString PendingMainMenuAlarmMessage;
	FString CurrentRoomCode;
	bool bHostTravelPending = false;
	bool bWasInLanSession = false;
	ESnowRumbleSessionOperation CurrentOperation = ESnowRumbleSessionOperation::None;
	ESnowRumbleSessionState CurrentState = ESnowRumbleSessionState::Idle;
};

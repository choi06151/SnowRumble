// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SnowRumblePlayerState.generated.h"

UENUM(BlueprintType)
enum class ESnowRumbleTeam : uint8
{
	None,
	Red,
	Blue
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSnowRumbleLobbyPlayerChanged);

UCLASS()
class SNOWRUMBLE_API ASnowRumblePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	/** 대기방 UI에 표시할 플레이어 이름을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Lobby")
	FString GetLobbyPlayerName() const;

	/** 현재 선택한 팀을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Lobby")
	ESnowRumbleTeam GetLobbyTeam() const;

	/** 현재 준비 상태를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Lobby")
	bool IsLobbyReady() const;

	/** 서버가 랜덤 팀 배정 결과를 적용한다. 클라이언트 직접 팀 선택에는 사용하지 않는다. */
	void AssignLobbyTeamFromServer(ESnowRumbleTeam NewTeam);

	/** 소유 클라이언트가 대기방 이름 변경을 서버에 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Lobby")
	void RequestSetLobbyPlayerName(const FString& NewName);

	/** 소유 클라이언트가 팀 변경을 서버에 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Lobby")
	void RequestSetLobbyTeam(ESnowRumbleTeam NewTeam);

	/** 소유 클라이언트가 준비 상태 변경을 서버에 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Lobby")
	void RequestSetLobbyReady(bool bNewReady);

	/** 호스트가 경기 시작을 서버에 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Lobby")
	void RequestStartLobbyMatch();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable, Category = "SnowRumble|Lobby")
	FOnSnowRumbleLobbyPlayerChanged OnLobbyPlayerChanged;

protected:
	/** 복제된 대기방 이름 변경을 UI에 알린다. */
	UFUNCTION()
	void OnRep_LobbyPlayerName();

	/** 복제된 팀 변경을 UI에 알린다. */
	UFUNCTION()
	void OnRep_LobbyTeam();

	/** 복제된 준비 상태 변경을 UI에 알린다. */
	UFUNCTION()
	void OnRep_LobbyReady();

	UFUNCTION(Server, Reliable)
	void ServerSetLobbyPlayerName(const FString& NewName);

	UFUNCTION(Server, Reliable)
	void ServerSetLobbyTeam(ESnowRumbleTeam NewTeam);

	UFUNCTION(Server, Reliable)
	void ServerSetLobbyReady(bool bNewReady);

	UFUNCTION(Server, Reliable)
	void ServerRequestStartLobbyMatch();

private:
	/** 서버에서 변경된 대기방 플레이어 정보를 GameState와 UI에 알린다. */
	void BroadcastLobbyPlayerChanged();

	/** 서버에 저장하기 전에 플레이어 이름 길이와 공백을 정리한다. */
	FString SanitizeLobbyPlayerName(const FString& NewName) const;

	UPROPERTY(ReplicatedUsing = OnRep_LobbyPlayerName)
	FString LobbyPlayerName;

	UPROPERTY(ReplicatedUsing = OnRep_LobbyTeam)
	ESnowRumbleTeam LobbyTeam = ESnowRumbleTeam::None;

	UPROPERTY(ReplicatedUsing = OnRep_LobbyReady)
	bool bLobbyReady = false;
};

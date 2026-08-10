// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../Interaction/LobbyInteractionBoard_C.h"
#include "../Game/SnowRumblePlayerState.h"
#include "GameFramework/Character.h"
#include "SnowRumbleCharacter.generated.h"

class UCameraComponent;
class UDamageType;
class UInputAction;
class UInputMappingContext;
class UAnimMontage;
class UEmoteRadialMenuWidget;
class UMainHUDWidget;
class UOverheadNameplateWidget;
class UNiagaraComponent;
class UOutlineComponent;
class USceneComponent;
class USphereComponent;
class USnowRumbleHealthComponent;
class USnowballCreationComponent;
class USnowballEquipmentComponent;
class USpringArmComponent;
class UWidgetInteractionComponent;
class UWidgetComponent;
class AController;
class ALobbyInteractionBoard;
class ASnowballItem;
struct FDamageEvent;
struct FInputActionValue;

UENUM(BlueprintType)
enum class ESnowballCarryState : uint8
{
	Normal,
	SmallSnowball,
	LargeSnowball
};

UENUM(BlueprintType)
enum class ESnowballActionState : uint8
{
	None,
	RollingSnowball
};

UENUM(BlueprintType)
enum class ESnowRumbleTimedActionState : uint8
{
	None,
	CreatingSnowball,
	RollingSnowball
};

UCLASS()
class SNOWRUMBLE_API ASnowRumbleCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASnowRumbleCharacter();

	virtual void Tick(float DeltaSeconds) override;

	/** Animation Blueprint에서 수평 이동 중인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsMoving() const;

	/** Animation Blueprint에서 공중 상태인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsInAir() const;

	/** Animation Blueprint에서 실제 스프린트 이동 중인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsSprinting() const;

	/** Animation Blueprint에서 얼기 상태인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsFrozen() const;

	/** Animation Blueprint와 UI에서 라운드 사망 상태인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Health")
	bool IsDead() const;

	/** UI에서 얼음 상태 사망까지 남은 시간을 표시하기 위해 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Health")
	float GetFrozenSecondsRemaining() const;

	/** Animation Blueprint와 UI에서 눈덩이 장착 여부를 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsHoldingSnowball() const;

	/** Animation Blueprint에서 평소·작은 눈·큰 눈 운반 상태를 구분한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	ESnowballCarryState GetSnowballCarryState() const;

	/** Animation Blueprint에서 운반 상태와 별개인 눈덩이 행동 상태를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	ESnowballActionState GetSnowballActionState() const;

	/** Animation Blueprint에서 눈덩이 조준 상태인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsAiming() const;

	/** Animation Blueprint에서 눈덩이 투척 충전 중인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsChargingSnowball() const;

	/** UI에서 사용할 0~1 정규화된 눈덩이 충전량을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	float GetSnowballChargeProgress() const;

	/** Animation Blueprint에서 눈덩이 제작 중인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsCreatingSnowball() const;

	/** Animation Blueprint에서 아이템 획득 동작 중인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsPickingUpItem() const;

	/** UI에서 사용할 0~1 정규화된 눈덩이 제작 진행도를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Snowball")
	float GetSnowballCreationProgress() const;

	/** UI에서 현재 머리 위에 표시할 진행형 행동 종류를 구분한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI")
	ESnowRumbleTimedActionState GetTimedActionState() const;

	/** 로컬 플레이어에게 현재 옵션 설정 기준 입력 매핑 컨텍스트를 적용한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Input")
	void ApplyInputMappingContext();

	/** UI에서 현재 머리 위 행동의 0~1 정규화된 진행도를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI")
	float GetTimedActionProgress() const;

	/** UI에서 선택한 이모션 인덱스를 서버에 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Emote")
	void RequestPlayEmote(int32 EmoteIndex);

	/** 로컬 플레이어 화면에서 이모션 원형 메뉴를 닫고 게임 입력으로 복구한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Emote|UI")
	void CloseEmoteRadialMenu();

	/** 눈덩이를 부착할 캐릭터의 조정 가능한 장착 위치를 반환한다. */
	USceneComponent* GetSnowballHoldPoint() const;

	/** 서버에서 굴리기 전용 충돌 프록시를 눈덩이 위치와 크기로 활성화한다. */
	void EnableRollingSnowballCollision(
		const FVector& InitialLocation,
		float CollisionRadius);

	/** 서버에서 굴리기 충돌 프록시를 목표 위치까지 Sweep하고 충돌 여부를 반환한다. */
	bool MoveRollingSnowballCollision(
		const FVector& TargetLocation,
		float CollisionRadius,
		FHitResult& OutSweepHit);

	/** 현재 굴리기 충돌 프록시의 서버 확정 위치를 반환한다. */
	FVector GetRollingSnowballCollisionLocation() const;

	/** 굴리기 종료 시 서버 충돌 프록시를 비활성화한다. */
	void DisableRollingSnowballCollision();

	/** 서버에서 아이템 획득 성공 애니메이션 상태를 시작한다. */
	void NotifyItemPickupSucceeded();

	/** 보유 장비가 바뀌면 스프린트와 현재 최대 이동속도를 다시 적용한다. */
	void RefreshHeldEquipmentMovementState();

	/** 서버에서 받은 피해를 HP 컴포넌트에 전달한다. */
	virtual float TakeDamage(
		float DamageAmount,
		FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser) override;

	/** 머리 위 이름표 WBP가 표시할 닉네임을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Identity")
	FString GetOverheadPlayerName() const;

	/** 머리 위 이름표 WBP가 적용할 팀 색을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Identity")
	FLinearColor GetOverheadTeamColor() const;

	/** 서버가 확정한 게시판 상호작용에 맞춰 소유 클라이언트 카메라를 게시판으로 돌린다. */
	UFUNCTION(Client, Reliable)
	void ClientFocusLobbyBoard(ALobbyInteractionBoard* Board);

	/** 로컬 플레이어 화면의 게시판 포커스를 해제하고 게임 입력으로 복구한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Lobby|Board")
	void CloseLobbyBoardFocus();

	/** 포커스 중인 게시판 UI 버튼 액션을 서버 검증 요청으로 전달한다. */
	void RequestLobbyBoardAction(ELobbyBoardAction BoardAction);

	/** 포커스 중인 게시판 팀 색 선택을 서버 검증 요청으로 전달한다. */
	void RequestLobbyTeamSelection(ESnowRumbleTeam NewTeam);

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_PlayerState() override;
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** 이동 입력을 카메라의 수평 방향 기준 이동으로 변환한다. */
	void Move(const FInputActionValue& Value);

	/** 마우스 입력을 로컬 플레이어의 카메라 회전에 적용한다. */
	void Look(const FInputActionValue& Value);

	/** 점프 입력이 시작되면 캐릭터 점프를 요청한다. */
	void StartJump();

	/** 점프 입력이 끝나면 점프 요청을 해제한다. */
	void StopJump();

	/** 스프린트 입력이 시작되면 로컬 예측과 서버 요청을 시작한다. */
	void HandleSprintStarted();

	/** 스프린트 입력이 끝나면 로컬 예측과 서버 상태를 해제한다. */
	void HandleSprintCompleted();

	/** 상호작용 입력의 누름 또는 해제 상태를 Blueprint에 전달한다. */
	void HandleInteractStarted();

	/** 상호작용 입력 해제 상태를 Blueprint에 전달한다. */
	void HandleInteractCompleted();

	/** 조준 입력 누름 상태를 Blueprint에 전달한다. */
	void HandleAimStarted();

	/** 조준 입력 해제 상태를 Blueprint에 전달한다. */
	void HandleAimCompleted();

	/** 상황별 행동 입력 누름 상태를 Blueprint에 전달한다. */
	void HandleActionStarted();

	/** 상황별 행동 입력 해제 상태를 Blueprint에 전달한다. */
	void HandleActionCompleted();

	/** 장비 내려놓기 입력을 Blueprint에 전달한다. */
	void HandleDropEquipment();

	/** 게시판 포커스 중 마우스 좌클릭을 월드 위젯 누름으로 전달한다. */
	void HandleLobbyBoardPointerPressed();

	/** 게시판 포커스 중 마우스 좌클릭 해제를 월드 위젯 해제로 전달한다. */
	void HandleLobbyBoardPointerReleased();

	/** 포커스 중인 게시판 위젯을 마우스 화면 좌표 기준 trace로 갱신한다. */
	bool UpdateLobbyBoardWidgetHitResult();

	/** 멀티 PIE/listen에서 월드 UI 입력이 서로 충돌하지 않도록 고유 포인터 값을 설정한다. */
	void ConfigureLobbyBoardWidgetInteraction(APlayerController* PlayerController);

	/** 이모션 입력 누름 상태를 Blueprint에 전달한다. */
	void HandleEmoteStarted();

	/** 이모션 입력 해제 상태를 Blueprint에 전달한다. */
	void HandleEmoteCompleted();

	/** 로컬 PlayerCameraManager에 안전한 상하 시야각을 적용한다. */
	void ApplyCameraPitchLimits();

	/** PlayerState 닉네임 변경 이벤트에 머리 위 이름표 갱신을 연결한다. */
	void BindOverheadNameToPlayerState();

	/** 에디터와 런타임에서 이름표 컴포넌트 위치와 클래스를 현재 설정값으로 맞춘다. */
	void RefreshOverheadNameplateComponentSettings();

	/** 월드 공간 이름표가 로컬 카메라를 향하도록 회전시킨다. */
	void RefreshOverheadNameplateFacing();

	/** 자신이 조종하는 캐릭터의 카메라에서만 눈 VFX를 활성화한다. */
	void RefreshLocalSnowEffect();

	/** PvP 맵의 로컬 플레이어 카메라에서만 눈 VFX를 표시해야 하는지 확인한다. */
	bool ShouldShowLocalSnowEffect() const;

	/** 로컬 플레이어용 이모션 원형 메뉴 위젯을 필요할 때 생성한다. */
	void EnsureEmoteRadialMenuWidget();

	/** 로컬 플레이어용 메인 HUD 위젯을 필요할 때 생성한다. */
	void EnsureMainHUDWidget();

	/** 로컬 플레이어 화면에서 이모션 원형 메뉴를 연다. */
	void OpenEmoteRadialMenu();

	/** 로컬 화면에 굴리기 충돌 프록시 범위를 디버그 Sphere로 표시한다. */
	void DrawRollingSnowballCollisionDebug() const;

	/** 얼기 상태에 따라 캐릭터 이동을 중지하거나 복구한다. */
	UFUNCTION()
	void HandleFrozenChanged(bool bIsFrozen);

	/** 사망 상태에 따라 캐릭터 이동과 행동을 중지한다. */
	UFUNCTION()
	void HandleDeathChanged(bool bIsDead);

	/** 조준 상태에 따라 로컬 카메라와 모든 화면의 이동속도를 갱신한다. */
	UFUNCTION()
	void HandleSnowballAimingChanged(bool bNewAiming);

	/** 복제된 PlayerState 닉네임으로 머리 위 이름표를 갱신한다. */
	UFUNCTION()
	void RefreshOverheadPlayerName();

	/** 로컬 플레이어가 상호작용할 가장 가까운 로비 게시판을 찾는다. */
	ALobbyInteractionBoard* FindClosestLobbyBoardCandidate() const;

	/** 소유 플레이어가 가까운 로비 게시판 상호작용을 서버에 요청한다. */
	void TryInteractWithLobbyBoard();

	/** 로컬 플레이어 화면의 로비 게시판 카메라 포커스를 원래 캐릭터 카메라로 복구한다. */
	void ClearLobbyBoardFocus();

	/** 서버가 현재 위치와 상태를 검사해 로비 게시판 상호작용을 확정한다. */
	UFUNCTION(Server, Reliable)
	void ServerTryInteractWithLobbyBoard(ALobbyInteractionBoard* Board);

	/** 서버가 현재 포커스 대상과 버튼 액션을 검사해 게시판 이벤트를 확정한다. */
	UFUNCTION(Server, Reliable)
	void ServerRequestLobbyBoardAction(
		ALobbyInteractionBoard* Board,
		ELobbyBoardAction BoardAction);

	/** 서버가 현재 포커스 게시판과 팀 색 요청을 검사해 PlayerState 팀을 변경한다. */
	UFUNCTION(Server, Reliable)
	void ServerRequestLobbyTeamSelection(
		ALobbyInteractionBoard* Board,
		ESnowRumbleTeam NewTeam);

	/** 현재 캐릭터가 이동과 일반 행동을 수행할 수 있는지 확인한다. */
	bool CanPerformGameplayAction() const;

	/** PvP 시작 카운트다운으로 입력이 잠겨 있는지 확인한다. */
	bool IsPvpMatchInputLocked() const;

	/** PvP 시작 잠금 상태에 맞춰 로컬 컨트롤러 입력 연결을 차단하거나 복구한다. */
	void RefreshPvpMatchInputLock();

	/** 스프린트 상태에 맞는 최대 이동속도를 CharacterMovement에 적용한다. */
	void ApplyMovementSpeed();

	/** 유효한 이모션 인덱스인지 확인한다. */
	bool IsValidEmoteIndex(int32 EmoteIndex) const;

	/** 현재 캐릭터에서 이모션 몽타주를 실행할 수 있는지 확인한다. */
	bool CanPlayEmote() const;

	/** 로컬 AnimInstance에 선택된 이모션 몽타주를 재생한다. */
	void PlayEmoteMontage(int32 EmoteIndex);

	/** 서버가 소유 클라이언트의 이모션 선택을 검사하고 확정한다. */
	UFUNCTION(Server, Reliable)
	void ServerRequestPlayEmote(int32 EmoteIndex);

	/** 서버가 확정한 이모션 몽타주를 모든 화면에서 재생한다. */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayEmote(int32 EmoteIndex);

	/** 서버가 소유 클라이언트의 스프린트 상태 요청을 검사하고 확정한다. */
	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bNewSprinting);

	/** 복제된 스프린트 상태를 다른 화면의 이동속도와 표현에 적용한다. */
	UFUNCTION()
	void OnRep_IsSprinting();

	/** 서버에서 아이템 획득 애니메이션 상태를 종료한다. */
	void FinishPickupAnimationState();

	/** 복제된 획득 상태에 따라 이동 잠금과 복구를 적용한다. */
	UFUNCTION()
	void OnRep_IsPickingUpItem();

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Input")
	void OnInteractInput(bool bPressed);

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Input")
	void OnAimInput(bool bPressed);

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Input")
	void OnActionInput(bool bPressed);

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Input")
	void OnDropEquipmentInput();

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Input")
	void OnEmoteInput(bool bPressed);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Camera")
	TObjectPtr<UNiagaraComponent> LocalSnowEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Health")
	TObjectPtr<USnowRumbleHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball")
	TObjectPtr<USnowballEquipmentComponent> SnowballEquipmentComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball")
	TObjectPtr<USnowballCreationComponent> SnowballCreationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball|Rolling")
	TObjectPtr<USphereComponent> RollingSnowballCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball")
	TObjectPtr<USceneComponent> SnowballHoldPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Interaction")
	TObjectPtr<UOutlineComponent> OutlineComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Interaction")
	TObjectPtr<UWidgetInteractionComponent> LobbyBoardWidgetInteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Identity")
	TObjectPtr<UWidgetComponent> OverheadNameplateComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Identity")
	TSubclassOf<UOverheadNameplateWidget> OverheadNameplateWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Identity")
	FVector OverheadNameRelativeLocation = FVector(0.0f, 0.0f, 130.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Identity", meta = (ClampMin = "1.0"))
	FVector2D OverheadNameplateDrawSize = FVector2D(220.0f, 64.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Identity", meta = (ClampMin = "0.001"))
	float OverheadNameplateWorldScale = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputMappingContext> PlayerMappingContext;

	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> RuntimePlayerMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> ActionAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> DropEquipmentAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> EmoteAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Movement", meta = (ClampMin = "0.0"))
	float WalkSpeed = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Movement", meta = (ClampMin = "0.0"))
	float SprintSpeed = 750.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Movement", meta = (ClampMin = "0.0"))
	float AimWalkSpeed = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snowball|Rolling|Debug")
	bool bDrawRollingSnowballCollisionDebug = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera", meta = (ClampMin = "5.0", ClampMax = "170.0"))
	float AimFieldOfView = 75.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera", meta = (ClampMin = "0.0"))
	float AimFieldOfViewInterpSpeed = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera", meta = (ClampMin = "0.0"))
	float DefaultShoulderOffset = 75.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera", meta = (ClampMin = "0.0"))
	float AimShoulderOffset = 90.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera", meta = (ClampMin = "0.0"))
	float AimCameraArmLength = 340.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera", meta = (ClampMin = "0.0"))
	float CameraPositionInterpSpeed = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera", meta = (ClampMin = "0.0"))
	float PostThrowCameraHoldSeconds = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera")
	float CameraPivotHeight = 65.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera", meta = (ClampMin = "-89.0", ClampMax = "0.0"))
	float CameraViewPitchMin = -65.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera", meta = (ClampMin = "0.0", ClampMax = "89.0"))
	float CameraViewPitchMax = 55.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation", meta = (ClampMin = "0.01"))
	float PickupAnimationStateDuration = 0.6f;

	/** 원형 선택 UI의 8개 칸에 대응하는 이모션 몽타주 슬롯이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Emote")
	TArray<TObjectPtr<UAnimMontage>> EmoteMontages;

	/** 로컬 플레이어 화면에 생성할 이모션 원형 메뉴 위젯 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Emote|UI")
	TSubclassOf<UEmoteRadialMenuWidget> EmoteRadialMenuWidgetClass;

	/** 로컬 플레이어가 소유한 이모션 원형 메뉴 위젯 인스턴스다. */
	UPROPERTY(Transient)
	TObjectPtr<UEmoteRadialMenuWidget> EmoteRadialMenuWidget;

	/** 로컬 플레이어 화면에 생성할 메인 HUD 위젯 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI")
	TSubclassOf<UMainHUDWidget> MainHUDWidgetClass;

	/** 로컬 플레이어가 소유한 메인 HUD 위젯 인스턴스다. */
	UPROPERTY(Transient)
	TObjectPtr<UMainHUDWidget> MainHUDWidget;

	bool bIsEmoteRadialMenuOpen = false;
	bool bPvpMatchInputIgnoreApplied = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_IsSprinting, Category = "SnowRumble|Movement")
	bool bIsSprinting = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_IsPickingUpItem, Category = "SnowRumble|Animation")
	bool bIsPickingUpItem = false;

	float DefaultFieldOfView = 90.0f;
	FVector DefaultCameraSocketOffset = FVector::ZeroVector;
	float DefaultCameraArmLength = 400.0f;
	float CameraShoulderSide = 1.0f;
	double PostThrowAimCameraEndTime = -1.0;

	FTimerHandle PickupAnimationTimerHandle;

	bool bIsInteractHeld = false;
	bool bUsedInteractForRolling = false;
	bool bLobbyBoardPointerPressed = false;
	bool bLocalSnowEffectActive = false;

	UPROPERTY(Transient)
	TObjectPtr<ALobbyInteractionBoard> FocusedLobbyBoard;

};

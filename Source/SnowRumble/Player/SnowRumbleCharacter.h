// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../Interaction/LobbyInteractionBoard_C.h"
#include "../Item/GiftItemTypes_C.h"
#include "../Game/SnowRumblePlayerState.h"
#include "GameFramework/Character.h"
#include "SnowRumbleCharacterAnimationTypes_C.h"
#include "SnowRumbleCharacter.generated.h"

class UCameraComponent;
class UDamageType;
class UInputAction;
class UInputMappingContext;
class UAnimMontage;
class UEmoteRadialMenuWidget;
class UGiftItemEffectComponent;
class UInteractionPromptWidget;
class UKeyGuideWidget;
class UMainHUDWidget;
class UMaterialInstanceDynamic;
class UOverheadNameplateWidget;
class UNiagaraComponent;
class UOutlineComponent;
class UPlayerGrabComponent;
class USceneComponent;
class USphereComponent;
class UStaticMesh;
class UStaticMeshComponent;
class USnowRumbleHealthComponent;
class USnowballCreationComponent;
class USnowballEquipmentComponent;
class USpringArmComponent;
class UUserWidget;
class UCanvas;
class UCanvasRenderTarget2D;
class UTexture;
class UWidgetInteractionComponent;
class UWidgetComponent;
class USoundAttenuation;
class USoundBase;
class AController;
class AGiftBox;
class AGiftBoxItemPickup;
class ALobbyInteractionBoard;
class APhotoInteractionActor;
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
	RollingSnowball,
	Frozen
};

UENUM(BlueprintType)
enum class ESnowRumbleHeldAnimationState : uint8
{
	BareHands,
	SmallSnowball,
	LargeSnowball,
	SnowShovel,
	SnowDuckMaker
};

UENUM(BlueprintType)
enum class ESnowRumbleCharacterFeedbackSoundType : uint8
{
	ItemPickup,
	SnowballPickup,
	SnowballThrow,
	ItemInteraction,
	LobbyBoardInteraction
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

	/** UI에서 얼음 사망 타이머를 1에서 0으로 표시할 진행도를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Health")
	float GetFrozenProgress() const;

	/** Animation Blueprint와 UI에서 눈덩이 장착 여부를 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsHoldingSnowball() const;

	/** Animation Blueprint에서 평소·작은 눈·큰 눈 운반 상태를 구분한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	ESnowballCarryState GetSnowballCarryState() const;

	/** Animation Blueprint에서 맨손, 눈덩이, 장착 도구 자세를 한 값으로 구분한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	ESnowRumbleHeldAnimationState GetHeldAnimationState() const;

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

	/** Animation Blueprint에서 선물상자와 선물 아이템 상호작용 중인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsInteractingWithItem() const;

	/** Animation Blueprint에서 피격 반응 중인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	bool IsHitReacting() const;

	/** Animation Blueprint와 Control Rig에서 팔 뻗기 잡기 상태인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	bool IsGrabReaching() const;

	/** Animation Blueprint와 Control Rig에서 현재 다른 캐릭터를 잡았는지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	bool IsGrabbingCharacter() const;

	/** Animation Blueprint와 Control Rig에서 손이 캐릭터나 월드에 붙었는지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	bool IsGrabAttached() const;

	/** Animation Blueprint와 Control Rig에서 벽이나 월드 오브젝트에 매달린 상태인지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	bool IsHangingFromWorldGrab() const;

	/** Animation Blueprint와 Control Rig에서 이 캐릭터가 다른 캐릭터에게 잡혔는지 확인한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	bool IsGrabbedByCharacter() const;

	/** Control Rig가 붙은 손을 고정할 월드 위치를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	FVector GetGrabAttachedWorldLocation() const;

	/** 잡힌 플레이어 Control Rig가 목/상체 보정에 사용할 잡힌 월드 위치를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	FVector GetGrabbedByCharacterWorldLocation() const;

	/** Control Rig가 사용할 오른손 잡기 목표 월드 위치를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	FVector GetRightHandGrabTargetLocation() const;

	/** Control Rig가 사용할 왼손 잡기 목표 월드 위치를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	FVector GetLeftHandGrabTargetLocation() const;

	/** 잡기 손 IK와 AnimDynamics 보간에 사용할 0~1 alpha를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	float GetGrabReachAlpha() const;

	/** 잡기 제한 시간이 남은 비율을 1에서 0으로 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Grab")
	float GetGrabRemainingTimeProgress() const;

	/** Control Rig spine 보정에 사용할 현재 시점 pitch 각도를 -180~180 범위로 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	float GetViewPitchDegrees() const;

	/** Control Rig spine 보정에 사용할 현재 시점 pitch를 0~1 값으로 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	float GetViewPitchAlpha() const;

	/** Control Rig spine 보정에 사용할 현재 시점 yaw 차이를 -180~180 범위로 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	float GetViewYawDegrees() const;

	/** Control Rig spine 보정에 사용할 현재 시점 yaw 차이를 -0.5~0.5 값으로 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Animation")
	float GetViewYawAlpha() const;

	/** 잡기 컴포넌트가 서버와 로컬에서 잡기 reach를 시작할 수 있는지 확인한다. */
	bool CanStartPlayerGrabReach() const;

	/** 빈손 좌클릭에서 잡기 대신 눈 제작을 우선할 만큼 아래를 보는지 확인한다. */
	bool ShouldPreferSnowCreationOverGrab() const;

	/** 서버가 이 캐릭터를 잡힌 상태로 만들고 이동을 잠근다. */
	void ApplyGrabbedByCharacter(ASnowRumbleCharacter* GrabbingCharacter);

	/** 서버가 잡힌 캐릭터의 AnimBP용 잡힌 월드 위치를 갱신한다. */
	void SetGrabbedByCharacterWorldLocationFromServer(
		const FVector& NewWorldLocation);

	/** 서버가 이 캐릭터의 잡힌 상태와 이동 잠금을 해제한다. */
	void ClearGrabbedByCharacter(ASnowRumbleCharacter* ExpectedGrabbingCharacter);

	/** 서버가 벽잡기 매달림 상태에 맞춰 입력과 이동 상태를 정리한다. */
	void HandleWorldGrabChanged(bool bNewWorldGrab);

	/** 잡기 손 목표 위치를 계산할 몸통 기준 높이를 반환한다. */
	float GetGrabReachOriginHeight() const;

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

	/** AnimNotify나 Blueprint에서 발이 닿은 순간 호출해 눈 밟힘 표현을 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Footstep")
	void RequestSnowFootstepEffect(FName FootSocketName);

	/** 던지기 몽타주의 AnimNotify 시점에 보류 중인 눈덩이 투척을 확정한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Snowball")
	void RequestSnowballThrowReleaseFromNotify();

	/** 로컬 플레이어 화면에서 이모션 원형 메뉴를 닫고 게임 입력으로 복구한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Emote|UI")
	void CloseEmoteRadialMenu();

	/** 로컬 플레이어 화면에서 키 가이드 위젯을 닫고 게임 입력으로 복구한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|KeyGuide|UI")
	void CloseKeyGuideWidget();

	/** 눈덩이를 부착할 캐릭터의 조정 가능한 장착 위치를 반환한다. */
	USceneComponent* GetSnowballHoldPoint() const;

	/** 눈덩이 크기에 맞는 캐릭터의 조정 가능한 장착 위치를 반환한다. */
	USceneComponent* GetSnowballHoldPointForSnowball(
		const ASnowballItem* Snowball) const;

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

	/** 서버에서 눈덩이 획득 성공 one-shot 애니메이션을 포함해 상태를 시작한다. */
	void NotifySnowballPickupSucceeded(bool bWasLargeSnowball);

	/** 서버에서 눈덩이 던지기 성공 one-shot 애니메이션을 모든 화면에 요청한다. */
	void NotifySnowballThrowSucceeded(bool bWasLargeSnowball);

	/** 서버에서 선물상자나 선물 아이템 상호작용 성공 애니메이션 상태를 시작한다. */
	void NotifyItemInteractionSucceeded();

	/** 서버에서 로비 게시판 상호작용 성공 애니메이션 상태를 시작한다. */
	void NotifyLobbyBoardInteractionSucceeded();

	/** 서버에서 선물상자 아이템 효과를 캐릭터에 적용한다. */
	bool ApplyGiftBoxItemEffectFromServer(ESnowRumbleGiftItemType ItemType);

	/** 서버가 단판 승부 비참가자를 관전자 상태로 전환한다. */
	void SetTiebreakerSpectatorFromServer(bool bNewTiebreakerSpectator);

	/** 서버가 물 침수 중 점프 차단 상태를 갱신한다. */
	void SetWaterSubmergedFromServer(bool bNewWaterSubmerged);

	/** 현재 아이템 효과 기준 눈덩이 제작 시간 배율을 반환한다. */
	float GetSnowballCreationDurationMultiplier() const;

	/** 현재 아이템 효과 기준 눈덩이 피해 배율을 반환한다. */
	float GetSnowballDamageMultiplier() const;

	/** 현재 눈오리 제작기 또는 황금 눈오리 제작기를 장착 중인지 확인한다. */
	bool HasEquippedSnowDuckMaker() const;

	/** 보유 장비가 바뀌면 스프린트와 현재 최대 이동속도를 다시 적용한다. */
	void RefreshHeldEquipmentMovementState();

	/** 서버에서 받은 피해를 HP 컴포넌트에 전달한다. */
	virtual float TakeDamage(
		float DamageAmount,
		FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser) override;

	/** 로컬 화면에서만 피격 화면 이펙트와 카메라 흔들림을 재생한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Damage")
	void OnLocalDamageFeedbackRequested(
		float AppliedDamage,
		FVector DamageCauserLocation);

	/** 머리 위 이름표 WBP가 표시할 닉네임을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Identity")
	FString GetOverheadPlayerName() const;

	/** 머리 위 이름표 WBP가 적용할 팀 색을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Identity")
	FLinearColor GetOverheadTeamColor() const;

	/** 캐릭터 외형 커스터마이징 데이터를 즉시 적용한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void ApplyCustomizationData(
		const FSnowRumbleCustomizationData& NewCustomizationData);

	/** 현재 캐릭터에 마지막으로 적용된 커스터마이징 데이터를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Customization")
	FSnowRumbleCustomizationData GetAppliedCustomizationData() const;

	/** 커스터마이징 드로잉 RenderTarget을 캐릭터 머티리얼에 적용한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Customization")
	void SetCustomizationPaintTexture(UTexture* PaintTexture);

	UFUNCTION(BlueprintPure, Category = "SnowRumble|Customization|Hat")
	int32 GetCustomizationHatOptionCount() const;

	UFUNCTION(BlueprintPure, Category = "SnowRumble|Customization|Hat")
	int32 NormalizeCustomizationHatMeshIndex(int32 HatMeshIndex) const;

	/** 서버가 확정한 게시판 상호작용에 맞춰 소유 클라이언트 카메라를 게시판으로 돌린다. */
	UFUNCTION(Client, Reliable)
	void ClientFocusLobbyBoard(ALobbyInteractionBoard* Board);

	/** 로컬 플레이어 화면의 게시판 포커스를 해제하고 게임 입력으로 복구한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Lobby|Board")
	void CloseLobbyBoardFocus();

	/** 서버가 확정한 사진 촬영 액터의 카메라를 로컬 화면에 적용한다. */
	UFUNCTION(Client, Reliable)
	void ClientFocusPhotoActor(APhotoInteractionActor* PhotoActor);

	/** 사진 촬영 액터 포커스를 해제하고 원래 캐릭터 카메라로 돌아간다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Photo")
	void ClosePhotoActorFocus();

	/** 현재 로컬 상호작용 후보 기준 안내 문구를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|Interaction")
	FText GetCurrentInteractionPromptText() const;

	/** 포커스 중인 게시판 UI 버튼 액션을 서버 검증 요청으로 전달한다. */
	void RequestLobbyBoardAction(ELobbyBoardAction BoardAction);

	/** 포커스 중인 게시판 팀 색 선택을 서버 검증 요청으로 전달한다. */
	void RequestLobbyTeamSelection(ESnowRumbleTeam NewTeam);

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** 이동 입력을 카메라의 수평 방향 기준 이동으로 변환한다. */
	void Move(const FInputActionValue& Value);

	/** 마우스 입력을 로컬 플레이어의 카메라 회전에 적용한다. */
	void Look(const FInputActionValue& Value);

	/** 마우스 휠 입력으로 로컬 카메라 줌 목표 거리를 조정한다. */
	void UpdateCameraZoomInput();

	/** 포디움 전용 컨트롤러일 때 PvP HUD와 원형 메뉴 생성을 막는다. */
	bool ShouldSuppressPvpWidgets() const;

	/** 점프 입력이 시작되면 캐릭터 점프를 요청한다. */
	void StartJump();

	/** 점프 입력이 끝나면 점프 요청을 해제한다. */
	void StopJump();

	/** 눌러서 말하기 입력을 PlayerController 마이크 상태로 전달한다. */
	void HandleMicrophonePushToTalkStarted();

	/** 눌러서 말하기 입력 해제를 PlayerController 마이크 상태로 전달한다. */
	void HandleMicrophonePushToTalkCompleted();

	/** 마이크 전체/팀 채널 전환 입력을 PlayerController에 전달한다. */
	void HandleMicrophoneChannelToggle();

	/** 플레이어 지정 음소거 입력을 PlayerController에 전달한다. */
	void HandleVoiceTargetMute();

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

	/** 키 가이드 입력 누름 상태를 처리한다. */
	void HandleKeyGuideStarted();

	/** 키 가이드 입력 해제 상태를 처리한다. */
	void HandleKeyGuideCompleted();

	/** 로컬 PlayerCameraManager에 안전한 상하 시야각을 적용한다. */
	void ApplyCameraPitchLimits();

	/** PlayerState 닉네임 변경 이벤트에 머리 위 이름표 갱신을 연결한다. */
	void BindOverheadNameToPlayerState();

	/** PlayerState 커스터마이징 변경 이벤트에 외형 갱신을 연결한다. */
	void BindCustomizationToPlayerState();

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

	/** 로컬 플레이어용 키 가이드 위젯을 필요할 때 생성한다. */
	void EnsureKeyGuideWidget();

	/** 로컬 플레이어용 메인 HUD 위젯을 필요할 때 생성한다. */
	void EnsureMainHUDWidget();

	/** 로컬 플레이어용 상호작용 안내 위젯을 필요할 때 생성한다. */
	void EnsureInteractionPromptWidget();

	/** 현재 상호작용 후보에 맞춰 로컬 안내 위젯을 표시하거나 숨긴다. */
	void RefreshInteractionPromptWidget();

	/** 현재 상호작용 후보의 안내 문구와 대상 액터를 반환한다. */
	bool GetCurrentInteractionPromptData(
		FText& OutPromptText,
		AActor*& OutPromptActor) const;

	/** 로컬 플레이어 화면에서 이모션 원형 메뉴를 연다. */
	void OpenEmoteRadialMenu();

	/** 로컬 플레이어 화면에서 키 가이드 위젯을 연다. */
	void OpenKeyGuideWidget();

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

	/** 복제된 PlayerState 커스터마이징 데이터로 캐릭터 외형을 갱신한다. */
	UFUNCTION()
	void RefreshCustomizationFromPlayerState();

	void RefreshCustomizationHatMesh();

	/** 목도리 Mesh를 캐릭터 Mesh 소켓에 붙이고 표시 상태를 갱신한다. */
	void RefreshScarfMesh();

	/** 현재 팀 색을 목도리 Dynamic Material에 반영한다. */
	void RefreshScarfTeamColorMaterial();

	/** 로컬 피격 카메라 흔들림 오프셋을 계산한다. */
	FVector CalculateLocalDamageCameraShakeOffset() const;

	UFUNCTION()
	void HandleGiftItemEffectsChanged();

	/** 복제된 아이템 효과 상태와 소켓 설정에 맞춰 장비 외형 슬롯 Mesh를 갱신한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|Item|Equipment Visual")
	void RefreshGiftItemEquipmentMeshes();

	/** 장비 외형 슬롯을 캐릭터 Mesh 소켓에 맞춰 붙이고 표시 상태를 갱신한다. */
	void RefreshGiftItemEquipmentMeshSlot(
		UStaticMeshComponent* SlotComponent,
		UStaticMesh* SlotMesh,
		FName AttachSocketName,
		const FVector& RelativeLocation,
		const FRotator& RelativeRotation,
		const FVector& RelativeScale) const;

	/** 로컬 플레이어가 상호작용할 가장 가까운 로비 게시판을 찾는다. */
	ALobbyInteractionBoard* FindClosestLobbyBoardCandidate() const;

	/** 로컬 플레이어가 상호작용할 가장 가까운 사진 촬영 액터를 찾는다. */
	APhotoInteractionActor* FindClosestPhotoInteractionCandidate() const;

	/** 로컬 플레이어가 상호작용할 가장 가까운 선물상자를 찾는다. */
	AGiftBox* FindClosestGiftBoxCandidate() const;

	/** 로컬 플레이어가 상호작용할 가장 가까운 선물상자 아이템을 찾는다. */
	AGiftBoxItemPickup* FindClosestGiftBoxItemPickupCandidate() const;

	/** 일반 핫팩으로 부활할 수 있는 같은 팀 얼음 아군을 찾는다. */
	ASnowRumbleCharacter* FindClosestFrozenTeammateCandidate() const;

	/** 로컬 E 홀드 부활 입력을 시작한다. */
	void TryStartTeammateRevive();

	/** E 홀드 부활 입력을 취소한다. */
	void CancelTeammateRevive();

	/** E 홀드가 완료되면 서버 부활 요청을 보낸다. */
	void CompleteTeammateRevive();

	/** 소유 플레이어가 가까운 로비 게시판 상호작용을 서버에 요청한다. */
	void TryInteractWithLobbyBoard();

	/** 소유 플레이어가 가까운 사진 촬영 액터 상호작용을 서버에 요청한다. */
	void TryInteractWithPhotoActor();

	/** 사진 촬영 상태에서 P 키를 눌러 로컬 스크린샷을 저장한다. */
	void HandlePhotoCapture();

	/** 소유 플레이어가 가까운 선물상자 개봉을 서버에 요청한다. */
	void TryInteractWithGiftBox();

	/** 소유 플레이어가 가까운 선물상자 아이템 획득을 서버에 요청한다. */
	void TryPickupGiftBoxItem();

	/** 로컬 플레이어 화면의 로비 게시판 카메라 포커스를 원래 캐릭터 카메라로 복구한다. */
	void ClearLobbyBoardFocus();

	/** 서버가 현재 위치와 상태를 검사해 로비 게시판 상호작용을 확정한다. */
	UFUNCTION(Server, Reliable)
	void ServerTryInteractWithLobbyBoard(ALobbyInteractionBoard* Board);

	/** 서버가 현재 위치와 상태를 검사해 사진 촬영 상호작용을 확정한다. */
	UFUNCTION(Server, Reliable)
	void ServerTryInteractWithPhotoActor(APhotoInteractionActor* PhotoActor);

	/** 서버가 현재 위치와 상태를 검사해 선물상자 개봉을 확정한다. */
	UFUNCTION(Server, Reliable)
	void ServerTryOpenGiftBox(AGiftBox* GiftBox);

	/** 서버가 현재 위치와 상태를 검사해 선물상자 아이템 획득을 확정한다. */
	UFUNCTION(Server, Reliable)
	void ServerTryPickupGiftBoxItem(AGiftBoxItemPickup* Pickup);

	/** 서버가 거리·팀·얼음 상태와 핫팩 보유를 검증해 아군을 부활시킨다. */
	UFUNCTION(Server, Reliable)
	void ServerReviveFrozenTeammate(ASnowRumbleCharacter* TargetCharacter);

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

	/** 잡힌 상태에 맞춰 이동 컴포넌트를 정지하거나 복구한다. */
	void HandleGrabbedByCharacterChanged(bool bNewGrabbed);

	/** 단판 승부 관전자면 로컬 카메라를 경기 참가자 시점으로 붙인다. */
	void RefreshTiebreakerSpectatorViewTarget();

	/** 로컬 관전 카메라가 따라갈 단판 승부 참가 캐릭터를 찾는다. */
	ASnowRumbleCharacter* FindTiebreakerSpectatorViewTarget() const;

	/** 스프린트 상태에 맞는 최대 이동속도를 CharacterMovement에 적용한다. */
	void ApplyMovementSpeed();

	/** 유효한 이모션 인덱스인지 확인한다. */
	bool IsValidEmoteIndex(int32 EmoteIndex) const;

	/** 현재 캐릭터에서 이모션 몽타주를 실행할 수 있는지 확인한다. */
	bool CanPlayEmote() const;

	/** 로컬 AnimInstance에 선택된 이모션 몽타주를 재생한다. */
	void PlayEmoteMontage(int32 EmoteIndex);

	/** 지정한 발 socket 아래에서 눈 표면을 찾는다. */
	bool FindSnowFootstepSurface(
		FName FootSocketName,
		FHitResult& OutFootstepHit) const;

	/** 지정한 발 socket 아래에서 눈길과 일반길을 포함한 바닥을 찾는다. */
	bool FindFootstepSurface(
		FName FootSocketName,
		FHitResult& OutFootstepHit) const;

	/** 지정한 월드 위치 주변에서 눈 표면을 찾는다. */
	bool FindSnowFootstepSurfaceAtLocation(
		const FVector& FootstepLocation,
		FHitResult& OutFootstepHit) const;

	/** 눈 표면 위에서 이동 거리에 맞춰 연속 눈길 stamp를 요청한다. */
	void UpdateDistanceBasedSnowTrail(float DeltaSeconds);

	/** 소유 클라이언트 또는 호스트에서 서버 검증 눈길 stamp를 요청한다. */
	void RequestSharedSnowTrailStamp(
		const FVector& FootstepLocation,
		const FVector& FootstepNormal,
		FName FootSocketName);

	/** 캐릭터 중심 기준으로 현재 발밑 눈 표면을 찾을 위치를 반환한다. */
	FVector GetSnowTrailProbeLocation() const;

	/** 서버 확정 애니메이션 trigger를 현재 화면의 AnimInstance로 전달한다. */
	void RequestAnimationTriggerFromServer(
		ESnowRumbleCharacterAnimTrigger Trigger);

	/** 서버가 확정한 one-shot 애니메이션 trigger를 모든 화면에 전달한다. */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRequestAnimationTrigger(
		ESnowRumbleCharacterAnimTrigger Trigger);

	/** 소유 클라이언트가 발걸음 위치를 서버에 보내 눈길 stamp를 요청한다. */
	UFUNCTION(Server, Unreliable)
	void ServerRequestSnowTrailStamp(
		FVector_NetQuantize FootstepLocation,
		FVector_NetQuantizeNormal FootstepNormal,
		FName FootSocketName);

	/** 서버가 검증한 눈길 stamp를 모든 클라이언트의 Manager에 전달한다. */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastStampSnowTrail(
		FVector_NetQuantize FootstepLocation,
		FVector_NetQuantizeNormal FootstepNormal,
		FName FootSocketName,
		float RadiusWorld);

	/** 실제 피해를 받은 소유 클라이언트에 로컬 피격 표현을 요청한다. */
	UFUNCTION(Client, Reliable)
	void ClientRequestLocalDamageFeedback(
		float AppliedDamage,
		FVector_NetQuantize DamageCauserLocation);

	/** 서버가 확정한 피격음을 모든 화면에서 피격자 위치 기준으로 재생한다. */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayDamageSound(FVector_NetQuantize DamageLocation);

	/** 서버가 소유 클라이언트의 이모션 선택을 검사하고 확정한다. */
	UFUNCTION(Server, Reliable)
	void ServerRequestPlayEmote(int32 EmoteIndex);

	/** 서버가 확정한 이모션 몽타주를 모든 화면에서 재생한다. */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayEmote(int32 EmoteIndex);

	/** 서버가 확정한 캐릭터 피드백 사운드를 모든 화면에서 재생한다. */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayCharacterFeedbackSound(
		ESnowRumbleCharacterFeedbackSoundType FeedbackSoundType);

public:
	/** 서버 전용 연출 흐름에서 지정한 이모션을 모든 화면에 재생한다. */
	void PlayServerDirectedEmote(int32 EmoteIndex);

	/** 서버가 승리 연출 등에 사용할 랜덤 이모션을 하나 재생한다. */
	void PlayRandomServerDirectedEmote();

	/** 서버가 소유 클라이언트의 스프린트 상태 요청을 검사하고 확정한다. */
	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bNewSprinting);

	/** 복제된 스프린트 상태를 다른 화면의 이동속도와 표현에 적용한다. */
	UFUNCTION()
	void OnRep_IsSprinting();

	/** 서버에서 아이템 획득 애니메이션 상태를 종료한다. */
	void FinishPickupAnimationState();

	/** 서버에서 아이템 상호작용 애니메이션 상태를 종료한다. */
	void FinishItemInteractionAnimationState();

	/** 서버에서 피격 반응 애니메이션 상태를 시작한다. */
	void StartHitReactAnimationState();

	/** 서버에서 피격 반응 애니메이션 상태를 종료한다. */
	void FinishHitReactAnimationState();

	/** 복제된 획득 상태에 따라 이동 잠금과 복구를 적용한다. */
	UFUNCTION()
	void OnRep_IsPickingUpItem();

	/** 복제된 아이템 상호작용 상태에 따라 이동 잠금과 복구를 적용한다. */
	UFUNCTION()
	void OnRep_IsInteractingWithItem();

	/** 복제된 단판 승부 관전자 상태에 따라 이동 잠금과 복구를 적용한다. */
	UFUNCTION()
	void OnRep_TiebreakerSpectator();

	/** 복제된 물 침수 상태에 따라 점프 입력을 정리한다. */
	UFUNCTION()
	void OnRep_WaterSubmerged();

	/** 복제된 잡힘 상태에 따라 이동 잠금과 복구를 적용한다. */
	UFUNCTION()
	void OnRep_GrabbedByCharacter();

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

	/** 눈 표면에서 발이 닿았을 때 현재 화면에서만 Niagara, Decal, Sound 같은 표현을 재생한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|Footstep")
	void OnSnowFootstepEffect(
		FName FootSocketName,
		FVector FootstepLocation,
		FVector FootstepNormal);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Camera")
	TObjectPtr<UNiagaraComponent> LocalSnowEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Health")
	TObjectPtr<USnowRumbleHealthComponent> HealthComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Damage Feedback", meta = (ClampMin = "0.01"))
	float DamageFeedbackTintDuration = 0.22f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Damage Feedback", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DamageFeedbackTintAlpha = 0.38f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Damage Feedback")
	FLinearColor DamageFeedbackTintColor = FLinearColor(0.45f, 0.78f, 1.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Damage Feedback", meta = (ClampMin = "0.01"))
	float DamageFeedbackCameraShakeDuration = 0.24f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Damage Feedback", meta = (ClampMin = "0.0"))
	float DamageFeedbackCameraShakeAmplitude = 18.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Damage Feedback", meta = (ClampMin = "0.0"))
	float DamageFeedbackCameraShakeFrequency = 34.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Effect")
	TObjectPtr<UGiftItemEffectComponent> GiftItemEffectComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball")
	TObjectPtr<USnowballEquipmentComponent> SnowballEquipmentComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball")
	TObjectPtr<USnowballCreationComponent> SnowballCreationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Grab")
	TObjectPtr<UPlayerGrabComponent> PlayerGrabComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball|Rolling")
	TObjectPtr<USphereComponent> RollingSnowballCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball")
	TObjectPtr<USceneComponent> SnowballHoldPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball")
	TObjectPtr<USceneComponent> LargeSnowballHoldPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Snowball")
	TObjectPtr<USceneComponent> SnowDuckBallHoldPoint;

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

	/** 커스터마이징 색을 적용할 Mesh 머티리얼 슬롯이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization", meta = (ClampMin = "0"))
	int32 CustomizationMaterialIndex = 0;

	/** 캐릭터 머티리얼에서 몸 색으로 사용할 Vector Parameter 이름이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization")
	FName CustomizationBodyColorParameterName = TEXT("BodyColor");

	/** 캐릭터 머티리얼에서 드로잉 텍스처로 사용할 Texture Parameter 이름이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization")
	FName CustomizationPaintTextureParameterName = TEXT("PaintTexture");

	/** 저장된 드로잉 stroke를 다시 그릴 RenderTarget 한 변의 픽셀 크기다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization", meta = (ClampMin = "64", ClampMax = "4096"))
	int32 CustomizationPaintRenderTargetSize = 1024;

	/** 저장된 드로잉 stroke를 다시 그릴 때 쓰는 선 두께다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization", meta = (ClampMin = "1.0", ClampMax = "256.0"))
	float CustomizationPaintStrokeThickness = 12.0f;

	/** 저장된 드로잉 stroke를 다시 그릴 때 쓰는 색이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization")
	FLinearColor CustomizationPaintBrushColor = FLinearColor::Black;

	/** 머티리얼 UV 방향에 맞춰 저장된 드로잉 RenderTarget Y축을 뒤집을지 정한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization")
	bool bFlipCustomizationPaintUvY = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Customization|Hat")
	TObjectPtr<UStaticMeshComponent> HatMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Customization|Scarf")
	TObjectPtr<UStaticMeshComponent> ScarfMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual")
	TObjectPtr<UStaticMeshComponent> LeftBootsMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual")
	TObjectPtr<UStaticMeshComponent> RightBootsMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual")
	TObjectPtr<UStaticMeshComponent> LeftGlovesMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual")
	TObjectPtr<UStaticMeshComponent> RightGlovesMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual")
	TObjectPtr<UStaticMeshComponent> PaddingMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual")
	TObjectPtr<UStaticMeshComponent> HotPackMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual")
	TObjectPtr<UStaticMeshComponent> ShovelMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual")
	TObjectPtr<UStaticMeshComponent> DuckMakerMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Boots")
	TObjectPtr<UStaticMesh> LeftBootsEquipmentMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Boots")
	TObjectPtr<UStaticMesh> RightBootsEquipmentMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Boots")
	FName LeftBootsEquipmentAttachSocketName = TEXT("LeftBootsSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Boots")
	FName RightBootsEquipmentAttachSocketName = TEXT("RightBootsSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Boots")
	FVector LeftBootsEquipmentRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Boots")
	FVector RightBootsEquipmentRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Boots")
	FRotator LeftBootsEquipmentRelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Boots")
	FRotator RightBootsEquipmentRelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Boots")
	FVector LeftBootsEquipmentRelativeScale = FVector::OneVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Boots")
	FVector RightBootsEquipmentRelativeScale = FVector::OneVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Gloves")
	TObjectPtr<UStaticMesh> LeftGlovesEquipmentMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Gloves")
	TObjectPtr<UStaticMesh> RightGlovesEquipmentMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Gloves")
	FName LeftGlovesEquipmentAttachSocketName = TEXT("LeftGlovesSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Gloves")
	FName RightGlovesEquipmentAttachSocketName = TEXT("RightGlovesSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Gloves")
	FVector LeftGlovesEquipmentRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Gloves")
	FVector RightGlovesEquipmentRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Gloves")
	FRotator LeftGlovesEquipmentRelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Gloves")
	FRotator RightGlovesEquipmentRelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Gloves")
	FVector LeftGlovesEquipmentRelativeScale = FVector::OneVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Gloves")
	FVector RightGlovesEquipmentRelativeScale = FVector::OneVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Padding")
	TObjectPtr<UStaticMesh> PaddingEquipmentMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Padding")
	FName PaddingEquipmentAttachSocketName = TEXT("PaddingSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Padding")
	FVector PaddingEquipmentRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Padding")
	FRotator PaddingEquipmentRelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Padding")
	FVector PaddingEquipmentRelativeScale = FVector::OneVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Hot Pack")
	TObjectPtr<UStaticMesh> HotPackEquipmentMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Hot Pack")
	TObjectPtr<UStaticMesh> GoldenHotPackEquipmentMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Hot Pack")
	FName HotPackEquipmentAttachSocketName = TEXT("HotPackSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Hot Pack")
	FVector HotPackEquipmentRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Hot Pack")
	FRotator HotPackEquipmentRelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Hot Pack")
	FVector HotPackEquipmentRelativeScale = FVector::OneVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Shovel")
	TObjectPtr<UStaticMesh> SnowShovelEquipmentMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Shovel")
	TObjectPtr<UStaticMesh> GoldenShovelEquipmentMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Shovel")
	FName ShovelEquipmentAttachSocketName = TEXT("ShovelSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Shovel")
	FVector ShovelEquipmentRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Shovel")
	FRotator ShovelEquipmentRelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Shovel")
	FVector ShovelEquipmentRelativeScale = FVector::OneVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Duck Maker")
	TObjectPtr<UStaticMesh> SnowDuckMakerEquipmentMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Duck Maker")
	TObjectPtr<UStaticMesh> GoldenDuckMakerEquipmentMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Duck Maker")
	FName DuckMakerEquipmentAttachSocketName = TEXT("DuckMakerSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Duck Maker")
	FVector DuckMakerEquipmentRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Duck Maker")
	FRotator DuckMakerEquipmentRelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Item|Equipment Visual|Duck Maker")
	FVector DuckMakerEquipmentRelativeScale = FVector::OneVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Hat")
	TArray<TObjectPtr<UStaticMesh>> CustomizationHatMeshes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Hat")
	FName CustomizationHatAttachSocketName = TEXT("HatSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Hat")
	FVector CustomizationHatRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Hat")
	FRotator CustomizationHatRelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Hat")
	FVector CustomizationHatRelativeScale = FVector::OneVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Scarf")
	TObjectPtr<UStaticMesh> ScarfMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Scarf")
	FName ScarfAttachSocketName = TEXT("ScarfSocket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Scarf")
	FName ScarfTeamColorParameterName = TEXT("TeamColor");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Scarf")
	FVector ScarfRelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Scarf")
	FRotator ScarfRelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Customization|Scarf")
	FVector ScarfRelativeScale = FVector::OneVector;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ScarfDynamicMaterial;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> KeyGuideAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> MicrophonePushToTalkAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> MicrophoneChannelToggleAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Input")
	TObjectPtr<UInputAction> VoiceTargetMuteAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Movement", meta = (ClampMin = "0.0"))
	float WalkSpeed = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Movement", meta = (ClampMin = "0.0"))
	float SprintSpeed = 750.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Movement", meta = (ClampMin = "0.0"))
	float AimWalkSpeed = 300.0f;

	/** 눈 밟힘 효과를 허용할 바닥 Actor 태그다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Footstep")
	FName SnowFootstepSurfaceTag = TEXT("SnowSurface");

	/** 왼발 AnimNotify에서 이름을 넘기지 않았을 때 사용할 기본 socket 이름이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Footstep")
	FName LeftFootSocketName = TEXT("foot_l");

	/** 오른발 AnimNotify에서 이름을 넘기지 않았을 때 사용할 기본 socket 이름이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Footstep")
	FName RightFootSocketName = TEXT("foot_r");

	/** 발 socket 위쪽에서 trace를 시작하는 높이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Footstep", meta = (ClampMin = "0.0"))
	float SnowFootstepTraceUpOffset = 80.0f;

	/** 발 socket 아래로 눈 표면을 찾는 거리다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Footstep", meta = (ClampMin = "0.0"))
	float SnowFootstepTraceDownDistance = 140.0f;

	/** 발걸음 효과가 너무 촘촘히 반복되지 않도록 막는 최소 간격이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Footstep", meta = (ClampMin = "0.0"))
	float SnowFootstepEffectCooldown = 0.08f;

	/** 눈 표면 발 착지 시 위치 기반으로 재생할 발걸음 사운드다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Footstep|Audio")
	TObjectPtr<USoundBase> FootstepSound;

	/** 발걸음 사운드의 월드 거리감과 공간감을 설정한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Footstep|Audio")
	TObjectPtr<USoundAttenuation> FootstepSoundAttenuation;

	/** 일반 바닥 발 착지 시 위치 기반으로 재생할 발걸음 사운드다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Footstep|Audio")
	TObjectPtr<USoundBase> NormalFootstepSound;

	/** 일반 바닥 발걸음 사운드의 월드 거리감과 공간감을 설정한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Footstep|Audio")
	TObjectPtr<USoundAttenuation> NormalFootstepSoundAttenuation;

	/** 눈 밟힘 위치를 서버 검증 후 지형 RenderTarget 눈길 stamp로 공유할지 정한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snow Trail")
	bool bEnableSharedSnowTrailStamps = true;

	/** 눈길 RenderTarget에 남길 기본 stamp 반지름(cm)이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snow Trail", meta = (ClampMin = "1.0"))
	float SnowTrailStampRadius = 38.0f;

	/** 서버에서 눈길 stamp 요청을 너무 자주 처리하지 않도록 막는 최소 간격이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snow Trail", meta = (ClampMin = "0.0"))
	float SnowTrailStampServerCooldown = 0.08f;

	/** 클라이언트가 보낸 발 위치가 서버 캐릭터 위치에서 허용되는 최대 거리다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snow Trail", meta = (ClampMin = "0.0"))
	float SnowTrailStampMaxClientDistance = 260.0f;

	/** AnimNotify로 눈 표면을 밟은 뒤 이동 거리 기준으로 연속 눈길 stamp를 찍을지 정한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snow Trail|Distance")
	bool bEnableDistanceBasedSnowTrailStamps = true;

	/** 연속 눈길 stamp를 새로 찍기 위한 최소 이동 거리(cm)다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snow Trail|Distance", meta = (ClampMin = "1.0"))
	float SnowTrailDistanceStampInterval = 42.0f;

	/** 이 속도보다 느리면 연속 눈길 stamp를 찍지 않는다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Snow Trail|Distance", meta = (ClampMin = "0.0"))
	float SnowTrailDistanceStampMinimumSpeed = 20.0f;

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

	/** 마우스 휠 한 칸당 카메라 SpringArm 길이 변화량이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera|Zoom", meta = (ClampMin = "1.0"))
	float CameraZoomStep = 60.0f;

	/** 마우스 휠 줌으로 허용하는 최소 SpringArm 길이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera|Zoom", meta = (ClampMin = "0.0"))
	float MinimumCameraArmLength = 250.0f;

	/** 마우스 휠 줌으로 허용하는 최대 SpringArm 길이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Camera|Zoom", meta = (ClampMin = "0.0"))
	float MaximumCameraArmLength = 650.0f;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation", meta = (ClampMin = "0.01"))
	float ItemInteractionAnimationStateDuration = 0.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation", meta = (ClampMin = "0.01"))
	float HitReactAnimationStateDuration = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Audio")
	TObjectPtr<USoundBase> ItemPickupSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Audio")
	TObjectPtr<USoundBase> SnowballPickupSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Audio")
	TObjectPtr<USoundBase> SnowballThrowSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Audio")
	TObjectPtr<USoundBase> JumpSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Audio")
	TObjectPtr<USoundBase> ItemInteractionSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Audio")
	TObjectPtr<USoundBase> LobbyBoardInteractionSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Audio")
	TObjectPtr<USoundBase> DamageSound;

	/** 피격음이 월드 거리감과 공간감을 갖도록 적용할 attenuation 설정이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Audio")
	TObjectPtr<USoundAttenuation> DamageSoundAttenuation;

	/** 사진 촬영 시 재생할 셔터 효과음이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Audio")
	TObjectPtr<USoundBase> PhotoCaptureSound;

	/** 원형 선택 UI의 8개 칸에 대응하는 이모션 몽타주 슬롯이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Emote")
	TArray<TObjectPtr<UAnimMontage>> EmoteMontages;

	/** 로컬 플레이어 화면에 생성할 이모션 원형 메뉴 위젯 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Emote|UI")
	TSubclassOf<UEmoteRadialMenuWidget> EmoteRadialMenuWidgetClass;

	/** 로컬 플레이어가 소유한 이모션 원형 메뉴 위젯 인스턴스다. */
	UPROPERTY(Transient)
	TObjectPtr<UEmoteRadialMenuWidget> EmoteRadialMenuWidget;

	/** 로컬 플레이어 화면에 생성할 키 가이드 위젯 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|KeyGuide|UI")
	TSubclassOf<UKeyGuideWidget> KeyGuideWidgetClass;

	/** 로컬 플레이어가 소유한 키 가이드 위젯 인스턴스다. */
	UPROPERTY(Transient)
	TObjectPtr<UKeyGuideWidget> KeyGuideWidget;

	/** 로컬 플레이어 화면에 생성할 메인 HUD 위젯 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI")
	TSubclassOf<UMainHUDWidget> MainHUDWidgetClass;

	/** 로컬 플레이어가 소유한 메인 HUD 위젯 인스턴스다. */
	UPROPERTY(Transient)
	TObjectPtr<UMainHUDWidget> MainHUDWidget;

	/** 사진 모드에서 표시할 전용 WBP 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Photo|UI")
	TSubclassOf<UUserWidget> PhotoInteractionWidgetClass;

	/** 로컬 플레이어가 사진 모드에서 소유한 전용 WBP 인스턴스다. */
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PhotoInteractionWidget;

	/** 로컬 플레이어 화면에 생성할 상호작용 안내 위젯 클래스다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Interaction|UI")
	TSubclassOf<UInteractionPromptWidget> InteractionPromptWidgetClass;

	/** 상호작용 대상 화면 좌표에서 안내 위젯을 얼마나 옮겨 표시할지 정한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Interaction|UI")
	FVector2D InteractionPromptScreenOffset = FVector2D(32.0f, -24.0f);

	/** 상호작용 대상 월드 위치에서 안내 기준점을 얼마나 위로 올릴지 정한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Interaction|UI")
	float InteractionPromptWorldHeightOffset = 70.0f;

	/** 일반 핫팩 부활 대상 탐색과 서버 검증에 사용할 최대 거리다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Interaction|Revive", meta = (ClampMin = "0.0"))
	float TeammateReviveInteractionDistance = 260.0f;

	/** 일반 핫팩 부활을 확정하기 위해 E를 유지해야 하는 시간이다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Interaction|Revive", meta = (ClampMin = "0.0"))
	float TeammateReviveHoldSeconds = 0.75f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation", meta = (ClampMin = "1.0"))
	float ViewPitchAlphaRangeDegrees = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Animation", meta = (ClampMin = "1.0"))
	float ViewYawAlphaRangeDegrees = 90.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|Grab", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SnowCreationPreferredViewPitchAlpha = 0.35f;

	/** 로컬 플레이어가 소유한 상호작용 안내 위젯 인스턴스다. */
	UPROPERTY(Transient)
	TObjectPtr<UInteractionPromptWidget> InteractionPromptWidget;

	bool bIsEmoteRadialMenuOpen = false;
	bool bIsKeyGuideWidgetOpen = false;
	bool bPvpMatchMoveInputIgnoreApplied = false;
	bool bPvpMatchLookInputIgnoreApplied = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_IsSprinting, Category = "SnowRumble|Movement")
	bool bIsSprinting = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_IsPickingUpItem, Category = "SnowRumble|Animation")
	bool bIsPickingUpItem = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_IsInteractingWithItem, Category = "SnowRumble|Animation")
	bool bIsInteractingWithItem = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "SnowRumble|Animation")
	bool bIsHitReacting = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_TiebreakerSpectator, Category = "SnowRumble|Match")
	bool bTiebreakerSpectator = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_WaterSubmerged, Category = "SnowRumble|Movement")
	bool bWaterSubmerged = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_GrabbedByCharacter, Category = "SnowRumble|Grab")
	bool bIsGrabbedByCharacter = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "SnowRumble|Grab")
	TObjectPtr<ASnowRumbleCharacter> GrabbedByCharacter;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "SnowRumble|Grab")
	FVector GrabbedByCharacterWorldLocation = FVector::ZeroVector;

	float DefaultFieldOfView = 90.0f;
	FVector DefaultCameraSocketOffset = FVector::ZeroVector;
	float DefaultCameraArmLength = 400.0f;
	float DesiredCameraArmLength = 400.0f;
	float CameraShoulderSide = 1.0f;
	double PostThrowAimCameraEndTime = -1.0;
	double LocalDamageCameraShakeStartTime = -1.0;
	double LocalDamageCameraShakeEndTime = -1.0;
	double LastSnowFootstepEffectTime = -1.0;
	double LastSnowTrailStampServerTime = -1.0;

	FTimerHandle PickupAnimationTimerHandle;
	FTimerHandle ItemInteractionAnimationTimerHandle;
	FTimerHandle HitReactAnimationTimerHandle;

	TEnumAsByte<EMovementMode> MovementModeBeforeGrabbed = MOVE_Walking;
	uint8 CustomMovementModeBeforeGrabbed = 0;
	bool bOrientRotationToMovementBeforeGrabbedByCharacter = true;
	bool bUseControllerRotationYawBeforeGrabbedByCharacter = false;
	bool bOrientRotationToMovementBeforeWorldGrab = true;
	bool bUseControllerRotationYawBeforeWorldGrab = false;

	bool bIsInteractHeld = false;
	bool bUsedInteractForRolling = false;
	bool bIsRevivingTeammate = false;
	TWeakObjectPtr<ASnowRumbleCharacter> TeammateReviveTarget;
	FTimerHandle TeammateReviveTimerHandle;
	bool bLobbyBoardPointerPressed = false;
	bool bLocalSnowEffectActive = false;
	bool bDistanceSnowTrailActive = false;
	FVector LastDistanceSnowTrailStampLocation = FVector::ZeroVector;
	FName LastDistanceSnowTrailFootSocketName = NAME_None;

	UPROPERTY(Transient)
	TObjectPtr<ALobbyInteractionBoard> FocusedLobbyBoard;

	UPROPERTY(Transient)
	TObjectPtr<APhotoInteractionActor> FocusedPhotoActor;

	bool bOrientRotationToMovementBeforePhotoFocus = true;
	bool bUseControllerRotationYawBeforePhotoFocus = false;
	float PhotoFocusViewPitchDegrees = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> CustomizationMaterialInstance;

	UPROPERTY(Transient)
	TMap<int32, TObjectPtr<UCanvasRenderTarget2D>> CustomizationPaintRenderTargets;

	FSnowRumbleCustomizationData AppliedCustomizationData;

private:
	/** 단판 승부 중에는 대상 팀과 공격자 팀이 모두 단판 승부 대상인지 확인한다. */
	bool IsDamageAllowedByTiebreaker(AController* EventInstigator) const;

	/** 적용된 커스터마이징 데이터의 stroke 배열로 RenderTarget을 다시 그린다. */
	void RedrawCustomizationPaintTexture();

	/** 캐릭터 외형용 드로잉 RenderTarget이 없으면 생성한다. */
	UCanvasRenderTarget2D* EnsureCustomizationPaintRenderTarget(
		int32 TargetMaterialIndex);

	/** 특정 MeshComponent에 드로잉 텍스처를 적용한다. */
	void ApplyCustomizationPaintTextureToMesh(
		USkeletalMeshComponent* MeshComponent,
		int32 TargetMaterialIndex,
		UTexture* PaintTexture);

	UFUNCTION()
	void HandleCustomizationPaintCanvasUpdate(
		UCanvas* Canvas,
		int32 Width,
		int32 Height);

	void DrawCustomizationPaintStrokeToCanvas(
		UCanvas* Canvas,
		const FSnowRumblePaintStroke& Stroke,
		int32 Width,
		int32 Height) const;

	FName ActiveCustomizationPaintMeshComponentName;
	int32 ActiveCustomizationPaintMaterialIndex = INDEX_NONE;
};

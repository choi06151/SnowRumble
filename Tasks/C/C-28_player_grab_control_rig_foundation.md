# Task C-28 - 플레이어 잡기와 Control Rig 기본틀

## 설명

좌클릭 유지 중 캐릭터가 팔을 앞으로 뻗고, 손 위치가 상대 캐릭터 Mesh에 닿으면 서버가 잡기 상태를 확정하는 C++ 기본틀을 제공한다.

## 상태 전이 기준

- 시작 가능: C-24 AnimBP 부모 계약과 기존 플레이어 입력 구조
- 완료 가능: 잡기 reach 상태, Control Rig용 손 목표값, 서버 검증된 잡기 확정·해제, 에디터 연결 절차가 정리됨

## 구현 항목

- [x] 잡기 상태와 서버 검증을 소유하는 `UPlayerGrabComponent`를 추가한다.
- [x] 빈손 상태에서 우클릭 조준 없이 좌클릭을 누르면 잡기 reach를 시작한다.
- [x] 빈손 상태라도 아래를 보고 좌클릭하면 잡기 reach 대신 기존 눈 제작을 우선한다.
- [x] 좌클릭 해제 시 잡기 reach와 현재 잡기를 해제한다.
- [x] AnimBP/Control Rig가 읽을 `bIsGrabReaching`, `bIsGrabbingCharacter`, 좌우 손 목표 위치, 0~1 보간 `GrabReachAlpha`를 제공한다.
- [x] AnimBP/Control Rig가 spine 보정에 사용할 카메라 pitch 각도와 0~1 Lerp 값을 제공한다.
- [x] AnimBP/Control Rig가 spine 보정에 사용할 카메라 yaw 각도와 -0.5~0.5 Lerp 값을 제공한다.
- [x] 서버가 손 목표 위치 기준 sphere trace로 상대 캐릭터 Mesh 접촉을 검사한다.
- [x] 잡기 성공 시 손 bone과 대상 Mesh bone을 `UPhysicsConstraintComponent`로 연결하는 기본 지점을 제공한다.
- [x] 손 목표 위치가 벽·월드 오브젝트에 닿으면 월드 붙음 상태와 붙은 위치를 복제한다.
- [x] 보행 중 좌클릭 연타로 바닥·천장·즉시 월드 붙음이 잘못 확정되어 캐릭터가 위로 튀는 경로를 차단한다.
- [x] 벽·월드 오브젝트를 잡으면 잡는 캐릭터의 이동 입력은 허용하고 점프·일반 행동은 잠그며, 붙은 손 위치 기준으로 몸을 매달린 위치에 유지한다.
- [x] 플레이어를 잡으면 잡힌 캐릭터의 이동 입력은 허용하고 점프·일반 행동은 해제 전까지 잠그며, 서버가 잡은 손 위치 쪽으로 몸을 부드럽게 끌어당긴다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: C++·문서 최재원(C), Control Rig·AnimBP·Physics Asset 연결은 사용자/S
- 생성 파일: `Source/SnowRumble/Player/PlayerGrabComponent_C.h`, `Source/SnowRumble/Player/PlayerGrabComponent_C.cpp`, `Tasks/C/C-28_player_grab_control_rig_foundation.md`
- 변경 파일: `Source/SnowRumble/Player/SnowRumbleCharacter.h`, `Source/SnowRumble/Player/SnowRumbleCharacter.cpp`, `Source/SnowRumble/Player/SnowRumbleCharacterAnimInstance_C.h`, `Source/SnowRumble/Player/SnowRumbleCharacterAnimInstance_C.cpp`, `Tasks/C/ROLE_C.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`
- 공유 확인 대상: 사용자/S의 `ABP_Modeling`, Control Rig, 캐릭터 Physics Asset
- 병합 순서: C++ 잡기 계약 반영 후 ABP/Control Rig와 Physics Asset 연결

## 공용 계약과 인계

- 제공받을 계약:
  - C-24: `USnowRumbleCharacterAnimInstance` 부모 기반 AnimBP 연결 구조
  - `ASnowRumbleCharacter`: 기존 좌클릭, 우클릭 조준, 눈덩이 보유 상태
- 제공할 계약:
  - `UPlayerGrabComponent::StartGrabReach()`: 소유 플레이어가 잡기 reach 시작을 요청한다. 로컬 예측 후 서버가 `CanStartPlayerGrabReach()`로 재검증한다.
  - `UPlayerGrabComponent::StopGrabReach()`: 소유 플레이어가 잡기 reach와 현재 잡기를 해제한다.
  - `ASnowRumbleCharacter::IsGrabReaching()`: AnimBP와 Control Rig가 팔 뻗기 상태를 읽는다.
  - `ASnowRumbleCharacter::IsGrabbingCharacter()`: AnimBP와 표현 Blueprint가 실제 잡기 확정 상태를 읽는다.
  - `ASnowRumbleCharacter::GetRightHandGrabTargetLocation()`, `GetLeftHandGrabTargetLocation()`: Control Rig 손 IK 목표 월드 위치를 읽는다.
  - `ASnowRumbleCharacter::IsGrabAttached()`: 손이 캐릭터 또는 월드에 붙은 상태인지 읽는다.
  - `ASnowRumbleCharacter::IsHangingFromWorldGrab()`: 이 캐릭터가 벽·월드 오브젝트를 잡고 매달린 상태인지 읽는다.
  - `ASnowRumbleCharacter::GetGrabAttachedWorldLocation()`: 손이 붙은 월드 위치를 읽는다.
  - `ASnowRumbleCharacter::IsGrabbedByCharacter()`: 이 캐릭터가 다른 캐릭터에게 잡혀 이동 잠금 상태인지 읽는다.
  - `ASnowRumbleCharacter::GetGrabReachAlpha()`: 잡기 pose와 AnimDynamics 강도 보간에 사용할 0~1 값이다. 좌클릭 유지 시 1로 올라가고, 해제 시 0으로 내려간다.
  - `ASnowRumbleCharacter::GetViewPitchDegrees()`: 현재 시점 pitch를 -180~180 각도로 읽는다.
  - `ASnowRumbleCharacter::GetViewPitchAlpha()`: 현재 시점 pitch를 `ViewPitchAlphaRangeDegrees` 기준 0~1 값으로 읽는다. 아래를 보면 0, 정면은 0.5, 위를 보면 1이다.
  - `ASnowRumbleCharacter::GetViewYawDegrees()`: 현재 캐릭터 정면 대비 카메라 yaw 차이를 -180~180 각도로 읽는다.
  - `ASnowRumbleCharacter::GetViewYawAlpha()`: 현재 시점 yaw 차이를 `ViewYawAlphaRangeDegrees` 기준 -0.5~0.5 값으로 읽는다. 왼쪽을 보면 -0.5, 정면은 0, 오른쪽을 보면 0.5다.
  - `ASnowRumbleCharacter::ShouldPreferSnowCreationOverGrab()`: `ViewPitchAlpha`가 `SnowCreationPreferredViewPitchAlpha` 이하이면 빈손 좌클릭을 잡기 대신 눈 제작으로 보낸다.
  - `USnowRumbleCharacterAnimInstance::bIsGrabReaching`, `bIsGrabbingCharacter`, `bIsGrabAttached`, `bIsHangingFromWorldGrab`, `bIsGrabbedByCharacter`, `GrabAttachedWorldLocation`, `RightHandGrabTargetLocation`, `LeftHandGrabTargetLocation`, `GrabReachAlpha`, `ViewPitchDegrees`, `ViewPitchAlpha`, `ViewYawDegrees`, `ViewYawAlpha`: AnimBP에서 Control Rig 변수로 넘길 읽기 전용 값이다.
- 인계 대상: 사용자/S. AnimBP는 잡기 reach 중 Control Rig로 손을 목표 위치까지 올리고, 잡기 중에는 팔 AnimDynamics alpha를 낮추거나 Control Rig를 AnimDynamics 뒤에 둔다.

## 범위 밖

- Control Rig 그래프 제작
- 손가락/팔 흐느적거림 최종 튜닝
- Physics Asset body 추가와 충돌 세부 설정
- 잡은 상대를 끌어당기는 게임 규칙
- 팀/적 판정 제한
- 잡기 전용 UI, 이펙트, 사운드

## 사전 전제

- C-24

## 결정 필요

- 눈 제작과 잡기를 같은 좌클릭에 둘지, 잡기는 별도 입력으로 분리할지 결정 필요. 현재 기본틀은 빈손 좌클릭을 잡기 reach로 우선 사용한다.
- 한 손만 사용할지 양손 모두 사용할지 결정 필요. 현재 서버 constraint는 오른손 기준으로 시작하며, Control Rig 목표값은 좌우 모두 제공한다.

## 변경 기록

- 2026-08-20: C-28을 추가했다. `UPlayerGrabComponent`가 빈손 좌클릭 잡기 reach, 서버 trace 검증, Physics Constraint 연결/해제 기본틀을 소유하고, `USnowRumbleCharacterAnimInstance`가 Control Rig용 잡기 상태와 손 목표 위치를 제공한다.
- 2026-08-20: `SnowRumbleEditor Win64 Development` 빌드가 성공했다. 기존 `SnowballItem.cpp`의 `NetUpdateFrequency`/`MinNetUpdateFrequency` deprecation warning 2개는 남아 있으며 이번 잡기 기본틀과는 별개다.
- 2026-08-20: 사용자 Control Rig 연결 결과에 맞춰 `GrabReachAlpha`를 즉시 0/1 전환에서 `GrabReachRaiseInterpSpeed`/`GrabReachLowerInterpSpeed` 기반 0~1 보간 값으로 변경했다.
- 2026-08-20: 손 접촉 붙음 상태를 확장했다. 서버가 `Pawn`, `WorldStatic`, `WorldDynamic` sphere sweep으로 플레이어 Mesh 또는 벽·월드 오브젝트 접촉을 확정하고, 붙은 월드 위치를 복제한다. 플레이어를 잡으면 잡힌 캐릭터의 이동·점프·일반 행동을 잠그고 좌클릭 해제 시 복구한다. UHT와 C++ 컴파일 및 `.lib` 생성은 통과했고, 최종 DLL 링크는 실행 중인 Unreal Editor의 DLL 잠금 `LNK1104`로 보류됐다.
- 2026-08-20: 잡힌 캐릭터의 Movement를 완전히 비활성화하지 않고 입력만 차단한 뒤, 서버 Tick에서 잡은 손 위치 쪽으로 속도를 보간하는 tether를 추가했다. `UPhysicsConstraintComponent`에는 제한된 linear/angular limit을 설정하고, 몸 끌림 느낌은 `GrabTetherSlackDistance`, `GrabTetherPullStrength`, `GrabTetherMaxPullSpeed`, `GrabTetherVelocityDamping`으로 조정한다. UHT와 C++ 컴파일 및 `.lib` 생성은 통과했고, 최종 DLL 링크는 실행 중인 Unreal Editor DLL 잠금 `LNK1104`로 보류됐다.
- 2026-08-20: 벽잡기 매달림 tether를 추가했다. 월드 표면에 손이 붙으면 잡는 캐릭터의 이동 입력·점프·일반 행동을 차단하고, 서버 Tick에서 붙은 손 위치 기준 뒤·아래 오프셋 위치로 몸을 유지한다. `WorldGrabBodyBackOffset`, `WorldGrabBodyDownOffset`, `WorldGrabTetherSlackDistance`, `WorldGrabTetherPullStrength`, `WorldGrabTetherMaxPullSpeed`로 매달림 위치와 강도를 조정한다. `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-20: 벽잡기 중 이동 입력은 허용하도록 조정했다. `Move()`는 벽잡기 중에도 입력을 처리하고, 월드 tether는 현재 입력 속도를 `WorldGrabInputVelocityRetention`만큼 남긴 뒤 붙은 손 위치로 돌아가는 보정 속도를 섞는다. UHT와 C++ 컴파일 및 `.lib` 생성은 통과했고, 최종 DLL 링크는 실행 중인 Unreal Editor DLL 잠금 `LNK1104`로 보류됐다.
- 2026-08-20: 벽잡기 중 몸이 이동 방향이나 카메라 방향으로 뒤돌아 보이지 않도록 회전 보정을 추가했다. 벽잡기 시작 시 `bOrientRotationToMovement`와 `bUseControllerRotationYaw`를 끄고, 서버 Tick에서 캐릭터 yaw를 붙은 손 위치 쪽으로 `WorldGrabFacingInterpSpeed` 속도로 보간하며, 해제 시 기존 회전 설정을 복구한다. UHT와 C++ 컴파일 및 `.lib` 생성은 통과했고, 최종 DLL 링크는 실행 중인 Unreal Editor DLL 잠금 `LNK1104`로 보류됐다.
- 2026-08-20: 플레이어에게 잡힌 캐릭터도 벽잡기와 같은 감각으로 조정했다. 잡힌 캐릭터의 `Move()` 입력은 허용하되 점프·일반 행동은 막고, 서버 Tick에서 잡힌 손 위치로 돌아가는 보정 속도를 `GrabbedCharacterInputVelocityRetention`과 함께 섞으며, 캐릭터 yaw는 잡힌 손 위치 쪽으로 `GrabbedCharacterFacingInterpSpeed` 속도로 보간한다. UHT와 C++ 컴파일 및 `.lib` 생성은 통과했고, 최종 DLL 링크는 실행 중인 Unreal Editor DLL 잠금 `LNK1104`로 보류됐다.
- 2026-08-20: 카메라 상하 시점에 따른 spine Control Rig 보정 계약을 추가했다. `ASnowRumbleCharacter`가 `ViewPitchDegrees`와 `ViewPitchAlpha`를 제공하고, `USnowRumbleCharacterAnimInstance`가 같은 값을 AnimBP 읽기 전용 값으로 갱신한다. `ViewPitchAlpha`는 아래 0, 정면 0.5, 위 1의 Lerp 값이며 `ViewPitchAlphaRangeDegrees`로 정규화 범위를 조정한다. `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-21: 빈손 좌클릭 입력 우선순위를 조정했다. `ViewPitchAlpha`가 `SnowCreationPreferredViewPitchAlpha` 이하일 때는 잡기 reach를 시작하지 않고 기존 눈 제작 경로로 내려가며, 정면 이상을 볼 때는 기존처럼 잡기 reach를 우선한다. UHT와 C++ 컴파일 및 `.lib` 생성은 통과했고, 최종 DLL 링크는 실행 중인 Unreal Editor DLL 잠금 `LNK1104`로 보류됐다.
- 2026-08-21: 카메라 좌우 시점에 따른 spine Control Rig 보정 계약을 추가했다. `ASnowRumbleCharacter`가 `ViewYawDegrees`와 `ViewYawAlpha`를 제공하고, `USnowRumbleCharacterAnimInstance`가 같은 값을 AnimBP 읽기 전용 값으로 갱신한다. `ViewYawAlpha`는 왼쪽 -0.5, 정면 0, 오른쪽 0.5의 Lerp 값이며 `ViewYawAlphaRangeDegrees`로 정규화 범위를 조정한다. UHT와 C++ 컴파일 및 `.lib` 생성은 통과했고, 최종 DLL 링크는 실행 중인 Unreal Editor DLL 잠금 `LNK1104`로 보류됐다.
- 2026-08-21: 벽잡기 또는 플레이어에게 잡힌 상태에서 우클릭 조준 해제 이벤트가 들어와도 잡기용 회전 잠금이 풀리지 않게 보강했다. 우클릭 연타로 `bOrientRotationToMovement`가 다시 켜져 tether 방향과 캐릭터 회전이 충돌하는 경로를 차단한다. `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-21: 보행 중 빈손 좌클릭 연타 시 월드 붙음 tether가 즉시 확정되어 캐릭터가 위로 튀는 회귀를 보강했다. `WorldGrabMinReachHoldSeconds` 이전에는 캐릭터 잡기 후보만 허용하고, 월드 붙음은 `WorldGrabMaxSurfaceNormalZ`, `WorldGrabMinAttachHeightFromActor`, `WorldGrabMaxAttachHeightFromActor` 조건을 통과한 수직 벽 계열 표면에서만 확정한다. 추가로 서버 잡기 확정 자체가 `MinGrabReachAlphaForAttachment` 이상일 때만 진행되도록 해 손이 올라가는 중 좌클릭 연타로 tether가 먼저 걸리는 경로를 막았다. 이후에도 남는 launch를 막기 위해 `WorldGrabTetherMaxUpwardSpeed`로 월드 tether 상승 보정 속도를 제한하고, 월드 잡기 해제 시 잔여 상승 속도를 제거한다. `git diff --check`와 충돌 표식 검색은 통과했고, `SnowRumbleEditor Win64 Development` 빌드는 Live Coding 활성화로 보류됐다.
- 2026-08-21: 플레이어를 잡았을 때 잡힌 캐릭터가 잡는 캐릭터의 가상 reach 위치가 아니라 실제 잡는 손 bone/socket 위치를 기준으로 끌려오게 보강했다. `BuildHandGrabAnchorLocation()`이 `RightGrabHandBoneName`/`LeftGrabHandBoneName`의 Mesh 월드 위치를 우선 사용하고, 없을 때만 기존 `BuildHandGrabTargetLocation()`으로 fallback한다.

## 수동 작업

- `ABP_Modeling` 또는 새 캐릭터 ABP에서 `bIsGrabReaching`, `RightHandGrabTargetLocation`, `LeftHandGrabTargetLocation`, `GrabReachAlpha`를 Control Rig 변수로 전달한다.
- 카메라 상하 시점에 맞춰 spine을 움직이려면 `ViewPitchAlpha`를 Control Rig 변수로 전달하고, 아래 pose와 위 pose를 Lerp한다. `ViewPitchAlpha` 0은 아래, 0.5는 기본 정면, 1은 위로 본다.
- 카메라 좌우 시점에 맞춰 spine을 움직이려면 `ViewYawAlpha`를 Control Rig 변수로 전달하고, 왼쪽 pose와 오른쪽 pose를 Lerp하거나 `-0.5~0.5` offset 값으로 사용한다. `ViewYawAlpha` -0.5는 왼쪽, 0은 기본 정면, 0.5는 오른쪽이다.
- Control Rig에서 `GrabReachAlpha`가 1에 가까울 때 오른손 또는 양손 IK를 손 목표 위치로 보간한다.
- 붙은 손을 고정하려면 `bIsGrabAttached`가 true일 때 `GrabAttachedWorldLocation` 또는 붙은 뒤 고정된 `RightHandGrabTargetLocation`을 Mesh Component space로 변환해 손 목표로 사용한다.
- 벽에 매달린 pose가 필요하면 `bIsHangingFromWorldGrab`으로 상체·하체 pose를 분기한다.
- 잡힌 캐릭터 쪽 표현이 필요하면 `bIsGrabbedByCharacter`로 몸 흔들림, 저항 pose, 입력 불가 pose를 분기한다.
- 잡기 중 손 위치를 정확히 유지하려면 Control Rig 노드를 AnimDynamics 뒤에 배치한다.
- 팔이 계속 흐느적거려야 하면 Control Rig 뒤에 약한 AnimDynamics를 두되, `GrabReachAlpha`가 높을 때 alpha를 낮춘다.
- 캐릭터 Physics Asset에 `hand_r`, `hand_l`과 잡힐 몸통/팔/머리 부위의 physics body를 설정한다.
- 필요하면 `BP_SnowRumbleCharacter`에서 `PlayerGrabComponent`의 `GrabReachForwardDistance`, `GrabReachUpOffset`, `GrabReachSideOffset`, `GrabTraceRadius`를 모델 팔 길이에 맞춘다.
- 보행 중 좌클릭으로 잡기 확정이 너무 민감하면 `MinGrabReachAlphaForAttachment` 또는 `WorldGrabMinReachHoldSeconds`를 올린다. 벽잡기가 너무 잘 안 잡히면 `MinGrabReachAlphaForAttachment`를 낮추거나 `WorldGrabMaxSurfaceNormalZ`, `WorldGrabMinAttachHeightFromActor`/`WorldGrabMaxAttachHeightFromActor` 범위를 조정한다. 월드 잡기 중 위로 끌리는 힘이 강하면 `WorldGrabTetherMaxUpwardSpeed`를 낮춘다.
- 손 올림/내림 속도는 `PlayerGrabComponent`의 `GrabReachRaiseInterpSpeed`, `GrabReachLowerInterpSpeed`로 조정한다.
- 잡힌 몸이 따라오는 느낌은 `PlayerGrabComponent`의 `GrabTetherSlackDistance`, `GrabTetherPullStrength`, `GrabTetherMaxPullSpeed`, `GrabTetherVelocityDamping`으로 조정한다. 더 팽팽하게 붙이려면 slack을 낮추고 pull strength를 올린다.
- 잡힌 몸이 따라오는 기준점은 `RightGrabHandBoneName`/`LeftGrabHandBoneName`의 실제 Mesh bone/socket 위치다. 손끝 socket을 따로 만들면 해당 이름으로 바꿔 더 끝부분 기준으로 끌 수 있고, 현재 기본값은 `hand_r`/`hand_l`이다.
- 잡힌 플레이어의 입력감은 `GrabbedCharacterInputVelocityRetention`으로 조정한다. 낮출수록 잡은 손 위치에 더 강하게 묶이고, 높일수록 잡힌 플레이어 이동 입력이 더 잘 먹는다.
- 잡힌 플레이어가 잡힌 손 위치를 바라보는 속도는 `GrabbedCharacterFacingInterpSpeed`로 조정한다.
- 벽 매달림 위치는 `WorldGrabBodyBackOffset`, `WorldGrabBodyDownOffset`으로 조정하고, 매달림 강도는 `WorldGrabTetherSlackDistance`, `WorldGrabTetherPullStrength`, `WorldGrabTetherMaxPullSpeed`로 조정한다. 이동 입력이 너무 잘 먹으면 `WorldGrabInputVelocityRetention`을 낮추고, 입력감이 너무 죽으면 올린다.
- 벽잡기 중 몸이 손이 붙은 방향을 바라보는 속도는 `WorldGrabFacingInterpSpeed`로 조정한다.
- spine pitch 반응 범위는 `BP_SnowRumbleCharacter`의 `ViewPitchAlphaRangeDegrees`로 조정한다. 기본 60도는 pitch -60도 이하를 `ViewPitchAlpha = 0`, 0도를 0.5, 60도 이상을 1로 본다.
- spine yaw 반응 범위는 `BP_SnowRumbleCharacter`의 `ViewYawAlphaRangeDegrees`로 조정한다. 기본 90도는 yaw -90도 이하를 `ViewYawAlpha = -0.5`, 0도를 0, 90도 이상을 0.5로 본다.
- 아래를 볼 때 눈 제작으로 전환되는 기준은 `BP_SnowRumbleCharacter`의 `SnowCreationPreferredViewPitchAlpha`로 조정한다. 기본 0.35 이하는 잡기 대신 눈 제작을 시도한다.

## 완료 조건

### 에이전트 확인

- [x] `UPlayerGrabComponent` 추가
- [x] 캐릭터 좌클릭 입력과 잡기 reach 연결
- [x] 아래 시점 좌클릭 눈 제작 우선순위 복구
- [x] AnimBP/Control Rig용 잡기 상태와 손 목표값 추가
- [x] AnimBP/Control Rig용 카메라 pitch spine 보정값 추가
- [x] AnimBP/Control Rig용 카메라 yaw spine 보정값 추가
- [x] 서버 검증 trace와 Physics Constraint 기본 연결 지점 추가
- [x] 플레이어·벽 붙음 상태와 붙은 위치 복제 추가
- [x] 벽잡기 매달림 서버 tether 추가
- [x] 잡힌 플레이어 입력 잠금과 서버 tether 이동 추가
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨
- [x] `SnowRumbleEditor Win64 Development` 빌드 성공

### 결과 확인

- [ ] 빈손 상태에서 우클릭 없이 좌클릭을 누르면 `bIsGrabReaching`이 true가 된다.
- [ ] 빈손 상태에서 아래를 보고 좌클릭하면 `bIsGrabReaching`이 켜지지 않고 눈 제작이 시작된다.
- [ ] 좌클릭을 떼면 `bIsGrabReaching`이 false가 된다.
- [ ] AnimBP에서 `GrabReachAlpha`와 손 목표 위치를 Control Rig에 전달할 수 있다.
- [ ] AnimBP에서 `ViewPitchAlpha`를 Control Rig에 전달할 수 있다.
- [ ] AnimBP에서 `ViewYawAlpha`를 Control Rig에 전달할 수 있다.
- [ ] 카메라가 위를 보면 spine이 위쪽으로, 아래를 보면 아래쪽으로 자연스럽게 보정된다.
- [ ] 카메라가 왼쪽을 보면 spine이 왼쪽으로, 오른쪽을 보면 오른쪽으로 자연스럽게 보정된다.
- [ ] 좌클릭 유지 시 `GrabReachAlpha`가 0에서 1로 자연스럽게 올라가고, 해제 시 1에서 0으로 자연스럽게 내려간다.
- [ ] Control Rig 연결 후 좌클릭 유지 중 손이 캐릭터 앞쪽 목표 위치로 올라간다.
- [ ] 호스트와 클라이언트 각각에서 상대 캐릭터 가까이 손을 대면 서버가 잡기 상태를 확정한다.
- [ ] 손 목표가 벽이나 월드 오브젝트에 닿으면 `bIsGrabAttached`가 true가 되고 손 목표 위치가 붙은 위치로 고정된다.
- [ ] 보행 중 빈손 좌클릭을 빠르게 연타해도 캐릭터가 공중으로 튀거나 계속 상승하지 않는다.
- [ ] 벽이나 월드 오브젝트를 잡으면 잡는 캐릭터는 이동 입력으로 몸을 흔들 수 있지만 점프·일반 행동은 할 수 없고, 붙은 손 위치 기준으로 몸이 매달린 위치에 유지되며 몸 방향은 붙은 손 위치 쪽을 유지한다.
- [ ] 플레이어를 잡으면 잡힌 플레이어는 이동 입력으로 몸을 흔들 수 있지만 점프·일반 행동은 할 수 없고, 잡는 사람이 움직이면 몸이 손 위치 쪽으로 끌려오며 몸 방향은 잡힌 손 위치 쪽을 유지하고, 좌클릭 해제 후 다시 자유롭게 움직일 수 있다.

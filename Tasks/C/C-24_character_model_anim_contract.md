# Task C-24 - 캐릭터 모델과 ABP 애니메이션 계약

## 설명

새 캐릭터 모델과 Animation Blueprint를 교체할 때 C++ 플레이어 상태를 안정적으로 읽고, ABP에서 지속 pose와 one-shot 동작을 분리해 연결하는 계약을 제공한다.

## 상태 전이 기준

- 시작 가능: C-01 기존 캐릭터 기반 인수, C-08 팀 식별·이름표 계약, C-11 커스터마이징 머티리얼 적용 경로
- 완료 가능: ABP 부모 클래스, 상태 변수, 지속 pose와 one-shot trigger 계약, 에디터 연결 절차와 결과 확인 항목이 정리됨

## 구현 항목

- [x] `ASnowRumbleCharacter` 상태를 읽는 ABP용 C++ AnimInstance 부모를 제공한다.
- [x] ABP가 직접 읽을 이동, 조준, 눈덩이, 얼음, 사망, 아이템 획득 상태 변수를 제공한다.
- [x] 새 모델용 ABP에서 직접 Sequence Player를 연결할 상태 계약을 제공한다.
- [x] 현재 상태 우선순위에 맞는 이동·상체·전체 몸 액션 상태를 계산한다.
- [x] 맨손, 작은 눈덩이, 큰 눈덩이, 눈삽, 눈오리 제작기 자세를 ABP 단일 enum 상태로 구분한다.
- [x] 이동, 상체 자세, 전체 몸 액션을 분리한 ABP용 파생 상태 enum을 제공한다.
- [x] 눈덩이 줍기, 눈덩이 던지기, 아이템 상호작용, 피격 반응 같은 one-shot 동작을 AnimBP 이벤트로 제공한다.
- [ ] 새 캐릭터 모델 Skeleton과 ABP 자산 연결을 에디터에서 확인한다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: C++·문서 최재원(C), 새 SkeletalMesh·Skeleton·ABP·캐릭터 BP 연결은 사용자/S
- 생성 파일: `Source/SnowRumble/Player/SnowRumbleCharacterAnimInstance_C.h`, `Source/SnowRumble/Player/SnowRumbleCharacterAnimInstance_C.cpp`, `Source/SnowRumble/Player/SnowRumbleCharacterAnimationTypes_C.h`, `Tasks/C/C-24_character_model_anim_contract.md`
- 변경 파일: `Source/SnowRumble/Player/SnowRumbleCharacter.h`, `Source/SnowRumble/Player/SnowRumbleCharacter.cpp`, `Source/SnowRumble/Player/AnimNotify_SnowballThrowRelease_C.*`, `Source/SnowRumble/Snowball/SnowballEquipmentComponent.*`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`
- 공유 확인 대상: S-01, S-05, S-08
- 병합 순서: C++ AnimInstance 계약 반영 후 새 모델·ABP 자산 연결

## 공용 계약과 인계

- 제공받을 계약:
  - `ASnowRumbleCharacter`: 이동, 조준, 눈덩이, 얼음, 사망, 아이템 획득 상태의 원본
  - C-08: 이름표 컴포넌트 위치와 팀 색 표시 계약
  - C-11: 캐릭터 머티리얼 커스터마이징 파라미터와 드로잉 텍스처 계약
- 제공할 계약:
  - `USnowRumbleCharacterAnimInstance`: 새 캐릭터 ABP가 부모로 상속할 C++ AnimInstance
  - `USnowRumbleCharacterAnimInstance::GroundSpeed`: ABP 이동 BlendSpace 또는 전환 조건에 사용할 지상 속도
  - `USnowRumbleCharacterAnimInstance::bIsMoving`: 수평 이동 여부
  - `USnowRumbleCharacterAnimInstance::bIsInAir`: 점프 또는 낙하 상태
  - `USnowRumbleCharacterAnimInstance::bIsSprinting`: 실제 스프린트 이동 상태
  - `USnowRumbleCharacterAnimInstance::bIsFrozen`: 얼음 상태
  - `USnowRumbleCharacterAnimInstance::bIsDead`: 라운드 사망 상태
  - `USnowRumbleCharacterAnimInstance::bIsAiming`: 눈덩이 조준 상태
  - `USnowRumbleCharacterAnimInstance::bIsChargingSnowball`: 눈덩이 투척 충전 상태
  - `USnowRumbleCharacterAnimInstance::bIsCreatingSnowball`: 눈덩이 제작 상태
  - `USnowRumbleCharacterAnimInstance::bIsPickingUpItem`: 아이템 획득 연출 상태
  - `USnowRumbleCharacterAnimInstance::bIsInteractingWithItem`: 선물상자 열기와 선물 아이템 획득 상호작용 연출 상태
  - `USnowRumbleCharacterAnimInstance::bIsHitReacting`: 실제 피해를 받은 직후 피격 반응 연출 상태
  - `USnowRumbleCharacterAnimInstance::SnowballCarryState`: 평소, 작은 눈덩이 보유, 큰 눈덩이 보유 구분
  - `USnowRumbleCharacterAnimInstance::HeldAnimationState`: 맨손, 작은 눈덩이, 큰 눈덩이, 눈삽, 눈오리 제작기 자세 구분
  - `USnowRumbleCharacterAnimInstance::SnowballActionState`: 굴리기 같은 운반과 별개인 눈덩이 행동 상태
  - `USnowRumbleCharacterAnimInstance::TimedActionState`: 머리 위 진행 행동과 같은 제작·굴리기 상태
  - `USnowRumbleCharacterAnimInstance::SnowballChargeProgress`: 투척 충전 0~1 진행도
  - `USnowRumbleCharacterAnimInstance::SnowballCreationProgress`: 눈덩이 제작 0~1 진행도
  - `USnowRumbleCharacterAnimInstance::LocomotionAnimState`: `Idle`, `Walk`, `Sprint`, `InAir` 중 하체 이동 pose 선택용 상태
  - `USnowRumbleCharacterAnimInstance::UpperBodyAnimState`: `None`, `SmallSnowball`, `SmallSnowballAim`, `SmallSnowballCharge`, `LargeSnowball`, `LargeSnowballAim`, `LargeSnowballCharge`, `SnowShovel`, `SnowShovelAim`, `SnowShovelCharge`, `SnowDuckMaker`, `SnowDuckMakerAim`, `SnowDuckMakerCharge` 중 상체 override pose 선택용 상태
  - `USnowRumbleCharacterAnimInstance::FullBodyAnimState`: `None`, `CreateSnowball`, `RollSnowball`, `Pickup`, `ItemInteraction`, `HitReact`, `Frozen`, `Dead` 중 최종 전체 몸 action pose 선택용 상태
  - `USnowRumbleCharacterAnimInstance::HasUpperBodyOverride()`: 상체 pose를 locomotion 위에 섞어야 하는지 반환
  - `USnowRumbleCharacterAnimInstance::HasFullBodyOverride()`: 전체 몸 action pose가 최종 pose를 덮어써야 하는지 반환
  - `ESnowRumbleCharacterAnimTrigger`: 서버가 확정한 one-shot 동작 enum. `PickupSmallSnowball`, `PickupLargeSnowball`, `ItemInteraction`, `ThrowSmallSnowball`, `ThrowLargeSnowball`, `ThrowSnowDuckMaker`, `HitReact`를 제공한다.
  - `USnowRumbleCharacterAnimInstance::OnAnimationTriggerRequested(ESnowRumbleCharacterAnimTrigger Trigger)`: AnimBP가 구현하는 이벤트. 캐릭터 C++이 서버 multicast로 호출하며, ABP에서 Montage나 Slot 재생 분기에 사용한다.
  - `UAnimNotify_SnowballThrowRelease`: 던지기 몽타주에서 실제 눈덩이가 손을 떠나는 프레임에 배치하는 AnimNotify. Notify가 호출되면 `ASnowRumbleCharacter::RequestSnowballThrowReleaseFromNotify()`를 거쳐 서버 pending throw를 실제 `ASnowballItem::Throw()`로 확정한다.
  - `ASnowRumbleCharacter::LargeSnowballHoldPoint`: 최대 성장 큰 눈덩이 전용 부착 위치 컴포넌트. 캐릭터 Mesh의 `LargeSnowballSocket`에 붙으며, 소켓이 없으면 기존 `SnowballSocket` 기반 `SnowballHoldPoint`를 사용한다.
  - `ASnowRumbleCharacter::GetSnowballHoldPointForSnowball(const ASnowballItem*)`: 눈덩이 성장 상태에 따라 작은 눈은 `SnowballHoldPoint`, 최대 성장 큰 눈은 `LargeSnowballHoldPoint`를 반환한다.
  - `ASnowRumbleCharacter::ScarfMeshComponent`: 목도리 StaticMesh 부착 슬롯. 기본 `ScarfSocket`에 붙고, `ScarfMesh`, `ScarfAttachSocketName`, `ScarfRelativeLocation`, `ScarfRelativeRotation`, `ScarfRelativeScale`로 Blueprint에서 모델과 위치를 조정한다. 목도리 Material은 `ScarfTeamColorParameterName` 기본 `TeamColor` Vector Parameter로 현재 팀 색을 받는다.
- 인계 대상: 사용자/S. 새 ABP는 `USnowRumbleCharacterAnimInstance`를 부모로 만들고, Anim Graph에서 지속 상태는 enum별 Sequence Player로 직접 연결한다. 줍기·던지기·피격 같은 순간 동작은 `OnAnimationTriggerRequested` 이벤트에서 Montage로 재생한다.

## 범위 밖

- 새 캐릭터 모델 제작
- 새 Skeleton 리타기팅과 애니메이션 에셋 제작
- `.uasset` 자동 수정
- 서버 판정, 복제 상태, HP·승패 규칙 변경

## 사전 전제

- C-01
- C-08
- C-11

## 결정 필요

- 새 캐릭터 Skeleton이 기존 Skeleton과 같은지, 리타기팅이 필요한지 확인
- 없음
- 눈삽과 눈오리 제작기를 동시에 장착했을 때 별도 선택 입력을 둘지 확인. 현재 애니메이션 기준은 눈덩이 보유가 최우선이고, 그 다음 눈삽, 눈오리 제작기 순서다.

## 변경 기록

- 2026-08-11: 새 캐릭터 모델과 ABP를 Codex 관리 계약으로 전환하기 위해 C-24를 추가했다. ABP 부모 `USnowRumbleCharacterAnimInstance`와 슬롯형 애니메이션 프로퍼티를 제공한다. UHT와 C++ 컴파일은 통과했으나 실행 중인 Unreal Editor PID 41016의 DLL 잠금으로 최종 링크는 보류됐다.
- 2026-08-12: 아이템 장착 표현에 맞춰 `ESnowRumbleHeldAnimationState`와 `HeldAnimationState`를 추가했다. ABP는 `BareHands`, `SmallSnowball`, `LargeSnowball`, `SnowShovel`, `SnowDuckMaker` 기준으로 자세 상태머신을 나눌 수 있고, 빠른 연결용 슬롯 `SnowShovelHoldAnimation`, `SnowDuckMakerHoldAnimation`을 제공한다.
- 2026-08-12: 선물상자/선물 아이템 상호작용과 피격 반응용 상태를 추가했다. `ItemInteractionAnimation`은 선물상자 열기와 선물 아이템 획득 성공 때, `HitReactAnimation`은 실제 HP 피해가 적용된 직후 짧게 재생하는 슬롯이다.
- 2026-08-12: `git diff --check`와 UHT/C++ 컴파일을 통과했다. 최종 링크는 실행 중인 Unreal Editor가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 `LNK1104`로 보류됐다.
- 2026-08-13: `GetPrimaryAnimation()` 단일 출력 방식은 조준+이동, 스프린트+장착처럼 상태 조합이 늘어날 때 ABP에서 쓰기 어렵고 thread-safe 경고를 만들 수 있어, C++ 부모가 `LocomotionAnimState`, `UpperBodyAnimState`, `FullBodyAnimState`를 계산하는 구조로 확장했다. ABP는 이 세 enum으로 pose를 조합한다. C++ 컴파일은 통과했고, 최종 링크는 실행 중인 Unreal Editor DLL 잠금 `LNK1104`로 보류됐다.
- 2026-08-13: ABP에서 애니메이션 에셋을 Sequence Player에 직접 연결하기로 결정해 `GetPrimaryAnimation()` 함수와 `IdleAnimation` 등 Class Defaults 슬롯 프로퍼티를 제거했다.
- 2026-08-13: 지속 pose만으로는 큰 눈덩이를 허리 펴며 드는 동작처럼 시작/종료가 있는 애니메이션을 표현하기 어려워 `ESnowRumbleCharacterAnimTrigger`와 `OnAnimationTriggerRequested` 이벤트를 추가했다. 눈덩이 줍기·던지기·아이템 상호작용·피격 반응은 서버 확정 후 모든 화면의 AnimBP로 trigger가 전달된다. UHT와 C++ 컴파일은 통과했고, 최종 링크는 실행 중인 Unreal Editor DLL 잠금 `LNK1104`로 보류됐다.
- 2026-08-18: 눈덩이 던지기 실제 발사 시점을 몽타주 Notify로 옮겼다. 입력 release 때 서버가 투척 방향·속도·충전량을 검증해 pending으로 저장하고 `ThrowSmallSnowball`/`ThrowLargeSnowball` 몽타주 trigger를 보낸 뒤, `UAnimNotify_SnowballThrowRelease`가 호출될 때 기존 `ASnowballItem::Throw()`를 실행한다.
- 2026-08-18: 위 변경은 `git diff --check`, UHT, C++ 컴파일과 `.lib` 생성을 통과했다. 최종 DLL 링크는 실행 중인 Unreal Editor의 `UnrealEditor-SnowRumble.dll` 잠금 `LNK1104`로 보류됐다.
- 2026-08-18: 큰 눈덩이 전용 부착 위치를 추가했다. 작은 눈은 기존 `SnowballSocket`, 최대 성장 큰 눈은 `LargeSnowballSocket`에 붙고, 큰 눈 소켓이 아직 없으면 기존 작은 눈 소켓으로 fallback한다.
- 2026-08-18: 큰 눈덩이 소켓 변경은 `git diff --check`, UHT, C++ 컴파일과 `.lib` 생성을 통과했다. 최종 DLL 링크는 실행 중인 Unreal Editor의 `UnrealEditor-SnowRumble.dll` 잠금 `LNK1104`로 보류됐다.
- 2026-08-18: 상체 조준 pose를 장착별로 분리했다. 기존 단일 `Aim` 대신 `SmallSnowballAim`, `LargeSnowballAim`, `SnowShovelAim`, `SnowDuckMakerAim`을 사용하고, 평상시 보유 pose는 기존 `SmallSnowball`, `LargeSnowball`, `SnowShovel`, `SnowDuckMaker`를 유지한다.
- 2026-08-18: 장착별 조준 pose 변경은 `git diff --check`, UHT, C++ 컴파일과 `.lib` 생성을 통과했다. 최종 DLL 링크는 실행 중인 Unreal Editor의 `UnrealEditor-SnowRumble.dll` 잠금 `LNK1104`로 보류됐다.
- 2026-08-18: 상체 충전 pose도 장착별로 분리했다. 기존 단일 `ChargeSnowball` 대신 `SmallSnowballCharge`, `LargeSnowballCharge`, `SnowShovelCharge`, `SnowDuckMakerCharge`를 사용한다.
- 2026-08-18: 장착별 충전 pose 변경은 `git diff --check`, UHT, C++ 컴파일과 `.lib` 생성을 통과했다. 최종 DLL 링크는 실행 중인 Unreal Editor의 `UnrealEditor-SnowRumble.dll` 잠금 `LNK1104`로 보류됐다.
- 2026-08-18: `OnAnimationTriggerRequested`의 던지기 one-shot trigger에 눈오리 제작기 전용 `ThrowSnowDuckMaker`를 추가했다. 눈오리 제작기를 장착한 상태에서 던지기 성공 trigger가 발생하면 작은/큰 눈덩이 trigger 대신 눈오리 제작기 trigger를 보낸다.
- 2026-08-18: 눈오리 제작기 던지기 trigger 변경은 `git diff --check`, UHT, C++ 컴파일과 `.lib` 생성을 통과했다. 최종 DLL 링크는 실행 중인 Unreal Editor의 `UnrealEditor-SnowRumble.dll` 잠금 `LNK1104`로 보류됐다.
- 2026-08-19: 캐릭터 목도리 표현용 `ScarfMeshComponent`를 StaticMesh 슬롯으로 추가했다. 기본 부착 소켓은 `ScarfSocket`이며, `BP_SnowRumbleCharacter`에서 목도리 StaticMesh와 상대 Transform을 조정한다. 목도리 Dynamic Material에는 현재 팀 색을 `TeamColor` Vector Parameter로 적용한다.

## 수동 작업

- 새 캐릭터용 Animation Blueprint를 만들고 부모 클래스를 `USnowRumbleCharacterAnimInstance`로 지정한다.
- Anim Graph의 기본 하체 pose는 `Blend Poses by ESnowRumbleLocomotionAnimState`로 만들고, `Idle`, `Walk`, `Sprint`, `InAir` 핀에 각 Sequence Player를 연결한다.
- 상체 pose는 `Blend Poses by ESnowRumbleUpperBodyAnimState`로 만들고, `SmallSnowball`, `SmallSnowballAim`, `SmallSnowballCharge`, `LargeSnowball`, `LargeSnowballAim`, `LargeSnowballCharge`, `SnowShovel`, `SnowShovelAim`, `SnowShovelCharge`, `SnowDuckMaker`, `SnowDuckMakerAim`, `SnowDuckMakerCharge` 핀에 각 Sequence Player를 연결한다.
- `HasUpperBodyOverride()`가 true이면 `Layered Blend Per Bone`으로 상체 pose를 기본 하체 pose 위에 섞는다. 시작 bone은 새 Skeleton 기준 spine 또는 chest 계열 본으로 잡는다.
- 전체 몸 action pose는 `Blend Poses by ESnowRumbleFullBodyAnimState`로 만들고, `CreateSnowball`, `RollSnowball`, `Pickup`, `ItemInteraction`, `HitReact`, `Frozen`, `Dead` 핀에 각 Sequence Player를 연결한다.
- `HasFullBodyOverride()`가 true이면 전체 몸 action pose가 최종 pose를 덮어쓰게 `Blend Poses by Bool`로 연결한다.
- Anim Graph에 Montage 재생용 Slot 노드를 최종 출력 직전 또는 full-body layer 뒤에 배치한다.
- Event Graph에서 `OnAnimationTriggerRequested`를 구현하고 `Switch on ESnowRumbleCharacterAnimTrigger`로 분기한다.
- `PickupSmallSnowball`/`PickupLargeSnowball`에는 눈덩이를 허리 펴며 드는 시작 동작 Montage를 연결하고, Montage 종료 뒤에는 `UpperBodyAnimState`의 보유 pose가 자연스럽게 유지되게 한다.
- `ThrowSmallSnowball`/`ThrowLargeSnowball`/`ThrowSnowDuckMaker`, `ItemInteraction`, `HitReact`도 같은 이벤트에서 각 Montage를 재생한다.
- 작은 눈덩이와 큰 눈덩이 Throw 몽타주에는 실제 눈덩이가 손을 떠나야 하는 프레임에 `Snow Rumble Snowball Throw Release` AnimNotify를 한 번 배치한다. 이 Notify가 없으면 서버가 pending throw를 실제 발사로 확정하지 않는다.
- Notify를 중복 배치해도 서버 pending throw는 한 번만 소비되지만, 몽타주당 한 번만 배치한다.
- 우선 넣을 애니메이션은 맨손 Idle/Walk/Run/Jump/Fall, 작은 눈덩이 Hold/Walk/Aim/ThrowCharge/Throw, 큰 눈덩이 Hold/HeavyWalk/Aim/ThrowCharge/Throw, 눈삽 Hold/Walk/Swing, 눈오리 제작기 Hold/Walk/Use, 눈 만들기, 굴리기, 눈덩이 줍기, 선물상자/선물 아이템 상호작용, 피격 반응, 얼음, 사망이다.
- `BP_SnowRumbleCharacter`의 Mesh에 새 SkeletalMesh를 지정한다.
- `BP_SnowRumbleCharacter`의 Mesh Anim Class에 새 ABP를 지정한다.
- 새 모델 머티리얼에 C-11의 `BodyColor` Vector Parameter와 `PaintTexture` Texture Parameter를 연결하거나, BP의 파라미터 이름을 실제 머티리얼 이름에 맞춘다.
- 새 모델 머리 높이가 다르면 `OverheadNameRelativeLocation`을 조정한다.
- 새 Skeleton에는 작은 눈용 `SnowballSocket`과 큰 눈용 `LargeSnowballSocket`을 만든다. 큰 눈 소켓은 큰 눈 Hold/Throw 몽타주에서 양손 또는 몸 앞 위치에 맞게 조정한다.
- 새 Skeleton 또는 캐릭터 Mesh에는 목도리용 `ScarfSocket`을 목/가슴 사이에 만들고, `BP_SnowRumbleCharacter`에서 `ScarfMesh`, `ScarfAttachSocketName`, `ScarfRelativeLocation`, `ScarfRelativeRotation`, `ScarfRelativeScale`을 조정한다. 목도리 Material에는 기본 이름 `TeamColor`의 Vector Parameter를 만들거나, 실제 파라미터 이름에 맞게 `ScarfTeamColorParameterName`을 바꾼다.

## 완료 조건

### 에이전트 확인

- [x] ABP용 C++ AnimInstance 부모 추가
- [x] 애니메이션 상태 변수 추가
- [x] 맨손·작은 눈·큰 눈·눈삽·눈오리 제작기 자세 구분 enum 추가
- [x] 이동·상체·전체 몸 액션 파생 상태 enum 추가
- [x] 선물 아이템 상호작용과 피격 반응 상태·trigger 추가
- [x] one-shot 애니메이션 trigger enum과 AnimBP 이벤트 추가
- [x] 목도리 StaticMesh 부착 컴포넌트와 `ScarfSocket`, 팀 색 머티리얼 파라미터 조정값 추가
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨
- [x] 로컬 정적 점검과 C++ 컴파일 통과. 실행 중인 Unreal Editor DLL 잠금으로 최종 링크는 보류
- [x] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [x] 공용 계약과 캡슐화 규칙 위반 없음

### 결과 확인

- [ ] 새 ABP 부모가 `USnowRumbleCharacterAnimInstance`로 설정된다.
- [ ] 새 ABP Anim Graph의 상태별 Sequence Player에 새 캐릭터 Skeleton용 애니메이션을 지정할 수 있다.
- [ ] `BP_SnowRumbleCharacter`에 새 SkeletalMesh와 새 ABP를 지정하면 PIE에서 캐릭터가 스폰된다.
- [ ] 걷기, 달리기, 점프/낙하, 조준, 눈덩이 보유, 눈덩이 제작, 굴리기, 아이템 획득, 얼음, 사망 상태가 ABP 변수로 갱신된다.
- [ ] 새 모델에서도 이름표, 팀 색, 커스터마이징 머티리얼, 눈덩이 손 부착 위치가 깨지지 않는다.
- [ ] 새 모델의 `ScarfSocket`에 목도리 StaticMesh가 붙고 팀 변경 시 목도리 Material 색이 현재 팀 색으로 바뀐다.

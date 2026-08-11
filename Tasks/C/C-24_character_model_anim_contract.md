# Task C-24 - 캐릭터 모델과 ABP 애니메이션 계약

## 설명

새 캐릭터 모델과 Animation Blueprint를 교체할 때 C++ 플레이어 상태를 안정적으로 읽고, ABP에서 애니메이션 슬롯에 에셋을 장착하는 계약을 제공한다.

## 상태 전이 기준

- 시작 가능: C-01 기존 캐릭터 기반 인수, C-08 팀 식별·이름표 계약, C-11 커스터마이징 머티리얼 적용 경로
- 완료 가능: ABP 부모 클래스, 상태 변수, 애니메이션 슬롯 계약, 에디터 연결 절차와 결과 확인 항목이 정리됨

## 구현 항목

- [x] `ASnowRumbleCharacter` 상태를 읽는 ABP용 C++ AnimInstance 부모를 제공한다.
- [x] ABP가 직접 읽을 이동, 조준, 눈덩이, 얼음, 사망, 아이템 획득 상태 변수를 제공한다.
- [x] 새 모델용 ABP에서 애니메이션 에셋을 장착할 슬롯 프로퍼티를 제공한다.
- [x] 현재 상태 우선순위에 맞는 주 애니메이션을 반환하는 조회 함수를 제공한다.
- [ ] 새 캐릭터 모델 Skeleton과 ABP 자산 연결을 에디터에서 확인한다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: C++·문서 최재원(C), 새 SkeletalMesh·Skeleton·ABP·캐릭터 BP 연결은 사용자/S
- 생성 파일: `Source/SnowRumble/Player/SnowRumbleCharacterAnimInstance_C.h`, `Source/SnowRumble/Player/SnowRumbleCharacterAnimInstance_C.cpp`, `Tasks/C/C-24_character_model_anim_contract.md`
- 변경 파일: `Tasks/C/PLAN_C.md`, `docs/PLANS.md`
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
  - `USnowRumbleCharacterAnimInstance::SnowballCarryState`: 평소, 작은 눈덩이 보유, 큰 눈덩이 보유 구분
  - `USnowRumbleCharacterAnimInstance::SnowballActionState`: 굴리기 같은 운반과 별개인 눈덩이 행동 상태
  - `USnowRumbleCharacterAnimInstance::TimedActionState`: 머리 위 진행 행동과 같은 제작·굴리기 상태
  - `USnowRumbleCharacterAnimInstance::SnowballChargeProgress`: 투척 충전 0~1 진행도
  - `USnowRumbleCharacterAnimInstance::SnowballCreationProgress`: 눈덩이 제작 0~1 진행도
  - `USnowRumbleCharacterAnimInstance::GetPrimaryAnimation()`: 현재 상태 우선순위에 맞는 주 애니메이션 슬롯 반환
  - 애니메이션 슬롯 프로퍼티: `IdleAnimation`, `WalkAnimation`, `SprintAnimation`, `JumpOrFallAnimation`, `AimIdleAnimation`, `AimWalkAnimation`, `SmallSnowballHoldAnimation`, `LargeSnowballHoldAnimation`, `SnowballChargeAnimation`, `CreateSnowballAnimation`, `RollSnowballAnimation`, `PickupAnimation`, `FrozenAnimation`, `DeadAnimation`
- 인계 대상: 사용자/S. 새 ABP는 `USnowRumbleCharacterAnimInstance`를 부모로 만들고 슬롯 프로퍼티에 새 Skeleton용 애니메이션 에셋을 지정한다.

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
- ABP가 단일 `GetPrimaryAnimation()` 기반으로 빠르게 붙을지, BlendSpace와 상태머신으로 확장할지 확인

## 변경 기록

- 2026-08-11: 새 캐릭터 모델과 ABP를 Codex 관리 계약으로 전환하기 위해 C-24를 추가했다. ABP 부모 `USnowRumbleCharacterAnimInstance`와 슬롯형 애니메이션 프로퍼티를 제공한다. UHT와 C++ 컴파일은 통과했으나 실행 중인 Unreal Editor PID 41016의 DLL 잠금으로 최종 링크는 보류됐다.

## 수동 작업

- 새 캐릭터용 Animation Blueprint를 만들고 부모 클래스를 `USnowRumbleCharacterAnimInstance`로 지정한다.
- 새 ABP의 Class Defaults에서 필요한 슬롯 프로퍼티에 새 Skeleton용 Animation Sequence를 지정한다.
- 빠른 확인용 ABP는 `GetPrimaryAnimation()` 반환값을 재생하는 구조로 시작하고, 이후 품질이 필요하면 같은 상태 변수로 BlendSpace와 상태머신을 구성한다.
- `BP_SnowRumbleCharacter`의 Mesh에 새 SkeletalMesh를 지정한다.
- `BP_SnowRumbleCharacter`의 Mesh Anim Class에 새 ABP를 지정한다.
- 새 모델 머티리얼에 C-11의 `BodyColor` Vector Parameter와 `PaintTexture` Texture Parameter를 연결하거나, BP의 파라미터 이름을 실제 머티리얼 이름에 맞춘다.
- 새 모델 머리 높이가 다르면 `OverheadNameRelativeLocation`을 조정한다.
- 새 Skeleton의 손 소켓 이름이 다르면 `SnowballSocket`을 만들거나 `SnowballHoldPoint` 부착 기준을 후속으로 조정한다.

## 완료 조건

### 에이전트 확인

- [x] ABP용 C++ AnimInstance 부모 추가
- [x] 애니메이션 상태 변수와 슬롯 프로퍼티 추가
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨
- [x] 로컬 정적 점검과 C++ 컴파일 통과. 실행 중인 Unreal Editor DLL 잠금으로 최종 링크는 보류
- [x] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [x] 공용 계약과 캡슐화 규칙 위반 없음

### 결과 확인

- [ ] 새 ABP 부모가 `USnowRumbleCharacterAnimInstance`로 설정된다.
- [ ] 새 ABP 슬롯 프로퍼티에 새 캐릭터 Skeleton용 애니메이션을 지정할 수 있다.
- [ ] `BP_SnowRumbleCharacter`에 새 SkeletalMesh와 새 ABP를 지정하면 PIE에서 캐릭터가 스폰된다.
- [ ] 걷기, 달리기, 점프/낙하, 조준, 눈덩이 보유, 눈덩이 제작, 굴리기, 아이템 획득, 얼음, 사망 상태가 ABP 변수로 갱신된다.
- [ ] 새 모델에서도 이름표, 팀 색, 커스터마이징 머티리얼, 눈덩이 손 부착 위치가 깨지지 않는다.

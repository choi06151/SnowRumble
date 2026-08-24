# Task C-06 - 얼음·사망·관전

## 설명

HP 0인 플레이어가 60초 동안 부활 가능한 얼음 상태가 되고 이후 사망하며, 두 상태 모두 생존 아군을 관전하게 한다.

## 상태 전이 기준
- 시작 가능: C-01 완료
- 완료 가능: 서버 상태 전이·팀 전멸 판정과 S-07 인계 확인

## 구현 항목
- [x] 서버가 HP 0에서 60초 얼음 상태를 시작한다.
- [x] 얼음 상태의 이동·공격·상호작용 제한과 충돌·눈덩이 차단을 유지한다.
- [x] 같은 팀 생존 플레이어가 얼음 상태 플레이어를 Grab으로 운반할 수 있게 한다.
- [x] 기존 `OverheadTimedActionWidget`이 얼음 상태의 60초 사망 타이머를 1에서 0으로 표시하게 한다.
- [x] 제한시간 뒤 해당 라운드 사망으로 전환한다.
- [ ] 얼음·사망 플레이어가 생존 중인 아군을 관전한다.
- [ ] 마지막 생존자가 얼면 진행 중 부활과 관계없이 팀 패배를 확정한다.

## 작업 배정
- 담당자·기능·계약 소유자: 최재원(C)
- 자산 수정자: C++·문서 최재원(C), 얼음/사망 표현 UI·VFX는 사용자 또는 S 인계
- 생성 파일: 없음
- 변경 파일: `Source/SnowRumble/Player/SnowRumbleHealthComponent.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Tasks/C/C-06_freeze_death_spectate.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`
- 공유 확인 대상: C-07, S-07, S-05
- 병합 순서: C-01 후, C-07·S-07 전

## 공용 계약과 인계
- 제공받을 계약: 기존 HP·얼기 상태
- 제공할 계약:
  - `USnowRumbleHealthComponent::IsDead()`: 해당 라운드 사망 상태를 반환한다.
  - `USnowRumbleHealthComponent::GetFrozenSecondsRemaining()`: 얼음 상태에서 사망까지 남은 서버 기준 시간을 반환한다.
  - `USnowRumbleHealthComponent::ReviveFromFrozen(float HealthRatio)`: 서버 핫팩 부활 완료 시 얼음 타이머를 취소하고 HP를 회복한다. C-13은 기본 `0.5f`로 호출한다.
  - `USnowRumbleHealthComponent::OnDeathChanged`: 사망 상태 변경을 UI/표현에 알린다.
  - `ASnowRumbleCharacter::IsDead()`: 캐릭터 사망 상태를 Animation Blueprint와 UI에서 읽는다.
  - `ASnowRumbleCharacter::GetFrozenSecondsRemaining()`: 캐릭터 기준 얼음 남은 시간을 UI에서 읽는다.
  - `ASnowRumbleCharacter::GetFrozenProgress()`: 얼음 60초 사망 타이머의 남은 진행도를 1에서 0으로 반환한다.
  - 관전 대상과 팀 전멸 이벤트는 C-06 후속 범위에서 제공한다.
- Grab 연동: 얼음 상태 대상은 서버가 같은 팀 Grabber에게만 잡히도록 허용하고, 이동·공격·상호작용 제한은 유지한 채 기존 Grab tether로 운반한다.
- 인계 대상: C-05, C-07, S-07

## 범위 밖
- 얼음 추가 공격 파괴
- 핫팩 아이템 소유와 UI 표현

## 사전 전제
- C-01

## 기존 구현 인수
- `USnowRumbleHealthComponent`가 `MaxHealth`, `CurrentHealth`, `bIsFrozen`을 서버 권한으로 관리하고 복제한다.
- `OnHealthChanged`, `OnFrozenChanged`, `ASnowRumbleCharacter::IsFrozen`, `TakeDamage`가 현재 HP·얼기 연결 지점이다.
- `ASnowRumbleCharacter::ClientRequestLocalDamageFeedback(float AppliedDamage, FVector DamageCauserLocation)`는 실제 HP 피해가 적용된 캐릭터의 소유 클라이언트에서만 호출되는 로컬 피격 화면 연출 경로다. C++ 기본값으로 파란 camera fade와 짧은 camera boom 흔들림을 재생하고, `OnLocalDamageFeedbackRequested` Blueprint 이벤트는 추가 사운드·위젯 연출용 선택 확장 지점으로 남긴다.
- 기존 얼기 상태는 HP 0에서 즉시 확정되는 프로토타입이며, 60초 부활 가능 얼음·사망·관전·팀 전멸 상태는 아직 없다.

## 결정 필요
- 관전 대상 순환 입력과 생존 아군이 없을 때 카메라 처리

## 수동 작업

- 얼음 상태 UI가 필요하면 `ASnowRumbleCharacter::GetFrozenSecondsRemaining()` 값을 초 단위로 표시한다.
- 사망 표현이 필요하면 `ASnowRumbleCharacter::IsDead()` 또는 `USnowRumbleHealthComponent::OnDeathChanged`를 읽어 WBP/Animation Blueprint/VFX에서 연결한다.
- 피격 화면 연출 기본값은 C++에서 `DamageFeedbackTintDuration`, `DamageFeedbackTintAlpha`, `DamageFeedbackTintColor`, `DamageFeedbackCameraShakeDuration`, `DamageFeedbackCameraShakeAmplitude`, `DamageFeedbackCameraShakeFrequency`로 조정한다. `OnLocalDamageFeedbackRequested`는 추가 사운드·위젯 효과가 필요할 때만 구현한다.

## 완료 조건
### 에이전트 확인
- [x] HP 0 → 60초 얼음 → 사망 서버 흐름 완료
- [x] 사망 상태 복제와 Blueprint 읽기 계약 제공
- [x] 사망 상태 행동 제한 연결
- [x] `git diff --check` 공백 점검 통과
- [x] 관련 C++ 컴파일 통과
- [ ] Unreal Editor 종료 후 `SnowRumbleEditor Win64 Development` 최종 링크 확인
- [ ] 관전 서버 흐름 완료
- [ ] 팀 전멸 우선순위 점검 완료
- [ ] S-07 인계 완료

### 검증 메모

- 2026-08-10: `USnowRumbleHealthComponent`가 HP 0에서 서버 권한으로 60초 얼음 타이머를 시작하고, 만료 시 얼음 상태를 해제한 뒤 `bIsDead`를 복제하게 했다. `ASnowRumbleCharacter`는 사망 상태에서 이동·행동·조준·점프를 중지한다. 핫팩 부활 연결을 위해 `ReviveFromFrozen(0.5f)` 계약을 제공한다. `git diff --check`는 통과했고 관련 C++ 컴파일도 통과했지만, 실행 중인 Unreal Editor가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크는 `LNK1104`로 실패했다. 에디터 종료 후 재빌드 확인이 필요하다.
- 2026-08-19: 실제 HP 피해가 적용된 경우 `ASnowRumbleCharacter::ClientRequestLocalDamageFeedback()`이 피해 받은 캐릭터의 소유 클라이언트에만 호출되고, C++에서 파란 camera fade와 짧은 camera boom 흔들림을 재생하게 했다. `OnLocalDamageFeedbackRequested(AppliedDamage, DamageCauserLocation)` Blueprint 이벤트는 추가 연출 확장용으로 유지한다.

### 결과 확인

- [ ] 서버와 클라이언트에서 HP가 0이 된 플레이어가 얼음 상태가 된다.
- [ ] 얼음 상태에서 60초 동안 이동, 점프, 조준, 공격, 상호작용이 동작하지 않는다.
- [ ] 얼음 상태의 같은 팀 플레이어가 Grab하면 얼음 대상이 입력·공격 제한을 유지한 채 함께 이동한다.
- [ ] 적 팀은 얼음 상태 플레이어를 Grab할 수 없다.
- [ ] 얼음 시간이 끝나 사망하면 Grab이 자동 해제되고 얼음 대상은 계속 이동할 수 없다.
- [ ] 얼음 상태에서 기존 `OverheadTimedActionWidget` ProgressBar가 1에서 0으로 60초 동안 감소한다.
- [ ] 얼음 상태에서 60초가 지나면 `IsDead()`가 true가 되고 계속 행동할 수 없다.
- [ ] 핫팩 부활 구현 전에는 60초 안에 자동 회복되지 않는다.
- [ ] 피해를 받은 플레이어 화면에만 파란 피격 화면 이펙트와 카메라 흔들림이 재생되고, 공격자나 다른 클라이언트 화면에는 재생되지 않는다.

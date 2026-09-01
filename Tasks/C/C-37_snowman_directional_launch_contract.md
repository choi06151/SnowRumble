# Task C-37 - 눈사람 방향성 캐릭터 Launch 계약

## 설명

눈사람 플레이어가 좌클릭을 누르면 현재 카메라가 바라보는 방향으로 자신의 캐릭터를 서버 권한으로 Launch한다. C는 재사용 가능한 서버 검증·실행 계약을 제공하고, K는 눈사람 전용 Pawn의 좌클릭 입력에 연결한다.

## 상태 전이 기준

- 시작 가능: 눈사람 전용 Pawn 입력 소유권 확인, 사용자 구현 승인
- 완료 가능: 서버 권한 Launch 계약 구현, K의 눈사람 좌클릭 연결 인계, 호스트·클라이언트 수동 확인

## 구현 항목

- [x] C 소유 캐릭터에 카메라 방향 기반 서버 Launch 요청 계약과 조정 가능한 Launch 속도·쿨다운을 추가한다.
- [x] 서버가 눈사람 모드 경기 진행·행동 가능 상태·쿨다운을 검사한 뒤 캐릭터를 Launch한다.
- [x] K가 `ASnowmanModeSnowmanCharacter`의 `ActionAction` 좌클릭 입력을 C 계약에 연결한다.
- [x] Launch 쿨다운을 `ESnowRumbleTimedActionState`와 `GetTimedActionProgress()`에 연결해 기존 눈 제작 ProgressBar를 쿨다운 시각화에 재사용한다.
- [x] 눈사람 이동 속도 튜닝값이 Blueprint Class Defaults에서 찾기 쉽도록 노출 이름과 카테고리를 보강한다.
- [ ] Launch 결과가 캐릭터 이동 복제로 호스트와 클라이언트에 동일하게 반영되도록 확인한다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 강혜원(K) — 눈사람 Pawn 입력 및 모드 기능
- 계약 소유자: 최재원(C) — 서버 Launch 요청·검증 계약
- 자산 수정자: 사용자/S — 필요 시 키 가이드·연출 연결
- 생성·변경 후보: 구현 승인 후 C 소유 `Source/SnowRumble/Player/SnowRumbleCharacter.*`, K 소유 `Source/SnowRumble/Player/SnowmanModeSnowmanCharacter_K.*`, 이 Task와 계획 문서
- 공유 확인 대상: K 눈사람 모드, C 플레이어 네트워크 계약, S UI·연출
- 병합 순서: C 서버 계약 구현 → K 눈사람 좌클릭 연결 → 수동 멀티플레이 확인

## 공용 계약과 인계

- 제공받을 계약: 기존 `ActionAction`, `CanPerformGameplayAction()`, 눈사람 모드 GameState 시작·종료 상태
- 제공할 계약: 눈사람 Pawn이 호출할 서버 권한 방향성 Launch 요청 함수와 Launch 조정값
- 인계 대상: K

## 범위 밖

- 눈사람 전용 애니메이션·VFX·사운드 제작
- 새 ProgressBar Widget 제작 — 기존 눈 제작용 `TimedActionProgressBar`를 재사용
- 공중 대시, 조준 보정, 피해 판정 또는 승패 규칙 추가
- 다른 플레이어 Pawn의 좌클릭 동작 변경

## 사전 전제

- 서버는 클라이언트가 보낸 방향을 그대로 신뢰하지 않고 현재 서버가 가진 Controller 시점으로 방향을 확정한다.
- Launch는 `LaunchCharacter` 기반으로 처리하고 이동 결과는 기존 CharacterMovement 복제를 사용한다.

## 결정 필요

- 없음. 기본 Launch 속도와 쿨다운은 Blueprint에서 조정 가능하게 둔다.

## 변경 기록

- 2026-08-29: 사용자 요청으로 눈사람 좌클릭 카메라 방향 Launch 계약 Task를 추가했다. 눈사람 Pawn 소유권은 K에 있어 입력 연결은 K 인계로 분리한다.
- 2026-08-29: 눈사람 이동 속도 값을 에디터에서 찾기 어렵다는 피드백을 반영해 GameMode와 Snowman Pawn의 이동 속도 튜닝 UPROPERTY를 `SnowRumble|Snowman|Tuning|Movement` 카테고리와 명시적 표시 이름으로 보강했다.
- 2026-08-29: 사용자 요청으로 눈사람 이동 속도 기본 배율을 2.0으로 변경했다. 이후 부츠 Blueprint 배율 1.5를 기준으로 부츠 스프린트 1125보다 조금 빠른 1150이 되도록 눈사람 기본 배율을 2.3으로 조정했다.
- 2026-08-30: 사용자 요청으로 눈사람 이동 속도 기본값을 장화 장착 스프린트 기준의 1.1배로 낮췄다. 현재 기준 스프린트 500, 장화 배율 1.7을 반영해 눈사람 기본 속도는 935, 기본 배율은 1.87이다.

## 수동 작업 (구현 후 구체화)

- `BP_SnowmanCharacter_K`가 `ActionAction`에 좌클릭 입력을 유지하고 있는지 확인한다. C++가 해당 액션을 눈사람 전용 Launch로 사용한다.
- `BP_SnowmanCharacter_K` 또는 공용 캐릭터 Blueprint에서 `DirectionalLaunchSpeed`와 `DirectionalLaunchCooldownSeconds`를 조정한다. 기본값은 각각 1200, 3초다.
- 눈사람 이동 속도는 기본값 기준 935다. `BP_SnowmanModeGameMode_K`에서 값이 보이지 않아도 C++ 기본값 `Normal Player Reference Walk Speed` 500과 `Snowman Movement Speed Multiplier` 1.87로 계산된다. 이는 장화 장착 스프린트 기준 850의 1.1배다.
- 눈사람 HUD의 기존 `UOverheadTimedActionWidget` WBP에 `TimedActionProgressBar`가 연결되어 있는지 확인한다. 별도 ProgressBar는 추가하지 않는다.
- 눈사람 모드에서 호스트 1명과 클라이언트 1명을 실행한 뒤 양쪽에서 좌클릭한다. Launch 방향은 각 플레이어의 현재 Controller 시점 기준이다.

## 완료 조건

### 에이전트 확인

- [x] C 서버 계약 구현 및 K 인계 완료
- [x] 정적 점검·빌드 확인 완료
- [x] 소유권·공용 계약·복제 규칙 위반 없음

### 결과 확인 (구현 후 구체화)

1. 눈사람 모드가 실제 시작된 뒤 눈사람이 좌클릭하면 카메라가 바라보는 방향으로 이동한다.
2. 호스트와 클라이언트에서 Launch 결과가 동일하게 보인다.
3. 시작 카운트다운·기절·모드 종료 중에는 Launch가 실행되지 않는다.
4. 쿨다운 중 좌클릭 연타로 과도한 Launch가 발생하지 않는다.
5. 쿨다운 동안 기존 눈 제작 ProgressBar가 표시되고 남은 쿨타임 비율을 시각화한다.

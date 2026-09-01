# Task C-38 - 플레이어 더블 점프

## 설명

일반 플레이어 캐릭터가 기본 점프 후 공중에서 한 번 더 점프할 수 있게 한다. 눈사람 전용 Pawn은 기존처럼 1회 점프만 허용한다.

## 상태 전이 기준

- 시작 가능: 사용자 구현 승인, 기존 점프 입력 흐름 확인
- 완료 가능: `ASnowRumbleCharacter` 기본 점프 횟수 변경, 문서 갱신, 수동 멀티플레이 확인

## 구현 항목

- [x] 공용 플레이어 캐릭터의 기본 점프 가능 횟수를 2회로 설정한다.
- [x] 눈사람 전용 Pawn은 점프 가능 횟수를 1회로 고정한다.
- [x] 기존 행동 제한 조건, 물 잠김, 눈덩이 굴리기 중 점프 제한은 유지한다.
- [x] 더블 점프 조작을 GDD 기본 조작에 반영한다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: 없음. 캐릭터 Blueprint 기본값 확인은 사용자/S 인계
- 생성·변경 후보: `Tasks/C/C-38_double_jump_movement.md`, `Source/SnowRumble/Player/SnowRumbleCharacter.cpp`, `Source/SnowRumble/Player/SnowmanModeSnowmanCharacter_K.cpp`, `docs/GDD/Game_GDD.md`, `Tasks/C/ROLE_C.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`
- 공유 확인 대상: K 눈사람 모드, S UI 키 가이드
- 병합 순서: C 공용 플레이어 기본값 변경 → 사용자/S가 필요 시 Blueprint 오버라이드 확인

## 공용 계약과 인계

- 제공받을 계약: 기존 `JumpAction`, `ASnowRumbleCharacter::StartJump()`, Unreal CharacterMovement 점프 카운트
- 제공할 계약: 일반 `ASnowRumbleCharacter` 기본 최대 점프 횟수 2회, 눈사람 전용 `ASnowmanModeSnowmanCharacter` 최대 점프 횟수 1회
- 인계 대상: 사용자/S, K

## 범위 밖

- 새 점프 애니메이션, VFX, 사운드 제작
- 공중 점프 전용 이동 보정, 스태미나, 쿨다운, 피해 판정
- 점프 UI 키 가이드 Widget 자산 수정

## 사전 전제

- 더블 점프는 Unreal `JumpMaxCount` 기본 기능을 사용한다.
- 눈사람 전용 Pawn은 상속된 기본값을 생성자에서 1로 되돌린다.
- 기존 `StartJump()`의 행동 가능 검사와 `StopJumping()` 흐름은 유지한다.

## 결정 필요

- 없음

## 변경 기록

- 2026-08-30: 사용자 요청과 승인으로 Task를 추가하고 구현했다. `ASnowRumbleCharacter` 생성자에서 `JumpMaxCount = 2`를 설정했다.
- 2026-08-30: 사용자 정정으로 눈사람은 더블 점프 대상에서 제외했다. `ASnowmanModeSnowmanCharacter` 생성자에서 `JumpMaxCount = 1`을 설정했다.

## 수동 작업

- `BP_SnowRumbleCharacter` 또는 상속 캐릭터 Blueprint의 Class Defaults에서 `Jump Max Count`가 1로 오버라이드되어 있으면 2로 재설정한다.
- 눈사람 Pawn Blueprint는 `Jump Max Count`를 1로 유지한다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드 변경 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료
- [x] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [x] 공용 계약과 캡슐화 규칙 위반 없음
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인

1. PvP 또는 테스트 맵에서 일반 플레이어가 `Space`를 누르면 1회 점프한다.
2. 착지 전 `Space`를 한 번 더 누르면 두 번째 점프가 실행된다.
3. 착지 전 세 번째 `Space` 입력으로는 추가 점프가 실행되지 않는다.
4. 물에 잠긴 상태, PvP 입력 잠금, 눈덩이 굴리기 중에는 기존처럼 점프가 실행되지 않는다.
5. 눈사람 Pawn은 공중에서 두 번째 `Space` 입력을 눌러도 추가 점프가 실행되지 않는다.
6. Listen Server 호스트와 클라이언트에서 일반 플레이어 더블 점프 이동과 눈사람 1회 점프 제한이 서로 보인다.

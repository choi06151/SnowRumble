# Task K-13 - 눈사람 이동과 감염

## 설명

눈사람으로 배정된 플레이어의 빠른 이동, 일반 플레이어와의 접촉 감염, 감염 대기 상태, 일정 시간 뒤 눈사람 전환을 구현한다.

## 상태 전이 기준

- 시작 가능: K-12 기반 완료, 기본 인간 플레이어 계약 확인
- 완료 가능: 호스트와 클라이언트에서 랜덤 눈사람 시작, 접촉 감염 대기, 눈사람 전환이 서버 권한으로 동일하게 보임

## 구현 항목

- [x] 서버가 참가자 중 무작위 한 명을 시작 눈사람으로 배정한다.
- [x] 눈사람은 일반 플레이어보다 빠른 이동 속도를 사용한다.
- [x] 눈사람과 일반 플레이어가 부딪히면 일반 플레이어가 유예 시간 없이 즉시 눈사람으로 전환된다.
- [x] 이전 감염 대기 기반 전환 경로는 `UpdateSnowmanInfectionFlow()`에서 사용하지 않는다.
- [x] 일반 플레이어가 던진 눈덩이가 눈사람에게 적용될 피격 결과는 K-13 범위 밖으로 보류하고 공용 눈 전투 계약 인계 요청으로 남긴다.

## 작업 배정

- 담당자: 강혜원(K)
- 기능 소유자: 강혜원(K)
- 계약 소유자: 눈사람 감염 상태는 강혜원(K), 공용 플레이어·눈 전투 계약은 최재원(C)
- 자산 수정자: 강혜원(K), 눈사람 모델·연출 자산은 사용자 또는 S 인계
- 생성·변경 후보: `Source/SnowRumble/Game/SnowmanModeGameMode_K.*`, `Source/SnowRumble/Game/SnowmanModeGameState_K.*`, `Source/SnowRumble/Player/SnowmanModeSnowmanCharacter_K.*`
- 공유 확인 대상: C, S, 사용자
- 병합 순서: K-12 후, K-14 전

## 공용 계약과 인계

- 제공받을 계약: 기본 인간 플레이어 이동·상태, C-09 눈덩이 피격 결과 또는 후속 공용 전투 계약
- 제공할 계약: 눈사람 여부, 눈사람 전용 Pawn 즉시 전환 결과. 감염 대기 여부와 남은 시간 조회 API는 기존 Blueprint 호환용으로 남아 있으나 현재 접촉 감염 흐름에서는 사용하지 않는다.
- 인계 대상: K-14, C 통합 검토, S UI·모델 표현

## 범위 밖

- 모드 진입과 제한시간 기반
- 최종 승패 UI와 결과 화면
- 치료제 아이템 구현

## 사전 전제

- 기본 인간 플레이어는 C가 개발한 공용 플레이어 기능을 사용한다.
- 눈사람 모드는 인간 플레이어의 공용 상태를 직접 덮어쓰지 않고 공개 계약을 사용한다.

## 결정 필요

- 접촉 후 감염 확정까지 걸리는 시간: 즉시 전환
- 눈사람 속도 배율: 일반 플레이어의 1.25배
- 눈덩이에 맞은 눈사람의 효과: K-13에서는 보류하고 C-09 및 공용 눈 전투 계약 이후 연결한다.
- 치료제 사용 여부와 해제 가능한 감염 단계: K-13에서는 구현하지 않는다.

## 이동 방식 검토

- K-13 구현 기준: 시작 눈사람과 감염 완료 플레이어는 서버가 눈사람 전용 Pawn으로 교체한다.
- 눈사람 전용 BP는 `ASnowmanModeSnowmanCharacter`를 부모로 만든다.
- `ASnowmanModeSnowmanCharacter`는 기존 캐릭터의 카메라, HUD, PlayerState 연동은 재사용하지만 입력 바인딩은 이동, 시야 회전, 점프만 허용한다.
- 눈사람은 눈덩이 제작, 줍기, 조준, 투척, 상호작용, 스프린트를 사용할 수 없고 공격은 접촉 감염만 사용한다.
- 실제 점프 기반 이동은 K-13에서는 구현하지 않는다. 강시처럼 점프하는 모습은 우선 눈사람 전용 BP/AnimBP에서 이동 애니메이션으로 표현한다.

## 변경 기록

- 2026-08-12: 최재원(C)의 분담 변경 요청에 따라 신규 작성.
- 2026-08-13: 감염 대기 10초, 눈사람 속도 1.25배, 치료제 없음, 눈덩이 피격 효과 보류로 확정했다.
- 2026-08-13: 눈사람은 일반 플레이어와 다른 BP/Pawn이어야 한다는 사용자 정정에 따라 K-13 구현 방향을 전용 `ASnowmanModeSnowmanCharacter` 기반 BP로 교체했다. 감염 확정 시 서버가 인간 Pawn을 눈사람 전용 Pawn으로 바꾸고 같은 PlayerController가 Possess한다.
- 2026-08-13: 실행 확인 중 시작 눈사람이 간헐적으로 배정되지 않거나 접촉 감염이 불안정한 문제를 반영했다. 시작 눈사람 전용 Pawn 전환이 실패하면 0.1초 뒤 재시도하고, 전환 성공 전에는 초기화 완료로 처리하지 않는다. 접촉 감염은 캡슐 반경을 포함한 2D 거리 판정으로 보강했다.
- 2026-08-13: 눈사람과 부딪혀도 감염되지 않는 문제를 추가 반영했다. 감염 판정 후보를 PlayerController 기반 Pawn 탐색에서 월드의 실제 `ASnowRumbleCharacter` Actor 스캔으로 바꿔 전용 Pawn 교체 직후에도 안정적으로 찾게 했다. 감염 대기 중인 플레이어는 `StartInfectionPendingFromServer()`에서 `Normal`이 아닐 경우 false를 반환하므로 중복 접촉으로 타이머가 덮어써지지 않는다.
- 2026-08-13: 접촉 감염이 간헐적으로 누락되는 문제를 추가 반영했다. 감염 스캔 시 Pawn의 `PlayerState`만 사용하지 않고 Controller의 `PlayerState`를 우선 사용해, 눈사람 전용 Pawn 교체 직후 PlayerState 연결 타이밍 차이로 후보 수집이 빠지는 경우를 줄였다.
- 2026-08-13: 접촉 감염 재현 확인을 위해 `LogSnowmanMode` 서버 로그를 추가했다. 1초마다 감염 후보 수를 출력하고, 접촉 반경 안에서는 눈사람/일반 플레이어, 거리, 유효 반경, 감염 시작 성공 여부를 출력한다.
- 2026-08-13: 실행 로그에서 접촉 거리 판정은 성공하지만 `Started=false`, `NormalRoleAfter=Normal`으로 남는 문제를 확인했다. 참가자 Entry 검색이 PlayerState 포인터 일치에만 의존해 전용 Pawn 교체/복제 타이밍에서 같은 플레이어를 못 찾을 수 있으므로 `PlayerId`와 `UniqueId` fallback 매칭을 추가했다.
- 2026-08-14: 결과 확인 중 감염 완료 후 `BP_SnowmanCharacter_K`로 전환될 때 캐릭터가 땅에 박혀 움직이지 못하는 경우를 반영했다. 전환 스폰 위치는 기존 인간 Pawn의 발 위치와 바닥 trace를 기준으로 새 눈사람 Capsule half height만큼 올리고, Spawn Collision Handling은 `AdjustIfPossibleButAlwaysSpawn`으로 보정한다.
- 2026-08-14: 사용자가 K-13 결과 확인을 완료했다. 테스트 횟수는 많지 않지만 감염 완료 후 `BP_SnowmanCharacter_K`가 땅에 박혀 생성되는 문제는 해결된 것으로 보인다고 확인했다. 실행 중 `BP_SnowmanCharacter_K`의 `CharacterMesh0` material slot 0 경고가 관찰됐지만, 감염·전환·이동을 막는 오류는 아니며 표현/머티리얼 정리는 후속 자산 다듬기 항목으로 남긴다.
- 2026-08-14: `master`에 K 브랜치를 병합한 뒤 UE unity build에서 `SnowmanModeGameMode_K.cpp`와 `SnowRumbleGameMode.cpp`의 익명 namespace helper `MakeRandomHorizontalOffset` 이름이 충돌해 컴파일이 실패하는 문제를 수정했다. K 소유 파일의 helper를 `MakeSnowmanModeRandomHorizontalOffset`으로 변경했으며, `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-26: 기획 변경에 따라 `ASnowmanModeGameMode::UpdateSnowmanInfectionFlow()`의 감염 Pending 완료 처리와 지연 전환 경로를 제거했다. 눈사람과 일반 플레이어가 접촉하면 서버가 컨트롤러, PlayerState, 캐릭터, CapsuleComponent, 역할, 전환 중 상태를 검사한 뒤 즉시 `BP_SnowmanCharacter_K`로 전환하고 GameState 역할을 Snowman으로 확정한다.

## 수동 작업 (구현 후 구체화)

- `ASnowmanModeSnowmanCharacter`를 부모로 `BP_SnowmanCharacter_K`를 만든다.
- `BP_SnowmanCharacter_K`에 눈사람 메시, 머티리얼, 강시형 이동 애니메이션, 필요한 VFX를 연결한다.
- `Content/Game/BP_SnowmanModeGameMode_K`에서 `Snowman Character Class`를 `BP_SnowmanCharacter_K`로 지정한다.
- `BP_SnowmanCharacter_K`는 이동, 시야 회전, 점프 입력만 사용한다. 눈덩이 제작, 줍기, 조준, 투척, 상호작용, 스프린트 입력은 C++ 부모 클래스에서 바인딩하지 않는다.
- 현재 접촉 감염은 Pending UI/VFX를 사용하지 않는다. 기존 `ASnowmanModeGameState::IsSnowmanModePlayerInfectionPending`와 `GetSnowmanModeInfectionRemainingSeconds`는 Blueprint 호환용으로만 남긴다.
- 실제 점프 기반 이동은 K-13에서는 구현하지 않는다. 강시형 이동은 우선 눈사람 전용 BP/AnimBP에서 애니메이션으로 표현하고, 실제 입력/Movement 변경은 C 공용 플레이어 계약 검토 후 별도 Task로 분리한다.

## 완료 조건

### 에이전트 확인

- [x] 랜덤 눈사람 배정 서버 권한 코드 확인
- [x] 눈사람 이동 속도 보정 코드 확인
- [x] 접촉 즉시 눈사람 전용 Pawn 전환 코드 확인
- [x] 눈덩이 피격 계약 보류와 통합 요청 기록 확인
- [x] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [x] 공용 계약과 캡슐화 규칙 위반 없음
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨
- [x] `master` 병합 후 `SnowRumbleEditor Win64 Development` 빌드 성공 확인

### 결과 확인 (구현 후 구체화)

- [x] Listen Server 2인 이상에서 시작 후 무작위 1명이 `BP_SnowmanCharacter_K`로 배정되는지 확인한다.
- [x] 여러 번 재시작해도 매번 정확히 1명이 `BP_SnowmanCharacter_K`로 배정되는지 확인한다.
- [x] 눈사람 플레이어가 일반 플레이어보다 빠른 속도로 이동하는지 확인한다.
- [x] 눈사람 플레이어가 눈덩이 제작, 줍기, 조준, 투척, 상호작용, 스프린트를 사용할 수 없는지 확인한다.
- [ ] 눈사람이 일반 플레이어와 접촉하면 감염 대기 없이 즉시 `BP_SnowmanCharacter_K`로 전환되는지 확인한다.
- [x] 눈사람이 일반 플레이어와 붙어서 움직일 때 Z축 차이나 캡슐 위치 때문에 감염이 누락되지 않는지 확인한다.
- [ ] 접촉 직후 해당 플레이어가 서버와 클라이언트 양쪽에서 `BP_SnowmanCharacter_K`로 보이는지 확인한다.
- [x] 감염 완료 후 `BP_SnowmanCharacter_K`가 평지와 경사에서 땅에 박히지 않고 즉시 이동 가능한지 확인한다.
- [x] 클라이언트에서도 눈사람/감염 대기 상태가 동일하게 보이는지 확인한다.

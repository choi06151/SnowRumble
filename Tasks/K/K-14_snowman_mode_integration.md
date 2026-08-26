# Task K-14 - 눈사람 모드 통합

## 설명

눈사람 모드의 10분 제한시간, 전원 감염 승리, 생존자 승리, 결과 인계와 로비 복귀 조건을 통합한다.

## 상태 전이 기준

- 시작 가능: K-12, K-13 완료와 결과 UI·로딩 인계 조건 확인
- 완료 가능: 눈사람과 일반 플레이어 양쪽 승리 조건이 호스트와 클라이언트에서 동일하게 확정되고 결과 흐름이 인계됨

## 구현 항목

- [x] 모든 일반 플레이어가 눈사람으로 감염되면 눈사람 승리를 확정한다.
- [x] 10분 제한시간 종료 시 일반 플레이어가 한 명이라도 생존해 있으면 일반 플레이어 승리를 확정한다.
- [x] 라운드 종료 후 조작 제한, 결과 상태 복제, 로비 복귀 또는 다음 흐름을 공용 계약에 맞춰 연결한다.
- [x] 눈사람 모드에서 필요한 HUD, 안내 문구, 결과 표시 정보를 UI 담당자가 읽을 수 있게 인계한다.
- [x] 결과 후 로비 복귀 시 이전 눈사람 모드 `?game=` travel 옵션이 남아 로비에서 다시 3-2-1 카운트다운이 뜨지 않도록, 로비 GameMode를 명시한 absolute server travel로 복귀한다.
- [x] 초기 지정 역할이 눈사람인 플레이어는 초기 스폰/RestartPlayer 경로에서도 `SnowmanCharacterClass`로 스폰되게 한다.
- [x] 컨트롤러를 잃은 감염 대기 참가자는 참가자 Entry를 유지한 채 Pending 상태만 1회 해제하고 반복 경고 로그를 남기지 않게 한다.
- [x] 기존 PvP 모드의 PlayerStart 랜덤 선택, 주변 분산, 바닥 보정, 캡슐 충돌 검사를 눈사람 모드 스폰 경로에도 적용한다.
- [x] 눈사람 모드에서는 Normal 플레이어끼리 눈덩이로 HP 피해를 주지 않게 하고, Normal이 Snowman을 맞추면 10초 기절을 적용하며 체력바 HUD를 숨긴다.
- [x] 눈사람이 기절 중 다시 눈덩이에 맞으면 기존 기절 타이머를 초기화하고 새 10초 기절로 갱신한다.
- [x] 눈사람 기절 상태와 남은 시간을 UI가 읽을 수 있도록 BlueprintPure 조회 함수를 제공한다.
- [x] 눈사람 모드 종료 후 포디움 결과 override 없이 결과 표시 시간 뒤 로비로 복귀한다.
- [x] 눈사람 Pawn 전환 중 컨트롤러 링크가 비는 시간을 줄이고, 스폰/전환 직후 접촉 감염 면역 시간을 적용한다.
- [x] 눈사람과 일반 플레이어 접촉 시 감염 Pending 없이 즉시 눈사람 Pawn으로 전환되도록 감염 흐름을 갱신한다.

## 작업 배정

- 담당자: 강혜원(K)
- 기능 소유자: 강혜원(K)
- 계약 소유자: 눈사람 모드 승패는 강혜원(K), 공용 결과·로비 복귀 계약은 최재원(C)
- 자산 수정자: 강혜원(K), 결과 UI·연출 자산은 사용자 또는 S 인계
- 생성·변경 후보: 구현 승인 전 확정
- 공유 확인 대상: C, S, 사용자
- 병합 순서: K-12, K-13 후, C 통합 검토 전

## 공용 계약과 인계

- 제공받을 계약: K-12 제한시간, K-13 눈사람·감염 상태, C 공용 결과·로비 복귀 흐름
- 제공할 계약: 눈사람 승리 팀/진영, 일반 플레이어 생존 승리, 결과 표시용 상태, `ASnowmanModeSnowmanCharacter::IsSnowballHitStunned()`, `GetSnowballHitStunSecondsRemaining()`
- 인계 대상: C 통합 검토, S UI·연출, 사용자 결과 확인

## 범위 밖

- 눈사람 이동과 감염 판정 신규 구현
- 새 눈사람 전용 맵 제작
- PvP 맵 환경 축소 기믹 수정

## 사전 전제

- 눈사람 모드는 기존 PvP 맵을 사용하며 환경 축소는 비활성화한다.
- 기본 인간 플레이어는 C가 개발한 공용 플레이어 구현을 사용한다.

## 결정 필요

- 결과 후 자동 로비 복귀 시간: 결과 표시 5초 후 자동 로비 복귀
- 눈사람 모드 결과 화면 문구: 눈사람 승리 `눈사람 승리! 모두 눈사람이 되었습니다`, 생존자 승리 `생존자 승리! 제한시간을 버텼습니다`
- 제한시간 종료 직전에 감염 대기 중인 플레이어 판정: 아직 10초가 끝나지 않았으면 생존자로 판정한다.

## 변경 기록

- 2026-08-12: 최재원(C)의 분담 변경 요청에 따라 신규 작성.
- 2026-08-14: K-14 시작 전 결과 후 5초 자동 로비 복귀, 눈사람/생존자 결과 문구, 제한시간 종료 순간 감염 대기 플레이어는 생존자로 판정하는 기준을 확정했다.
- 2026-08-19: 결과 후 로비 복귀 시 로비 맵이 눈사람 GameMode처럼 다시 시작되어 3-2-1 표시가 뜨는 문제를 반영했다. `ASnowmanModeGameMode`의 로비 복귀 URL은 `ASnowRumbleLobbyGameMode`를 명시하고 absolute `ServerTravel`을 사용해 이전 travel 옵션을 끊는다.
- 2026-08-21: 초기 역할이 `Snowman`인 플레이어가 초기 스폰/RestartPlayer 경로에서 일반 인간 Pawn으로 생성되는 문제를 반영했다. `ASnowmanModeGameMode::GetDefaultPawnClassForController_Implementation`이 GameState 역할을 확인해 Snowman이면 `SnowmanCharacterClass`를 반환한다.
- 2026-08-21: 컨트롤러가 없는 감염 대기 플레이어의 `Infection pending cleanup: controller is missing` 경고가 반복되는 문제를 반영했다. 처음에는 Pending Entry 제거로 처리했으나 참가자 수가 줄어드는 문제가 있어, `ASnowmanModeGameState::CancelControllerlessPendingFromServer`로 참가자 Entry를 유지한 채 Pending 상태만 해제하게 고쳤다.
- 2026-08-21: 기존 PvP 모드의 PlayerStart 랜덤 스폰 보정 로직을 눈사람 모드에 맞춰 이식했다. `ChoosePlayerStart_Implementation`의 PlayerStart 랜덤 선택은 유지하고, `BuildScatteredPlayerStartTransform`에 바닥 trace, 캡슐 점유 검사, 분산 위치 fallback을 추가했다. 캡슐 검사는 초기 역할 분기에 맞춰 기본 인간 Pawn과 `SnowmanCharacterClass` 중 더 큰 캡슐 크기를 기준으로 한다.
- 2026-08-21: 눈사람 모드 전용 눈덩이 피격 규칙을 반영했다. `ASnowRumbleCharacter::TakeDamage`는 Snowman GameState가 있는 월드에서 Normal이 Normal을 눈덩이로 맞추면 HP 피해를 0으로 처리하고, Normal이 Snowman을 맞추면 HP 피해 대신 `ASnowmanModeSnowmanCharacter::ApplySnowballHitStunFromServer`로 이동 정지를 적용한다. `UMainHUDWidget`은 눈사람 모드 중 로컬/타 플레이어 체력바를 숨긴다. PvP 모드에서는 기존 대미지와 체력바 경로를 유지한다.
- 2026-08-21: 눈사람 모드 결과 후 로비로 바로 돌아가던 흐름을 기존 PvP와 같은 포디움 이동 흐름으로 바꿨다. `ASnowmanModeGameMode`는 종료 시 `USnowRumbleMatchSubsystem::SetPodiumOverrideResults`에 눈사람/생존자 승리, 참가자/눈사람/생존자 수, 생존자 이름을 저장하고 `/Game/Maps/L_Podium?listen`으로 이동한다. `APodiumGameMode`는 override 결과가 있으면 기존 PvP 팀 승수 텍스트 대신 눈사람 결과 텍스트를 위젯에 전달한다.
- 2026-08-21: 눈사람 Pawn 전환 안정성과 시작 직후 감염 문제를 반영했다. `ConvertPlayerToSnowmanPawn`은 기존 Pawn을 먼저 `UnPossess`하지 않고 새 눈사람 Pawn을 스폰한 뒤 `Possess`, `SetViewTarget`, `ClientRestart`, `ForceNetUpdate`를 순서대로 호출하고 성공 후 기존 Pawn을 제거한다. 또한 `SpawnInfectionGraceSeconds` 기본 3초를 추가해 초기 스폰, 경기 시작 직후, 눈사람 전환 직후에는 접촉 감염 시작을 무시한다.
- 2026-08-26: 사용자 요청에 따라 눈사람 모드 결과를 포디움에 덮어쓰던 override 흐름을 제거했다. `ASnowmanModeGameMode`는 결과 표시 시간 뒤 다시 로비로 복귀하고, `USnowRumbleMatchSubsystem`의 포디움 override API와 `APodiumGameMode`의 override 분기는 삭제해 일반 PvP 포디움은 팀 승수 기반 기본 동작만 사용한다.
- 2026-08-26: 사용자 요청에 따라 `ASnowmanModeGameMode::UpdateSnowmanInfectionFlow()`를 즉시 감염 전환 기준으로 수정했다. 아이템 생성이나 감염 Pending 처리는 함수에서 사용하지 않고, 서버가 접촉 판정 후 중복 전환·이미 눈사람·컨트롤러 없음·Pawn 교체 중·CapsuleComponent 없음 상태를 건너뛴 뒤 `ConvertPlayerToSnowmanPawn()`과 `SetSnowmanPlayerFromServer()`를 순서대로 실행한다.
- 2026-08-26: 사용자 요청에 따라 눈사람 모드 눈덩이 피격 기절을 10초로 확정했다. `ASnowmanModeSnowmanCharacter::ApplySnowballHitStunFromServer()`는 서버에서 기존 `SnowballHitStunTimerHandle`을 지운 뒤 새 타이머를 설정하고, `ApplySnowballHitStunMovementState()`는 기절 중 `StopMovementImmediately()`, `DisableMovement()`, `StopJumping()`으로 이동과 점프를 막는다. 기절 해제 시 다른 행동 잠금이 없으면 `MOVE_Walking`으로 복구한다.
- 2026-08-26: 눈사람 기절 UI 연동용 조회 함수를 추가했다. `IsSnowballHitStunned()`는 현재 기절 여부를 반환하고, `GetSnowballHitStunSecondsRemaining()`은 복제된 `SnowballHitStunEndServerTime`과 `GameState` 서버 시각 기준으로 남은 초를 계산해 반환한다.

## 수동 작업

- 별도 에디터 자산 수정은 없다.
- 기절 표시 WBP를 만들거나 연결할 때 눈사람 Pawn을 `ASnowmanModeSnowmanCharacter`로 캐스팅한 뒤 `IsSnowballHitStunned()`가 true이면 `GetSnowballHitStunSecondsRemaining()` 값을 반올림 또는 올림 처리해 `기절 중! 남은 시간: X초` 형식으로 표시한다.
- `BP_SnowmanModeGameMode_K` 같은 눈사람 모드 GameMode Blueprint가 있다면 `LobbyReturnTravelUrl`이 `/Game/Maps/L_Lobby?listen` 또는 로비 맵 경로를 가리키는지 확인한다. `LobbyReturnGameModeClass`는 기본값 `SnowRumbleLobbyGameMode`를 그대로 사용한다.
- `BP_SnowmanModeGameMode_K`에 이전 `PodiumTravelUrl` 노출값이 보이면 컴파일/저장으로 제거된 C++ 프로퍼티를 정리한다.
- `BP_SnowmanModeGameMode_K`의 `SnowmanCharacterClass`가 실제 눈사람 전용 Character Blueprint로 지정되어 있는지 확인한다.

## 완료 조건

### 에이전트 확인

- [x] 눈사람 전원 감염 승리 조건 코드 경로 확인
- [x] 일반 플레이어 제한시간 생존 승리 조건 코드 경로 확인
- [x] 결과 상태 복제와 UI 인계 API 확인
- [x] 로비 복귀가 로비 GameMode 명시 absolute travel로 구성되는지 확인
- [x] 눈사람 모드 포디움 override 저장·조회 코드 제거 확인
- [x] 눈사람 접촉 즉시 전환 코드 경로 확인
- [x] 눈사람 눈덩이 피격 10초 기절과 연속 피격 타이머 리셋 코드 경로 확인
- [x] 눈사람 기절 상태와 남은 시간 BlueprintPure 조회 함수 확인
- [x] 초기 역할 Snowman 스폰 클래스 분기 코드 경로 확인
- [x] PvP PlayerStart 랜덤 스폰 보정 로직의 눈사람 모드 적용 경로 확인
- [x] 컨트롤러 소실 Pending 상태 해제와 반복 로그 방지 코드 경로 확인
- [x] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [x] 공용 계약과 캡슐화 규칙 위반 없음
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인

- Listen Server 호스트와 클라이언트 1명 이상으로 눈사람 모드를 시작한다.
- 모든 일반 플레이어가 눈사람으로 감염되면 `눈사람 승리! 모두 눈사람이 되었습니다` 결과 상태가 호스트와 클라이언트에 동일하게 보이는지 확인한다.
- 결과 표시 약 5초 뒤 `/Game/Maps/L_Podium`으로 이동하지 않고 `/Game/Maps/L_Lobby`로 돌아가는지 확인한다.
- 일반 PvP 매치 종료 경로에서는 기존처럼 `/Game/Maps/L_Podium`으로 이동하고, 포디움 UI가 팀 승수 기반 기본 결과를 표시하는지 확인한다.
- 최종 로비 복귀 후 3-2-1 카운트다운이나 눈사람 모드 HUD가 다시 표시되지 않고, 로비 UI와 로비 대기 상태가 보이는지 확인한다.
- 로비에서 사용자가 다시 시작 버튼 또는 게시판 시작 액션을 누르기 전까지 새 게임이 진행되지 않는지 확인한다.
- 제한시간 종료 경로에서는 아직 눈사람으로 즉시 전환되지 않은 일반 플레이어가 생존자로 취급되어 `생존자 승리! 제한시간을 버텼습니다` 결과가 확정되는지 확인한다.
- 초기 눈사람으로 지정된 플레이어가 게임 시작 또는 RestartPlayer 경로에서 일반 인간 BP가 아니라 `SnowmanCharacterClass`에 지정된 눈사람 BP로 스폰되는지 확인한다.
- 눈사람 모드 시작 시 플레이어들이 0,0,0이나 고정 위치가 아니라 맵의 PlayerStart 후보 중 랜덤으로 선택된 위치 주변에 분산 스폰되는지 확인한다.
- 감염 대기 중 컨트롤러가 소실된 플레이어가 있을 때 `Infection pending cleanup: controller is missing` 경고가 1회만 남고 이후 틱/스캔마다 반복되지 않는지 확인한다.
- 위 상황 이후에도 `Infection scan summary`의 `Entries` 값이 정상 참가자 수에서 줄어들지 않는지 확인한다.
- 눈사람 모드에서 Normal 플레이어가 다른 Normal 플레이어를 눈덩이로 맞춰도 HP가 감소하지 않고, Normal이 Snowman을 맞추면 눈사람이 10초 동안 이동과 점프를 할 수 없으며, PvP 모드에서는 기존 눈덩이 HP 피해가 유지되는지 확인한다.
- 눈사람이 기절 중 두 번째 눈덩이를 맞으면 첫 번째 기절 타이머 기준으로 풀리지 않고, 두 번째 피격 시점부터 다시 10초 뒤에 이동과 점프가 복구되는지 확인한다.
- 호스트와 클라이언트 화면에서 눈사람 기절 UI가 `IsSnowballHitStunned()`와 `GetSnowballHitStunSecondsRemaining()` 기준으로 표시되고, 남은 시간이 두 화면에서 같은 서버 시각 기준으로 감소하는지 확인한다.
- 눈사람 모드 HUD에서 로컬/다른 플레이어 체력바가 보이지 않고, 머리 위 닉네임은 `WBP_OverheadNamePlate`로 표시되는지 확인한다.
- Normal에서 `BP_SnowmanCharacter_K`로 전환될 때 컨트롤러 소유, 카메라, HUD/위젯 바인딩이 끊기지 않는지 확인한다.
- 게임 시작 직후와 눈사람 전환 직후 약 3초 동안 가까운 충돌이나 낙하 접촉만으로 즉시 전환되지 않고, 면역 시간이 지난 뒤 접촉하면 Pending 없이 바로 눈사람으로 전환되는지 확인한다.

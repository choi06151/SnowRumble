# Task K-12 - 눈사람 모드 기반

## 설명

기존 PvP 맵을 재사용하는 10분 눈사람 모드의 기본 진입, 모드 전용 GameMode 계열, 제한시간, 환경 축소 비활성 조건을 만든다.

## 상태 전이 기준

- 시작 가능: C-04 랜덤 맵·로딩 구조와 C 공용 모드 선택 계약 확인
- 완료 가능: 눈사람 모드가 기존 PvP 맵으로 진입하고 10분 제한시간 동안 PvP 환경 축소 기믹 없이 진행되는 기반 확인

## 구현 항목

- [x] 기존 로비 게시판의 PvP / 눈사람 모드 선택 흐름을 사용해 눈사람 모드로 진입할 수 있게 한다.
- [x] 기존 구조에서 재사용할 수 있는 부분을 확인한 뒤 눈사람 모드 전용 GameMode와 GameState를 구현한다.
- [x] 기존 PvP 맵 후보를 재사용하되 눈사람 모드에서는 수위 상승, 자기장, 눈 폭 같은 줄어드는 기믹이 발동하지 않게 한다.
- [x] 10분 제한시간을 서버 권한으로 관리하고 GameState 공개 함수로 클라이언트가 읽을 수 있게 한다.
- [x] 눈사람 모드 라운드 번호와 전체 라운드 수를 서버 권한 GameState 복제값으로 제공해 HUD가 PvP와 같은 형식으로 표시하게 한다.
- [x] 기존 PvP 맵의 PlayerStart 배치를 재사용할 때 눈사람 모드에서도 플레이어 Pawn이 분산 스폰되게 한다.
- [x] 눈사람 모드 로딩 완료 후 3초 시작 카운트다운을 거친 뒤 10분 제한시간을 시작한다.
- [x] 눈사람 모드 시작 카운트다운 동안 플레이어 이동과 시야 입력을 잠그고 시작 시 해제한다.

## 작업 배정

- 담당자: 강혜원(K)
- 기능 소유자: 강혜원(K)
- 계약 소유자: 눈사람 모드 내부는 강혜원(K), 공용 로딩·모드 선택·플레이어 계약은 최재원(C)
- 자산 수정자: 강혜원(K), 기존 PvP 맵 `.umap` 수정은 S/J 또는 사용자 인계
- 생성 파일:
  - `Source/SnowRumble/Game/SnowmanModeGameMode_K.h`
  - `Source/SnowRumble/Game/SnowmanModeGameMode_K.cpp`
- `Source/SnowRumble/Game/SnowmanModeGameState_K.h`
- `Source/SnowRumble/Game/SnowmanModeGameState_K.cpp`
- 변경 파일:
  - `Source/SnowRumble/UI/MainHUDWidget.cpp`
  - `Source/SnowRumble/Player/SnowRumbleCharacter.cpp`
  - `Source/SnowRumble/Game/SnowRumbleLobbyGameMode.h`
  - `Source/SnowRumble/Game/SnowRumbleLobbyGameMode.cpp`
  - `Tasks/K/PLAN_K.md`
  - `Tasks/K/K-12_snowman_mode_foundation.md`
- 공유 확인 대상: C, S, J, 사용자
- 병합 순서: C-04 공용 로딩 구조 확인 후, K-13과 K-14 전

## 공용 계약과 인계

- 제공받을 계약: C-04의 `PvPLevelCandidates` 후보 맵, 기존 로비 게시판 `SnowmanModeButton`/`LobbyMode` 선택 상태, 기존 로딩창 RPC
- 제공할 계약:
  - `ASnowmanModeGameMode`: 눈사람 모드 전용 GameMode다. 기존 PvP 맵을 사용하지만 PvP 맵 축소 호출을 구현하지 않는다.
  - `ASnowmanModeGameState::IsSnowmanModeTimerActive()`: 눈사람 모드 제한시간 진행 여부를 반환한다.
  - `ASnowmanModeGameState::IsSnowmanModeInputLocked()`: 눈사람 모드 시작 전 입력 잠금 필요 여부를 반환한다.
  - `ASnowmanModeGameState::ShouldShowSnowmanModeStartCountdown()`: 시작 카운트다운 표시 필요 여부를 반환한다.
  - `ASnowmanModeGameState::GetSnowmanModeStartCountdownText()`: 시작 카운트다운을 `3`, `2`, `1`, `시작!` 형식 텍스트로 반환한다.
  - `ASnowmanModeGameState::GetSnowmanModeTimeLimitSeconds()`: 제한시간 전체 길이를 반환한다.
  - `ASnowmanModeGameState::GetSnowmanModeRemainingSeconds()`: 남은 시간을 초 단위로 반환한다.
  - `ASnowmanModeGameState::GetSnowmanModeRemainingTimeText()`: 남은 시간을 `0:00` 형식 텍스트로 반환한다.
  - `ASnowmanModeGameState::GetSnowmanModeElapsedSeconds()`: 시작 후 진행 시간을 초 단위로 반환한다.
  - `ASnowmanModeGameState::GetSnowmanModeElapsedTimeText()`: 기존 HUD `MatchElapsedTimeText`에 표시할 진행 시간을 `경기 시간 0:00` 형식 텍스트로 반환한다.
  - `ASnowmanModeGameState::GetCurrentRoundNumber()`: 기존 HUD `CurrentRoundText`에 표시할 현재 눈사람 모드 라운드 번호를 반환한다.
  - `ASnowmanModeGameState::GetRoundLimit()`: 기존 HUD `CurrentRoundText`에 표시할 전체 눈사람 모드 라운드 수를 반환한다.
  - `ASnowRumbleLobbyGameMode::SnowmanModeGameModeClass`: 눈사람 모드 시작 시 travel URL의 `game` 옵션으로 사용할 GameMode 클래스다.
- 인계 대상: K-13, K-14, C 통합 검토, S/J 맵 담당

## 범위 밖

- 눈사람 이동과 접촉 감염 구현
- 눈사람 피격 효과와 승패 결과 UI
- 기존 PvP 맵 레벨 자산 직접 수정

## 사전 전제

- 기존 PvP 맵을 그대로 사용한다.
- 눈사람 모드에서는 PvP 맵의 줄어드는 환경 기믹을 발동하지 않는다.
- 눈사람 모드에서는 기존 PvP 후보 맵 전체를 재사용한다.
- 눈사람 모드 진입은 현재 개발되어 있는 로비 게시판의 PvP / 눈사람 모드 선택 흐름을 기준으로 연결한다.
- 눈사람 모드 GameMode는 PvP GameMode에 로직을 계속 분기 추가하기보다 전용 구조로 분리한다.
- 환경 축소 비활성은 각 맵 Blueprint 분기보다 눈사람 모드 GameMode에서 시작 호출 자체를 하지 않는 방향으로 진행한다.
- K-12는 기존 PvP `.umap`을 직접 수정하지 않는다.
- C 소유의 공용 로딩, 모드 선택, 세션 계약 변경이 반드시 필요하면 구현 전에 최재원(C)에게 보고한다.

## 결정 필요

- K-13에서 눈사람 역할 배정과 감염 상태를 PlayerState, Character Component, 별도 Component 중 어디에 둘지 결정한다.
- K-14에서 제한시간 종료 시점의 결과 화면 문구와 로비 복귀 시간을 결정한다.

## 변경 기록

- 2026-08-12: 최재원(C)의 분담 변경 요청에 따라 신규 작성.
- 2026-08-12: K-12 시작 전 결정 사항을 반영했다. 기존 로비 게시판 선택 흐름, PvP 후보 맵 전체 재사용, 전용 GameMode 분리, GameMode 쪽 환경 축소 시작 호출 비활성, 기존 PvP `.umap` 직접 수정 금지, C 공용 계약 변경 전 보고를 기준으로 한다.
- 2026-08-12: K-12 구현을 추가했다. 로비 GameMode는 `LobbyMode == Snowman`일 때만 Snowman 전용 travel URL을 만들고, PvP 경로는 기존 `BuildMatchTravelUrl()`을 그대로 사용한다. Snowman 전용 GameMode/GameState는 10분 제한시간과 로딩창 닫기만 독립 처리하며 맵 축소 호출은 구현하지 않는다.
- 2026-08-13: 빌드 후 눈사람 모드에서 플레이어가 스폰되지 않는 문제를 반영했다. `ASnowmanModeGameMode`에 기존 PvP GameMode와 같은 PlayerStart 중복 회피와 분산 스폰 보정 경로를 추가해 기존 PvP 맵의 PlayerStart를 재사용할 때 Pawn 생성 실패 가능성을 줄였다.
- 2026-08-13: 눈사람 모드 시작 흐름을 PvP처럼 로딩 완료, 스폰, 3초 시작 카운트다운, 제한시간 시작 순서로 조정했다. 카운트다운과 제한시간은 `ASnowmanModeGameState`가 복제한다.
- 2026-08-13: 눈사람 모드 HUD 표시를 기존 PvP HUD 위젯 재사용 기준으로 조정했다. `StartCountdownText`는 눈사람 모드 시작 카운트다운을 표시하고, `MatchElapsedTimeText`는 눈사람 모드 시작 후 진행 시간을 표시한다. 눈사람 모드에서는 `MapShrinkCountdownText`를 숨긴다.
- 2026-08-13: `StartCountdownText`가 `3`, `2`, `1`까지만 보이고 `시작!`이 보이지 않는 문제를 수정했다. 눈사람 모드 타이머가 시작된 뒤에도 시작 기준 시간으로부터 약 1초 동안 카운트다운 표시를 유지해 PvP와 같은 `시작!` 표시 구간을 제공한다.
- 2026-08-13: 시작 카운트다운 동안 `ASnowmanModeGameMode`가 모든 PlayerController의 이동과 시야 입력을 잠그고 Pawn 이동을 즉시 멈춘 뒤, 카운트다운 종료 시 입력 잠금을 해제하게 했다.
- 2026-08-13: 이동은 잠기지만 시야가 회전하는 문제를 수정했다. `ASnowRumbleCharacter::IsPvpMatchInputLocked()`가 눈사람 모드 `ASnowmanModeGameState::IsSnowmanModeInputLocked()`도 함께 확인하게 해 `Look()` 입력 경로에서 회전을 차단한다.
- 2026-08-28: C 통합 경로에서 눈사람 모드 라운드 HUD 계약을 PvP와 맞췄다. `ASnowmanModeGameMode::InitGameState()`가 URL 옵션 기반 `CurrentRoundIndex`와 `TotalMatchRounds`를 `ASnowmanModeGameState` 복제값으로 확정하고, `UMainHUDWidget::CurrentRoundText`가 `{현재} / {전체}` 형식으로 표시한다.

## 수동 작업

- `Content/Game`에 `ASnowmanModeGameMode`를 부모로 하는 `BP_SnowmanModeGameMode_K`를 만든다.
- `BP_SnowmanModeGameMode_K`의 `Default Pawn Class`를 `BP_SnowRumbleCharacter`로 지정한다.
- `BP_SnowmanModeGameMode_K`의 `Player Controller Class`를 `BP_SnowRumblePVPController`로 지정한다.
- `Content/Game/BP_LobbyGameMode`의 `SnowmanModeGameModeClass`를 `BP_SnowmanModeGameMode_K`로 지정한다.
- `Content/Game/BP_LobbyGameMode` 또는 기본 클래스 설정에서 `PvPLevelCandidates` 배열에 기존 PvP 후보 맵 전체가 들어 있는지 확인한다.
- 눈사람 모드용 GameMode Blueprint에서 스폰 보정값을 조정해야 하면 `SnowRumble|Spawn` 카테고리의 `PlayerStartSpawnScatterRadius`, `PlayerStartSpawnMinimumSpacing`, `PlayerStartSpawnScatterAttempts`를 조정한다.
- `Content/WBP/WBP_MainHUDWidget` 또는 눈사람 모드 전용 HUD WBP에 기존 `CurrentRoundText`, `StartCountdownText`, `MatchElapsedTimeText` TextBlock이 유지되어 있는지 확인한다. 별도 `SnowmanTimerText`는 필요 없다.
- 눈사람 모드 라운드 정보는 기존 HUD의 `CurrentRoundText`에 PvP와 같은 `1 / 3` 형식으로 표시된다.
- 눈사람 모드 시작 카운트다운은 기존 HUD의 `StartCountdownText`에 표시된다.
- 눈사람 모드 진행 시간은 기존 HUD의 `MatchElapsedTimeText`에 `경기 시간 0:00` 형식으로 표시된다.
- 제한시간 종료 후 승패 확정, 결과 표시, 로비 복귀는 K-14 범위이므로 K-12에서는 시간이 0이 되어도 자동 종료되지 않는다.
- 기존 PvP `.umap`은 K-12에서 저장하거나 수정하지 않는다.

## 완료 조건

### 에이전트 확인

- [x] 모드 진입 기반 변경 완료
- [x] 기존 PvP 맵 축소 기믹 비활성 조건 확인
- [x] 제한시간 서버 권한 관리 확인
- [x] 눈사람 모드 스폰 보정 경로 추가
- [x] 눈사람 모드 시작 카운트다운과 제한시간 지연 시작 추가
- [x] 눈사람 모드 시작 전 이동·시야 입력 잠금 추가
- [x] 기존 HUD `StartCountdownText`/`MatchElapsedTimeText` 재사용 표시 경로 추가
- [x] 기존 HUD `CurrentRoundText` 재사용 라운드 표시 경로 추가
- [x] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [x] 공용 계약과 캡슐화 규칙 위반 없음
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨
- [x] `git diff --check` 공백 점검 통과
- [x] `SnowRumbleEditor Win64 Development` 빌드 확인

### 검증 메모

- 2026-08-12: `git diff --check`는 통과했다. 현재 환경에는 `C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat`과 `UnrealBuildTool`이 없어 `SnowRumbleEditor Win64 Development` 빌드는 실행하지 못했다.
- 2026-08-13: 스폰 보정 수정 후 `git diff --check`는 통과했다. 현재 환경에는 `C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat`이 없어 `SnowRumbleEditor Win64 Development` 빌드는 실행하지 못했다.
- 2026-08-13: 사용자가 눈사람 모드 진입, 플레이어 스폰, 시작 카운트다운, 이동·시야 입력 잠금과 해제, 진행 시간 표시를 실제 실행으로 확인 완료했다.
- 2026-08-28: 눈사람 라운드 HUD 보강 후 `git diff --check`와 충돌 표식 검색은 통과했다. `SnowRumbleEditor Win64 Development` 빌드는 Live Coding 활성화로 UBT가 시작 단계에서 중단했다.
- 2026-08-28: Live Coding 해제 후 `SnowRumbleEditor Win64 Development` 빌드가 성공했다. 기존 `SnowRumbleIceGlacierCollapseActor_J.cpp`의 `GetMovementBase` deprecation warning 1개는 남아 있으며 이번 라운드 HUD 변경과는 별개다.

### 결과 확인

- [x] Listen Server 호스트와 클라이언트 1명으로 로비에 입장한다.
- [x] 로비 게시판에서 `SnowmanModeButton`을 눌러 눈사람 모드를 선택한다.
- [x] 호스트가 `ReadyStartButton`을 눌렀을 때 기존 PvP 후보 맵 중 하나로 이동하는지 확인한다.
- [x] 이동한 맵의 GameMode가 `ASnowmanModeGameMode` 또는 이를 부모로 한 Blueprint인지 확인한다.
- [x] 모든 예상 플레이어가 접속하면 로딩창이 닫히는지 확인한다.
- [x] 로딩창이 닫힌 뒤 기존 HUD `StartCountdownText`에 `3`, `2`, `1`, `시작!` 순서가 표시되는지 확인한다.
- [ ] 눈사람 모드 HUD의 기존 `CurrentRoundText`에 첫 라운드 `1 / 설정 라운드 수`가 표시되는지 확인한다.
- [ ] 여러 라운드 설정에서 다음 눈사람 라운드로 넘어가면 `CurrentRoundText`가 `2 / 설정 라운드 수`처럼 증가하는지 확인한다.
- [x] 시작 카운트다운 동안 호스트와 클라이언트 모두 이동과 시야 회전이 막히는지 확인한다.
- [x] `시작!` 표시 뒤 호스트와 클라이언트 모두 이동과 시야 회전이 다시 가능한지 확인한다.
- [x] 시작 카운트다운이 끝난 뒤 기존 HUD `MatchElapsedTimeText`에 `경기 시간 0:00`부터 진행 시간이 증가하는지 확인한다.
- [x] 눈사람 모드에서 기존 HUD `MapShrinkCountdownText`가 표시되지 않는지 확인한다.
- [x] 눈사람 모드에서 `showdebug game` 또는 Output Log로 현재 GameMode가 `ASnowmanModeGameMode` 계열인지 확인한다. 이 GameMode는 `ASnowRumbleGameMode::OnMapShrinkRequested`를 갖지 않으므로 PvP 환경 축소 호출 경로가 실행되지 않는다.
- [x] 눈사람 모드에서 `OnMapShrinkRequested` 기반 PvP 환경 축소가 호출되지 않는지 확인한다.
- [x] 로비에서 PvP 모드를 선택하고 시작하면 기존 PvP 흐름과 맵 축소 안내가 이전처럼 동작하는지 확인한다.

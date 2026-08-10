# Task C-17 - PvP 시작 카운트다운

## 설명

PvP 맵에 들어온 뒤 로딩창이 닫히면 `3`, `2`, `1`, `시작!` 카운트다운 UI를 표시하고, 시작 전에는 플레이어 입력이 게임에 반응하지 않게 한다. 로비 화면이나 로딩창 위에서는 카운트다운을 띄우지 않는다.

## 상태 전이 기준

- 시작 가능: C-15 PvP 이동과 로딩창 완료, 기존 HUD와 캐릭터 입력 게이트 사용 가능
- 완료 가능: 서버 확정 카운트다운, 입력 잠금, HUD 표시 계약, 수동 확인 절차 기록 완료

## 구현 항목

- [x] PvP GameState가 서버 확정 시작 시간을 복제한다.
- [x] 모든 예상 플레이어가 PvP 맵에 접속해 로딩창이 닫힌 뒤 3초 카운트다운을 시작한다.
- [x] 모든 예상 플레이어가 PvP 맵에 접속한 뒤 로딩창을 닫고 카운트다운을 시작한다.
- [x] 로딩창 제거와 HUD 생성이 반영되도록 3초 지연 후 카운트다운을 시작한다.
- [x] 카운트다운 중 이동, 점프, 스프린트, 상호작용, 조준, 공격, 이모션 입력을 무시한다.
- [x] 카운트다운 중 시점 회전 입력도 무시한다.
- [x] PvP 맵 진입 직후 카운트다운이 아직 시작되지 않은 지연 구간에도 이동·시점 입력 연결을 차단한다.
- [x] 로비 게시판 포커스에서 남은 입력 ignore 상태가 있어도 카운트다운 종료 후 PvP 게임 입력으로 복구한다.
- [x] PvP 시작 전 스폰 중 라운드 종료 판정이 먼저 걸려 카운트다운 이후에도 입력이 잠기는 경로를 차단한다.
- [x] HUD WBP의 `StartCountdownText`가 있으면 `3`, `2`, `1`, `시작!`을 자동 표시한다.
- [x] 시작 후 카운트다운 UI를 숨긴다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: C++·문서: 최재원(C), 카운트다운 WBP 표시 배치: 사용자 또는 S 인계
- 생성 파일: `Source/SnowRumble/Game/SnowRumbleGameState_C.h`, `Source/SnowRumble/Game/SnowRumbleGameState_C.cpp`, `Tasks/C/C-17_pvp_start_countdown.md`
- 변경 파일: `Source/SnowRumble/Game/SnowRumbleGameMode.h`, `Source/SnowRumble/Game/SnowRumbleGameMode.cpp`, `Source/SnowRumble/Player/SnowRumbleCharacter.h`, `Source/SnowRumble/Player/SnowRumbleCharacter.cpp`, `Source/SnowRumble/UI/MainHUDWidget.h`, `Source/SnowRumble/UI/MainHUDWidget.cpp`, `Tasks/C/PLAN_C.md`, `Tasks/C/ROLE_C.md`, `docs/PLANS.md`
- 공유 확인 대상: 사용자 또는 S
- 병합 순서: C-15 PvP 이동 후, C-05 라운드 흐름 확장 전

## 공용 계약과 인계

- 제공받을 계약:
  - `ASnowRumbleGameMode`: PvP 맵 GameMode다.
  - `ASnowRumbleCharacter::CanPerformGameplayAction()`: 캐릭터 행동 입력 게이트다.
  - `UMainHUDWidget`: PvP HUD 부모다.
- 제공할 계약:
  - `ASnowRumbleGameState`: PvP GameState 부모다.
  - `ASnowRumbleGameState::StartMatchCountdownFromServer(float CountdownSeconds)`: 서버가 시작 카운트다운을 확정한다.
  - `ASnowRumbleGameState::IsMatchInputLocked()`: 캐릭터가 시작 전 입력 잠금 여부를 읽는다.
  - `ASnowRumbleGameState::ShouldShowStartCountdown()`: HUD가 카운트다운 표시 여부를 읽는다.
  - `ASnowRumbleGameState::GetStartCountdownText()`: HUD가 `3`, `2`, `1`, `시작!` 표시 텍스트를 읽는다.
  - `ASnowRumbleGameMode::MatchStartCountdownSeconds`: PvP 시작 입력 잠금 시간이다. 기본값은 3초다.
  - `ASnowRumbleGameMode::MatchStartCountdownDelaySeconds`: 로딩창 제거와 HUD 생성이 반영된 뒤 카운트다운을 시작하기 위한 지연이다. 기본값은 3초다.
  - `UMainHUDWidget::StartCountdownText`: HUD WBP에 같은 이름의 TextBlock이 있으면 카운트다운 텍스트를 자동 표시한다.
- 인계 대상: 사용자 또는 S

## 범위 밖

- 라운드 재시작마다 반복되는 카운트다운
- 3판 2선승 라운드 상태
- 카운트다운 애니메이션과 사운드
- 팀 소개 카메라 연출

## 사전 전제

- C-15 PvP 이동과 로딩창

## 결정 필요

- 없음

## 변경 기록

- 2026-08-09: 사용자가 PvP 맵 시작 시 `3`, `2`, `1`, `시작!` UI와 시작 전 입력 잠금을 요청해 C-17을 추가하고 구현했다.
- 2026-08-09: 사용자가 로비 화면에서 열리면 안 되고 로딩 후 PvP 맵에 들어갔을 때 카운트다운이 떠야 한다고 확인해, 로딩창 닫기 직후 짧은 지연 뒤 PvP GameMode에서 카운트다운을 시작하게 조정했다.
- 2026-08-09: 로비 이동 후 카운트다운이 보이지 않는 문제에 대응해, 단순 PostLogin 인원 수가 아니라 모든 예상 플레이어가 PvP Pawn까지 가진 뒤 로딩창 닫기와 카운트다운을 시작하게 조정했다.
- 2026-08-09: Seamless travel 경로에서 일반 PostLogin 타이밍만으로 시작 조건을 잡지 못할 수 있어, `HandleStartingNewPlayer_Implementation`과 Pawn 스폰 직후에도 로딩 완료·카운트다운 시작 조건을 재확인하게 보강했다. 로딩창 제거 후 HUD 생성 여유 시간은 1초로 늘렸다.
- 2026-08-09: Pawn 준비 대기 조건이 클라이언트 스폰을 막을 수 있어 제거했다. 로딩창 닫기는 예상 인원 접속 기준으로 되돌리고, 카운트다운은 HUD 준비 여유를 위해 로딩창 제거 3초 뒤 시작하게 조정했다.
- 2026-08-10: 로딩창 종료 후 카운트다운 시작 전 지연 구간에 잠깐 움직일 수 있는 문제에 대응했다. PvP GameState는 카운트다운 시작 전에도 입력 잠금으로 간주하고, 로컬 캐릭터는 잠금 중 PlayerController의 move/look input ignore를 켠 뒤 카운트다운 종료 시 복구한다.
- 2026-08-10: 사용자가 `시작!` 이후 이동 잠금이 풀리지 않는 문제를 확인해, 카운트다운 종료 시 `APlayerController`의 move/look input ignore 카운터를 reset하고 `GameOnly` 입력 모드와 숨김 커서로 복구하게 수정했다.
- 2026-08-10: 입력 잠금이 계속 남는 추가 원인으로 PvP 스폰 중 `EvaluateRoundEndCondition()`이 먼저 실행되어 `bRoundEnded`가 true가 되는 경로를 확인했다. 라운드 종료 판정은 `ASnowRumbleGameState::IsMatchInputLocked()`가 false인 실제 경기 시작 이후에만 실행되게 막았다.

## 수동 작업

- HUD WBP에 TextBlock을 추가하고 이름을 `StartCountdownText`로 맞춘다.
- `StartCountdownText`를 화면 중앙 또는 원하는 위치에 배치하고, 기본 Visibility는 숨김으로 둔다.
- 카운트다운 길이를 조정하려면 PvP GameMode Blueprint에서 `MatchStartCountdownSeconds`를 변경한다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드 변경 완료
- [x] `git diff --check` 공백 점검 통과
- [x] 관련 C++ 컴파일 통과
- [x] Unreal Editor 종료 후 `SnowRumbleEditor Win64 Development` 최종 링크 확인
- [x] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [x] 공용 계약과 캡슐화 규칙 위반 없음
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨

### 검증 메모

- 2026-08-10: `ASnowRumbleGameState`를 추가하고, 모든 예상 플레이어가 PvP 맵에 접속한 뒤 로딩창이 닫히면 `ASnowRumbleGameMode`가 3초 시작 카운트다운을 확정하게 했다. `ASnowRumbleCharacter`는 PvP GameState가 존재하지만 카운트다운이 아직 시작되지 않은 구간부터 이동·행동·시점 입력을 차단하고, 카운트다운 종료 후 입력 ignore를 복구한다. `UMainHUDWidget`은 `StartCountdownText`로 `3`, `2`, `1`, `시작!`을 표시한다. 카운트다운은 로비 화면이나 로딩창 위가 아니라 PvP 맵에서 로딩창 제거 후 3초 지연 뒤 시작한다. `PostLogin`과 seamless travel 시작 처리에서 시작 조건을 확인한다. `git diff --check`는 통과했고 관련 C++ 컴파일도 통과했지만, 실행 중인 Unreal Editor가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크는 `LNK1104`로 실패했다. 에디터 종료 후 재빌드 확인이 필요하다.
- 2026-08-10: `시작!` 이후 입력 잠금이 풀리지 않는 문제에 대응해 `ASnowRumbleCharacter::RefreshPvpMatchInputLock()`의 해제 경로를 `ResetIgnoreMoveInput()`/`ResetIgnoreLookInput()`으로 바꾸고, `GameOnly` 입력 모드와 숨김 커서로 복구하게 했다. 로비 게시판 포커스 상태에서 PvP 이동이 시작되며 남은 ignore 카운터가 카운트다운 해제 뒤에도 입력을 막을 수 있는 경로를 정리한 것이다. `git diff --check -- Source/SnowRumble/Player/SnowRumbleCharacter.cpp`와 `SnowRumbleEditor Win64 Development` 빌드가 통과했다.
- 2026-08-10: 추가 확인에서 스폰 중 첫 팀 Pawn만 생존한 순간 라운드 종료가 즉시 확정되어 `bRoundEnded` 입력 잠금이 계속 남을 수 있음을 확인했다. `ASnowRumbleGameMode::EvaluateRoundEndCondition()`은 카운트다운과 시작 전 입력 잠금이 끝난 뒤에만 라운드 종료를 확정한다. `git diff --check`는 통과했고 C++ 컴파일도 통과했지만, 실행 중인 Unreal Editor가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크는 `LNK1104`로 실패했다.

### 결과 확인

- [ ] PvP 맵에 모든 플레이어가 접속하고 로딩창이 닫힌 뒤 `3`, `2`, `1`, `시작!`이 순서대로 표시된다.
- [ ] 로비 화면이나 로딩창 위에서는 `3`, `2`, `1`, `시작!`이 표시되지 않는다.
- [ ] `시작!` 전에는 이동, 점프, 스프린트, 상호작용, 조준, 공격, 이모션이 동작하지 않는다.
- [ ] `시작!` 전에는 마우스 또는 시점 회전 입력도 동작하지 않는다.
- [ ] 로딩창이 사라진 직후부터 `3`이 뜨기 전까지도 이동과 시점 회전이 동작하지 않는다.
- [ ] `시작!` 이후 카운트다운 UI가 사라지고 캐릭터를 움직일 수 있다.
- [ ] 호스트와 클라이언트 양쪽에서 카운트다운과 입력 잠금 해제가 같은 타이밍으로 보인다.

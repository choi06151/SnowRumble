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
  - `ASnowmanModeGameState::GetSnowmanModeTimeLimitSeconds()`: 제한시간 전체 길이를 반환한다.
  - `ASnowmanModeGameState::GetSnowmanModeRemainingSeconds()`: 남은 시간을 초 단위로 반환한다.
  - `ASnowmanModeGameState::GetSnowmanModeRemainingTimeText()`: 남은 시간을 `0:00` 형식 텍스트로 반환한다.
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

## 수동 작업

- 로비 GameMode Blueprint 또는 기본 클래스 설정에서 `PvPLevelCandidates` 배열에 기존 PvP 후보 맵 전체가 들어 있는지 확인한다.
- 눈사람 모드에서 C++ 기본 GameMode를 그대로 쓸 경우 별도 설정은 필요 없다.
- 눈사람 모드용 GameMode Blueprint를 만들고 싶다면 `ASnowmanModeGameMode`를 부모로 만든 뒤 로비 GameMode Blueprint의 `SnowmanModeGameModeClass`에 지정한다.
- 제한시간 UI를 표시하려면 HUD 또는 임시 WBP에서 현재 GameState를 `ASnowmanModeGameState`로 캐스팅해 `GetSnowmanModeRemainingTimeText()`를 읽는다.
- 기존 PvP `.umap`은 K-12에서 저장하거나 수정하지 않는다.

## 완료 조건

### 에이전트 확인

- [x] 모드 진입 기반 변경 완료
- [x] 기존 PvP 맵 축소 기믹 비활성 조건 확인
- [x] 제한시간 서버 권한 관리 확인
- [x] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [x] 공용 계약과 캡슐화 규칙 위반 없음
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨
- [x] `git diff --check` 공백 점검 통과
- [ ] `SnowRumbleEditor Win64 Development` 빌드 확인

### 검증 메모

- 2026-08-12: `git diff --check`는 통과했다. 현재 환경에는 `C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat`과 `UnrealBuildTool`이 없어 `SnowRumbleEditor Win64 Development` 빌드는 실행하지 못했다.

### 결과 확인

- [ ] Listen Server 호스트와 클라이언트 1명으로 로비에 입장한다.
- [ ] 로비 게시판에서 `SnowmanModeButton`을 눌러 눈사람 모드를 선택한다.
- [ ] 호스트가 `ReadyStartButton`을 눌렀을 때 기존 PvP 후보 맵 중 하나로 이동하는지 확인한다.
- [ ] 이동한 맵의 GameMode가 `ASnowmanModeGameMode` 또는 이를 부모로 한 Blueprint인지 확인한다.
- [ ] 모든 예상 플레이어가 접속하면 로딩창이 닫히는지 확인한다.
- [ ] GameState를 통해 `GetSnowmanModeRemainingTimeText()` 값이 10분에서 감소하는지 확인한다.
- [ ] 눈사람 모드에서 `OnMapShrinkRequested` 기반 PvP 환경 축소가 호출되지 않는지 확인한다.
- [ ] 로비에서 PvP 모드를 선택하고 시작하면 기존 PvP 흐름과 맵 축소 안내가 이전처럼 동작하는지 확인한다.

# Task C-18 - Steam 세션 최종 통합

## 설명

현재 개발과 빠른 검증은 `OnlineSubsystem NULL` 기반 LAN 세션으로 유지하되, 출시 준비 단계에서 Steam Session, Steam Overlay, 친구 초대와 초대 수락 흐름을 통합한다.

## 상태 전이 기준
- 시작 가능: 핵심 로비·PvP·라운드·메인메뉴 복귀 흐름이 LAN 기준으로 안정화된 뒤
- 완료 가능: Steam 빌드에서 방 생성·검색·참가·초대·초대 수락·세션 정리가 검증된 뒤

## 구현 항목
- [x] `OnlineSubsystemSteam` 플러그인·Steam NetDriver 설정과 Steam App ID 적용 경로를 정리한다. 실제 App ID는 사용자 확인 전까지 placeholder로 유지한다.
- [x] `USnowRumbleSessionSubsystem`의 공개 API는 유지하고 내부 구현만 LAN/Steam 경로로 분리한다.
- [x] Steam 사용 가능 상태에서는 Steam Presence/Lobby 세션 생성·검색·참가를 사용한다.
- [x] Steam을 사용할 수 없는 에디터·로컬 개발 환경에서는 기존 NULL LAN 설정을 유지한다.
- [ ] 로비 ESC 메뉴의 친구 부르기 버튼을 Steam Overlay 친구 초대 또는 초대창으로 연결한다.
- [ ] Steam 초대 수락 후 메인메뉴·로비·PvP 상태에서 안전하게 세션 참가하는 흐름을 제공한다.
- [ ] 메인메뉴 복귀, 로비 복귀, 게임 종료 시 세션 정리 정책을 Steam 기준으로 재검증한다.
- [x] Steam 호스트 종료·메인메뉴 복귀 시 클라이언트가 기존 호스트 이탈 알람과 함께 메인메뉴로 복귀한다.

## 작업 배정
- 담당자·기능·계약 소유자: 최재원(C)
- 자산 수정자: C++·문서 최재원(C), Steam App ID·테스트 계정·빌드 환경은 사용자 확인 필요
- 생성 파일: 미정
- 변경 파일 후보: `Config/DefaultEngine.ini`, `Source/SnowRumble/Online/SnowRumbleSessionSubsystem.*`, `Source/SnowRumble/UI/LobbyEscapeMenuWidget.*`, `Source/SnowRumble/UI/MainMenuWidget.*`, `Source/SnowRumble/UI/LobbyPlayerController.*`, `Tasks/C/C-18_steam_session_integration.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`
- 공유 확인 대상: C-02, S-02, 사용자
- 병합 순서: LAN 기반 MVP 흐름 안정화 후, 출시 준비 통합 전

## 공용 계약과 인계
- 제공받을 계약:
  - C-02 LAN 세션 공개 API와 메인메뉴 방 만들기·검색·참가 UI 계약
  - C-05 로비 ESC 메뉴 `OnInviteFriendsRequested`와 메인메뉴 이동 흐름
- 제공할 계약:
  - Steam 사용 가능 여부를 UI가 표시하거나 기능 활성화에 사용할 수 있는 상태
  - Steam 친구 초대 버튼 실행 결과와 실패 사유
  - 초대 수락 후 세션 참가와 travel 결과
  - LAN fallback 유지 정책

## 범위 밖
- Steam 상점 페이지, 업적, 리더보드
- 전용 서버 전환
- 플랫폼별 비Steam 친구 초대

## 사전 전제
- Steam App ID 또는 테스트용 Spacewar App ID 사용 결정
- Steam 클라이언트 로그인과 Overlay 사용 가능 환경
- Standalone 또는 패키지 빌드 기준 테스트 환경

## 결정 필요
- 출시 App ID 사용 시점
- Steam 초대 UI를 단순 친구창으로 먼저 열지, 세션 초대창까지 바로 연결할지
- 방 코드 기능을 Steam 검색 필터와 병행할 방식

## 수동 작업

- Steamworks/Steam 계정 테스트 환경을 준비한다.
- Steam Overlay가 동작하는 Standalone 또는 패키지 테스트 절차를 확정한다.
- 필요하면 테스트용 Steam 계정 2개 이상 또는 2PC 환경을 준비한다.
- 호스트가 로비 또는 PvP에서 메인메뉴로 이동하거나 게임을 종료할 때 클라이언트가 `L_MainMenu`로 이동하고 `호스트의 연결이 해제되었습니다.` 알람을 한 번 표시하는지 확인한다.
- 위 복귀 때 이전 네트워크 플레이어 Pawn이 메뉴에 남지 않고 메뉴 전용 프리뷰만 표시되는지 확인한다.

## 완료 조건
### 에이전트 확인
- [ ] Steam 설정과 빌드 구성이 문서화되어 있다.
- [ ] LAN fallback이 기존 메인메뉴·로비 테스트 흐름을 깨지 않는다.
- [ ] Steam 세션 생성·검색·참가 코드 경로가 분리되어 있다.
- [ ] 친구 부르기 버튼이 Steam 가능 상태에서 Steam 초대 UI를 연다.
- [ ] 초대 수락 후 참가 흐름이 처리된다.
- [ ] `git diff --check` 공백 점검 통과
- [ ] `SnowRumbleEditor Win64 Development` 빌드 성공

### 검증 메모

- 2026-08-27: 로비에서 선택한 `Snowman`/`TeamPvP` 모드를 온라인 세션 광고값에 갱신하고, 로비 밖에서 수락된 Steam 초대는 기존 세션을 정리하지 않고 거절하도록 보강했다.
- 2026-08-10: 사용자 결정에 따라 현재 개발과 테스트는 LAN/NULL 세션으로 계속 진행하고, Steam 세션 전환은 최종 통합 Task로 분리했다. 앞으로 새 로비·메인메뉴·PvP 기능은 `USnowRumbleSessionSubsystem` 등 공개 세션 계약을 통해서만 세션 기능을 사용하고, UI나 게임 규칙 코드가 LAN/Steam 구현 세부사항에 직접 의존하지 않게 유지한다.
- 2026-08-25: `LanToSteam` 브랜치에서 공개 LAN API를 유지한 Steam/NULL 세션 분기를 추가했다. Steam 사용 시 `bIsLANMatch=false`, Presence/Lobby 검색, Steam 초대 허용을 사용한다. 개발 테스트를 위해 기본 서비스를 Steam으로 전환하고 SpaceWar App ID 480을 적용했으며, 출시 전 전용 App ID로 교체한다.

### 결과 확인

- [ ] LAN 모드에서 기존 방 만들기·빠른 참가·방 코드 참가가 유지된다.
- [ ] Steam 모드에서 호스트가 세션을 만들 수 있다.
- [ ] Steam 모드에서 다른 클라이언트가 세션을 검색하고 참가할 수 있다.
- [ ] 친구 부르기 버튼이 Steam Overlay 초대 흐름을 연다.
- [ ] 초대받은 클라이언트가 초대를 수락해 세션에 참가할 수 있다.
- [ ] 메인메뉴 복귀 또는 게임 종료 시 세션이 올바르게 정리된다.
- [ ] Steam 호스트 이탈 시 클라이언트가 엉뚱한 맵에 남지 않고 메인메뉴로 복귀하며 호스트 이탈 알람을 표시한다.

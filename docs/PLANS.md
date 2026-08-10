# Plans

## 목적

이 문서는 네 역할 사이의 필수 의존성, 공용 계약, 현재 선점, 병합 순서와 MVP 통합 상태를 관리하는 중앙 보드입니다.

역할 내부 Task 순서와 상태는 각 담당자의 Codex가 아래 계획 문서에서 관리합니다.

| 담당자 | 역할 계획 | 기본 소유 영역 |
| --- | --- | --- |
| 최재원(C) | `Tasks/C/PLAN_C.md` | 공용 네트워크, PvP, 플레이어와 통합 |
| 강혜원(K) | `Tasks/K/PLAN_K.md` | 아이템 기본 로직 |
| 서유정(S) | `Tasks/S/PLAN_S.md` | UI·아트·모델·눈숲속 |
| 정다영(J) | `Tasks/J/PLAN_J.md` | 얼음 빙하·눈섬 레벨 |

## 운영 원칙

- 개발자는 계획 문서를 직접 작성하지 않고 각 담당자 Codex와 대화해 원하는 결과·오류·우선순위 변화를 전달합니다.
- 각 역할 Codex는 자기 `PLAN_*.md`와 Task를 자율적으로 추가·분할·재정렬할 수 있습니다.
- Task ID는 고정하며 우선순위 변경을 이유로 파일 이름이나 번호를 바꾸지 않습니다.
- 비메인 Codex는 파트 간 영향이 있는 변경을 자기 계획의 `통합 변경 요청`에 기록합니다.
- 최재원(C) Codex만 요청을 반영해 이 중앙 보드의 의존성·선점·병합 순서를 갱신합니다.

## 파트 간 필수 의존성

| 계약 | 제공 Task | 소비 Task·파트 | 필요한 결과 | 상태 |
| --- | --- | --- | --- | --- |
| I-01 기존 기반 | C-01 | 모든 역할 | 재사용 파일·자산과 공용 소유권 | 완료 |
| I-02 세션·방 | C-02 | C-03, S-02 | 방 설정·검색·참가 상태와 요청 | 진행중 |
| I-03 팀·대기방 | C-03 | C-04, C-08, C-14, S-03 | 팀 색 선택·준비·다색 팀 시작 상태 | 진행중 |
| I-04 랜덤 맵·로딩 | C-04 | C-05, S-04, S-10, J | 선택 맵·로딩 인원·시작 결과 | 진행중 |
| I-05 얼음·사망 | C-06 | C-05, C-07, C-13, K-07, S-07 | 얼음 잔여시간·사망·관전·전멸 결과 | 예정 |
| I-06 플레이어 효과 | C-07 | K-01, K-03~K-06, K-08, S-09, S-10, J-02, J-04 | 피해·회복·무적·능력 보정 요청 | 예정 |
| I-07 팀 식별 | C-08 | S-05 | 닉네임·팀 색·이름표용 상태 | 진행중 |
| I-08 눈 전투 | C-09 | K-05, K-06, K-08, S-06 | 눈 제작·조준·충전·피격과 큰 눈 광역 결과 | 예정 |
| I-09 경기 흐름 | C-05 | K-11, S-11, 맵 담당 | 라운드 시간·승수·결과·금색 상자 시점 | 진행중 |
| I-10 커스터마이징 | C-11 | S-01, S-02, S-08 | 외형 저장·복제·적용 상태 | 예정 |
| I-11 아이템 | K-01~K-11 | C-12, S-08, S-09, S-10, J | 아이템 상태·모델 요구·배치·UI 계약 | 예정 |
| I-12 레벨·비주얼 | S-12, J-05 | C-12 | 세 맵과 UI·모델·표현 자산 | 예정 |
| I-13 부활 계약 | C-13 | K-07, S-07, C-05 | 핫팩 부활 진행·취소·완료와 무적 결과 | 예정 |
| I-14 팀 등장 | C-14 | S-10, J-02, J-04 | 팀 스폰·시작 제한·팀 소개 상태 | 예정 |
| I-15 맵 환경 압박 | C-05, S-10, J-02, J-04 | C-12 | C-05 게임 속도별 축소 주기·경기 시간·축소 호출 계약, 레벨 담당 맵별 수위·자기장·눈 폭 서버 로직과 배치 | 진행중 |
| I-16 로비 게시판 상호작용 | C-15 | S-03, 사용자 | 게시판 outline, E 입력, 서버 검증 상호작용 이벤트, 카메라 컴포넌트 기반 포커스, 월드 UI 버튼 액션, 팀 색 선택, 매치 시작 로딩창 | 진행중 |
| I-17 Steam 세션 전환 | C-18 | C-02, S-02, 사용자 | LAN fallback을 유지하면서 Steam 세션·Overlay 친구 초대·초대 수락·세션 정리 통합 | 예정 |

## 세션 개발 정책

- 현재 MVP 구현과 빠른 로컬 검증은 `OnlineSubsystem NULL` 기반 LAN 세션으로 계속 진행한다.
- Steam 출시는 최종 목표로 유지하며 Steam 세션 통합은 C-18에서 별도 최종 Task로 처리한다.
- 앞으로 메인메뉴, 로비, PvP, ESC 메뉴, 친구 초대 UI는 `USnowRumbleSessionSubsystem` 등 공개 세션 계약만 호출하고 LAN/Steam 구현 세부사항에 직접 의존하지 않는다.
- 친구 부르기 버튼은 현재 Blueprint 이벤트와 UI 계약만 유지하고, 실제 Steam Overlay 초대 연결은 C-18에서 처리한다.
- 새 기능이 세션 생성·검색·참가·초대·퇴장·복귀에 영향을 주면 C-18 Steam 전환 영향 여부를 함께 기록한다.

## 통합 관문

| 관문 | 완료 조건 | 상태 |
| --- | --- | --- |
| G-01 기반 인수 | C-01 완료와 기존 진행중 선점 재정리 | 완료 |
| G-02 공용 계약 | C-02~C-09, C-11, C-13, C-14 중 소비 파트에 필요한 계약 완료 | 예정 |
| G-03 파트 통합 | K-10, S-12, J-05 완료 | 예정 |
| G-04 MVP 통합 | C-12 결과 확인 완료 | 예정 |

## 통합 변경 요청 대기열

각 비메인 Codex의 `PLAN_*.md`에 요청이 생기면 C Codex가 이 표에 옮겨 검토합니다.

| 요청 | 요청 역할 | 영향받는 계약·자산 | 결정 | 상태 |
| --- | --- | --- | --- | --- |
| 없음 | - | - | - | - |

## 현재 작업 배정

진행 중 Task가 생기면 최재원(C) Codex가 한 행을 추가합니다. 배정되지 않은 파일과 자산은 수정하지 않습니다.

| Task | 담당자 | 기능 소유자 | 계약 소유자 | 자산 수정자 | 선점 파일·자산 | 상태 |
| --- | --- | --- | --- | --- | --- | --- |
| C-01 | C | 최재원(C) | 최재원(C) | 문서: 최재원(C), 코드·자산: 구현 승인 전 확정 | `Tasks/C/C-01_existing_foundation_migration.md`, `Tasks/C/PLAN_C.md`, 기존 코드·자산 조사 대상은 승인 전 확정 | 완료 |
| C-02 | C | 최재원(C) | 최재원(C) | 문서·C++: 최재원(C), UI 자산: S-02 인계 | `Tasks/C/C-02_session_room_flow.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Online/SnowRumbleSessionSubsystem.*`, `Source/SnowRumble/UI/MainMenuWidget.*`, `Source/SnowRumble/UI/LobbyWidget.*` | 진행중 |
| C-03 | C | 최재원(C) | 최재원(C) | 문서·C++: 최재원(C), 로비 맵 PlayerStart 배치: 사용자/S·J 인계 | `Tasks/C/C-03_random_team_lobby.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Game/SnowRumbleLobbyGameMode.*`, `Source/SnowRumble/Game/SnowRumbleLobbyGameState.*`, `Source/SnowRumble/Game/SnowRumblePlayerState.*`, `Source/SnowRumble/UI/LobbyPlayerController.*` | 진행중 |
| C-04 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), PvP 후보 레벨 자산 지정: 사용자/S/J 인계 | `Tasks/C/C-04_random_map_loading.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Game/SnowRumbleLobbyGameMode.*`, `Source/SnowRumble/Game/SnowRumbleMatchSubsystem_C.*` | 진행중 |
| C-06 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 얼음/사망 표현 UI·VFX: 사용자/S 인계 | `Tasks/C/C-06_freeze_death_spectate.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Player/SnowRumbleHealthComponent.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*` | 진행중 |
| C-05 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 결과 UI/연출: 사용자/S 인계 | `Tasks/C/C-05_round_match_flow.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Game/SnowRumbleGameMode.*`, `Source/SnowRumble/Game/SnowRumbleGameState_C.*`, `Source/SnowRumble/Game/SnowRumbleLobbyGameState.*`, `Source/SnowRumble/Game/SnowRumbleMatchSubsystem_C.*`, `Source/SnowRumble/UI/LobbyBoardWidget_C.*`, `Source/SnowRumble/UI/MainHUDWidget.cpp`, `Source/SnowRumble/Player/SnowRumbleHealthComponent.cpp` | 진행중 |
| C-08 | C | 최재원(C) | 최재원(C) | C++: 최재원(C), 이름표 WBP·그래픽: S-05 인계 | `Tasks/C/C-08_spawn_intro_identity.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Source/SnowRumble/UI/OverheadNameplateWidget_C.*` | 진행중 |
| C-15 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 게시판 Blueprint·맵 배치·로딩 WBP: 사용자/S 인계 | `Tasks/C/C-15_lobby_board_interaction.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Interaction/LobbyInteractionBoard_C.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Source/SnowRumble/UI/*LoadingScreen*`, `Source/SnowRumble/UI/SnowRumblePlayerController.*`, `Source/SnowRumble/Game/SnowRumbleGameMode.*` | 진행중 |
| C-16 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), HUD WBP 배치: 사용자/S 인계 | `Tasks/C/C-16_teammate_health_hud.md`, `Tasks/C/PLAN_C.md`, `Tasks/C/ROLE_C.md`, `docs/PLANS.md`, `Source/SnowRumble/UI/MainHUDWidget.*` | 완료 |
| C-17 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 카운트다운 WBP 표시 배치: 사용자/S 인계 | `Tasks/C/C-17_pvp_start_countdown.md`, `Tasks/C/PLAN_C.md`, `Tasks/C/ROLE_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Game/SnowRumbleGameState_C.*`, `Source/SnowRumble/Game/SnowRumbleGameMode.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Source/SnowRumble/UI/MainHUDWidget.*` | 진행중 |
| C-18 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), Steam App ID·테스트 계정·빌드 환경: 사용자 확인 | `Tasks/C/C-18_steam_session_integration.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Config/DefaultEngine.ini`, `Source/SnowRumble/Online/SnowRumbleSessionSubsystem.*`, `Source/SnowRumble/UI/LobbyEscapeMenuWidget.*`, `Source/SnowRumble/UI/MainMenuWidget.*`, `Source/SnowRumble/UI/LobbyPlayerController.*` | 예정 |
| C-19 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 채팅 WBP 배치·스타일: 사용자/S 인계 | `Tasks/C/C-19_text_chat.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/UI/ChatWidget_C.*`, `Source/SnowRumble/UI/SnowRumblePlayerController.*`, `Source/SnowRumble/UI/LobbyPlayerController.*` | 진행중 |
| C-20 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 로비/HUD 로그 WBP 배치·스타일: 사용자/S 인계 | `Tasks/C/C-20_event_log_ui.md`, `Tasks/C/ROLE_C.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/UI/SnowRumblePlayerController.*`, `Source/SnowRumble/UI/LobbyWidget.*`, `Source/SnowRumble/UI/MainHUDWidget.*`, `Source/SnowRumble/Game/SnowRumblePlayerState.*`, `Source/SnowRumble/Game/SnowRumbleGameMode.*` | 진행중 |
| C-21 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 옵션 WBP 배치·스타일: 사용자/S 인계 | `Tasks/C/C-21_options_menu.md`, `Tasks/C/ROLE_C.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/UI/OptionsWidget_C.*`, `Source/SnowRumble/UI/OptionsKeyBindingRowWidget_C.*`, `Source/SnowRumble/UI/MainMenuWidget.*`, `Source/SnowRumble/UI/MainMenuPlayerController.*`, `Source/SnowRumble/UI/LobbyEscapeMenuWidget.*`, `Source/SnowRumble/UI/LobbyPlayerController.*`, `Source/SnowRumble/UI/SnowRumblePlayerController.*`, `Source/SnowRumble/Player/SnowRumbleUserSettingsSubsystem_C.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*` | 진행중 |

## 상태 범례

- `예정`: 아직 시작하지 않은 상태
- `진행중`: 구현·인계·결과 확인 중인 상태
- `대기`: 사용자 결정·외부 계약·오류 해결을 기다리는 상태
- `완료`: 필요한 계약·통합과 결과 확인이 끝난 상태
- `대체`: 다른 Task나 계약으로 범위를 옮긴 상태

## 최근 통합 로그

- 2026-08-10: C-05 팀 점수 HUD를 참가 팀만 표시하도록 정리함. 팀별 Row 또는 ScoreText는 현재 참가 팀 기준으로 표시·숨김만 처리하고 WBP 배치 순서는 유지함.
- 2026-08-10: C-08 머리 위 이름표를 Screen space에서 World space로 전환함. 이름표가 로컬 카메라를 향하고 거리감에 따라 자연스럽게 크기가 달라지도록 함.
- 2026-08-10: C-04 PvP 후보 레벨 랜덤 진입 첫 범위를 구현함. `ASnowRumbleLobbyGameMode`는 `PvPLevelCandidates` 후보 배열에서 서버가 레벨을 선택하고 `?listen?ExpectedPlayers=` travel URL을 생성함.
- 2026-08-10: C-05 로비 설정 기반 1/3/5 라운드 매치 흐름을 추가함. `USnowRumbleMatchSubsystem`이 travel 사이 누적 점수와 라운드 번호를 유지하고, 남은 라운드가 있으면 다음 PvP 후보 맵으로 이동하며, 마지막 라운드 후 `ASnowRumbleGameState`가 매치 1등 팀을 복제함.
- 2026-08-10: C-05 모든 라운드 종료 후 임시 로비 복귀를 추가함. 매치 결과를 짧게 보여준 뒤 `/Game/Maps/L_Lobby?listen`으로 이동하고, 복귀 직전 `USnowRumbleMatchSubsystem` 누적 상태를 초기화함.
- 2026-08-10: Steam 출시는 최종 목표로 유지하되 현재 개발과 테스트는 LAN/NULL 세션으로 계속 진행하기로 결정함. Steam 세션, Overlay 친구 초대, 초대 수락과 세션 정리는 C-18 최종 통합 Task로 분리하고, 앞으로 세션 관련 새 기능은 공개 세션 계약을 통해 개발해 LAN/Steam 세부 구현이 UI·PvP 코드로 새지 않게 함.
- 2026-08-10: C-05 게임 속도별 맵 축소 호출 계약을 추가함. 로비 게시판은 90/60/30초 축소 주기 설정을 제공하고, PvP HUD는 경기 시간과 다음 축소 안내를 표시하며, 실제 맵 축소는 `ASnowRumbleGameMode::OnMapShrinkRequested`와 `CompleteMapShrinkFromBlueprint()`로 J/S 맵 담당에게 인계함.
- 2026-08-10: C-19 전체/팀 텍스트 채팅 계약을 추가함. `ASnowRumblePlayerController`가 Enter 입력, 서버 검증, 전체/팀 라우팅을 맡고 `UChatWidget` WBP 부모를 로비·PvP·추후 모드에서 재사용하게 함.
- 2026-08-10: C-19 채팅 채널 정책을 조정함. 로비는 전체 채팅만 허용하고, PvP와 추후 모드는 채팅 입력 중 Tab으로 전체/팀 채팅을 전환하게 함.
- 2026-08-10: C-19 채팅 입력 UX를 완료함. Enter 입력 시 TextBox 포커스, Enter 제출 후 GameOnly 복귀, 입력 중 스크롤, 채널 표시, 로그 스크롤바와 테두리 표시 정책을 정리하고 빌드 통과 후 완료 처리함.
- 2026-08-10: C-19 채팅 로그 표시 수명주기를 추가함. 마지막 채팅 갱신 또는 Enter 입력 후 5초 동안 표시하고 이후 서서히 사라지며, Enter 입력이나 새 메시지 수신 시 즉시 다시 표시함.
- 2026-08-10: C-20 이벤트 로그 UI 계약을 추가함. 로비/HUD WBP는 `EventLogText` 선택 바인딩을 통해 누적 로그를 표시하고, 서버 이벤트는 `ASnowRumblePlayerController::ClientReceiveEventLogMessage()`로 각 클라이언트에 전달함.
- 2026-08-10: C-21 옵션 메뉴 기능 정의를 추가함. 옵션은 메인메뉴와 로비에서 같은 WBP로 열고, PvP와 눈사람 모드에서는 ESC 옵션 메뉴를 열지 않는 정책으로 정리함.
- 2026-08-10: C-21 옵션 WBP 부모 `UOptionsWidget`을 추가함. 상단 카테고리 버튼 4개와 하단 `OptionsContentSwitcher` 인덱스 0 감도, 1 사운드, 2 키 설정, 3 마이크 구조를 제공함.
- 2026-08-11: C-21 메인메뉴와 로비 옵션 진입 연결을 추가함. 메인메뉴 `SettingsButton`과 로비 ESC 메뉴 `SettingsButton`은 각 PlayerController의 `OptionsWidgetClass`로 같은 옵션 WBP를 열 수 있음.
- 2026-08-11: C-21 옵션 WBP 포커스 경고를 막기 위해 `UOptionsWidget` 기본 포커스를 생성자와 런타임 구성 단계에서 활성화하고, 키바인딩 패널 표시용 `UOptionsKeyBindingRowWidget` 부모와 기본 조작 목록 계약을 추가함.
- 2026-08-11: C-21 키바인딩 행의 변경 버튼을 누르면 다음 키보드/마우스 버튼 입력을 캡처해 UI 할당값을 변경하고, `Esc` 입력으로 대기 상태를 취소하게 함.
- 2026-08-11: C-21 키바인딩 변경값을 로컬 사용자 설정에 저장하고, 캐릭터 Enhanced Input 매핑과 채팅 열기 직접 키 바인딩에 실제 적용하게 함.

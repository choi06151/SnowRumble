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
| I-04 랜덤 맵·로딩 | C-04 | C-05, S-04, S-10, J | 선택 맵·로딩 인원·시작 결과 | 예정 |
| I-05 얼음·사망 | C-06 | C-05, C-07, C-13, K-07, S-07 | 얼음 잔여시간·사망·관전·전멸 결과 | 예정 |
| I-06 플레이어 효과 | C-07 | K-01, K-03~K-06, K-08, S-09, S-10, J-02, J-04 | 피해·회복·무적·능력 보정 요청 | 예정 |
| I-07 팀 식별 | C-08 | S-05 | 닉네임·팀 색·이름표용 상태 | 진행중 |
| I-08 눈 전투 | C-09 | K-05, K-06, K-08, S-06 | 눈 제작·조준·충전·피격과 큰 눈 광역 결과 | 예정 |
| I-09 경기 흐름 | C-05 | K-11, S-11, 맵 담당 | 라운드 시간·승수·결과·금색 상자 시점 | 예정 |
| I-10 커스터마이징 | C-11 | S-01, S-02, S-08 | 외형 저장·복제·적용 상태 | 예정 |
| I-11 아이템 | K-01~K-11 | C-12, S-08, S-09, S-10, J | 아이템 상태·모델 요구·배치·UI 계약 | 예정 |
| I-12 레벨·비주얼 | S-12, J-05 | C-12 | 세 맵과 UI·모델·표현 자산 | 예정 |
| I-13 부활 계약 | C-13 | K-07, S-07, C-05 | 핫팩 부활 진행·취소·완료와 무적 결과 | 예정 |
| I-14 팀 등장 | C-14 | S-10, J-02, J-04 | 팀 스폰·시작 제한·팀 소개 상태 | 예정 |
| I-15 맵 환경 압박 | S-10, J-02, J-04 | C-05, C-12 | 레벨 담당 맵별 수위·자기장·눈 폭 서버 로직과 배치 | 예정 |
| I-16 로비 게시판 상호작용 | C-15 | S-03, 사용자 | 게시판 outline, E 입력, 서버 검증 상호작용 이벤트, 카메라 컴포넌트 기반 포커스, 월드 UI 버튼 액션, 팀 색 선택, 매치 시작 로딩창 | 진행중 |

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
| C-06 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 얼음/사망 표현 UI·VFX: 사용자/S 인계 | `Tasks/C/C-06_freeze_death_spectate.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Player/SnowRumbleHealthComponent.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*` | 진행중 |
| C-05 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 결과 UI/연출: 사용자/S 인계 | `Tasks/C/C-05_round_match_flow.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Game/SnowRumbleGameMode.*`, `Source/SnowRumble/Game/SnowRumbleGameState_C.*`, `Source/SnowRumble/Player/SnowRumbleHealthComponent.cpp` | 진행중 |
| C-08 | C | 최재원(C) | 최재원(C) | C++: 최재원(C), 이름표 WBP·그래픽: S-05 인계 | `Tasks/C/C-08_spawn_intro_identity.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Source/SnowRumble/UI/OverheadNameplateWidget_C.*` | 진행중 |
| C-15 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 게시판 Blueprint·맵 배치·로딩 WBP: 사용자/S 인계 | `Tasks/C/C-15_lobby_board_interaction.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Interaction/LobbyInteractionBoard_C.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Source/SnowRumble/UI/*LoadingScreen*`, `Source/SnowRumble/UI/SnowRumblePlayerController.*`, `Source/SnowRumble/Game/SnowRumbleGameMode.*` | 진행중 |
| C-16 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), HUD WBP 배치: 사용자/S 인계 | `Tasks/C/C-16_teammate_health_hud.md`, `Tasks/C/PLAN_C.md`, `Tasks/C/ROLE_C.md`, `docs/PLANS.md`, `Source/SnowRumble/UI/MainHUDWidget.*` | 완료 |
| C-17 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 카운트다운 WBP 표시 배치: 사용자/S 인계 | `Tasks/C/C-17_pvp_start_countdown.md`, `Tasks/C/PLAN_C.md`, `Tasks/C/ROLE_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Game/SnowRumbleGameState_C.*`, `Source/SnowRumble/Game/SnowRumbleGameMode.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Source/SnowRumble/UI/MainHUDWidget.*` | 진행중 |

## 상태 범례

- `예정`: 아직 시작하지 않은 상태
- `진행중`: 구현·인계·결과 확인 중인 상태
- `대기`: 사용자 결정·외부 계약·오류 해결을 기다리는 상태
- `완료`: 필요한 계약·통합과 결과 확인이 끝난 상태
- `대체`: 다른 Task나 계약으로 범위를 옮긴 상태

## 최근 통합 로그

- 2026-08-08: C-03 서버 랜덤 팀 배정과 1vs1~4vs4 동수·전원 준비 시작 조건을 구현하고 `SnowRumbleEditor Win64 Development` 빌드 성공을 확인함.
- 2026-08-08: C-08 서버 화면의 클라이언트 이름표가 초기 `DESKTOP-...` 값으로 남는 오류에 대응해 이름표 위젯 생성 보장과 표시 이름 변경 감지 갱신을 추가하고 빌드 성공을 확인함.
- 2026-08-08: C-15 로비 게시판 상호작용을 추가함. 게시판 C++ 부모, outline 후보 탐색, E 입력 서버 검증 이벤트를 구현하고 직접 팀 선택은 범위 밖으로 유지함.
- 2026-08-08: C-15 게시판 outline 후보 검사를 mesh bounds 기준으로 바꾸고 기본 상호작용 반경을 320cm로 늘려 큰 게시판 표면 근처에서도 잡히게 조정함.
- 2026-08-08: C-15 게시판 상호작용을 현재 outline 대상 기준으로 제한하고, 상호작용 성공 시 소유 클라이언트 카메라가 게시판 bounds 중심을 바라보게 조정함. 눈덩이 기본 획득 반경은 180cm로 유지함.
- 2026-08-08: C-08 닉네임 표시 하위 범위를 선점함. 캐릭터 `WidgetComponent`와 `UOverheadNameplateWidget` WBP 부모를 제공해 S-05가 이름표 WBP를 디자인할 수 있게 함.
- 2026-08-08: 로비 입장 후 서버가 소유 클라이언트에 저장 닉네임 제출을 요청하는 RPC 핸드셰이크를 추가해 서버 화면의 클라이언트 이름표도 복제 닉네임으로 갱신되게 함.
- 2026-08-08: 서버 닉네임 적용 시 기본 `APlayerState::PlayerName`도 함께 갱신해 WBP 또는 기본 이름 경로에서 `DESKTOP-...` 값이 남지 않도록 보강함.
- 2026-08-09: C-15 게시판 포커스를 `E` 토글 방식으로 바꾸고, 게시판 Blueprint의 `FocusCameraComponent` 구도를 포커스 뷰로 쓰도록 공용 계약을 확장함.
- 2026-08-09: C-15 게시판 포커스 중 마우스 UI 입력 모드와 이동·시점 입력 차단을 추가하고, `ULobbyBoardWidget` 버튼 클릭을 서버 검증된 게시판 액션 이벤트로 전달하게 함.
- 2026-08-09: 로비 팀 구조를 Red/Blue 2팀에서 8개 팀 색 선택으로 확장함. 게시판 팀 색 버튼이 PlayerState 팀을 서버 검증 변경하고 이름표 색에 반영되며, 시작 조건은 선택된 팀 색 2개 이상·각 팀 1~4명으로 변경됨.
- 2026-08-09: C-15 listen 환경 팀 변경 실패에 대응해 게시판 포커스 시점의 소유 `ALobbyPlayerController`를 월드 위젯에 직접 전달하고, 팀 색 버튼은 해당 컨트롤러 서버 RPC로 우선 요청하게 보강함.
- 2026-08-09: C-15 listen 환경 팀 변경 경로를 캐릭터 소유 `ServerRequestLobbyTeamSelection` RPC로 직접화함. 서버는 게시판 거리와 캐릭터 상태를 검증한 뒤 해당 캐릭터의 PlayerState 팀을 변경함.
- 2026-08-09: C-15 기존 `WBP_Lobby` 표시 계약을 확장함. `ULobbyWidget`은 준비 인원 수·현재 게임모드·내 이름·팀색·준비 상태를 표시하고, 로비 GameState에 복제 로비 모드 상태를 추가함.
- 2026-08-09: C-15 매치 시작 로딩창 계약을 추가함. 호스트가 게임 시작을 누르면 모든 참여자에게 로딩창 표시 RPC를 보내고, PvP GameMode가 예상 참여 인원 PostLogin 완료 후 로딩창 닫기 RPC를 보냄.
- 2026-08-09: C-15 PvP 스폰이 같은 PlayerStart 위치에 겹치는 문제를 보강함. 서버가 선택된 PlayerStart 주변 900cm 반경 안에서 실제 Pawn 생성 위치를 분산하고, 이미 확정한 스폰 위치와 최소 240cm 간격을 우선 확보함.
- 2026-08-09: C-16 팀원 HP HUD를 추가함. `UMainHUDWidget`은 로컬 플레이어와 같은 팀 색을 가진 다른 캐릭터만 `OtherPlayersHealthPanel`에 동적 `UHealthBarWidget`으로 표시함.
- 2026-08-09: C-16 팀원 HP HUD는 사용자 실행 확인으로 완료 처리함.
- 2026-08-09: C-17 PvP 시작 카운트다운을 추가함. PvP GameState가 시작 서버 시간을 복제하고, 시작 전 캐릭터 입력을 잠그며 HUD `StartCountdownText`에 `3`, `2`, `1`, `시작!`을 표시함.
- 2026-08-09: C-17 카운트다운을 로딩 후 PvP 맵 전용으로 조정함. 로비 화면이나 로딩창 위에서는 표시하지 않고, PvP GameMode가 로딩창을 닫은 뒤 짧은 지연 후 시작함.
- 2026-08-09: 로비 이동 후 C-17 카운트다운이 보이지 않는 문제에 대응해, 모든 예상 플레이어가 PvP Pawn까지 가진 뒤 로딩창 닫기와 카운트다운을 시작하게 조정함.
- 2026-08-09: C-17 seamless travel 시작 경로를 보강함. `PostLogin`, `HandleStartingNewPlayer_Implementation`, Pawn 스폰 직후에 로딩 완료 조건을 재확인하고 로딩창 제거 후 1초 뒤 카운트다운을 시작함.
- 2026-08-09: C-17 Pawn 준비 대기 조건이 클라이언트 스폰을 막을 수 있어 제거함. 로딩창 닫기는 예상 인원 접속 기준으로 되돌리고, 카운트다운은 로딩창 제거 3초 뒤 시작함.
- 2026-08-10: C-17 로딩창 종료 후 카운트다운 시작 전 지연 구간의 초기 입력 누수를 막음. PvP GameState는 카운트다운 시작 전에도 입력 잠금으로 처리하고, 로컬 캐릭터는 잠금 중 PlayerController move/look ignore를 적용함.
- 2026-08-10: C-06 얼음 60초 후 사망 첫 범위를 구현함. `USnowRumbleHealthComponent`가 HP 0에서 얼음 타이머를 시작하고 만료 시 라운드 사망 상태를 복제하며, 핫팩 부활용 `ReviveFromFrozen` 계약을 제공함.
- 2026-08-10: C-05 단일 라운드 종료 판정을 구현함. 얼음/사망 상태가 아닌 생존 플레이어가 한 팀 색에만 남으면 `ASnowRumbleGameState`가 라운드 종료와 승리 팀 색을 복제하고 전체 입력을 잠금.
- 2026-08-10: C-05 라운드 종료 HUD 연결을 추가함. `UMainHUDWidget`은 `EndRoundPanel`과 `EndRoundResultText` 선택 바인딩으로 라운드 종료 패널과 승리 팀 문구를 표시함.

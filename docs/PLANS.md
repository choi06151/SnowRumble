# Plans

## 목적

이 문서는 현재 진행 중인 단계 상태와 최근 작업 흐름을 빠르게 확인하기 위한 기록 문서입니다.
<!-- 새 프로젝트 시작 시 작업리스트와 최근 작업 로그의 기록을 초기화한다. -->

## 메인 프로그래머 작업리스트
| 단계 | Main Task 문서 | 해야 할 항목 | 상태 |
| --- | --- | --- | --- |
| 01-1 | `Tasks/01-1_project_baseline.md` | 프로젝트 기반과 시작 상태 확인 | 완료 |
| 01-2 | `Tasks/01-2_player_movement_camera_input.md` | 기본 캐릭터 이동, 카메라, 스프린트, 2인 PIE | 완료 |
| 02-1 | `Tasks/02-1_basic_network_snowball.md` | 서버 생성 눈덩이의 복제와 충돌 | 완료 |
| 02-2 | `Tasks/02-2_health_damage_freeze.md` | 최소 눈덩이 피해, HP, 얼기 | 완료 |
| 03-1 | `Tasks/03-1_lan_host_join.md` | LAN 세션 Host/Join | 완료 |
| 03-2 | `Tasks/03-2_lobby_team_ready.md` | 대기방, 팀 선택, 준비 상태 | 진행중 |
| 04-1 | `Tasks/04-1_snow_creation_interaction.md` | 눈 만들기, 잡기, 내려놓기 | 완료 |
| 04-2 | `Tasks/04-2_snow_aim_charge_throw.md` | 조준, 충전, 작은 눈 투척 | 완료 |
| 04-3 | `Tasks/04-3_snow_roll_and_large_snow.md` | 눈 굴리기, 성장, 큰 눈 투척 | 완료 |
| 05-1 | `Tasks/05-1_spectate_and_hotpack_revive.md` | 아군 관전과 핫팩 부활 | 예정 |
| 05-2 | `Tasks/05-2_gift_box.md` | 선물상자 생성과 공격 개봉 | 예정 |
| 05-3 | `Tasks/05-3_basic_items.md` | MVP 기본 아이템 | 예정 |
| 05-4 | `Tasks/05-4_gameplay_ui.md` | HUD와 상호작용 UI용 데이터 연결 | 진행중 |
| 06-1 | `Tasks/06-1_snow_island_match_flow.md` | 눈 쌓인 섬과 팀 승패 | 진행중 |
| 06-2 | `Tasks/06-2_result_and_mvp_validation.md` | 결과 데이터, 사운드, MVP 통합 검증 | 예정 |
| 07-1 | `Tasks/07-1_emote_radial_montage.md` | 임시 이모션 원형 선택과 몽타주 실행 | 진행중 |

## SUB 프로그래머 UI 작업리스트

SUB Task는 연결된 Main Task가 완료되고 실제 UI 연결 지점이 기록된 뒤 시작합니다.

| 단계 | SUB UI Task 문서 | 선행 Main Task | 해야 할 항목 | 상태 |
| --- | --- | --- | --- | --- |
| S03-1 | `Tasks/S03-1_lan_host_join_ui.md` | 03-1 | LAN Host·검색·Join UI | 예정 |
| S03-2 | `Tasks/S03-2_lobby_team_ready_ui.md` | 03-2 | 대기방 팀 선택·준비 UI | 예정 |
| S04-1 | `Tasks/S04-1_interaction_ui.md` | 04-1 | 상호작용·눈 제작 UI | 예정 |
| S04-2 | `Tasks/S04-2_aim_charge_ui.md` | 04-2, 04-3 | 조준·작은 눈·큰 눈 충전 UI | 예정 |
| S04-3 | `Tasks/S04-3_overhead_timed_action_ui.md` | 04-1, 04-3 | 투척 충전을 제외한 자기 캐릭터 머리 위 행동 진행 바 | 진행중 |
| S05-1 | `Tasks/S05-1_spectate_revive_ui.md` | 05-1 | 관전·핫팩 부활 UI | 예정 |
| S05-2 | `Tasks/S05-2_item_status_ui.md` | 05-2, 05-3 | 보상·MVP 아이템 상태 UI | 예정 |
| S05-3 | `Tasks/S05-3_gameplay_hud.md` | 02-2, 05-4 및 관련 SUB Task | 게임플레이 HUD 통합 | 예정 |
| S06-1 | `Tasks/S06-1_result_ui.md` | 06-1, 06-2 | 승패 결과와 다음 흐름 UI | 예정 |

## 상태 범례

- `예정`: 아직 시작 전
- `진행중`: 구현 중이거나, 구현 후 사용자 수동 작업 또는 결과 확인을 기다리는 상태
- `완료`: 구현 및 현재 확인 모드 기준 확인 완료

## 현재 작업 배정

진행 중 Task가 생기면 메인 프로그래머가 아래 표에 한 행을 추가합니다. 배정되지 않은 파일과 자산은 수정하지 않습니다.

| Task | 메인 프로그래머 선점 파일·비UI 자산 | SUB 프로그래머 선점 UI 자산 | 상태 |
| --- | --- | --- | --- |
| 03-2 대기방 팀 선택과 준비 | `Source/SnowRumble/Game/SnowRumblePlayerState.h`, `Source/SnowRumble/Game/SnowRumblePlayerState.cpp`, `Source/SnowRumble/Game/SnowRumbleLobbyGameState.h`, `Source/SnowRumble/Game/SnowRumbleLobbyGameState.cpp`, `Source/SnowRumble/Game/SnowRumbleLobbyGameMode.h`, `Source/SnowRumble/Game/SnowRumbleLobbyGameMode.cpp`, `Source/SnowRumble/Game/SnowRumbleMainMenuGameMode.h`, `Source/SnowRumble/Game/SnowRumbleMainMenuGameMode.cpp`, `Source/SnowRumble/UI/MainMenuWidget.h`, `Source/SnowRumble/UI/MainMenuWidget.cpp`, `Source/SnowRumble/UI/MainMenuPlayerController.h`, `Source/SnowRumble/UI/MainMenuPlayerController.cpp`, `Source/SnowRumble/UI/LobbyWidget.h`, `Source/SnowRumble/UI/LobbyWidget.cpp`, `Source/SnowRumble/UI/LobbyPlayerController.h`, `Source/SnowRumble/UI/LobbyPlayerController.cpp`, `Source/SnowRumble/Online/SnowRumbleSessionSubsystem.cpp`, `Tasks/03-2_lobby_team_ready.md`, `Tasks/S03-2_lobby_team_ready_ui.md`, `docs/ARCHITECTURE.md`, `docs/PLANS.md` | 없음 | 진행중 |
| 05-4 플레이어 체력 UI 바 연결 | `Source/SnowRumble/Player/SnowRumbleCharacter.h`, `Source/SnowRumble/Player/SnowRumbleCharacter.cpp`, `Source/SnowRumble/UI/MainHUDWidget.h`, `Source/SnowRumble/UI/MainHUDWidget.cpp`, `Source/SnowRumble/UI/HealthBarWidget.h`, `Source/SnowRumble/UI/HealthBarWidget.cpp`, `Tasks/05-4_gameplay_ui.md`, `docs/ARCHITECTURE.md`, `docs/PLANS.md` | 없음 | 진행중 |
| 07-1 임시 이모션 원형 선택과 몽타주 실행 | `Source/SnowRumble/Player/SnowRumbleCharacter.h`, `Source/SnowRumble/Player/SnowRumbleCharacter.cpp`, `Source/SnowRumble/UI/EmoteRadialMenuWidget.h`, `Source/SnowRumble/UI/EmoteRadialMenuWidget.cpp`, `Tasks/07-1_emote_radial_montage.md`, `docs/ARCHITECTURE.md`, `docs/PLANS.md` | 없음 | 진행중 |
| S04-3 행동 진행도 UI 연결 | `Source/SnowRumble/SnowRumble.Build.cs`, `Source/SnowRumble/Player/SnowRumbleCharacter.h`, `Source/SnowRumble/Player/SnowRumbleCharacter.cpp`, `Source/SnowRumble/UI/OverheadTimedActionWidget.h`, `Source/SnowRumble/UI/OverheadTimedActionWidget.cpp`, `Content/WBP/WBP_OverheadActionProgress.uasset`, `Tasks/S04-3_overhead_timed_action_ui.md`, `docs/ARCHITECTURE.md`, `docs/PENDING_ISSUES.md`, `docs/PLANS.md` | 없음 | 진행중 |

## 최근 작업 로그
- 2026-08-05: Task 03-2 대기방 팀 선택·준비 C++ 기본틀을 추가해 `PlayerState` 이름·팀·준비 복제, 대기방 시작 조건, 시작화면·대기방 UI 부모 위젯과 Host의 `L_Lobby` 이동 흐름을 구현함.
- 2026-08-05: Task 03-2 시작화면과 대기방 UI를 Level Blueprint가 아니라 `MainMenuPlayerController`, `LobbyPlayerController`에서 자동 생성하도록 변경함.
- 2026-08-05: Task 03-2 메인 메뉴 전용 `SnowRumbleMainMenuGameMode`를 추가해 시작화면 맵도 GameMode/PlayerController 구조로 통일함.
- 2026-08-05: Task 03-2 `WBP_MainMenu`의 `HostButton`, `FindButton`, `JoinFirstButton`을 C++에서 자동 클릭 바인딩하도록 변경함.
- 2026-08-05: Task 03-2 Host 버튼이 이미 ListenServer인 PIE 메뉴 맵에서 세션만 만들고 로비로 이동하지 않던 흐름을 수정해, Host 요청 시 항상 `L_Lobby`로 이동 후 세션을 생성하도록 변경함.
- 2026-08-05: Task 05-4의 첫 구현 단위로 로컬 플레이어 HP 컴포넌트를 자동 추적해 Progress Bar와 선택 Text Block을 갱신하는 `UHealthBarWidget` C++ 부모 위젯을 추가함.
- 2026-08-05: Task 05-4 체력 UI를 `BP_SnowRumbleCharacter`의 `HealthBarWidgetClass` 지정만으로 로컬 플레이어 화면에 자동 생성하도록 연결함.
- 2026-08-05: Task 05-4 체력 UI 구조를 `UMainHUDWidget` 중심으로 바꿔 Main HUD가 내 체력 바와 다른 플레이어 체력 바 목록을 배치·생성하도록 변경함.
- 2026-08-05: Task 05-4 Main HUD에 조준점, 투척 충전 Progress Bar, 머리 위 행동 진행 바 배치 슬롯을 통합하고 기존 충전 디버그 문구를 제거함.
- 2026-08-05: Task 07-1 이모션 원형 메뉴의 화면 디버그 메시지를 제거하고, 호버 중인 버튼에서 `Tab`을 떼면 해당 이모션을 선택하도록 변경함.
- 2026-08-05: Task 07-1 임시 이모션 원형 선택과 몽타주 실행 Task를 추가하고 기존 Tab 이모션 입력에서 0~7 선택 인덱스의 서버 검증·전체 화면 몽타주 재생 연결 지점을 구현함.
- 2026-08-05: Task 07-1에 `UEmoteRadialMenuWidget`을 추가해 `EmoteButton0~7` 클릭을 순서대로 `RequestPlayEmote(0~7)`에 자동 연결함.
- 2026-08-05: 사용자가 Task 04-3 큰 눈 굴리기 지면 접촉 결과 확인 완료를 보고해 메인 작업리스트 상태를 완료로 갱신함.
- 2026-08-05: Task 04-3 성장 프록시가 낮은 이전 중심에서 반지름부터 커져 지면 겹침 해소로 상승하던 경로를 수정해, 반지름 증가 전 중심을 같은 양만큼 먼저 올리도록 변경함.
- 2026-08-05: Task 04-3의 충돌 프록시 전환 뒤 불필요해진 최초 성장 `2cm` 지면 여유를 제거하고 성장 반지름 증가분만큼만 눈덩이 중심을 올리도록 수정함.
- 2026-08-05: S04-3 머리 위 행동 바가 눈 굴리기 중 현재 눈덩이의 복제 성장률을 표시하고 굴리기 종료 시 숨도록 C++ 통합 진행도에 `RollingSnowball`을 추가함.
- 2026-08-05: S04-3에서 투척 충전을 제외한 머리 위 행동 상태·진행도와 별도 Progress Bar의 값·표시·로컬 머리 위치를 자동 갱신하는 C++ 부모 위젯을 추가함.
- 2026-08-05: Task 04-2의 충전량 기반 직접 피해·넉백과 카메라 Line Trace 조준점 투척이 정상 동작함을 확인하고 다시 완료함.
- 2026-08-05: Task 04-2·04-3 투척 방향을 카메라 평행 방향에서 서버 Line Trace 조준점과 손의 눈덩이 위치를 잇는 방향으로 변경해 3인칭 카메라 시차를 보정함.
- 2026-08-05: Task 04-2·04-3 투척 시 서버 확정 충전량을 눈덩이에 보존하고 작은 눈·큰 눈의 직접 피해와 플레이어 넉백이 충전량에 따라 증가하도록 구현함.
- 2026-07-31: Task 04-3 굴리기 중 실제 눈덩이 충돌을 끄고 캐릭터 소유 Sphere Collision을 서버 Sweep 프록시로 사용하도록 변경했으며 호스트·클라이언트용 디버그 Sphere 표시를 추가함.
- 2026-07-31: Task 04-3 큰 눈의 긴 충전·느린 포물선 투척과 기존 작은 눈 투척 유지를 호스트·클라이언트에서 확인함.

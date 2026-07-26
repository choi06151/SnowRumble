# Plans

## 목적

이 문서는 현재 진행 중인 단계 상태와 최근 작업 흐름을 빠르게 확인하기 위한 기록 문서입니다.
<!-- 새 프로젝트 시작 시 작업리스트와 최근 작업 로그의 기록을 초기화한다. -->

## 메인 프로그래머 작업리스트
| 단계 | Main Task 문서 | 해야 할 항목 | 상태 |
| --- | --- | --- | --- |
| 01-1 | `Tasks/01-1_project_baseline.md` | 프로젝트 기반과 시작 상태 확인 | 완료 |
| 01-2 | `Tasks/01-2_player_movement_camera_input.md` | 기본 캐릭터 이동, 카메라, 2인 PIE | 진행중 |
| 02-1 | `Tasks/02-1_basic_network_snowball.md` | 서버 생성 눈덩이의 복제와 충돌 | 예정 |
| 02-2 | `Tasks/02-2_health_damage_freeze.md` | 최소 눈덩이 피해, HP, 얼기 | 예정 |
| 03-1 | `Tasks/03-1_lan_host_join.md` | LAN 세션 Host/Join | 예정 |
| 03-2 | `Tasks/03-2_lobby_team_ready.md` | 대기방, 팀 선택, 준비 상태 | 예정 |
| 04-1 | `Tasks/04-1_snow_creation_interaction.md` | 눈 만들기, 잡기, 내려놓기 | 예정 |
| 04-2 | `Tasks/04-2_snow_aim_charge_throw.md` | 조준, 충전, 작은 눈 투척 | 예정 |
| 04-3 | `Tasks/04-3_snow_roll_and_large_snow.md` | 눈 굴리기, 성장, 큰 눈 투척 | 예정 |
| 05-1 | `Tasks/05-1_spectate_and_hotpack_revive.md` | 아군 관전과 핫팩 부활 | 예정 |
| 05-2 | `Tasks/05-2_gift_box.md` | 선물상자 생성과 공격 개봉 | 예정 |
| 05-3 | `Tasks/05-3_basic_items.md` | MVP 기본 아이템 | 예정 |
| 05-4 | `Tasks/05-4_gameplay_ui.md` | HUD와 상호작용 UI용 데이터 연결 | 예정 |
| 06-1 | `Tasks/06-1_snow_island_match_flow.md` | 눈 쌓인 섬과 팀 승패 | 예정 |
| 06-2 | `Tasks/06-2_result_and_mvp_validation.md` | 결과 데이터, 사운드, MVP 통합 검증 | 예정 |

## SUB 프로그래머 UI 작업리스트

SUB Task는 연결된 Main Task가 완료되고 실제 UI 연결 지점이 기록된 뒤 시작합니다.

| 단계 | SUB UI Task 문서 | 선행 Main Task | 해야 할 항목 | 상태 |
| --- | --- | --- | --- | --- |
| S03-1 | `Tasks/S03-1_lan_host_join_ui.md` | 03-1 | LAN Host·검색·Join UI | 예정 |
| S03-2 | `Tasks/S03-2_lobby_team_ready_ui.md` | 03-2 | 대기방 팀 선택·준비 UI | 예정 |
| S04-1 | `Tasks/S04-1_interaction_ui.md` | 04-1 | 상호작용·눈 제작 UI | 예정 |
| S04-2 | `Tasks/S04-2_aim_charge_ui.md` | 04-2, 04-3 | 조준·작은 눈·큰 눈 충전 UI | 예정 |
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
| 01-2 | `Source/SnowRumble/Player/`, `Source/SnowRumble/Game/`, `Content/Characters/BP_SnowRumbleCharacter`, `Content/Input/`, `Content/Game/BP_SnowRumbleGameMode`, `Content/Maps/L_Prototype`, `Tasks/01-2_player_movement_camera_input.md`, `docs/ARCHITECTURE.md`, `docs/PENDING_ISSUES.md`, `docs/PLANS.md` | 없음 | 진행중 |

## 최근 작업 로그
- 2026-07-27: Task 01-1에서 `/Game/Maps/L_Prototype` 기본 테스트 맵 생성, 시작 맵 설정, 1인 PIE 결과 확인을 완료함.
- 2026-07-27: 숙련된 메인 프로그래머가 UI를 제외한 전체 개발을, 초보 SUB 프로그래머가 UI만 담당하도록 역할을 조정하고 15개 Main Task와 8개 SUB UI Task를 분리함.
- 2026-07-27: 메인 프로그래머는 기획·Task·규칙·C++·통합, SUB 프로그래머는 배정된 Blueprint·에셋을 단독 소유하도록 2인 충돌 방지 규칙을 추가함.
- 2026-07-26: 전체 MVP Task를 프로그래머와 Blueprint 개발자의 2인 협업 구조로 개편하고 C++→Blueprint 인계 기준을 추가함.
- 2026-07-26: 멀티플레이 초심자 학습 흐름에 맞춰 2인 PIE → 최소 눈덩이 복제 → HP·얼기를 LAN 세션보다 먼저 검증하도록 Task 순서를 조정함.
- 2026-07-26: SnowRumble GDD와 러프 기획을 기준으로 MVP 6개 Phase와 14개 Task 초안을 구성함.

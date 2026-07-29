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
| 03-2 | `Tasks/03-2_lobby_team_ready.md` | 대기방, 팀 선택, 준비 상태 | 예정 |
| 04-1 | `Tasks/04-1_snow_creation_interaction.md` | 눈 만들기, 잡기, 내려놓기 | 완료 |
| 04-2 | `Tasks/04-2_snow_aim_charge_throw.md` | 조준, 충전, 작은 눈 투척 | 완료 |
| 04-3 | `Tasks/04-3_snow_roll_and_large_snow.md` | 눈 굴리기, 성장, 큰 눈 투척 | 진행중 |
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
| 04-3 큰 눈 양손 운반 | `Source/SnowRumble/Player/SnowRumbleCharacter.h`, `Source/SnowRumble/Player/SnowRumbleCharacter.cpp`, `Source/SnowRumble/Snowball/SnowballEquipmentComponent.h`, `Source/SnowRumble/Snowball/SnowballEquipmentComponent.cpp`, `Source/SnowRumble/Snowball/SnowballItem.h`, `Source/SnowRumble/Snowball/SnowballItem.cpp`, `Tasks/04-3_snow_roll_and_large_snow.md`, `docs/GDD/Game_GDD.md`, `docs/PENDING_ISSUES.md`, `docs/PLANS.md` | 없음 | 진행중 |

## 최근 작업 로그
- 2026-07-29: Task 04-3에서 최대 성장값이 허용 오차 때문에 `1` 직전에 멈춰 큰 눈 스프린트 차단이 누락되는 문제를 공통 `IsFullyGrown()` 판정과 최대값 고정으로 수정함.
- 2026-07-29: Task 04-3에 최대 성장 큰 눈 보유 판정, 기본 `200cm/s` 운반속도와 스프린트 해제·차단을 구현함.
- 2026-07-29: Task 04-3의 E 탭 획득, E 유지 굴리기, 성장, 장애물 밀림과 AnimBP 상태를 호스트·클라이언트에서 확인함.
- 2026-07-29: Task 04-3 장애물 밀림을 이동 입력과 경쟁하는 Impulse에서 눈덩이 충돌 즉시 적용되는 서버 Sweep 위치 이동으로 변경함.
- 2026-07-29: Task 04-3 장애물 밀림 방향의 기준을 충돌면 법선에서 눈덩이 중심과 플레이어 위치 관계로 변경함.
- 2026-07-29: Task 04-3에서 굴리는 눈덩이가 장애물에 막히면 서버가 충돌면 바깥 방향으로 플레이어를 조금씩 밀어내도록 구현함.
- 2026-07-29: Task 04-1의 제작·내려놓기 눈덩이 지면 고정과 재획득·굴리기 후 새 지면 재고정을 확인하고 다시 완료함.
- 2026-07-28: Task 04-3의 `2cm` 지면 여유가 성장마다 누적되어 눈덩이가 뜨는 문제를 수정하고 굴리기 최초 성장에만 적용함.
- 2026-07-28: Task 04-3에서 첫 성장 후 눈덩이가 지면 접촉으로 멈추는 회귀에 대응해 반지름 보정에 `2cm` 굴리기 지면 여유를 추가함.
- 2026-07-28: Task 04-3에서 로컬 굴리기 대상의 아웃라인을 유지하고 성장별 굴리기 이동속도를 `300 → 150`으로 낮춤.
- 2026-07-28: Task 04-1 눈 만들기 오류 수정 결과를 확인하고 로컬·서버 제작 Trace 선과 좌클릭 화면 진단 출력을 제거함.

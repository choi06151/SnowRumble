# 최재원(C) 작업 계획

## 운영 방식

- 이 문서는 최재원과 대화하는 Codex가 작성·갱신합니다. 개발자가 직접 편집할 필요는 없습니다.
- Task ID는 고정하고 실제 실행 순서는 이 표의 우선순위로 조정합니다.
- 최재원(C) Codex는 자기 Task와 중앙 `docs/PLANS.md`의 공용 의존성·선점을 함께 관리할 수 있습니다.
- 상세 변경 절차는 `docs/TASK_WRITING.md`의 `대화 기반 자율 계획`을 따릅니다.

## 현재 집중 Task

- 없음

## 작업 대기열

| 우선순위 | Task | 결과 | 필수 선행 | 상태 |
| --- | --- | --- | --- | --- |
| 1 | [C-01](C-01_existing_foundation_migration.md) | 기존 기반과 소유권 마이그레이션 | 없음 | 예정 |
| 2 | [C-02](C-02_session_room_flow.md) | 세션·방 설정 | C-01·기획 결정 | 예정 |
| 3 | [C-03](C-03_random_team_lobby.md) | 랜덤 팀 대기방 | C-02 | 예정 |
| 4 | [C-06](C-06_freeze_death_spectate.md) | 얼음·사망·관전 | C-01 | 예정 |
| 5 | [C-07](C-07_shared_effect_contracts.md) | 부활·아이템·환경 공용 계약 | C-06 | 예정 |
| 6 | [C-04](C-04_random_map_loading.md) | 랜덤 맵과 로딩 | C-03 | 예정 |
| 7 | [C-08](C-08_spawn_intro_identity.md) | 팀 등장과 식별 데이터 | C-03 | 예정 |
| 8 | [C-09](C-09_snow_combat_completion.md) | 눈 전투 완성 | C-01, C-07 | 예정 |
| 9 | [C-05](C-05_round_match_flow.md) | 3판 2선승 경기 흐름 | C-04, C-06 | 예정 |
| 10 | [C-10](C-10_emote_revalidation.md) | 이모션 재검증 | C-01, C-06, C-08 | 예정 |
| 11 | [C-11](C-11_customization_contract.md) | 커스터마이징 데이터 계약 | C-02·기획 결정 | 예정 |
| 12 | [C-12](C-12_mvp_integration.md) | MVP 최종 통합 | 모든 파트 통합 Task | 예정 |

## 통합 변경 요청

- 없음

## 계획 변경 기록

- 2026-08-07: 새 GDD와 4인 구조를 기준으로 최초 대기열 작성.

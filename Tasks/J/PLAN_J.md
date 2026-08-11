# 정다영(J) 작업 계획

## 운영 방식

- 이 문서는 정다영과 대화하는 Codex가 작성·갱신합니다. 개발자가 직접 편집할 필요는 없습니다.
- Task ID는 고정하고 실제 실행 순서는 이 표의 우선순위로 조정합니다.
- J Codex는 `Tasks/J/` 안의 계획·Task만 직접 갱신하고 C++·공용 계약 변경은 `통합 변경 요청`에 기록합니다.
- 상세 변경 절차는 `docs/TASK_WRITING.md`의 `대화 기반 자율 계획`을 따릅니다.

## 현재 집중 Task

- [J-03 눈섬 블록아웃](J-03_snow_island_blockout.md)

## 개발 스타일

- 상태: Blueprint 위주
- 역할 적용: Level Editor와 `.umap` 작업 중심
- J-03 적용: Gameplay Blueprint와 C++ 구현 없이 레벨 블록아웃만 진행

## 작업 대기열

| 우선순위 | Task | 결과 | 필수 선행 | 상태 |
| --- | --- | --- | --- | --- |
| 1 | [J-03](J-03_snow_island_blockout.md) | 눈섬 블록아웃 | 없음 | 진행중 |
| 2 | [J-01](J-01_ice_glacier_blockout.md) | 얼음 빙하 블록아웃 | 없음 | 예정 |
| 3 | [J-02](J-02_ice_glacier_gameplay_layout.md) | 얼음 빙하 게임플레이 배치·수위 로직 | J-01, C-04, C-05, C-07, C-14, K-02, K-08 | 예정 |
| 4 | [J-04](J-04_snow_island_gameplay_layout.md) | 눈섬 게임플레이 배치·자기장 로직 | J-03, C-04, C-05, C-07, C-14, K-02, K-08 | 예정 |
| 5 | [J-05](J-05_level_integration.md) | 두 맵 통합 | J-02, J-04, K-10 | 예정 |

## 통합 변경 요청

- J-03: C가 `Content/Maps/L_SnowIsland_J.umap`의 J 브랜치 생성·단독 수정·push 진행을 승인함. 중앙 `docs/PLANS.md` 현재 작업 배정 기록은 C 소유이므로 등록 요청 상태를 유지함. `DemoMap`과 `Content/LowpolyStyle/WinterEnvironment/` 원본 자산은 참고·참조만 하고 수정하지 않음.

## 계획 변경 기록

- 2026-08-07: 새 GDD와 4인 구조를 기준으로 최초 대기열 작성.
- 2026-08-07: 환경 압박 로직을 레벨 담당자가 구현하도록 J-02·J-04 범위와 선행 계약을 조정.
- 2026-08-07: 최초 Task 시작 전 개발 스타일을 확인하고 Task 대기열을 스타일에 맞게 재검토하도록 계획 항목 추가.
- 2026-08-10: 회의 결정에 따라 얼음 빙하보다 눈섬을 먼저 제작하도록 J-03을 최우선으로 조정.
- 2026-08-10: J 개발 스타일을 Blueprint 위주로 확정하고, Level Editor·`.umap` 중심이며 J-03에서는 Gameplay Blueprint와 C++를 구현하지 않는다고 기록.
- 2026-08-10: J-03을 현재 집중 Task와 진행중 상태로 전환하고, 신규 눈섬 맵 후보의 J 단독 수정 등록을 C 통합 경로에 요청.
- 2026-08-10: C 확인에 따라 `L_SnowIsland_J.umap`을 J 브랜치에서 생성·단독 수정·push할 권한을 승인된 것으로 기록하고 중앙 기록 요청은 유지.

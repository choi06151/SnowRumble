# 역할 J - 정다영 레벨 담당

## 책임

- 얼음 빙하와 눈섬의 레벨 디자인
- 두 맵의 배치, 동선, 환경 위험 공간과 시각적 안내
- 배정된 `.umap`의 단독 수정과 레벨 결과 인계

## 작업 순서

1. `AGENTS.md`와 이 역할 문서를 읽습니다.
2. `$snowrumble-run-task`를 사용해 요청을 실행·변경·오류 대응·통합 중 하나로 분류합니다.
3. `PLAN_J.md`에서 현재 우선순위와 시작 가능한 J Task를 확인합니다.
4. 블록아웃은 계약 없이 가능한 범위에서 먼저 진행할 수 있습니다.
5. 피해·축소·스폰 기능 연결 전 C·K 선행 계약을 확인합니다.
6. 배정된 맵과 레벨 전용 자산만 선점합니다.
7. 대화에서 생긴 순서 변경·오류·추가 작업은 Codex가 `PLAN_J.md`와 관련 Task에 기록합니다.

## 금지와 인계

- C++와 Config, 공용 플레이어·아이템 Blueprint를 직접 수정하지 않습니다.
- Level Blueprint에서 서버 판정과 공용 경기 상태를 별도로 구현하지 않습니다.
- S가 소유한 눈숲속 `.umap`을 수정하지 않습니다.
- 필요한 환경 Actor·Component가 없으면 C 또는 K에게 계약과 배치 대상을 요청합니다.

## Task 목록

- [J-01 얼음 빙하 블록아웃](J-01_ice_glacier_blockout.md)
- [J-02 얼음 빙하 게임플레이 배치](J-02_ice_glacier_gameplay_layout.md)
- [J-03 눈섬 블록아웃](J-03_snow_island_blockout.md)
- [J-04 눈섬 게임플레이 배치](J-04_snow_island_gameplay_layout.md)
- [J-05 레벨 통합](J-05_level_integration.md)

역할 내부 상태와 우선순위는 `PLAN_J.md`, 파트 간 계약·선점은 `docs/PLANS.md`를 기준으로 판단합니다.

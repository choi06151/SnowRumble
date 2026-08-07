# 역할 K - 강혜원 서브 프로그래머

## 책임

- 아이템 공통 수명주기와 기본 로직
- 선물상자, 일반·황금 아이템과 모닥불의 멀티플레이 동작
- C가 제공한 플레이어·PvP 계약을 통한 효과 적용

## 작업 순서

1. `AGENTS.md`와 이 역할 문서를 읽습니다.
2. `$snowrumble-run-task`를 사용해 요청을 실행·변경·오류 대응·통합 중 하나로 분류합니다.
3. `PLAN_K.md`에서 현재 우선순위와 시작 가능한 K Task를 확인합니다.
4. C 선행 계약과 S 모델·UI 인계 여부를 확인합니다.
5. 아이템 소유 범위만 선점하고 구현합니다.
6. 공용 플레이어·경기 변경이 필요하면 C에게 계약을 요청합니다.
7. 대화에서 생긴 순서 변경·오류·추가 작업은 Codex가 `PLAN_K.md`와 관련 Task에 기록합니다.

## 금지와 인계

- 공용 Player, GameMode, GameState, PlayerState의 내부 상태를 직접 변경하지 않습니다.
- S가 소유한 아이템 모델·아이콘·UI 자산을 수정하지 않습니다.
- 맵 배치는 S·J에게 인계하고 아이템 Actor·Blueprint의 사용 조건을 제공합니다.

## Task 목록

- [K-01 아이템 기반](K-01_item_foundation.md)
- [K-02 선물상자](K-02_gift_box.md)
- [K-03 회복·무적 소비 아이템](K-03_consumables.md)
- [K-04 착용 장비](K-04_wearable_equipment.md)
- [K-05 눈삽](K-05_snow_shovel.md)
- [K-06 눈오리 제작기](K-06_snow_duck_maker.md)
- [K-07 핫팩](K-07_hotpacks.md)
- [K-08 모닥불](K-08_campfire.md)
- [K-09 황금 보상](K-09_golden_rewards.md)
- [K-10 아이템 통합](K-10_item_integration.md)
- [K-11 선물상자 라운드 스케줄 연결](K-11_gift_box_round_schedule.md)

역할 내부 상태와 우선순위는 `PLAN_K.md`, 파트 간 계약·선점은 `docs/PLANS.md`를 기준으로 판단합니다.

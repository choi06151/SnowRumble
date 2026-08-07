# 역할 S - 서유정 UI·아트·레벨 담당

## 책임

- UI/UX, 그래픽과 Widget Blueprint
- 캐릭터·아이템 모델링과 장비 외형
- 눈숲속 맵의 레벨 디자인과 폭설 VFX 표현

## 작업 순서

1. `AGENTS.md`와 이 역할 문서를 읽습니다.
2. `$snowrumble-run-task`를 사용해 요청을 실행·변경·오류 대응·통합 중 하나로 분류합니다.
3. `PLAN_S.md`에서 현재 우선순위와 시작 가능한 S Task를 확인합니다.
4. UI 데이터나 게임 상태가 필요하면 C·K 선행 계약을 확인합니다.
5. 배정된 UI·모델·눈숲속 자산만 선점합니다.
6. 서버 판정이 필요하면 Blueprint에 중복 구현하지 않고 계약 소유자에게 요청합니다.
7. 대화에서 생긴 순서 변경·오류·추가 작업은 Codex가 `PLAN_S.md`와 관련 Task에 기록합니다.

## 금지와 인계

- C++와 Config, 공용 게임플레이 Blueprint를 직접 수정하지 않습니다.
- 팀, 피해, 아이템, 얼기와 승패를 UI 로컬 값으로 확정하지 않습니다.
- J가 소유한 얼음 빙하·눈섬 `.umap`을 수정하지 않습니다.
- 모델과 UI 자산의 실제 연결 이름과 요구 조건을 C·K에 인계합니다.

## Task 목록

- [S-01 캐릭터 모델과 드로잉 외형](S-01_character_customization_art.md)
- [S-02 메인 로비·방 UI](S-02_main_lobby_room_ui.md)
- [S-03 대기방 팀·준비 UI](S-03_waiting_room_ui.md)
- [S-04 로딩 화면](S-04_loading_screen_ui.md)
- [S-05 머리 위 팀 이름표](S-05_overhead_nameplate.md)
- [S-06 게임플레이 HUD](S-06_gameplay_hud.md)
- [S-07 얼음·관전·부활 UI](S-07_freeze_revive_ui.md)
- [S-08 아이템 모델과 장비 외형](S-08_item_models.md)
- [S-09 아이템 상태·보상 UI](S-09_item_status_ui.md)
- [S-10 눈숲속 맵](S-10_snow_forest_level.md)
- [S-11 결과 UI](S-11_result_ui.md)
- [S-12 비주얼 통합](S-12_visual_integration.md)

역할 내부 상태와 우선순위는 `PLAN_S.md`, 파트 간 계약·선점은 `docs/PLANS.md`를 기준으로 판단합니다.

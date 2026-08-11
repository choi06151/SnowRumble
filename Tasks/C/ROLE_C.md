# 역할 C - 최재원 메인 프로그래머

## 책임

- 공용 네트워크 계약과 최종 통합
- 세션·로비, PvP 라운드·매치와 승패
- 공용 플레이어 구조, 눈 전투와 커스터마이징 데이터 계약
- GDD·하네스·Task·PLANS 관리

## 작업 순서

1. `AGENTS.md`와 이 역할 문서를 읽습니다.
2. `$snowrumble-run-task`를 사용해 요청을 실행·변경·오류 대응·통합 중 하나로 분류합니다.
3. `PLAN_C.md`에서 현재 우선순위와 시작 가능한 C Task를 확인합니다.
4. `docs/PLANS.md`에서 공용 계약·선점과 파트 간 통합 영향을 확인합니다.
5. 현재 Task와 직접 연결된 선행 Task·인계만 읽습니다.
6. 공용 계약 영향과 다른 역할의 소비 Task를 확인합니다.
7. 구현 승인 후 선점한 범위만 수정합니다.
8. 대화에서 생긴 순서 변경·오류·추가 작업은 Codex가 `PLAN_C.md`와 관련 Task에 기록합니다.

## 금지와 인계

- K의 아이템 내부 로직, S의 UI·모델·눈숲속 자산, J의 얼음 빙하·눈섬 자산을 직접 수정하지 않습니다.
- 공용 계약을 제공한 뒤 소비 파트가 내부 구현을 선택하도록 둡니다.
- 다른 담당자의 `.uasset`·`.umap` 충돌은 임의 병합하지 않습니다.

## Task 목록

- [C-01 기존 기반 마이그레이션](C-01_existing_foundation_migration.md)
- [C-02 세션·방 설정](C-02_session_room_flow.md)
- [C-03 랜덤 팀 대기방](C-03_random_team_lobby.md)
- [C-04 랜덤 맵과 로딩 동기화](C-04_random_map_loading.md)
- [C-05 3판 2선승 경기 흐름](C-05_round_match_flow.md)
- [C-06 얼음·사망·관전](C-06_freeze_death_spectate.md)
- [C-07 플레이어 효과 공용 계약](C-07_shared_effect_contracts.md)
- [C-08 팀 식별 데이터](C-08_spawn_intro_identity.md)
- [C-09 눈 전투 완성](C-09_snow_combat_completion.md)
- [C-10 이모션 재검증](C-10_emote_revalidation.md)
- [C-11 커스터마이징 데이터 계약](C-11_customization_contract.md)
- [C-12 MVP 통합](C-12_mvp_integration.md)
- [C-13 핫팩 부활 계약](C-13_revive_contract.md)
- [C-14 팀 스폰과 시작 연출](C-14_spawn_intro_flow.md)
- [C-15 로비 게시판 상호작용](C-15_lobby_board_interaction.md)
- [C-16 팀원 HP HUD](C-16_teammate_health_hud.md)
- [C-17 PvP 시작 카운트다운](C-17_pvp_start_countdown.md)
- [C-18 Steam 세션 최종 통합](C-18_steam_session_integration.md)
- [C-19 텍스트 채팅](C-19_text_chat.md)
- [C-20 이벤트 로그 UI](C-20_event_log_ui.md)
- [C-21 옵션 메뉴](C-21_options_menu.md)
- [C-22 상호작용 안내 UI](C-22_interaction_prompt_ui.md)
- [C-23 마우스 휠 카메라 줌](C-23_camera_wheel_zoom.md)
- [C-24 캐릭터 모델과 ABP 애니메이션 계약](C-24_character_model_anim_contract.md)

역할 내부 상태와 우선순위는 `PLAN_C.md`, 파트 간 계약·선점과 통합 상태는 `docs/PLANS.md`를 기준으로 판단합니다.

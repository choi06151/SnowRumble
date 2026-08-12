# 최재원(C) 작업 계획

## 운영 방식

- 이 문서는 최재원과 대화하는 Codex가 작성·갱신합니다. 개발자가 직접 편집할 필요는 없습니다.
- Task ID는 고정하고 실제 실행 순서는 이 표의 우선순위로 조정합니다.
- 최재원(C) Codex는 자기 Task와 중앙 `docs/PLANS.md`의 공용 의존성·선점을 함께 관리할 수 있습니다.
- 상세 변경 절차는 `docs/TASK_WRITING.md`의 `대화 기반 자율 계획`을 따릅니다.

## 현재 집중 Task

- [C-25](C-25_pvp_gift_box_item_foundation.md) PvP 선물상자와 아이템 기본 계약

## 개발 스타일

- 상태: C++ 위주
- 확인일: 2026-08-07
- 적용 기준: 서버 권한, 복제 상태, 공용 계약, 플레이어·PvP 상태 수명주기는 C++ 중심으로 설계한다.
- Blueprint 책임: 표현, 에디터 자산 연결, 조정값, UI·연출 연결에 집중한다.
- Task 조정 기준: C Task는 공개 계약과 서버 검증을 먼저 고정하고, S/K/J가 연결할 Blueprint·자산 요구사항을 실제 이름으로 인계한다.

## 작업 대기열

| 우선순위 | Task | 결과 | 필수 선행 | 상태 |
| --- | --- | --- | --- | --- |
| 1 | [C-01](C-01_existing_foundation_migration.md) | 기존 기반과 소유권 마이그레이션 | 없음 | 완료 |
| 2 | [C-02](C-02_session_room_flow.md) | 세션·방 설정 | C-01·기획 결정 | 진행중 |
| 3 | [C-03](C-03_random_team_lobby.md) | 랜덤 팀 대기방 | C-02 | 진행중 |
| 4 | [C-15](C-15_lobby_board_interaction.md) | 로비 게시판 상호작용 | 기존 입력·outline | 진행중 |
| 5 | [C-06](C-06_freeze_death_spectate.md) | 얼음·사망·관전 | C-01 | 진행중 |
| 6 | [C-07](C-07_shared_effect_contracts.md) | 피해·회복·무적·능력 보정 계약 | C-06 | 예정 |
| 7 | [C-04](C-04_random_map_loading.md) | 랜덤 맵과 로딩 | C-03 | 진행중 |
| 8 | [C-08](C-08_spawn_intro_identity.md) | 팀 식별 데이터 | C-03 | 진행중 |
| 9 | [C-09](C-09_snow_combat_completion.md) | 눈 전투 완성 | C-01, C-07 | 예정 |
| 10 | [C-05](C-05_round_match_flow.md) | 3판 2선승 경기 흐름 | C-04, C-06 | 진행중 |
| 11 | [C-10](C-10_emote_revalidation.md) | 이모션 재검증 | C-01, C-06, C-08 | 예정 |
| 12 | [C-11](C-11_customization_contract.md) | 커스터마이징 데이터 계약 | C-02·기획 결정 | 진행중 |
| 13 | [C-13](C-13_revive_contract.md) | 핫팩 부활 계약 | C-06, C-07 | 예정 |
| 14 | [C-14](C-14_spawn_intro_flow.md) | 팀 스폰과 시작 연출 | C-03, C-04 | 예정 |
| 15 | [C-12](C-12_mvp_integration.md) | MVP 최종 통합 | 모든 파트 통합 Task | 예정 |
| 16 | [C-16](C-16_teammate_health_hud.md) | 팀원 HP HUD | C-03·기존 HP UI | 완료 |
| 17 | [C-17](C-17_pvp_start_countdown.md) | PvP 시작 카운트다운 | C-15·기존 HUD·캐릭터 입력 | 진행중 |
| 18 | [C-18](C-18_steam_session_integration.md) | Steam 세션 최종 통합 | LAN 기반 MVP 흐름 안정화·Steam 테스트 환경 | 예정 |
| 19 | [C-19](C-19_text_chat.md) | 텍스트 채팅 | C-03 팀 색·공용 PlayerController | 진행중 |
| 20 | [C-20](C-20_event_log_ui.md) | 이벤트 로그 UI | C-03, C-06, 공용 PlayerController | 진행중 |
| 21 | [C-21](C-21_options_menu.md) | 옵션 메뉴 | 메인메뉴·로비 ESC 메뉴 | 진행중 |
| 22 | [C-22](C-22_interaction_prompt_ui.md) | 상호작용 안내 UI | C-15·기존 눈덩이 획득 | 진행중 |
| 23 | [C-23](C-23_camera_wheel_zoom.md) | 마우스 휠 카메라 줌 | 기존 플레이어 카메라 | 진행중 |
| 24 | [C-24](C-24_character_model_anim_contract.md) | 캐릭터 모델과 ABP 애니메이션 계약 | C-01, C-08, C-11 | 진행중 |
| 25 | [C-25](C-25_pvp_gift_box_item_foundation.md) | PvP 선물상자와 아이템 기본 계약 | C-01, C-05, C-22 | 진행중 |

## 통합 변경 요청

- 없음

## 계획 변경 기록

- 2026-08-07: 새 GDD와 4인 구조를 기준으로 최초 대기열 작성.
- 2026-08-07: Task 분할 재검토에 따라 C-07을 기본 효과 계약으로 축소하고, 핫팩 부활은 C-13, 팀 스폰·시작 연출은 C-14로 분리.
- 2026-08-07: 최초 Task 시작 전 개발 스타일을 확인하고 Task 대기열을 스타일에 맞게 재검토하도록 계획 항목 추가.
- 2026-08-07: 구조 개편 커밋 `a1d4bfb`를 `master`, `C`, `K`, `S`, `J` 원격 브랜치에 배포했고, 해당 내용을 C-01의 사전 정리 기록으로 반영. 실제 코드·자산 기준 분류와 소유권 인계는 C-01의 남은 작업으로 유지.
- 2026-08-07: 최재원(C)의 개발 스타일을 C++ 위주로 확정. C Task는 서버 권한·복제·공용 계약을 C++ 중심으로 먼저 고정하고 Blueprint는 표현·연결 책임으로 분리한다.
- 2026-08-08: 개편 전 `master` 구현을 현재 C-01 기준으로 적용했다. 세션·로비, HP·얼기, 눈 전투, UI 부모, 이모션, 레거시 투사체의 유지·수정·대체·폐기 보류·재검증 분류를 C-01과 관련 C Task에 반영했다.
- 2026-08-08: 최재원(C)이 프로젝트 정상 동작을 확인해 C-01을 완료 처리하고, 다음 후보를 C-02 세션·방 설정으로 전환했다.
- 2026-08-08: C-02의 빠른 게임 기준을 빈자리 있는 첫 LAN 방 자동 참가로 확정하고, 직접 방 찾기는 방 코드 입력 참가로 확정해 C-02를 진행중으로 전환했다.
- 2026-08-08: C-02 세션 C++ 계약 구현과 공백 점검을 완료했다. 최종 빌드는 MSVC PCH 가상 메모리 부족 `C3859/C1076`으로 환경에서 막혀 page file 여유 확보 후 재확인이 필요하다.
- 2026-08-08: page file 여유 확보 후 C-02 최종 `SnowRumbleEditor Win64 Development` 빌드가 성공했다. 최종 완료 처리는 호스트·클라이언트 수동 결과 확인 후 진행한다.
- 2026-08-08: 빠른 참가·코드 참가 반응 없음 확인을 돕기 위해 세션 흐름 로그와 클라이언트 방 만들기 방어 메시지를 추가하고 재빌드 성공을 확인했다.
- 2026-08-08: 참가하기 버튼은 바로 첫 검색 결과에 참가하지 않고 `OnRoomCodeJoinPromptRequested`로 방 코드 입력 UI를 열도록 C++ 부모 계약을 조정했다.
- 2026-08-08: WBP는 바인딩만 하도록 `UMainMenuWidget`에 `RoomCodeJoinPanel`, `RoomCodeTextBox`, `ConfirmRoomCodeJoinButton`, `CancelRoomCodeJoinButton` 선택 바인딩을 추가했다.
- 2026-08-08: 방 코드 입력 패널을 여는 버튼 이름을 `JoinFirstButton` 대신 `FindButton`으로 정리했다.
- 2026-08-08: 빠른 참여 버튼 이름을 `QuickJoinButton`으로 명확히 하고 C++ 자동 바인딩을 추가했다.
- 2026-08-08: 로비 우측 상단 방 코드 표시는 `RoomCodeTextBlock` 선택 바인딩 이름만 맞추면 C++ 부모가 자동 갱신하도록 보강했다.
- 2026-08-08: 닉네임 정체성 데이터를 먼저 잡기 위해 `ULocalPlayerIdentitySubsystem`을 추가했다. 현재는 `PlayerNameTextBox` 닉네임만 저장·적용하고, 추후 커스터마이징 데이터 확장을 고려해 세션 책임과 분리했다.
- 2026-08-08: 최초 닉네임이 비어 있을 때 랜덤 기본 닉네임을 `PlayerNameTextBox`에 자동 표시하도록 후보 목록과 초기화 흐름을 추가했다.
- 2026-08-08: 로비 입장 후 기본 캐릭터 스폰과 이동 가능 상태를 먼저 확인하기 위해 C-03을 진행중으로 전환했다.
- 2026-08-08: 로비 기본 상태는 `GameOnly` 입력과 숨김 커서로 두고, 추후 로비 설정 UI는 `EnableLobbyUiInput`/`EnableLobbyGameInput` 전환 함수로 열고 닫게 조정했다.
- 2026-08-08: 로비와 인게임에서 닉네임이 먼저 보이도록 C-08 닉네임 표시 하위 범위를 진행중으로 전환했다.
- 2026-08-08: 머리 위 닉네임 표시를 `TextRenderComponent`가 아니라 `UOverheadNameplateWidget` WBP 부모와 `WidgetComponent` 방식으로 구현하도록 조정했다.
- 2026-08-08: 서버 화면에서 클라이언트 닉네임이 기본 PC 이름으로 남는 문제를 해결하기 위해 로비 `PostLogin` 이후 클라이언트 저장 닉네임 제출 RPC 핸드셰이크를 추가했다.
- 2026-08-08: 닉네임 적용 시 `LobbyPlayerName`과 기본 `APlayerState::PlayerName`을 함께 갱신해 서버 화면 이름표의 기본 PC 이름 노출 경로를 추가로 차단했다.
- 2026-08-08: C-03 서버 랜덤 팀 배정과 1vs1~4vs4 동수·전원 준비 시작 조건을 구현하고, 직접 팀 선택 요청은 MVP에서 무시하도록 정리했다.
- 2026-08-08: 서버 화면의 클라이언트 이름표가 초기 `DESKTOP-...` 값으로 남는 오류에 대응해 이름표 위젯 생성 보장과 표시 이름 변경 감지 갱신을 추가했다.
- 2026-08-08: 사용자가 게시판 outline과 `E` 상호작용을 요청해 C-15를 추가하고, 직접 팀 선택은 범위 밖으로 둔 채 게시판 C++ 부모와 서버 검증 이벤트를 구현했다.
- 2026-08-08: 게시판 크기 때문에 outline 후보가 잘 잡히지 않아 C-15 거리 검사를 게시판 중심 기준에서 mesh bounds 기준으로 바꾸고 기본 반경을 320cm로 늘렸다.
- 2026-08-08: C-15에서 눈덩이 기본 상호작용 거리는 180cm로 유지하고, 게시판은 자체 `InteractionRadius`로 조절하게 정리했다. `E`는 현재 outline 게시판에만 반응하고 성공 시 카메라가 게시판을 보게 했다.
- 2026-08-09: C-15 게시판 포커스를 토글 방식으로 보강했다. 포커스 중 `E`를 다시 누르면 캐릭터 카메라로 돌아오고, 게시판 Blueprint의 `FocusCameraComponent` 위치·회전으로 포커스 구도를 조정하게 했다.
- 2026-08-09: C-15 게시판 포커스 중 마우스 커서와 UI 입력 모드를 켜고 이동·시점 입력을 차단하게 했다. `ULobbyBoardWidget`과 `BoardWidgetComponent`를 추가해 게시판 WBP 버튼 클릭을 서버 검증된 액션 이벤트로 전달한다.
- 2026-08-09: C-15 게시판 클릭 대상을 여러 WidgetComponent로 확장했다. `ULobbyBoardWidget` 기반 WBP가 연결된 컴포넌트는 자식 Blueprint에서 추가해도 같은 포커스 클릭 경로를 사용한다.
- 2026-08-09: 로비 팀 모델을 8색 팀으로 확장했다. 게시판 팀 색 버튼은 서버 검증 후 PlayerState 팀을 변경하고 이름표 글자색·배경색에 복제 팀 색을 반영한다.
- 2026-08-09: Listen 환경에서 게시판 월드 위젯이 팀 변경 요청자를 잘못 찾는 문제에 대응해 포커스 시점의 소유 `ALobbyPlayerController`를 직접 저장하고 그 RPC 경로로 팀 색 변경을 우선 요청하게 보강했다.
- 2026-08-09: Listen 환경에서 팀 변경이 계속 실패해 버튼 클릭 경로를 캐릭터 소유 `ServerRequestLobbyTeamSelection` RPC로 직접화하고, 서버가 게시판 거리 검증 후 해당 캐릭터의 PlayerState 팀을 변경하게 보강했다.
- 2026-08-09: Listen 2창에서 팀 변경이 정상 동작하는 것을 확인하고, 추적용 로그와 화면 디버그 메시지는 제거했다. 멀티 PIE/listen 월드 UI 입력 충돌 방지를 위한 고유 `VirtualUserIndex`/`PointerIndex` 설정은 유지한다.
- 2026-08-09: 게시판 포커스 중에는 outline을 숨기고, 포커스 해제 후에는 근처 게시판 outline이 기존 후보 탐색으로 다시 표시되게 조정했다.
- 2026-08-09: 게시판 WBP에 `ReadyStartButton`/`ReadyStartButtonText` 자동 바인딩을 추가했다. 같은 버튼을 호스트는 `게임 시작`, 클라이언트는 ready 상태에 따라 `준비 완료`/`준비 취소`로 보게 하고 클릭 동작도 시작 요청과 ready 토글로 분기한다.
- 2026-08-09: 기존 `WBP_Lobby`가 쓰는 `ULobbyWidget`에 준비 인원 수·현재 게임모드·내 이름·팀색·준비 상태 자동 표시를 추가했다. 로비 GameState에는 복제 로비 모드 상태를 유지한다.
- 2026-08-09: `ReadyPlayerCountText` 표시 기준을 호스트 제외로 조정했다. 2인 listen 대기방에서는 클라이언트 준비 전 `0 / 1`, 준비 후 `1 / 1`로 표시한다.
- 2026-08-09: 클라이언트 화면에서 호스트가 준비 필요 인원에 포함되는 문제를 해결하기 위해 PlayerState에 복제 `IsLobbyHost` 상태를 추가하고, 준비 인원 수 계산은 이 복제값으로 호스트를 제외하게 했다.
- 2026-08-09: 이름표 WBP에 `ReadyStateImage`와 `HostStateImage` 선택 바인딩을 추가했다. 비호스트 ready 상태와 로비 호스트 상태를 PlayerState 복제값 기준으로 표시한다.
- 2026-08-09: 로비와 PvP가 같은 캐릭터를 공유해 로비에서도 카메라 snow VFX가 보이는 문제를 막았다. `LocalSnowEffect`는 로비 GameState가 아닌 PvP 맵에서만 활성화한다.
- 2026-08-09: 게임 시작 조건을 호스트 제외 전원 ready 기준으로 정리했다. 호스트가 `게임 시작`을 누르면 현재 모드가 PvP이고 조건을 만족할 때 `/Game/Maps/L_Prototype?listen`으로 이동한다.
- 2026-08-09: 매치 시작 로딩창 흐름을 추가했다. 게임 시작 직전 모든 참여자에게 로딩창 표시 RPC를 보내고, PvP GameMode에서 예상 참여 인원이 모두 PostLogin되면 로딩창을 닫는다.
- 2026-08-09: 로딩창 WBP에 접속 완료 인원 기준 ProgressBar와 상태 텍스트 바인딩을 추가했다. `LoadingProgressBar`, `LoadingStatusText`, `LoadingMessageText` 이름으로 자동 갱신된다.
- 2026-08-09: PvP 이동 후 이름표가 기본 PC 이름과 흰색으로 되돌아가는 문제를 해결했다. 매치 이동을 seamless travel로 전환하고 PlayerState의 로비 이름·팀 색 상태를 복사한다.
- 2026-08-09: PvP 스폰을 전체 PlayerStart 랜덤 선택으로 변경하고, 기본 이동 맵을 `L_Prototype`에서 겨울 환경 `DemoMap`으로 바꿨다. PlayerStart 태그는 스폰 규칙에 사용하지 않는다.
- 2026-08-09: 4명 이상이 같은 PlayerStart 위치에 겹쳐 생성되는 문제에 대응해 실제 Pawn 생성 위치를 선택된 PlayerStart 주변 900cm 반경 안에서 최소 240cm 간격을 우선 만족하도록 분산했다.
- 2026-08-09: C-16 팀원 HP HUD를 추가하고 진행중으로 전환했다. `UMainHUDWidget`은 자기 HP 바를 유지하면서 같은 팀 색의 다른 플레이어 HP 바만 좌측 상단 패널에 동적으로 생성한다.
- 2026-08-09: 사용자가 팀원 HP HUD 동작을 확인해 C-16을 완료로 전환했다.
- 2026-08-09: C-17 PvP 시작 카운트다운을 추가하고 진행중으로 전환했다. PvP GameState가 서버 확정 시작 시간을 복제하고, HUD는 `StartCountdownText`로 `3`, `2`, `1`, `시작!`을 표시한다.
- 2026-08-09: C-17 카운트다운 조건을 로딩 후 PvP 맵 전용으로 조정했다. 로비 화면이나 로딩창 위에서는 표시하지 않고, PvP GameMode가 로딩창을 닫은 뒤 짧은 지연 후 시작한다.
- 2026-08-09: 로비 이동 후 C-17 카운트다운이 보이지 않는 문제에 대응해, 모든 예상 플레이어가 PvP Pawn까지 가진 뒤 로딩창 닫기와 카운트다운을 시작하게 조정했다.
- 2026-08-09: C-17 seamless travel 시작 경로를 보강했다. `PostLogin`, `HandleStartingNewPlayer_Implementation`, Pawn 스폰 직후에 로딩 완료 조건을 재확인하고 로딩창 제거 후 1초 뒤 카운트다운을 시작한다.
- 2026-08-09: C-17 Pawn 준비 대기 조건이 클라이언트 스폰을 막을 수 있어 제거했다. 로딩창 닫기는 예상 인원 접속 기준으로 되돌리고, 카운트다운은 로딩창 제거 3초 뒤 시작한다.
- 2026-08-10: C-17 로딩창 종료 후 카운트다운 시작 전 지연 구간의 초기 입력 누수를 막았다. PvP GameState는 카운트다운 시작 전에도 입력 잠금으로 처리하고, 로컬 캐릭터는 잠금 중 PlayerController move/look ignore를 적용한다.
- 2026-08-10: 사용자가 PvP 종료 조건의 선행으로 얼음 60초 후 사망을 요청해 C-06을 진행중으로 전환했다. HP 0에서 60초 얼음 타이머를 시작하고 만료 시 라운드 사망 상태를 복제하는 첫 범위를 구현했다.
- 2026-08-10: 사용자가 PvP 종료 조건을 요청해 C-05 단일 라운드 종료 판정을 진행중으로 전환했다. 얼음/사망 상태가 아닌 생존 플레이어가 한 팀 색에만 남으면 라운드 종료와 승리 팀 복제를 확정한다.
- 2026-08-10: 라운드 종료 시 HUD WBP가 표시할 `EndRoundPanel`과 `EndRoundResultText` 선택 바인딩을 추가했다.
- 2026-08-10: C-17 `시작!` 이후 입력 잠금이 풀리지 않는 문제를 수정했다. 카운트다운 종료 시 PlayerController move/look ignore 카운터를 reset하고 PvP 게임 입력 모드로 복구한다.
- 2026-08-10: C-17 입력 잠금이 계속 남는 추가 원인을 수정했다. PvP 스폰 중 첫 팀 Pawn만 생존한 순간 C-05 라운드 종료가 조기 확정되지 않도록, 시작 전 입력 잠금 중에는 `EvaluateRoundEndCondition()`이 판정을 건너뛰게 했다.
- 2026-08-10: C-05 팀별 승리 점수 HUD 계약을 추가했다. 라운드 승리 팀 점수를 GameState가 1점 올려 복제하고, `WBP_MainHUDWidget`의 팀별 ScoreText 선택 바인딩으로 표시한다.
- 2026-08-10: C-05 팀 점수 HUD를 참가 팀만 보이게 확장했다. 팀별 Row 또는 ScoreText는 현재 참가 팀 기준으로 표시되고 점수 내림차순으로 같은 VerticalBox 안에서 재배치된다.
- 2026-08-10: C-05 팀 점수 Row 탐색을 보강했다. `*TeamScoreRow` 이름의 HorizontalBox를 WidgetTree에서 직접 찾아 Row 전체를 숨김·정렬 대상으로 쓴다.
- 2026-08-10: C-05 팀 점수 Row 정렬을 VerticalBox 전용으로 보강했다. Row의 직접 부모가 VerticalBox일 때만 `AddChildToVerticalBox()`로 재배치한다.
- 2026-08-10: C-05 팀 점수 Row 정렬을 `ShiftChild()` 방식으로 변경했다. 기존 슬롯을 유지해 Row 겹침 가능성을 줄이고, 참가 팀 ScoreText visibility를 복구한다.
- 2026-08-10: C-08 머리 위 이름표를 Screen space에서 World space로 전환했다. 이름표가 로컬 카메라를 향하고 거리감에 따라 자연스럽게 크기가 달라지도록 했다.
- 2026-08-10: 사용자가 PvP 레벨 후보 지정과 랜덤 진입을 요청해 C-04를 진행중으로 전환했다. `ASnowRumbleLobbyGameMode`에 `PvPLevelCandidates` 후보 배열과 서버 랜덤 선택 travel URL 생성을 추가했다.
- 2026-08-10: 사용자가 로비 설정 기반 1/3/5 라운드와 라운드 사이 랜덤 PvP 맵 변경을 요청해 C-05 범위를 확장했다. `USnowRumbleMatchSubsystem`이 travel 사이 매치 누적 점수를 유지하고, 남은 라운드가 있으면 다음 PvP 후보 맵으로 이동하며, 마지막 라운드 후 매치 1등 팀을 복제한다.
- 2026-08-10: Steam 출시는 최종 목표로 유지하되 현재 개발과 테스트는 LAN/NULL 세션으로 계속 진행하기로 결정했다. Steam 세션, Overlay 친구 초대, 초대 수락과 세션 정리는 C-18 최종 통합 Task로 분리하고, 앞으로 UI·PvP·로비 코드는 세션 구현 세부사항에 직접 의존하지 않도록 공개 세션 계약을 통해 개발한다.
- 2026-08-10: C-05 게임 속도별 맵 축소 호출 준비를 추가했다. 로비 게시판은 느리게/보통/빠르게 버튼으로 90/60/30초 축소 주기를 설정하고, PvP HUD는 `MatchElapsedTimeText`와 `MapShrinkCountdownText`로 경기 시간과 다음 축소 안내를 표시한다. 실제 맵 축소는 `ASnowRumbleGameMode::OnMapShrinkRequested` 이벤트와 `CompleteMapShrinkFromBlueprint()` 완료 신호를 J 또는 맵 담당 Blueprint에 인계한다.
- 2026-08-10: 사용자가 로비, PvP, 추후 좀비맵에서 사용할 전체/팀 텍스트 채팅을 요청해 C-19를 추가하고 진행중으로 전환했다.
- 2026-08-10: C-19 채팅 채널 전환을 버튼에서 입력 중 Tab으로 변경하고, 로비는 전체 채팅만 허용하도록 조정했다.
- 2026-08-10: 사용자가 C-19 채팅 입력, 스크롤, 포커스, 채널 표시와 로그 테두리 표시 정책을 확인하고 push를 요청해 C-19를 완료로 전환했다.
- 2026-08-10: 사용자가 채팅 최신화 후 5초가 지나면 서서히 사라지고 Enter 입력 시 다시 보이기를 요청해 C-19를 진행중으로 되돌리고 후속 수정했다.
- 2026-08-10: 사용자가 채팅 입력을 Enter로 다시 열 때마다 전체 채팅으로 초기화되는 문제를 보고해, C-19에서 PvP/추후 팀 채팅 가능 모드는 Tab으로 선택한 마지막 채널을 유지하도록 후속 수정했다.
- 2026-08-10: 사용자가 팀 채팅 메시지를 노란색 대신 하늘색으로 구분하길 원해, C-19에서 `ChatLogScrollBox` 메시지 행 색을 채널별로 적용하도록 후속 수정했다.
- 2026-08-10: 사용자가 로비와 PvP HUD에서 채팅과 분리된 누적 이벤트 로그를 요청해 C-20을 추가하고 진행중으로 전환했다. 아이템 획득 로그는 공용 로그 RPC 인계 후 K 아이템 Task 또는 후속 통합에서 연결한다.
- 2026-08-10: 사용자가 옵션 기능 정의를 먼저 정리하길 원해 C-21을 추가했다. 옵션은 메인메뉴와 로비에서 같은 WBP로 열고, PvP와 눈사람 모드에서는 ESC 옵션 메뉴를 막는 정책으로 정리했다.
- 2026-08-10: C-21 옵션 WBP 틀을 상단 카테고리 버튼 4개와 하단 `OptionsContentSwitcher` 구조로 확정하고, `UOptionsWidget` 부모를 추가했다.
- 2026-08-11: C-21에서 메인메뉴 `SettingsButton`과 로비 ESC 메뉴 `SettingsButton`이 같은 `OptionsWidgetClass` 옵션 WBP를 열도록 연결했다.
- 2026-08-11: C-21 옵션 WBP 포커스 경고를 막기 위해 `UOptionsWidget` 기본 포커스를 생성자와 런타임 구성 단계에서 활성화하고, 키바인딩 패널용 `UOptionsKeyBindingRowWidget` 부모와 기본 조작 표시 목록을 추가했다.
- 2026-08-11: C-21 키바인딩 행의 변경 버튼을 누르면 다음 키보드/마우스 버튼 입력을 캡처해 UI 할당값을 변경하고, `Esc` 입력으로 대기 상태를 취소하게 했다. `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-11: C-21 키바인딩 변경값을 로컬 사용자 설정에 저장하고, 캐릭터는 원본 `IMC_Player`를 런타임 복제해 저장된 키를 실제 Enhanced Input 매핑에 적용하게 했다. 채팅 열기 키도 PlayerController 직접 바인딩을 로컬 설정 기준으로 재구성한다.
- 2026-08-11: C-21 이동 키 변경 후 플레이어가 움직이지 않는 문제를 수정했다. 캐릭터 런타임 입력 매핑 적용을 원본 매핑 스냅샷 기준으로 처리해 저장 키 치환이 다른 이동 매핑에 연쇄 적용되지 않게 했다.
- 2026-08-11: C-21 옵션 또는 로비 ESC 메뉴 복귀 후 입력 차단이 남는 문제를 수정하고, 하단 `ResetButton`이 현재 선택된 WidgetSwitcher 카테고리만 초기화하도록 연결했다.
- 2026-08-11: C-21 키 설정 변경을 적용 전 UI 임시값으로만 유지하고, `ApplyButton`을 누를 때만 저장·입력 매핑 반영을 실행하게 했다. 변경사항이 없으면 `ApplyButton`은 비활성화되고, 옵션 닫기 또는 ESC 복귀 시 임시 변경은 폐기된다.
- 2026-08-11: C-21 감도 설정을 추가했다. 옵션 WBP는 `SensitivitySlider`와 `SensitivityValueText`로 감도 임시값과 퍼센트 텍스트를 갱신하고, 적용 시 로컬 설정에 저장하며 캐릭터 카메라 입력은 저장된 감도 값을 곱한다.
- 2026-08-11: C-21 배경음악/효과음 볼륨 설정을 추가했다. 옵션 WBP는 `BgmVolumeSlider`/`BgmVolumeValueText`, `SfxVolumeSlider`/`SfxVolumeValueText`로 소리 임시값과 퍼센트 텍스트를 갱신하고, 적용 시 로컬 설정과 선택 SoundClass 볼륨에 반영한다. C++ 컴파일은 통과했으나 에디터 DLL 잠금으로 최종 링크는 보류됐다.
- 2026-08-11: C-21 마이크 설정을 추가했다. 옵션 WBP는 `MicrophoneVolumeSlider`/`MicrophoneVolumeValueText`와 `MicrophonePushToTalkButton`/`MicrophoneAlwaysOnButton`을 사용하고, 기본 `K`인 `MicrophonePushToTalk` 키바인딩과 PlayerController 마이크 상태 이벤트를 제공한다. C++ 컴파일은 통과했으나 에디터 DLL 잠금으로 최종 링크는 보류됐다.
- 2026-08-11: C-21 선택형 버튼 표시를 보강했다. 옵션 메뉴의 현재 카테고리와 마이크 방식, 로비 게시판의 현재 팀 색·모드·라운드 수·게임 속도·ready 상태는 기존 Pressed 스타일을 유지해 선택된 버튼처럼 보인다.
- 2026-08-11: C-21 마이크 입력 상태를 엔진 네트워크 음성 송출에 연결했다. PlayerController는 눌러서 말하기 또는 항상 말하기 상태에 맞춰 `StartTalking()`/`StopTalking()`을 호출하고, 캐릭터 Blueprint용 `MicrophonePushToTalkAction` Enhanced Input 슬롯을 제공한다. `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-11: C-21 마이크 송출 중 플레이어 이름 표시를 추가했다. PlayerState가 음성 송출 중 여부를 복제하고, 로비/HUD WBP는 `VoiceSpeakingContainer` 안의 `VoiceSpeakingIcon` Image와 `VoiceSpeakingNamesText` TextBlock으로 현재 말하는 플레이어를 표시한다. C++ 컴파일은 통과했으나 에디터 DLL 잠금으로 최종 링크는 보류됐다.
- 2026-08-11: C-21 마이크 채널 전환을 추가했다. 기본 `M` 키로 전체/팀 말하기를 전환하고, PlayerState 복제 채널과 gameplay mute로 팀 말하기 수신 범위를 제한하며, `PersonalAlarmText`/`PersonalAlarmAnimation`으로 로컬 상태 알림을 표시한다. `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-11: C-21 마이크 채널 전환 기본 키를 `N`으로 옮기고, `M`은 플레이어 지정 음소거 입력으로 분리했다. 캐릭터 Blueprint 슬롯은 `MicrophoneChannelToggleAction`과 `VoiceTargetMuteAction`을 제공한다.
- 2026-08-11: C-21 `M` 플레이어 지정 음소거 메뉴를 추가했다. `UVoiceMuteMenuWidget`/`UVoiceMutePlayerRowWidget` 부모는 현재 인게임 플레이어 목록으로 행을 동적 생성하고, 각 행의 버튼으로 로컬 수동 음소거를 토글한다.
- 2026-08-11: C-11 첫 하위 범위로 메인메뉴 커스터마이징 진입 버튼을 추가했다. `CustomizationButton`은 메인메뉴 PlayerController의 `CustomizationLevelUrl`로 커스터마이징 레벨 이동을 수행한다.
- 2026-08-11: C-11 커스터마이징 레벨 전용 GameMode, PlayerController, WBP 부모를 추가했다. 레벨 진입 시 커스터마이징 WBP를 표시하고 `CustomizationCamera` 태그 카메라를 ViewTarget으로 사용하며, WidgetSwitcher로 메인/시점변경/색칠하기 화면을 전환한다.
- 2026-08-11: C-11 몸 색상 커스터마이징 첫 세로 슬라이스를 구현했다. 로컬 `USnowRumbleCustomizationSubsystem`, 복제 `ASnowRumblePlayerState::CustomizationData`, 캐릭터 `BodyColor` 머티리얼 적용, 로비 입장 시 저장 커마 제출 경로를 추가했고 `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-11: C-11 커스터마이징 방 프리뷰 캐릭터 애니메이션 설정을 추가했다. `PreviewAnimationAsset`과 `PreviewAnimationPositionSeconds`로 커마 전용 포즈를 지정하고, 기본적으로 `bPausePreviewAnimation`으로 정지 상태를 유지한다.
- 2026-08-11: C-11 페인트 trace 좌표에 UI DPI viewport scale을 반영해 커서보다 왼쪽에서 그려지는 현상을 보정했다. C++ 컴파일은 통과했으나 에디터 DLL 잠금으로 최종 링크는 보류됐다.
- 2026-08-11: C-11 `PaintCursorScreenOffset`을 추가해 커스터마이징 PlayerController BP에서 페인트 trace X/Y 보정값을 직접 조정할 수 있게 했다.
- 2026-08-11: C-11 커스터마이징 드로잉 stroke를 `FSnowRumbleCustomizationData::PaintStrokes`로 즉시 저장·복제하고, 로비/PvP 캐릭터가 복제 데이터에서 RenderTarget을 재생성해 `PaintTexture` 머티리얼 파라미터에 적용하게 했다.
- 2026-08-11: C-11 stroke 저장값에 대상 SkeletalMeshComponent 이름을 포함하고 로비/PvP 캐릭터에서 컴포넌트별 RenderTarget을 적용해 머리에 그린 선이 몸에 나타나는 문제를 수정했다.
- 2026-08-11: C-11 커스터마이징 방의 UV Y축 flip 기준을 저장 데이터에 포함해 로비/PvP 캐릭터가 같은 기준으로 stroke RenderTarget을 재생성하게 했다.
- 2026-08-11: C-11 페인트 화면에서 `Ctrl+Z`를 누르면 `BackButton`과 같은 누적 undo 스택으로 마지막 완료 stroke를 하나씩 제거하게 했다.
- 2026-08-11: C-11 페인트 입력을 `PaintMode` 페이지에서만 처리하게 막아 색칠하기 화면 진입 전 좌클릭으로 선이 생기던 문제를 수정했다.
- 2026-08-11: C-11 시점변경 화면을 프리뷰 캐릭터 회전으로 구현했다. `RotateLeftButton`/`RotateRightButton`은 press 중 `PreviewRotationSpeedDegrees` 기준으로 캐릭터 yaw를 계속 회전한다.
- 2026-08-11: C-11 별도 시점변경 화면 없이 `RotateLeftButton`/`RotateRightButton`을 누르면 현재 페이지에서 바로 프리뷰 캐릭터가 좌우 회전하게 했다.
- 2026-08-11: C-11 회전 버튼 좌우 방향을 실제 화면 기준에 맞게 뒤집고, 커스터마이징 레벨 프리뷰 캐릭터의 오버헤드 이름표 위젯을 숨기게 했다.
- 2026-08-11: C-21 `AlwaysOn` 마이크 입력 활성 상태가 곧바로 speaking 표시로 복제되어 좌측 음성 송출 표시가 항상 뜨던 문제를 수정했다. speaking 표시는 `PushToTalk` 입력 중에만 입력 상태를 그대로 반영하고, 로비/HUD 생성 시 기본 숨김으로 초기화한다.
- 2026-08-11: 로비에서 메인메뉴로 돌아온 뒤 LAN 세션 검색·빠른 참가·방 코드 참가가 다시 동작하지 않는 문제를 수정했다. `USnowRumbleSessionSubsystem::LeaveLanSession()`으로 로컬 named session, 검색 결과, 진행 상태를 정리하고 로비 복귀 및 메인메뉴 진입 시 호출한다.
- 2026-08-11: Listen Server 호스트 연결이 끊기면 클라이언트가 세션을 정리하고 메인메뉴로 이동하며, `MainMenuAlarmText`/`MainMenuAlarmAnimation`으로 "방장이 나가 방이 종료되었습니다." 알림을 한 번 표시하게 했다.
- 2026-08-11: 메인메뉴 호스트 이탈 알림 WBP 바인딩에 `AlarmText`/`AlarmAnimation` 호환 이름을 추가했다. 기존 `MainMenuAlarmText`/`MainMenuAlarmAnimation`도 그대로 지원한다.
- 2026-08-11: 호스트 연결 끊김 알림 판정을 named session 존재 여부 대신 실제 LAN 세션 참가/호스트 성공 플래그 기준으로 보강했다. `OnMainMenuAlarmRequested` 이벤트도 추가해 WBP가 알림 애니메이션을 직접 재생할 수 있게 했다.
- 2026-08-11: 빠른참여, 검색, 방 코드 참가, 직접 Join 실패 시 메인메뉴 알림 전용 경로로 실패 메시지를 표시하게 했다. 검색 결과가 0개면 `방이 존재하지 않습니다.`를 `MainMenuAlarmText`/`MainMenuAlarmAnimation` 또는 `OnMainMenuAlarmRequested`로 전달한다.
- 2026-08-11: 메인메뉴 알림 표시를 세션 델리게이트 타이밍에만 의존하지 않도록 보강했다. 세션 실패 메시지를 `USnowRumbleSessionSubsystem` pending 알림에 저장하고 `UMainMenuWidget::NativeTick`에서 소비해 `OnMainMenuAlarmRequested`를 호출한다.
- 2026-08-11: 메인메뉴 세션 요청 진행 중 안내를 추가했다. 방 만들기는 `방 생성중...`, 방 찾기는 `방 찾는중...`, 빠른참여/참가는 `방 참가중...`을 `ShowMainMenuAlarm()` 경로로 표시한다.
- 2026-08-11: 닉네임 부적합어 필터를 추가했다. 메인메뉴는 닉네임 입력 확정 시 `적합하지 않은 이름입니다.` 알람을 표시하고 기존 저장 이름으로 입력창을 되돌린다. 로비와 서버 PlayerState도 같은 필터를 사용해 부적합 이름 반영을 차단한다.
- 2026-08-11: C-11 색칠하기 화면 버튼 계약을 구체화했다. `BrushColorButton`은 언리얼 기본 컬러 피커, `BrushSizeButton`은 press 중 휠 크기 조정, `FillBodyColorButton`은 현재 브러시 색 기반 `BodyColor` 전체 칠하기로 동작하며, stroke별 색과 두께를 저장·복제한다.
- 2026-08-11: C-11 커스터마이징 WBP 실제 구조에 맞춰 `PaintMode`가 `CustomizationContentSwitcher` 1번 인덱스의 색칠하기 `DrawPanel`을 열도록 조정했다.
- 2026-08-11: C-11 현재 브러시 색 표시 칸 계약을 추가했다. `BrushColorPreviewBorder` 또는 `BrushColorPreviewImage`를 배치하면 현재 브러시 색을 자동 표시한다.
- 2026-08-11: C-11 마우스 커서 슬롯 계약을 게임 전체 기본 커서로 확장했다. 메인메뉴, 로비/PvP 계열, 커스터마이징 PlayerController BP에 `DefaultMouseCursorWidgetClass`를 지정하면 각 UI 커서 표시 시 기본 커서가 적용되고, 커스터마이징 `PaintMouseCursorWidgetClass`는 색칠하기 원형 커서로 전환된다.
- 2026-08-11: C-11 브러시 색 버튼으로 여는 언리얼 기본 컬러 피커가 버튼 왼쪽에 뜨도록 위치 계산 경로를 추가했다.
- 2026-08-11: C-11 페인트 커서 색상 갱신을 추가했다. 원형 커서 WBP에 `BrushCursorColorBorder` 또는 `BrushCursorColorImage`를 배치하면 현재 브러시 색 변경 시 커서 표시 색도 같은 색으로 바뀐다.
- 2026-08-11: 사용자가 상호작용 가능한 물건 옆에 `E - 게시판` 형식의 안내 위젯 표시를 요청해 C-22를 추가하고 진행중으로 전환했다.
- 2026-08-11: C-22 상호작용 안내 WBP 부모와 캐릭터 로컬 표시 경로를 구현했다. 게시판 후보를 눈덩이보다 우선 표시하고, 대상 bounds 화면 좌표 옆에 `E - 게시판`/`E - 눈덩이` 문구를 띄운다. `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-11: 사용자가 로비와 인게임에서 마우스 휠 업/다운으로 카메라 SpringArm 길이를 조절하는 기능을 요청해 C-23을 추가하고 진행중으로 전환했다.
- 2026-08-11: C-23 마우스 휠 카메라 줌을 구현했다. 로컬 Tick에서 `MouseScrollUp`/`MouseScrollDown`으로 기본 카메라 SpringArm 목표 길이를 최소·최대 범위 안에서 조정한다. C++ 컴파일은 통과했으나 실행 중인 Unreal Editor DLL 잠금으로 최종 링크는 보류됐다.
- 2026-08-11: 사용자가 새 캐릭터 모델 교체와 ABP 애니메이션 슬롯 관리를 Codex에게 위임하길 원해 C-24를 추가하고 진행중으로 전환했다.
- 2026-08-11: C-24 ABP 부모 `USnowRumbleCharacterAnimInstance`를 추가했다. ABP는 캐릭터 상태 변수와 `IdleAnimation` 등 슬롯 프로퍼티를 읽어 새 모델 Skeleton용 애니메이션을 장착할 수 있다. UHT와 C++ 컴파일은 통과했으나 실행 중인 Unreal Editor PID 41016의 DLL 잠금으로 최종 링크는 보류됐다.
- 2026-08-12: C-11 페인트 trace를 기본 몸 머티리얼 slot 0 전용으로 제한했다. `PaintAllowedMaterialIndex`는 기본 0, -1이면 모든 slot 허용이며, `bShowPaintHitDebug`로 hit 컴포넌트·slot·UV를 확인할 수 있다. `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-12: C-11 페인트 trace에 원형 커서 중심 자동 보정을 추가했다. `bUsePaintCursorCenterTraceOffset`이 켜져 있으면 현재 브러시 커서 지름의 절반만큼 trace 위치를 보정하고, `PaintCursorScreenOffset`은 마지막 미세 조정값으로 남긴다. UHT와 C++ 컴파일은 통과했으나 실행 중인 Unreal Editor PID 46944의 DLL 잠금으로 최종 링크는 보류됐다.
- 2026-08-12: C-11 모자 커스터마이징 첫 범위를 추가했다. 캐릭터 `HatMeshComponent` 빈 슬롯과 `CustomizationHatMeshes` 후보 배열을 제공하고, 커스터마이징 UI의 `HatModeButton`/`HatPreviousButton`/`HatNextButton`으로 `HatMeshIndex`를 순환·저장·복제하게 했다. UHT와 C++ 컴파일은 통과했으나 실행 중인 Unreal Editor DLL 잠금으로 최종 링크는 보류됐다.
- 2026-08-12: 사용자가 PvP 내부 아이템 기능의 첫 단계로 선물상자를 요청해 C-25를 추가하고 현재 집중 Task로 전환했다. 첫 범위는 TargetPoint 후보 기반 서버 스폰, 공중 낙하, 전체 알림, `E` 개봉, 랜덤 아이템 이름 확정과 로그/알림 계약까지로 제한한다.
- 2026-08-12: C-25 선물상자 C++ 기반을 구현했다. `AGiftBox`는 빨간색/황금색 등급과 등급별 보상 후보, 낙하·착지·개봉 Blueprint 이벤트를 제공하고, PvP GameMode는 레벨 담당자가 배치한 `TargetPoint` 후보에서 상자를 공중 스폰한다. `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-12: C-25 선물상자 보상 흐름을 즉시 장착에서 아이템 Pickup 스폰 후 `E` 획득으로 변경했다. `AGiftBoxItemPickup`을 추가하고, 선물상자 `TakeDamage()`로 눈덩이 피격 개봉을 지원한다. UHT와 C++ 컴파일은 통과했으나 실행 중인 Unreal Editor DLL 잠금으로 최종 링크는 보류됐다.
- 2026-08-12: C-25 아이템 Pickup이 살짝 둥둥 떠다니도록 `FloatAmplitude`와 `FloatSpeed` 조정값을 추가했다. UHT와 C++ 컴파일은 통과했으나 실행 중인 Unreal Editor DLL 잠금으로 최종 링크는 보류됐다.
- 2026-08-12: C-25 아이템 Pickup 기본 `FloatSpeed`를 5로 변경하고, 선물상자 BP에서 같은 StaticMesh에 등급별 머티리얼을 입힐 수 있도록 `GradeMaterialIndex`, `RedGiftBoxMaterial`, `GoldGiftBoxMaterial`을 추가했다. 상자 개봉/사라짐 이펙트용 `OpenedEffect` Niagara 슬롯도 추가했다. UHT와 C++ 컴파일은 통과했으나 실행 중인 Unreal Editor DLL 잠금으로 최종 링크는 보류됐다.
- 2026-08-12: C-25 아이템 BP 내부 연결용 `ESnowRumbleGiftItemType` enum 계약을 추가했다. `FSnowRumbleGiftBoxReward::ItemType`이 `AGiftBoxItemPickup`에 복제되고, Pickup BP는 `GetItemType()` 또는 `OnItemDataChanged()`에서 기능·외형을 분기할 수 있다. `SnowRumbleEditor Win64 Development` 빌드가 성공했다.

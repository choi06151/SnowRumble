# 최재원(C) 작업 계획

## 운영 방식

- 이 문서는 최재원과 대화하는 Codex가 작성·갱신합니다. 개발자가 직접 편집할 필요는 없습니다.
- Task ID는 고정하고 실제 실행 순서는 이 표의 우선순위로 조정합니다.
- 최재원(C) Codex는 자기 Task와 중앙 `docs/PLANS.md`의 공용 의존성·선점을 함께 관리할 수 있습니다.
- 상세 변경 절차는 `docs/TASK_WRITING.md`의 `대화 기반 자율 계획`을 따릅니다.

## 현재 집중 Task

- [C-05](C-05_round_match_flow.md) 1/3/5 라운드 매치 흐름

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
| 12 | [C-11](C-11_customization_contract.md) | 커스터마이징 데이터 계약 | C-02·기획 결정 | 예정 |
| 13 | [C-13](C-13_revive_contract.md) | 핫팩 부활 계약 | C-06, C-07 | 예정 |
| 14 | [C-14](C-14_spawn_intro_flow.md) | 팀 스폰과 시작 연출 | C-03, C-04 | 예정 |
| 15 | [C-12](C-12_mvp_integration.md) | MVP 최종 통합 | 모든 파트 통합 Task | 예정 |
| 16 | [C-16](C-16_teammate_health_hud.md) | 팀원 HP HUD | C-03·기존 HP UI | 완료 |
| 17 | [C-17](C-17_pvp_start_countdown.md) | PvP 시작 카운트다운 | C-15·기존 HUD·캐릭터 입력 | 진행중 |
| 18 | [C-18](C-18_steam_session_integration.md) | Steam 세션 최종 통합 | LAN 기반 MVP 흐름 안정화·Steam 테스트 환경 | 예정 |
| 19 | [C-19](C-19_text_chat.md) | 텍스트 채팅 | C-03 팀 색·공용 PlayerController | 완료 |

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

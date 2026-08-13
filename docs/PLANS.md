# Plans

## 목적

이 문서는 네 역할 사이의 필수 의존성, 공용 계약, 현재 선점, 병합 순서와 MVP 통합 상태를 관리하는 중앙 보드입니다.

역할 내부 Task 순서와 상태는 각 담당자의 Codex가 아래 계획 문서에서 관리합니다.

| 담당자 | 역할 계획 | 기본 소유 영역 |
| --- | --- | --- |
| 최재원(C) | `Tasks/C/PLAN_C.md` | 공용 네트워크, PvP, 플레이어와 통합 |
| 강혜원(K) | `Tasks/K/PLAN_K.md` | 눈사람 모드 |
| 서유정(S) | `Tasks/S/PLAN_S.md` | UI·아트·모델·눈숲속 |
| 정다영(J) | `Tasks/J/PLAN_J.md` | 얼음 빙하·눈섬 레벨 |

## 운영 원칙

- 개발자는 계획 문서를 직접 작성하지 않고 각 담당자 Codex와 대화해 원하는 결과·오류·우선순위 변화를 전달합니다.
- 각 역할 Codex는 자기 `PLAN_*.md`와 Task를 자율적으로 추가·분할·재정렬할 수 있습니다.
- Task ID는 고정하며 우선순위 변경을 이유로 파일 이름이나 번호를 바꾸지 않습니다.
- 비메인 Codex는 파트 간 영향이 있는 변경을 자기 계획의 `통합 변경 요청`에 기록합니다.
- 최재원(C) Codex만 요청을 반영해 이 중앙 보드의 의존성·선점·병합 순서를 갱신합니다.

## 파트 간 필수 의존성

| 계약 | 제공 Task | 소비 Task·파트 | 필요한 결과 | 상태 |
| --- | --- | --- | --- | --- |
| I-01 기존 기반 | C-01 | 모든 역할 | 재사용 파일·자산과 공용 소유권 | 완료 |
| I-02 세션·방 | C-02 | C-03, S-02 | 방 설정·검색·참가 상태와 요청 | 진행중 |
| I-03 팀·대기방 | C-03 | C-04, C-08, C-14, S-03 | 팀 색 선택·준비·다색 팀 시작 상태 | 진행중 |
| I-04 랜덤 맵·로딩 | C-04 | C-05, S-04, S-10, J | 선택 맵·로딩 인원·시작 결과 | 진행중 |
| I-05 얼음·사망 | C-06 | C-05, C-07, C-13, K-07, S-07 | 얼음 잔여시간·사망·관전·전멸 결과 | 예정 |
| I-06 플레이어 효과 | C-07 | K-01, K-03~K-06, K-08, S-09, S-10, J-02, J-04 | 피해·회복·무적·능력 보정 요청 | 예정 |
| I-07 팀 식별 | C-08 | S-05 | 닉네임·팀 색·이름표용 상태 | 진행중 |
| I-08 눈 전투 | C-09 | K-05, K-06, K-08, S-06 | 눈 제작·조준·충전·피격과 큰 눈 광역 결과 | 예정 |
| I-09 경기 흐름 | C-05 | K-11, S-11, 맵 담당 | 라운드 시간·승수·결과·금색 상자 시점 | 진행중 |
| I-10 커스터마이징 | C-11 | S-01, S-02, S-08 | 외형 저장·복제·적용 상태 | 진행중 |
| I-11 아이템 | C-25, 기존 K-01~K-11 대기 | C-12, S-08, S-09, S-10, J | 선물상자 스폰·개봉·랜덤 보상 확정, 아이템 상태·모델 요구·배치·UI 계약 | 진행중 |
| I-12 레벨·비주얼 | S-12, J-05 | C-12 | 세 맵과 UI·모델·표현 자산 | 예정 |
| I-13 부활 계약 | C-13 | K-07, S-07, C-05 | 핫팩 부활 진행·취소·완료와 무적 결과 | 예정 |
| I-14 팀 등장 | C-14 | S-10, J-02, J-04 | 팀 스폰·시작 제한·팀 소개 상태 | 예정 |
| I-15 맵 환경 압박 | C-05, S-10, J-02, J-04 | C-12 | C-05 게임 속도별 축소 주기·경기 시간·축소 호출 계약, 레벨 담당 맵별 수위·자기장·눈 폭 서버 로직과 배치 | 진행중 |
| I-16 로비 게시판 상호작용 | C-15 | S-03, 사용자 | 게시판 outline, E 입력, 서버 검증 상호작용 이벤트, 카메라 컴포넌트 기반 포커스, 월드 UI 버튼 액션, 팀 색 선택, 매치 시작 로딩창 | 진행중 |
| I-17 Steam 세션 전환 | C-18 | C-02, S-02, 사용자 | LAN fallback을 유지하면서 Steam 세션·Overlay 친구 초대·초대 수락·세션 정리 통합 | 예정 |
| I-18 캐릭터 모델·ABP | C-24 | S-01, S-05, S-08, 사용자 | 새 캐릭터 모델용 ABP 부모, 애니메이션 슬롯 이름, 캐릭터 상태 변수 | 진행중 |
| I-19 눈사람 모드 | K-12~K-14 | C-04, C-05, C-09, S/J 맵 담당, 사용자 | 기존 PvP 맵 재사용, 환경 축소 비활성, 10분 제한시간, 랜덤 눈사람 시작, 감염·전환·승패 결과 | 예정 |

## 세션 개발 정책

- 현재 MVP 구현과 빠른 로컬 검증은 `OnlineSubsystem NULL` 기반 LAN 세션으로 계속 진행한다.
- Steam 출시는 최종 목표로 유지하며 Steam 세션 통합은 C-18에서 별도 최종 Task로 처리한다.
- 앞으로 메인메뉴, 로비, PvP, ESC 메뉴, 친구 초대 UI는 `USnowRumbleSessionSubsystem` 등 공개 세션 계약만 호출하고 LAN/Steam 구현 세부사항에 직접 의존하지 않는다.
- 친구 부르기 버튼은 현재 Blueprint 이벤트와 UI 계약만 유지하고, 실제 Steam Overlay 초대 연결은 C-18에서 처리한다.
- 새 기능이 세션 생성·검색·참가·초대·퇴장·복귀에 영향을 주면 C-18 Steam 전환 영향 여부를 함께 기록한다.

## 통합 관문

| 관문 | 완료 조건 | 상태 |
| --- | --- | --- |
| G-01 기반 인수 | C-01 완료와 기존 진행중 선점 재정리 | 완료 |
| G-02 공용 계약 | C-02~C-09, C-11, C-13, C-14 중 소비 파트에 필요한 계약 완료 | 예정 |
| G-03 파트 통합 | 아이템 담당 재배정 또는 MVP 범위 조정 결정, S-12, J-05 완료 | 예정 |
| G-04 MVP 통합 | C-12 결과 확인 완료 | 예정 |

## 통합 변경 요청 대기열

각 비메인 Codex의 `PLAN_*.md`에 요청이 생기면 C Codex가 이 표에 옮겨 검토합니다.

| 요청 | 요청 역할 | 영향받는 계약·자산 | 결정 | 상태 |
| --- | --- | --- | --- | --- |
| 없음 | - | - | - | - |

## 현재 작업 배정

진행 중 Task가 생기면 최재원(C) Codex가 한 행을 추가합니다. 배정되지 않은 파일과 자산은 수정하지 않습니다.

| Task | 담당자 | 기능 소유자 | 계약 소유자 | 자산 수정자 | 선점 파일·자산 | 상태 |
| --- | --- | --- | --- | --- | --- | --- |
| C-01 | C | 최재원(C) | 최재원(C) | 문서: 최재원(C), 코드·자산: 구현 승인 전 확정 | `Tasks/C/C-01_existing_foundation_migration.md`, `Tasks/C/PLAN_C.md`, 기존 코드·자산 조사 대상은 승인 전 확정 | 완료 |
| C-02 | C | 최재원(C) | 최재원(C) | 문서·C++: 최재원(C), UI 자산: S-02 인계 | `Tasks/C/C-02_session_room_flow.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Online/SnowRumbleSessionSubsystem.*`, `Source/SnowRumble/UI/MainMenuWidget.*`, `Source/SnowRumble/UI/LobbyWidget.*` | 진행중 |
| C-03 | C | 최재원(C) | 최재원(C) | 문서·C++: 최재원(C), 로비 맵 PlayerStart 배치: 사용자/S·J 인계 | `Tasks/C/C-03_random_team_lobby.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Game/SnowRumbleLobbyGameMode.*`, `Source/SnowRumble/Game/SnowRumbleLobbyGameState.*`, `Source/SnowRumble/Game/SnowRumblePlayerState.*`, `Source/SnowRumble/UI/LobbyPlayerController.*` | 진행중 |
| C-04 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), PvP 후보 레벨 자산 지정: 사용자/S/J 인계 | `Tasks/C/C-04_random_map_loading.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Game/SnowRumbleLobbyGameMode.*`, `Source/SnowRumble/Game/SnowRumbleMatchSubsystem_C.*` | 진행중 |
| C-06 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 얼음/사망 표현 UI·VFX: 사용자/S 인계 | `Tasks/C/C-06_freeze_death_spectate.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Player/SnowRumbleHealthComponent.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*` | 진행중 |
| C-05 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 결과 UI/연출: 사용자/S 인계 | `Tasks/C/C-05_round_match_flow.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Game/SnowRumbleGameMode.*`, `Source/SnowRumble/Game/SnowRumbleGameState_C.*`, `Source/SnowRumble/Game/SnowRumbleLobbyGameState.*`, `Source/SnowRumble/Game/SnowRumbleMatchSubsystem_C.*`, `Source/SnowRumble/Game/PodiumGameMode.*`, `Source/SnowRumble/Game/PodiumPlayerController.*`, `Source/SnowRumble/UI/LobbyBoardWidget_C.*`, `Source/SnowRumble/UI/MainHUDWidget.cpp`, `Source/SnowRumble/UI/PodiumWidget.*`, `Source/SnowRumble/Player/SnowRumbleHealthComponent.cpp`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Content/Game/BP_SnowRumblePodiumGameMode.uasset`, `Content/Controller/BP_SnowRumblePodiumController.uasset`, `Content/Maps/L_Podium.umap` | 진행중 |
| C-08 | C | 최재원(C) | 최재원(C) | C++: 최재원(C), 이름표 WBP·그래픽: S-05 인계 | `Tasks/C/C-08_spawn_intro_identity.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Source/SnowRumble/UI/OverheadNameplateWidget_C.*` | 진행중 |
| C-15 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 게시판 Blueprint·맵 배치·로딩 WBP: 사용자/S 인계 | `Tasks/C/C-15_lobby_board_interaction.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Interaction/LobbyInteractionBoard_C.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Source/SnowRumble/UI/*LoadingScreen*`, `Source/SnowRumble/UI/SnowRumblePlayerController.*`, `Source/SnowRumble/Game/SnowRumbleGameMode.*` | 진행중 |
| C-16 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), HUD WBP 배치: 사용자/S 인계 | `Tasks/C/C-16_teammate_health_hud.md`, `Tasks/C/PLAN_C.md`, `Tasks/C/ROLE_C.md`, `docs/PLANS.md`, `Source/SnowRumble/UI/MainHUDWidget.*` | 완료 |
| C-17 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 카운트다운 WBP 표시 배치: 사용자/S 인계 | `Tasks/C/C-17_pvp_start_countdown.md`, `Tasks/C/PLAN_C.md`, `Tasks/C/ROLE_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Game/SnowRumbleGameState_C.*`, `Source/SnowRumble/Game/SnowRumbleGameMode.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Source/SnowRumble/UI/MainHUDWidget.*` | 진행중 |
| C-18 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), Steam App ID·테스트 계정·빌드 환경: 사용자 확인 | `Tasks/C/C-18_steam_session_integration.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Config/DefaultEngine.ini`, `Source/SnowRumble/Online/SnowRumbleSessionSubsystem.*`, `Source/SnowRumble/UI/LobbyEscapeMenuWidget.*`, `Source/SnowRumble/UI/MainMenuWidget.*`, `Source/SnowRumble/UI/LobbyPlayerController.*` | 예정 |
| C-19 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 채팅 WBP 배치·스타일: 사용자/S 인계 | `Tasks/C/C-19_text_chat.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/UI/ChatWidget_C.*`, `Source/SnowRumble/UI/SnowRumblePlayerController.*`, `Source/SnowRumble/UI/LobbyPlayerController.*` | 진행중 |
| C-20 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 로비/HUD 로그 WBP 배치·스타일: 사용자/S 인계 | `Tasks/C/C-20_event_log_ui.md`, `Tasks/C/ROLE_C.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/UI/SnowRumblePlayerController.*`, `Source/SnowRumble/UI/LobbyWidget.*`, `Source/SnowRumble/UI/MainHUDWidget.*`, `Source/SnowRumble/Game/SnowRumblePlayerState.*`, `Source/SnowRumble/Game/SnowRumbleGameMode.*` | 진행중 |
| C-21 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 옵션 WBP 배치·스타일: 사용자/S 인계 | `Tasks/C/C-21_options_menu.md`, `Tasks/C/ROLE_C.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/UI/OptionsWidget_C.*`, `Source/SnowRumble/UI/OptionsKeyBindingRowWidget_C.*`, `Source/SnowRumble/UI/MainMenuWidget.*`, `Source/SnowRumble/UI/MainMenuPlayerController.*`, `Source/SnowRumble/UI/LobbyEscapeMenuWidget.*`, `Source/SnowRumble/UI/LobbyPlayerController.*`, `Source/SnowRumble/UI/SnowRumblePlayerController.*`, `Source/SnowRumble/Player/SnowRumbleUserSettingsSubsystem_C.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*` | 진행중 |
| C-11 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 커스터마이징 레벨과 WBP 버튼·머티리얼 파라미터: 사용자/S 인계 | `Tasks/C/C-11_customization_contract.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/UI/MainMenuWidget.*`, `Source/SnowRumble/UI/MainMenuPlayerController.*`, `Source/SnowRumble/Game/SnowRumblePlayerState.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Source/SnowRumble/Player/SnowRumbleCustomizationData_C.h`, `Source/SnowRumble/Player/SnowRumbleCustomizationSubsystem_C.*`, `Source/SnowRumble/UI/CustomizationPlayerController_C.*`, `Source/SnowRumble/UI/CustomizationWidget_C.*`, `Source/SnowRumble/UI/LobbyWidget.*` | 진행중 |
| C-22 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 안내 WBP 배치·스타일: 사용자/S 인계 | `Tasks/C/C-22_interaction_prompt_ui.md`, `Tasks/C/ROLE_C.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/UI/InteractionPromptWidget_C.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Source/SnowRumble/Snowball/SnowballEquipmentComponent.*`, `Source/SnowRumble/Interaction/LobbyInteractionBoard_C.*` | 진행중 |
| C-23 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 캐릭터 BP 조정값 확인: 사용자/S 인계 | `Tasks/C/C-23_camera_wheel_zoom.md`, `Tasks/C/ROLE_C.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Player/SnowRumbleCharacter.*` | 진행중 |
| C-24 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 새 SkeletalMesh·Skeleton·ABP·캐릭터 BP 연결: 사용자/S 인계 | `Tasks/C/C-24_character_model_anim_contract.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Player/SnowRumbleCharacterAnimInstance_C.*` | 진행중 |
| C-25 | C | 최재원(C) | 최재원(C) | C++·문서: 최재원(C), 선물상자 Blueprint·아이템 Pickup Blueprint·모델·모닥불 Blueprint·맵 TargetPoint 배치: 사용자/S/J 인계 | `Tasks/C/C-25_pvp_gift_box_item_foundation.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `docs/ARCHITECTURE.md`, `docs/PENDING_ISSUES.md`, `Source/SnowRumble/Item/GiftBox_C.*`, `Source/SnowRumble/Item/GiftBoxItemPickup_C.*`, `Source/SnowRumble/Item/GiftItemEffectComponent_C.*`, `Source/SnowRumble/Item/Campfire_C.*`, `Source/SnowRumble/Game/SnowRumbleGameMode.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Source/SnowRumble/Player/SnowRumbleHealthComponent.*`, `Source/SnowRumble/Snowball/SnowballCreationComponent.*`, `Source/SnowRumble/Snowball/SnowballItem.*` | 진행중 |
| K-12 | K | 강혜원(K) | 강혜원(K), 공용 로딩·플레이어 계약: 최재원(C) | 문서·C++·Blueprint: 강혜원(K), 기존 PvP 맵 자산 수정 없음 | `Tasks/K/K-12_snowman_mode_foundation.md`, `Tasks/K/PLAN_K.md`, 눈사람 모드 GameMode/GameState/PlayerController 후보는 구현 승인 전 확정 | 예정 |
| K-13 | K | 강혜원(K) | 강혜원(K), 공용 플레이어·눈 전투 계약: 최재원(C) | 문서·C++·Blueprint: 강혜원(K) | `Tasks/K/K-13_snowman_infection_flow.md`, `Tasks/K/PLAN_K.md`, 눈사람 이동·감염 관련 생성 후보는 구현 승인 전 확정 | 예정 |
| K-14 | K | 강혜원(K) | 강혜원(K), 공용 경기 결과 계약: 최재원(C) | 문서·C++·Blueprint: 강혜원(K), UI 표현 자산은 사용자/S 인계 | `Tasks/K/K-14_snowman_mode_integration.md`, `Tasks/K/PLAN_K.md`, 눈사람 모드 승패·HUD 인계 후보는 구현 승인 전 확정 | 예정 |

## 상태 범례

- `예정`: 아직 시작하지 않은 상태
- `진행중`: 구현·인계·결과 확인 중인 상태
- `대기`: 사용자 결정·외부 계약·오류 해결을 기다리는 상태
- `완료`: 필요한 계약·통합과 결과 확인이 끝난 상태
- `대체`: 다른 Task나 계약으로 범위를 옮긴 상태

## 최근 통합 로그

- 2026-08-11: C-24 캐릭터 모델·ABP 계약을 추가함. 새 ABP는 `USnowRumbleCharacterAnimInstance`를 부모로 사용하고, 캐릭터 상태 변수와 `IdleAnimation` 등 슬롯 프로퍼티에 새 Skeleton용 애니메이션을 장착할 수 있음. UHT와 C++ 컴파일은 통과했으나 실행 중인 Unreal Editor PID 41016의 DLL 잠금으로 최종 링크는 보류됨.
- 2026-08-12: C-11 페인트 trace를 기본 몸 머티리얼 slot 0 전용으로 제한함. `PaintAllowedMaterialIndex`는 기본 0, -1이면 모든 slot 허용이며, `bShowPaintHitDebug`로 hit 컴포넌트·slot·UV를 확인할 수 있음. `SnowRumbleEditor Win64 Development` 빌드가 성공함.
- 2026-08-12: C-11 페인트 trace에 원형 커서 중심 자동 보정을 추가함. `bUsePaintCursorCenterTraceOffset`이 켜져 있으면 현재 브러시 커서 지름의 절반만큼 trace 위치를 보정하고, `PaintCursorScreenOffset`은 마지막 미세 조정값으로 유지함. UHT와 C++ 컴파일은 통과했으나 실행 중인 Unreal Editor PID 46944의 DLL 잠금으로 최종 링크는 보류됨.
- 2026-08-12: 강혜원(K) 담당을 아이템 중심에서 눈사람 모드 중심으로 변경함. 눈사람 모드는 10분 제한시간, 기존 PvP 맵 재사용, 환경 축소 비활성, 랜덤 눈사람 시작, 접촉 감염과 전원 감염/생존자 승리 조건을 기준으로 K-12~K-14 후속 Task에서 개발함.
- 2026-08-12: C-11 모자 커스터마이징 계약을 추가함. `FSnowRumbleCustomizationData::HatMeshIndex`, 캐릭터 `HatMeshComponent`/`CustomizationHatMeshes`, 커스터마이징 WBP `HatModeButton`/`HatPreviousButton`/`HatNextButton`으로 모자 후보를 순환·저장·복제·적용하게 함. UHT와 C++ 컴파일은 통과했으나 실행 중인 Unreal Editor DLL 잠금으로 최종 링크는 보류됨.
- 2026-08-12: PvP 내부 아이템 첫 범위로 C-25 선물상자 계약을 추가함. 기존 대기 상태의 I-11 아이템 계약은 C-25가 선물상자 스폰·개봉·랜덤 보상 확정 기반을 먼저 제공하고, 실제 개별 아이템 효과와 모델·맵 배치는 후속 인계로 처리함.
- 2026-08-12: C-25 선물상자 C++ 기반을 구현함. `AGiftBox`는 빨간색/황금색 등급과 등급별 보상 후보, 낙하·착지·개봉 Blueprint 이벤트를 제공하고, PvP GameMode는 레벨 담당자가 배치한 `TargetPoint` 후보에서 상자를 공중 스폰함. `SnowRumbleEditor Win64 Development` 빌드가 성공함.
- 2026-08-12: C-25 선물상자 보상 흐름을 아이템 Pickup 스폰 후 `E` 획득 구조로 변경함. `AGiftBox::TakeDamage()`로 눈덩이 피격 개봉을 지원하고, `AGiftBoxItemPickup`이 획득 확정과 로그/알림을 담당함. UHT와 C++ 컴파일은 통과했으나 실행 중인 Unreal Editor DLL 잠금으로 최종 링크는 보류됨.
- 2026-08-12: C-25 아이템 효과 1차 구현을 추가함. `UGiftItemEffectComponent`가 핫팩 보유, 즉시 회복, 에너지 드링크 5초 무적, 부츠 이동속도, 패딩 피해 감소, 장갑 눈 제작 시간 감소, 눈오리 제작기 눈덩이 피해 증가, 황금 붕어빵 지속 회복, 눈삽 내구도와 모닥불 키트 보유 수를 서버 권한으로 관리함. UHT와 C++ 컴파일은 통과했으나 실행 중인 Unreal Editor DLL 잠금으로 최종 링크는 보류됨.
- 2026-08-12: C-25 핫팩과 모닥불 키트 정책을 사용자 결정에 맞게 변경함. 일반 핫팩은 1개까지만 장착하고, 황금 핫팩은 획득 즉시 같은 팀의 얼음 상태 아군을 50% HP로 부활시키며, 모닥불 키트는 보유하지 않고 `ACampfire`를 캐릭터 앞에 즉시 설치함. `SnowRumbleEditor Win64 Development` 빌드가 성공함.
- 2026-08-12: C-25 장비 외형 슬롯을 추가함. 캐릭터는 부츠, 장갑, 패딩, 핫팩, 눈삽, 눈오리 제작기용 고정 StaticMeshComponent를 갖고 `UGiftItemEffectComponent` 복제 상태에 따라 표시함. UHT는 통과했으나 현재 시스템 page file 부족 `C3859/C1076`으로 C++ 컴파일 전 PCH 생성 단계에서 빌드가 보류됨.
- 2026-08-12: C-24 애니메이션 장착 자세 계약을 보강함. `ESnowRumbleHeldAnimationState`/`HeldAnimationState`로 맨손, 작은 눈덩이, 큰 눈덩이, 눈삽, 눈오리 제작기를 ABP에서 한 값으로 분기할 수 있고, 빠른 연결용 `SnowShovelHoldAnimation`, `SnowDuckMakerHoldAnimation` 슬롯을 추가함.
- 2026-08-12: C-24/C-25 애니메이션 연동을 보강함. 선물상자 열기와 선물 아이템 획득 성공은 `bIsInteractingWithItem`/`ItemInteractionAnimation`으로, 실제 HP 피해 피격은 `bIsHitReacting`/`HitReactAnimation`으로 ABP가 분기할 수 있음.
- 2026-08-12: C-24/C-25 애니메이션 연동 변경은 `git diff --check`와 UHT/C++ 컴파일을 통과함. 최종 링크는 실행 중인 Unreal Editor DLL 잠금 `LNK1104`로 보류됨.
- 2026-08-13: C-24 ABP 부모를 이동·상체·전체 몸 액션 3계층 상태 구조로 확장함. `LocomotionAnimState`, `UpperBodyAnimState`, `FullBodyAnimState`와 override helper를 통해 ABP가 조준+이동, 스프린트+장착 같은 조합을 enum blend와 상체 레이어로 처리하게 함. C++ 컴파일은 통과했으나 실행 중인 Unreal Editor DLL 잠금으로 최종 링크는 보류됨.
- 2026-08-13: C-24 Class Defaults 애니메이션 슬롯 계약을 제거함. `GetPrimaryAnimation()`과 `IdleAnimation` 등 슬롯 프로퍼티는 삭제하고, 새 ABP는 Anim Graph의 상태별 Sequence Player에 애니메이션 에셋을 직접 연결하는 구조로 확정함.
- 2026-08-13: C-24 one-shot 애니메이션 trigger 계약을 추가함. `ESnowRumbleCharacterAnimTrigger`와 `OnAnimationTriggerRequested`로 눈덩이 줍기/던지기, 아이템 상호작용, 피격 반응을 서버 확정 후 모든 화면의 AnimBP에 전달함. UHT와 C++ 컴파일은 통과했으나 실행 중인 Unreal Editor DLL 잠금으로 최종 링크는 보류됨.
- 2026-08-12: C-11 색칠하기 브러시 색 선택을 기본 컬러 피커에서 고정 팔레트 버튼으로 변경함. WBP는 빨강, 주황, 노랑, 초록, 파랑, 남색, 보라, 검정, 하양 버튼만 배치하고, 선택된 색은 Pressed 스타일로 유지됨.
- 2026-08-12: C-11 고정 팔레트 버튼 변경은 `git diff --check`와 `SnowRumbleEditor Win64 Development` 빌드를 통과함.
- 2026-08-12: C-11 팔레트 버튼 클릭 시 실제 브러시 색이 해당 버튼의 WBP 스타일 Normal Tint와 BackgroundColor를 기준으로 정해지게 함.
- 2026-08-12: C-11 버튼 tint 기반 브러시 색 변경은 `git diff --check`와 UHT/C++ 컴파일을 통과함. 최종 링크는 실행 중인 Unreal Editor DLL 잠금 `LNK1104`로 보류됨.
- 2026-08-13: C-05 정규 라운드 최종 공동 1등 처리로 단판 승부 타이브레이커를 추가함. `ASnowRumbleGameMode::TiebreakerTravelUrl`에 전용 PvP 맵을 지정하면 공동 1등 팀끼리만 최종 판정과 피해 적용 대상이 되며, 같은 GameMode/Controller/HUD를 사용함. HUD는 `단판승부`와 경기 시간만 표시하고 맵 축소는 비활성화하며, 선물상자 아이템 스폰은 기존 PvP와 동일하게 유지함. UHT와 C++ 컴파일은 통과했으나 실행 중인 Unreal Editor DLL 잠금 `LNK1104`로 최종 링크는 보류됨.
- 2026-08-13: C-05 단판 승부 비동점 팀은 관전자 상태로 전환함. `ASnowRumbleCharacter::SetTiebreakerSpectatorFromServer()`가 관전자 상태를 복제하고, 관전자는 이동·상호작용·아이템 사용과 충돌 간섭이 막히며 `RefreshTiebreakerSpectatorViewTarget()`으로 경기 참가자 시점을 봄. `git diff --check`와 `SnowRumbleEditor Win64 Development` 빌드를 통과함.
- 2026-08-13: C-05 매치 종료 후 포디움 레벨 이동을 추가함. `APodiumGameMode`가 현재 매치 참가 팀 색만 기준으로 1~3등을 산정하고 `Podium_Team1`/`Podium_Team2`/`Podium_Team3` PlayerStart에 배치하며, `UPodiumWidget`에 결과 문구를 전달한 뒤 10초 후 `ResetPvPMatch()`와 로비 복귀를 수행함. `git diff --check`와 `SnowRumbleEditor Win64 Development` 빌드가 성공함.
- 2026-08-13: 레벨별 책임 분리를 위해 포디움 전용 `APodiumGameMode`/`APodiumPlayerController`에서 PvP GameMode/PlayerController 상속을 제거함. 포디움은 결과 표시·배치·로비 복귀만 담당하고, PvP HUD·채팅·ESC·라운드 입력 복구 흐름은 섞이지 않게 함. 커스터마이징 커서 숨김 문제도 공용 캐릭터 입력 복구 경로에서 `ACustomizationPlayerController`를 제외해 수정함. C++ 컴파일은 통과했으나 실행 중인 Unreal Editor DLL 잠금으로 최종 링크는 보류됨.

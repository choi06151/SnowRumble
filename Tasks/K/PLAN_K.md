# 강혜원(K) 작업 계획

## 운영 방식

- 이 문서는 강혜원과 대화하는 Codex가 작성·갱신합니다. 개발자가 직접 편집할 필요는 없습니다.
- Task ID는 고정하고 실제 실행 순서는 이 표의 우선순위로 조정합니다.
- K Codex는 `Tasks/K/` 안의 계획·Task만 직접 갱신하고 공용 계약 변경은 `통합 변경 요청`에 기록합니다.
- 상세 변경 절차는 `docs/TASK_WRITING.md`의 `대화 기반 자율 계획`을 따릅니다.

## 현재 집중 Task

- [K-14](K-14_snowman_mode_integration.md) 눈사람 모드 통합

## 개발 스타일

- 상태: C++ 기반 개발, Blueprint로 다듬기
- 확인일: 2026-08-12
- 적용 기준: 서버 권한, 복제 상태, 눈사람 모드 GameMode/GameState/PlayerController, 감염 판정과 승패 수명주기는 C++ 중심으로 설계한다.
- Blueprint 책임: 눈사람 모델·애니메이션·VFX·UI·맵 배치 연결, 조정값과 표현 다듬기에 집중한다.
- Task 조정 기준: K Task는 C++에서 권한과 상태 원본을 먼저 고정하고, Blueprint는 표현·자산 연결과 디자이너 조정값을 담당하게 인계한다.

## 작업 대기열

| 우선순위 | Task | 결과 | 필수 선행 | 상태 |
| --- | --- | --- | --- | --- |
| 1 | [K-12](K-12_snowman_mode_foundation.md) | 눈사람 모드 기반 | C-04, C 공용 로딩·모드 선택 계약 확인 | 완료 |
| 2 | [K-13](K-13_snowman_infection_flow.md) | 눈사람 이동과 감염 | K-12, C-09 또는 눈덩이 피격 계약 확인 | 완료 |
| 3 | [K-14](K-14_snowman_mode_integration.md) | 눈사람 모드 통합 | K-12, K-13, 결과 UI·로딩 인계 확인 | 진행중 |
| 4 | [K-01](K-01_item_foundation.md) | 아이템 기반 | 재배정 결정 필요 | 대기 |
| 5 | [K-03](K-03_consumables.md) | 회복·무적 소비 아이템 | 재배정 결정 필요 | 대기 |
| 6 | [K-04](K-04_wearable_equipment.md) | 착용 장비 | 재배정 결정 필요 | 대기 |
| 7 | [K-02](K-02_gift_box.md) | 선물상자 기본 기능 | 재배정 결정 필요 | 대기 |
| 8 | [K-07](K-07_hotpacks.md) | 일반·황금 핫팩 | 재배정 결정 필요 | 대기 |
| 9 | [K-05](K-05_snow_shovel.md) | 눈삽 | 재배정 결정 필요 | 대기 |
| 10 | [K-06](K-06_snow_duck_maker.md) | 눈오리 제작기 | 재배정 결정 필요 | 대기 |
| 11 | [K-08](K-08_campfire.md) | 모닥불 | 재배정 결정 필요 | 대기 |
| 12 | [K-09](K-09_golden_rewards.md) | 황금 보상 | 재배정 결정 필요 | 대기 |
| 13 | [K-11](K-11_gift_box_round_schedule.md) | 선물상자 라운드 스케줄 연결 | 재배정 결정 필요 | 대기 |
| 14 | [K-10](K-10_item_integration.md) | 아이템 통합 | 재배정 결정 필요 | 대기 |

## 통합 변경 요청

- 아이템 Task K-01~K-11은 K의 현재 담당 범위에서 제외되었으므로, MVP 아이템을 계속 진행하려면 담당자 재배정 또는 범위 축소 결정이 필요하다.
- K-14에서 눈사람 모드 최소 눈덩이 피격 규칙을 연결했다. Normal끼리의 눈덩이 HP 피해는 무효화하고, Normal이 Snowman을 맞추면 HP 피해 대신 10초 기절을 적용한다. 기절 중 추가 피격은 기존 기절 타이머를 초기화하고 새 10초 기절로 갱신한다. 전용 VFX와 별도 밸런스 값은 후속 K/S 작업에서 조정한다.
- K-14에서 공용 `UHealthBarWidget`에 눈사람 모드 감지 시 `Collapsed` 예외를 추가했다. HUD의 다른 PvP UI는 유지하고 HP 바 위젯만 숨기는 목적이며, 공용 UI 파일 변경이므로 C 통합 검토가 필요하다.

## 계획 변경 기록

- 2026-08-07: 새 GDD와 4인 구조를 기준으로 최초 대기열 작성.
- 2026-08-07: Task 분할 재검토에 따라 K-02를 선물상자 기본 기능으로 축소하고, 라운드 타이밍 연결을 K-11로 분리.
- 2026-08-07: 최초 Task 시작 전 개발 스타일을 확인하고 Task 대기열을 스타일에 맞게 재검토하도록 계획 항목 추가.
- 2026-08-12: 최재원(C)의 회의 후 분담 변경 요청에 따라 K 담당을 아이템 중심에서 눈사람 모드 중심으로 변경했다. 눈사람 모드는 10분 제한시간, 기존 PvP 맵 재사용, 환경 축소 비활성, 랜덤 눈사람 시작, 접촉 감염, 생존자/전원 감염 승패를 기준으로 K-12~K-14에 분리했다. 기존 K-01~K-11 아이템 Task는 재배정 결정 전까지 대기로 전환했다.
- 2026-08-12: 강혜원(K)의 개발 스타일을 C++ 기반 개발, Blueprint로 다듬기로 확정했다. 눈사람 모드의 서버 권한·복제 상태·감염·승패 원본은 C++ 중심으로 두고, Blueprint는 표현·자산 연결·조정값 다듬기를 담당한다.
- 2026-08-12: K-12 시작 전 구현 기준을 확정했다. 기존 로비 게시판의 PvP / 눈사람 모드 선택 흐름을 사용하고, 기존 PvP 후보 맵 전체를 재사용하며, 눈사람 모드 전용 GameMode 구조와 GameMode 쪽 환경 축소 시작 호출 비활성을 우선 검토한다. C 공용 계약 변경이 필요하면 구현 전에 보고한다.
- 2026-08-12: K-12 구현을 승인받아 진행중으로 전환했다. PvP 흐름은 유지하고 `LobbyMode == Snowman`일 때만 Snowman 전용 GameMode travel 분기를 추가하는 유지안으로 진행한다.
- 2026-08-12: K-12 첫 구현을 추가했다. `ASnowmanModeGameMode`/`ASnowmanModeGameState`가 10분 제한시간과 로딩창 닫기를 독립 처리하고, 로비 GameMode는 `LobbyMode == Snowman`일 때만 기존 PvP 후보 맵을 Snowman 전용 GameMode override로 travel한다. 현재 환경에 UE 5.8 빌드 경로가 없어 최종 빌드는 사용자 확인이 필요하다.
- 2026-08-13: K-12 빌드 후 눈사람 모드에서 플레이어가 스폰되지 않는 문제를 반영했다. `ASnowmanModeGameMode`에 기존 PvP GameMode와 같은 PlayerStart 중복 회피와 분산 스폰 보정 경로를 추가했으며, 재빌드와 Listen Server 수동 확인이 필요하다.
- 2026-08-13: 눈사람 모드 스폰 성공 후 PvP처럼 3초 뒤 실제 시작되길 원한다는 확인을 반영했다. `ASnowmanModeGameState`에 시작 카운트다운 복제 상태와 표시 함수를 추가하고, `ASnowmanModeGameMode`는 로딩 완료 후 3초 카운트다운 뒤 10분 제한시간을 시작하게 했다. 시작 전 이동·시야 입력 잠금은 Snowman GameMode가 PlayerController 입력 잠금으로 처리한다.
- 2026-08-13: 눈사람 모드 HUD 표시 요구를 기존 PvP HUD 재사용 기준으로 정정했다. `StartCountdownText`는 시작 카운트다운, `MatchElapsedTimeText`는 시작 후 진행 시간을 표시하며, 별도 `SnowmanTimerText`는 필요 없도록 `MainHUDWidget` 표시 분기를 조정했다.
- 2026-08-13: 눈사람 모드 시작 전 이동 잠금은 되지만 시야 회전이 계속되는 문제를 반영했다. `ASnowRumbleCharacter`의 입력 잠금 확인이 `ASnowmanModeGameState::IsSnowmanModeInputLocked()`도 보게 해 `Look()` 입력을 차단한다.
- 2026-08-13: 사용자가 K-12 눈사람 모드 기반 결과 확인을 완료했다. 눈사람 모드 진입, 스폰, 3초 시작 카운트다운, 이동·시야 입력 잠금과 해제, 진행 시간 표시, 환경 축소 비활성 기반을 확인했으므로 K-12를 완료로 전환하고 현재 집중 Task를 K-13으로 넘겼다.
- 2026-08-13: K-13 구현 방향을 확정하고 진행중으로 전환했다. 감염 대기는 10초, 눈사람 속도는 일반 플레이어 1.25배, 치료제와 눈덩이 피격 효과는 K-13 범위 밖으로 둔다. 눈덩이 피격 효과는 C-09 및 공용 눈 전투 계약 이후 연결 요청으로 남긴다.
- 2026-08-13: K-13 눈사람 구현 방향을 전용 BP/Pawn 교체 구조로 정정했다. `ASnowmanModeSnowmanCharacter`는 기존 캐릭터 기반 카메라/HUD/PlayerState 연동을 재사용하되 이동, 시야 회전, 점프만 바인딩하고 눈덩이 제작·줍기·조준·투척·상호작용·스프린트는 허용하지 않는다. Snowman GameMode는 시작 눈사람과 감염 완료 플레이어를 `SnowmanCharacterClass`로 교체한다.
- 2026-08-13: K-13 실행 중 시작 눈사람이 간헐적으로 전용 BP로 전환되지 않거나 접촉 감염이 누락되는 문제를 반영했다. Snowman GameMode는 PlayerState/Controller/Pawn 준비가 늦으면 시작 눈사람 초기화를 재시도하고, 전용 Pawn 전환 성공 전에는 초기화 완료로 처리하지 않는다. 접촉 감염은 캡슐 반경을 포함한 2D 거리 판정으로 보강했다.
- 2026-08-13: K-13 접촉 감염이 여전히 누락되는 실행 결과를 반영했다. 감염 판정은 PlayerController 기반 Pawn 탐색 대신 월드 Character Actor 스캔으로 눈사람/일반 플레이어를 수집하게 바꿨고, 이미 감염 대기인 플레이어에 대한 중복 접촉은 GameState에서 false로 무시한다.
- 2026-08-13: K-13 접촉 감염이 간헐적으로 실패하는 실행 결과를 추가 반영했다. 감염 판정 후보의 PlayerState 해석은 Pawn 복제값보다 Controller의 PlayerState를 우선 사용해 전용 Pawn 교체 직후 타이밍 차이를 줄인다.
- 2026-08-13: K-13 실행 로그에서 접촉 거리 판정은 통과하지만 감염 시작이 false로 반환되는 문제를 확인했다. Snowman GameState의 참가자 Entry 검색은 PlayerState 포인터 외에 PlayerId/UniqueId fallback을 사용해 같은 플레이어 매칭 실패를 줄인다.
- 2026-08-14: K-13 결과 확인 중 감염 완료 후 눈사람 BP로 전환된 캐릭터가 땅에 박혀 움직이지 못하는 문제를 반영했다. Snowman GameMode는 전환 스폰 시 기존 Pawn의 발 위치와 바닥 trace를 기준으로 새 눈사람 Capsule 높이를 보정하고, collision handling을 `AdjustIfPossibleButAlwaysSpawn`으로 변경한다.
- 2026-08-14: 사용자가 K-13 결과 확인을 완료했다. 랜덤 시작 눈사람, 접촉 감염 대기, 10초 뒤 눈사람 전환, 전환 후 이동, 클라이언트 상태 동기화를 확인했고, 땅에 박혀 생성되는 문제는 테스트 횟수가 많지는 않지만 해결된 것으로 보인다고 판단했다. K-13을 완료로 전환하고 다음 집중 Task를 K-14로 넘긴다.
- 2026-08-14: 사용자가 다음 Task 진행을 요청해 K-14를 진행중으로 전환했다. K-14는 눈사람 전원 감염 승리, 제한시간 종료 시 일반 플레이어 생존 승리, 결과 상태 복제와 로비 복귀 인계를 다룬다. 구현 전 결과 후 복귀 시간, 결과 문구, 제한시간 종료 직전 감염 대기 플레이어 판정을 확정한다.
- 2026-08-19: K-14 확인 중 결과 후 로비 복귀가 로비 대기 상태가 아니라 눈사람 모드 시작처럼 3-2-1 표시를 띄우는 문제를 반영했다. Snowman GameMode 복귀 travel은 로비 GameMode를 명시하고 absolute travel로 실행해 이전 눈사람 `?game=` 옵션이 남지 않게 한다.
- 2026-08-21: K-14 확인 중 초기 눈사람이 초기 스폰/RestartPlayer 경로에서 인간 Pawn으로 생성되는 문제와 컨트롤러 소실 Pending cleanup 경고 반복 문제를 반영했다. Snowman GameMode는 초기 역할이 `Snowman`이면 `SnowmanCharacterClass`를 기본 Pawn으로 반환하고, 컨트롤러 없는 Pending 참가자는 Entry를 삭제하지 않고 Pending 상태만 1회 해제해 참가자 목록 유실과 반복 로그를 막는다.
- 2026-08-14: `master`에 K 브랜치를 병합한 뒤 UE unity build에서 `SnowmanModeGameMode_K.cpp`와 `SnowRumbleGameMode.cpp`의 익명 namespace helper 이름이 충돌해 컴파일이 실패하는 문제를 수정했다. K 소유 helper를 `MakeSnowmanModeRandomHorizontalOffset`으로 변경했고, `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-21: K-14 눈사람 모드 스폰을 기존 PvP 모드 PlayerStart 랜덤 스폰 로직과 맞췄다. PlayerStart 후보 랜덤 선택은 유지하고, 주변 분산 위치에 바닥 trace와 캡슐 점유 검사를 적용하며 기본 인간 Pawn과 `SnowmanCharacterClass` 중 더 큰 캡슐 기준으로 안전 위치를 고른다.
- 2026-08-21: K-14 눈사람 모드 전용 눈덩이 피격 규칙을 추가했다. Snowman GameState가 있는 월드에서 Normal이 Normal을 눈덩이로 맞추면 HP 피해를 무시하고, Normal이 Snowman을 맞추면 HP 피해 대신 복제되는 눈사람 기절을 적용한다. 눈사람 모드 HUD에서는 로컬/타 플레이어 체력바를 숨기며, PvP 모드의 기존 눈덩이 대미지와 체력바는 유지한다.
- 2026-08-21: K-14 눈사람 모드 종료 흐름을 기존 PvP 포디움 이동과 연결했다. Snowman GameMode는 결과 텍스트를 MatchSubsystem 포디움 override로 저장하고 `/Game/Maps/L_Podium?listen`으로 이동하며, PodiumGameMode는 override가 있으면 PvP 팀 승수 대신 눈사람 승리/참가자/생존자 정보를 위젯에 전달한다.
- 2026-08-21: K-14 눈사람 Pawn 전환 안정성과 스폰 직후 감염 버그를 보완했다. 눈사람 전환은 새 Pawn 스폰과 Possess 성공 확인 뒤 기존 Pawn을 제거하며, 스폰/경기 시작/전환 직후 `SpawnInfectionGraceSeconds` 기본 3초 동안 접촉 감염 시작을 무시한다.
- 2026-08-26: 사용자 요청에 따라 K-14의 눈사람 모드 포디움 결과 override를 제거했다. Snowman GameMode는 결과 표시 후 로비로 복귀하고, MatchSubsystem/PodiumGameMode의 override API와 분기를 삭제해 PvP 포디움 기본 동작과 분리했다. C 소유 공용 결과 파일 변경이 포함되어 통합 검토가 필요하다.
- 2026-08-26: 사용자 요청에 따라 눈사람 접촉 감염을 Pending 없이 즉시 전환 기준으로 바꿨다. `ASnowmanModeGameMode::UpdateSnowmanInfectionFlow()`는 아이템 생성·지연 감염 처리 없이 서버에서 역할, 컨트롤러, Pawn, CapsuleComponent, 전환 중 상태를 검사하고 `BP_SnowmanCharacter_K` 전환과 Snowman 역할 복제를 바로 확정한다.
- 2026-08-26: 사용자 요청에 따라 눈사람 모드 눈덩이 피격 기절을 10초로 확정하고 연속 피격 예외 처리를 보강했다. `ASnowmanModeSnowmanCharacter`는 피격 때 기존 스턴 타이머를 지운 뒤 새 10초 타이머를 걸며, 스턴 중 `DisableMovement()`와 `StopJumping()`으로 이동과 점프를 막는다.
- 2026-08-26: 눈사람 기절 UI 연동을 위해 `ASnowmanModeSnowmanCharacter::IsSnowballHitStunned()`와 `GetSnowballHitStunSecondsRemaining()` BlueprintPure 조회 함수를 추가했다. 남은 시간은 복제된 `SnowballHitStunEndServerTime`과 서버 시각 기준으로 계산한다.
- 2026-08-27: 사용자 요청에 따라 눈사람 모드 전용 포디움 결과를 보정했다. K 전용 포디움 컨트롤러가 부모 PvP 우승 UI를 생성하지 않게 하고 `/Game/WBP/WBP_PodiumWinnerWidget_Snowman_K`를 기본 결과 WBP로 사용하게 했으며, 결과 문구를 `눈사람팀 우승`/`사람팀 우승`으로 변경했다. 눈사람팀 승리는 모든 플레이어, 사람팀 승리는 생존자만 배치하도록 수정했다.
- 2026-08-27: 사용자 요청에 따라 눈사람 모드 플레이 중 HP 바 UI만 보이지 않도록 `UHealthBarWidget`에 `ASnowmanModeGameState` 기반 `Collapsed` 예외를 추가했다. 기존 `UMainHUDWidget`의 다른 PvP UI 표시 분기는 유지한다.

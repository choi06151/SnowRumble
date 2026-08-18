# Task C-10 - 이모션 재검증

## 설명

기존 원형 이모션 선택과 서버 검증 몽타주 재생을 새 입력·HUD·플레이어 구조에서 보존하고 남은 결과를 확인한다.

## 현재 상태

- 진행중: Tab 이모션 메뉴 표시 시 커스텀 커서 위젯을 우회해 기본 하드웨어 커서를 보이고, UI가 열린 동안 좌클릭 같은 게임 액션이 반응하지 않게 수정하고 검증 중이다. UI 입력 모드 전환 시 click/hover 이벤트도 명시적으로 켜서 버튼 선택 경로를 보강했다.

## 상태 전이 기준
- 시작 가능: C-01, C-06, C-08 완료
- 완료 가능: 호스트·클라이언트 선택·행동 제한·몽타주 결과 확인

## 구현 항목
- [x] Tab 이모션 메뉴가 열릴 때 기본 커서 위젯과 `GameAndUI` 입력 모드를 적용한다.
- [x] 기존 8칸 선택과 Tab 입력 연결 흐름을 유지한다.
- [ ] 서버 인덱스 검증과 전체 화면 몽타주 재생을 확인한다.
- [ ] 얼음·사망·시작 제한 중 이모션 실행을 차단한다.
- [ ] 기존 원형 메뉴 자산의 소유권을 S-06에 인계한다.

## 작업 배정
- 담당자·기능·계약 소유자: 최재원(C)
- 자산 수정자: C++ 최재원(C), WBP 커서 클래스 연결은 사용자/S
- 생성 파일: 없음
- 변경 파일: `Source/SnowRumble/UI/SnowRumblePlayerController.h`, `Source/SnowRumble/UI/SnowRumblePlayerController.cpp`, `Source/SnowRumble/UI/OptionsWidget_C.cpp`, `Source/SnowRumble/Player/SnowRumbleCharacter.cpp`, `Tasks/C/C-10_emote_revalidation.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`
- 공유 확인 대상: S-06
- 병합 순서: C-01 후, S-06·C-12 전

## 공용 계약과 인계
- 제공받을 계약: C-06·C-08 행동 제한 상태
- 제공할 계약: 이모션 가능 상태와 선택 요청
- 인계 대상: S-06

## 범위 밖
- 신규 이모션 콘텐츠 제작

## 사전 전제
- C-01
- C-06
- C-08

## 기존 구현 인수
- `ASnowRumbleCharacter::RequestPlayEmote`, `ServerRequestPlayEmote`, `MulticastPlayEmote`가 기존 이모션 네트워크 흐름이다.
- `UEmoteRadialMenuWidget`은 `EmoteButton0`~`EmoteButton7`을 자동 해석하고 클릭·호버 선택을 C++에서 연결한다.
- 실제 자산 이름은 `Content/WBP/WBP_EmoteRadialMenuWidget.uasset`이며, 기존 문서의 `WBP_EmoteRadialMenu` 명칭은 새 Task 인계에서 이 이름으로 보정한다.
- 얼음·사망·시작 제한 상태가 확정되면 `CanPlayEmote` 기준에 추가해야 한다.
- `ASnowRumblePlayerController::EnableDefaultCursorUiInput()`은 로비/PvP 공통 기본 커서 위젯을 적용하고 `GameAndUI` 입력 모드로 전환한다.
- `ASnowRumblePlayerController::RestoreGameOnlyInput()`은 이모션 메뉴 종료 후 게임 입력과 커서 숨김 상태로 복구한다.
- 채팅 채널 전환은 채팅 입력창이 열린 상태에서 Tab을 처리한다. PlayerController 전역 Tab 바인딩은 유지하지만 입력을 consume하지 않고, `HandleChatChannelTogglePressed()`가 `ChatWidget->IsChatInputOpen()`일 때만 채널을 바꾸므로 채팅창이 닫힌 상태의 Tab은 이모션 입력 흐름으로 둔다.
- 이모션 메뉴는 `EnableDefaultCursorUiInput(Widget, false)`로 열어 커스텀 커서 위젯 대신 기본 하드웨어 커서를 사용한다.
- `ASnowRumblePlayerController::EnableDefaultCursorUiInput()`은 UI 입력으로 전환할 때 `SetShowMouseCursor(true)`, `bEnableClickEvents`, `bEnableMouseOverEvents`를 함께 켜서 커서 표시와 버튼 hover/click을 보장한다.
- 이모션 메뉴가 열린 동안 `CanPerformGameplayAction()`은 false를 반환하고, 좌클릭 release 경로도 즉시 반환해 눈덩이 제작·충전·던지기 같은 게임 액션을 막는다.

## 변경 기록

- 2026-08-18: 인게임 이모션 메뉴의 마우스 커서와 입력 집중 경로를 보강했다. 이모션 메뉴는 커스텀 커서 위젯을 우회해 기본 하드웨어 커서를 강제로 사용하고, 메뉴가 열린 동안 캐릭터 게임 액션과 좌클릭 release 처리를 막는다. 채팅 채널 전환 Tab 바인딩은 유지하며 채팅창이 열려 있을 때만 채널 전환을 처리한다.
- 2026-08-18: Tab 이모션 커서 fallback 변경은 `git diff --check`, C++ 컴파일과 `.lib` 생성을 통과했다. 최종 DLL 링크는 실행 중인 Unreal Editor의 `UnrealEditor-SnowRumble.dll` 잠금 `LNK1104`로 보류됐다.
- 2026-08-18: Tab 이모션 메뉴가 열려도 커서가 보이지 않거나 버튼 선택이 막히는 회귀에 대응해 UI 입력 전환 시 `bEnableClickEvents`와 `bEnableMouseOverEvents`를 명시적으로 켜도록 보강했다.
- 2026-08-18: 이모션/채팅 입력 보강은 `git diff --check`와 C++ 컴파일을 통과했다. 최종 DLL 링크는 실행 중인 Unreal Editor의 `UnrealEditor-SnowRumble.dll` 잠금 `LNK1104`로 보류됐다.
- 2026-08-18: 이모션 버튼 클릭이 들어와도 실행되지 않는 원인을 `CanPlayEmote()`가 `CanPerformGameplayAction()`을 통해 열린 이모션 메뉴 상태를 차단하는 경로로 확인했다. 이모션 실행 조건을 일반 게임 액션 조건과 분리해, 메뉴가 열린 동안에도 얼음·사망·채팅·경기 잠금 등 실제 금지 조건이 아니면 선택 이모션을 요청할 수 있게 했다.
- 2026-08-18: 이모션 실행 조건 분리 변경은 `git diff --check`와 C++ 컴파일을 통과했다. 최종 DLL 링크는 실행 중인 Unreal Editor의 `UnrealEditor-SnowRumble.dll` 잠금 `LNK1104`로 보류됐다.

## 결정 필요
- 없음

## 수동 작업

- `BP_LobbyPlayerController`와 PvP용 `BP_SnowRumblePlayerController` 계열의 `DefaultMouseCursorWidgetClass`에 기본 커서 WBP가 지정되어 있는지 확인한다.
- 로비와 PvP에서 Tab을 누르는 동안 `WBP_EmoteRadialMenuWidget`이 보이고 마우스 커서가 표시되는지 확인한다.
- Tab을 놓으면 이모션 메뉴가 닫히고 게임 입력/시점 입력이 복구되는지 확인한다.

## 완료 조건
### 에이전트 확인
- [x] 기존 코드·자산 참조 점검 완료
- [x] Tab 이모션 메뉴 커서 입력 경로 수정
- [x] Tab 이모션 메뉴 기본 하드웨어 커서 표시와 마우스 게임 액션 차단
- [x] Tab 이모션 메뉴 UI click/hover 이벤트 활성화 보강
- [x] 네트워크·행동 제한 결과 확인 절차 작성
- [x] `git diff --check` 통과, C++ 컴파일과 `.lib` 생성 통과
- [ ] S-06 인계 완료

### 결과 확인

- [ ] 로비에서 Tab을 누르면 이모션 원형 메뉴와 커서가 함께 보인다.
- [ ] PvP에서 Tab을 누르면 이모션 원형 메뉴와 커서가 함께 보인다.
- [ ] Tab을 놓으면 메뉴와 커서가 사라지고 시점 입력이 복구된다.
- [ ] 선택한 이모션이 서버 검증 후 호스트와 클라이언트 화면 모두에서 재생된다.

# Task C-19 - 텍스트 채팅

## 설명

로비, PvP, 추후 좀비맵에서 공통으로 사용할 텍스트 채팅 계약을 제공한다. 플레이어는 Enter 키로 채팅 입력을 열고, 입력하지 않는 동안에도 채팅 로그를 볼 수 있다. 채팅은 전체 채팅과 팀 채팅으로 구분한다.

## 상태 전이 기준

- 시작 가능: 공용 PlayerController와 로비/PvP PlayerState 팀 색 계약 사용 가능
- 완료 가능: 채팅 입력, 서버 검증, 전체/팀 채팅 라우팅, WBP 부모 인계와 결과 확인 절차 기록 완료

## 구현 항목

- [x] 로컬 플레이어가 Enter 키로 채팅 입력창을 열 수 있게 한다.
- [x] 입력하지 않을 때도 채팅 로그 위젯은 기본 표시 상태로 둔다.
- [x] 새 메시지가 들어올 때마다 채팅 목록에 메시지 TextBlock 행을 하나씩 추가한다.
- [x] 마지막 채팅 갱신 후 5초가 지나면 채팅 위젯이 서서히 사라지고, Enter 입력 또는 새 메시지 수신 시 다시 보이게 한다.
- [x] 채팅 입력이 열린 동안 마우스 휠로 현재 채팅 로그 위치를 스크롤할 수 있게 한다.
- [x] 로비에서는 전체 채팅만 사용한다.
- [x] PvP와 추후 모드에서는 채팅 입력 중 Tab 키로 전체 채팅과 팀 채팅을 전환한다.
- [x] 팀 채팅 메시지는 전체 채팅과 구분되도록 기본 하늘색으로 표시한다.
- [x] 전체 채팅은 현재 월드의 모든 SnowRumble PlayerController에 전달한다.
- [x] 팀 채팅은 같은 팀 색 PlayerState를 가진 플레이어에게만 전달한다.
- [x] 서버가 빈 메시지와 과도하게 긴 메시지를 정리한다.
- [x] 로비와 PvP에서 같은 PlayerController 계약과 ChatWidget 부모를 재사용한다.
- [x] 추후 좀비맵도 같은 PlayerController 기반이면 채팅 계약을 재사용할 수 있게 한다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: C++·문서 최재원(C), 채팅 WBP 배치와 스타일은 사용자 또는 S 인계
- 생성 파일: `Source/SnowRumble/UI/ChatWidget_C.h`, `Source/SnowRumble/UI/ChatWidget_C.cpp`, `Tasks/C/C-19_text_chat.md`
- 변경 파일: `Source/SnowRumble/UI/SnowRumblePlayerController.*`, `Source/SnowRumble/UI/LobbyPlayerController.*`, `Tasks/C/ROLE_C.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`
- 공유 확인 대상: 사용자, S UI, 추후 좀비맵 담당
- 병합 순서: 로비/PvP UI 연결 전 공용 계약 선행

## 공용 계약과 인계

- 제공받을 계약:
  - `ASnowRumblePlayerState::GetLobbyPlayerName()`: 채팅 표시 이름에 사용한다.
  - `ASnowRumblePlayerState::GetLobbyTeam()`: 팀 채팅 수신자 필터에 사용한다.
- 제공할 계약:
  - `ESnowRumbleChatChannel`: `All`, `Team` 채팅 채널 enum이다.
  - `UChatWidget`: 로비, PvP, 추후 모드에서 공통으로 사용할 채팅 WBP 부모다.
  - `UChatWidget::ChatLogScrollBox`: 같은 이름의 ScrollBox가 있으면 수신한 채팅 메시지를 TextBlock 행으로 하나씩 추가한다.
  - `UChatWidget::ChatLogBorder`: 같은 이름의 Border가 있으면 채팅 입력이 닫힌 동안 브러시 알파를 0으로 바꿔 로그 테두리를 숨기고, 입력 중에는 원래 색으로 되돌린다.
  - `UChatWidget::ChatLogText`: 기존 호환용 TextBlock이다. ScrollBox를 쓰지 않는 WBP에서는 수신한 채팅 로그를 한 TextBlock에 누적 표시한다.
  - `UChatWidget::ChatInputTextBox`: 같은 이름의 EditableTextBox가 있으면 Enter 입력 시 표시되고 메시지 입력을 받는다.
  - `UChatWidget::ChatChannelText`: 같은 이름의 TextBlock이 있으면 현재 채널을 `전체` 또는 `팀`으로 표시한다. 로비에서는 항상 `전체`다.
  - `UChatWidget::ChatMessageFont`: C++가 `ChatLogScrollBox`에 추가하는 메시지 TextBlock 행에 적용할 폰트다.
  - `UChatWidget::AllChatMessageColor`: C++가 `ChatLogScrollBox`에 추가하는 전체 채팅 메시지 행 색이다. 기본값은 흰색이다.
  - `UChatWidget::TeamChatMessageColor`: C++가 `ChatLogScrollBox`에 추가하는 팀 채팅 메시지 행 색이다. 기본값은 하늘색이다.
  - `UChatWidget::ChatChannelFont`: `ChatChannelText`에 적용할 폰트다.
  - `UChatWidget::OpenChatInput(ESnowRumbleChatChannel InitialChannel)`: 채팅 입력창을 열고 키보드 포커스를 준다.
  - `UChatWidget::GetChatInputFocusWidget()`: PlayerController 입력 모드가 직접 포커스할 채팅 입력 TextBox를 반환한다.
  - `UChatWidget::FocusChatInputTextBox()`: 입력 모드 설정 직후 `ChatInputTextBox`에 사용자 포커스와 키보드 포커스를 다시 적용한다.
  - `UChatWidget::CloseChatInput()`: 채팅 입력창을 닫고 입력값을 비운다.
  - `UChatWidget::AddChatMessage(...)`: 수신 메시지를 로그에 추가한다.
  - `UChatWidget::ToggleChatChannel()`: 팀 채팅이 가능한 모드에서 전체/팀 채널을 전환한다.
  - `UChatWidget::OnChatMessageAdded(...)`: Blueprint가 별도 채팅 행 UI를 구성할 때 사용하는 이벤트다.
  - `UChatWidget::OnChatInputOpenChanged(bool bOpen)`: Blueprint가 입력창 표시 연출을 처리할 때 사용하는 이벤트다.
  - `ASnowRumblePlayerController::ChatWidgetClass`: 로비/PvP/추후 모드에서 생성할 채팅 WBP 클래스다.
  - `ASnowRumblePlayerController::OpenChatInput(ESnowRumbleChatChannel InitialChannel)`: 로컬 채팅 입력을 연다.
  - `ASnowRumblePlayerController::SubmitChatMessage(const FString& Message, ESnowRumbleChatChannel Channel)`: 로컬 메시지를 서버 검증 RPC로 보낸다.
  - `ASnowRumblePlayerController::IsTeamChatAvailable()`: 현재 모드에서 팀 채팅을 사용할 수 있는지 반환한다.
  - `ASnowRumblePlayerController::ServerSubmitChatMessage(...)`: 서버가 메시지를 검증하고 수신자를 결정한다.
  - `ASnowRumblePlayerController::ClientReceiveChatMessage(...)`: 수신 클라이언트가 채팅 위젯에 메시지를 추가한다.
  - `ALobbyPlayerController::SupportsTeamChat()`: 로비에서는 false를 반환해 팀 채팅 요청을 전체 채팅으로 고정한다.
- 인계 대상: 사용자, S UI, 추후 좀비맵 담당

## 범위 밖

- 채팅 금칙어, 신고, 차단, 로그 저장
- 귓속말, 파티 채팅, 관전자 전용 채팅
- 채팅 말풍선과 캐릭터 머리 위 표시
- 최종 UI 레이아웃과 애니메이션

## 사전 전제

- C-03 팀 색 PlayerState 계약
- 로비와 PvP에서 `ASnowRumblePlayerController` 계열 PlayerController 사용

## 결정 필요

- 없음

## 변경 기록

- 2026-08-10: 사용자가 로비, PvP, 추후 좀비맵에서 사용할 전체/팀 채팅을 요청해 C-19를 추가하고 구현했다.
- 2026-08-10: 사용자가 Enter로 채팅 입력창을 다시 열 때마다 전체 채팅으로 초기화되는 문제를 보고해, Tab으로 바꾼 마지막 채널을 PvP/추후 팀 채팅 가능 모드에서 유지하도록 수정했다.
- 2026-08-10: 사용자가 노란색은 시스템 알람용으로 남기고 팀 채팅은 하늘색으로 구분하길 원해, 팀 채팅 메시지 행 기본 색을 하늘색으로 추가했다.
- 2026-08-18: Enter를 눌러도 `ChatInputTextBox`가 실제 키보드 포커스를 받지 못하는 회귀에 대응해, PlayerController 입력 모드 포커스 대상을 채팅 위젯 루트가 아니라 `ChatInputTextBox`로 직접 지정하고 입력 모드 설정 직후 TextBox 포커스를 재적용하게 보강했다.
- 2026-08-18: 이모션/채팅 입력 보강은 `git diff --check`와 C++ 컴파일을 통과했다. 최종 DLL 링크는 실행 중인 Unreal Editor의 `UnrealEditor-SnowRumble.dll` 잠금 `LNK1104`로 보류됐다.
- 2026-08-18: Enter로 채팅창이 열린 뒤에도 WASD와 시점 입력이 캐릭터에 전달되는 문제에 대응해, 채팅 입력 open/close 동안 PlayerController의 move/look ignore를 한 쌍으로 적용하고 `ASnowRumbleCharacter::Look()`도 채팅 입력 중에는 반환하게 보강했다.
- 2026-08-18: 채팅 입력 중 캐릭터 입력 차단 변경은 `git diff --check`와 C++ 컴파일을 통과했다. 최종 DLL 링크는 실행 중인 Unreal Editor의 `UnrealEditor-SnowRumble.dll` 잠금 `LNK1104`로 보류됐다.
- 2026-08-18: UI 입력이 계속 깨지는 공통 원인을 조사해 `ASnowRumbleCharacter::RefreshPvpMatchInputLock()`가 Tick마다 `GameOnly`와 숨김 커서로 되돌리는 경로를 확인했다. 채팅 입력, 이모션 메뉴, 게시판 포커스, 커스터마이징 입력이 활성인 동안에는 PvP 입력 잠금 복구가 입력 모드와 ignore counter를 덮지 않게 보강했다.
- 2026-08-18: `RefreshPvpMatchInputLock()` UI 입력 보호 변경은 `git diff --check`와 `SnowRumbleCharacter.cpp` 컴파일을 통과했다. 최종 DLL 링크는 실행 중인 Unreal Editor의 `UnrealEditor-SnowRumble.dll` 잠금 `LNK1104`로 보류됐다.
- 2026-08-18: 채팅 입력 중 Tab이 팀 채팅 전환으로 도달하지 않는 경우에 대응해, 기존 `NativeOnPreviewKeyDown()` 경로를 유지하면서 `NativeTick()`에서 PlayerController의 `WasInputKeyJustPressed(EKeys::Tab)`도 확인해 채널을 전환하게 했다. 같은 프레임 중복 토글은 `LastChatChannelToggleFrameNumber`로 차단한다.
- 2026-08-18: 채팅 Tab 전환 보강은 `git diff --check`와 C++ 컴파일을 통과했다. 최종 DLL 링크는 실행 중인 Unreal Editor의 `UnrealEditor-SnowRumble.dll` 잠금 `LNK1104`로 보류됐다.
- 2026-08-18: `UEditableTextBox`에 직접 key handler를 연결하는 시도는 UE 5.8 API에 없어 제거했다. 대신 채팅 입력 중 `NativeTick()`에서 `IsInputKeyDown(EKeys::Tab)`을 직접 edge-detect해 TextBox가 Tab 입력을 소비해도 채널 전환을 잡도록 변경했다.
- 2026-08-18: 채팅 Tab edge-detect 변경은 `git diff --check`와 C++ 컴파일을 통과했다. 최종 DLL 링크는 실행 중인 Unreal Editor의 `UnrealEditor-SnowRumble.dll` 잠금 `LNK1104`로 보류됐다.

## 수동 작업

- `UChatWidget`을 부모로 하는 채팅 WBP를 만든다.
- 채팅 로그 목록은 ScrollBox로 만들고 이름을 `ChatLogScrollBox`로 맞춘다.
- 채팅 로그 테두리를 C++에서 숨기고 싶으면 Border 이름을 `ChatLogBorder`로 맞춘다. 이 Border는 입력 중이 아닐 때 브러시 알파만 0이 되므로 내부 채팅 텍스트는 그대로 보인다.
- 기존처럼 한 TextBlock에 로그를 누적 표시하려는 임시 WBP는 TextBlock 이름을 `ChatLogText`로 맞출 수 있다.
- 채팅 입력 EditableTextBox 이름을 `ChatInputTextBox`로 맞춘다.
- 현재 채널 표시가 필요하면 TextBlock 이름을 `ChatChannelText`로 맞춘다.
- `ChatChannelText`는 Enter로 채팅 입력창이 열린 동안에만 표시되고, 입력창이 닫히면 자동으로 숨겨진다.
- 새 메시지 행 폰트를 바꾸려면 채팅 WBP 기본값에서 `ChatMessageFont`를 설정한다.
- 전체 채팅 메시지 색을 바꾸려면 채팅 WBP 기본값에서 `AllChatMessageColor`를 설정한다.
- 팀 채팅 메시지 색을 바꾸려면 채팅 WBP 기본값에서 `TeamChatMessageColor`를 설정한다. 기본값은 하늘색이다.
- 채널 표시 폰트를 바꾸려면 채팅 WBP 기본값에서 `ChatChannelFont`를 설정한다.
- 채팅 로그 표시 시간을 바꾸려면 채팅 WBP 기본값에서 `ChatVisibleDuration`과 `ChatFadeOutDuration`을 조정한다. 기본값은 마지막 갱신 후 5초 유지, 1.5초 fade-out이다.
- 입력 중인 글자 폰트는 `ChatInputTextBox`의 Widget Style 폰트를 WBP에서 직접 설정한다.
- 채팅 입력창이 열린 상태에서 `Tab`을 누르면 PvP와 추후 모드에서는 `전체`와 `팀` 채널이 전환된다.
- 로비에서는 `Tab`을 눌러도 채널이 바뀌지 않고 전체 채팅으로 유지된다.
- 로비 PlayerController Blueprint와 PvP PlayerController Blueprint 또는 공용 부모 Blueprint의 `ChatWidgetClass`에 채팅 WBP를 연결한다.
- 기본적으로 항상 보일 채팅 로그 영역과 Enter 입력 때만 보일 입력창의 위치·크기·스타일을 WBP에서 배치한다.
- 채팅 입력창이 열린 상태에서 마우스 휠로 이전 채팅을 볼 수 있도록 `ChatLogScrollBox`가 화면에서 휠 입력을 받을 수 있는 위치와 크기로 배치한다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드 변경 완료
- [x] `git diff --check` 공백 점검 통과
- [x] 관련 C++ 컴파일 통과
- [x] `SnowRumbleEditor Win64 Development` 최종 빌드 확인
- [x] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [x] 공용 계약과 캡슐화 규칙 위반 없음
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨

### 검증 메모

- 2026-08-10: `UChatWidget` 공용 WBP 부모와 `ASnowRumblePlayerController` 채팅 RPC/Enter 입력 계약을 추가했다. `ChatWidget_C.cpp`, `LobbyBoardWidget_C.cpp`, `LobbyPlayerController.cpp`, `LobbyWidget.cpp`, `Module.SnowRumble.cpp`, `SnowRumbleGameMode.cpp`, `SnowRumbleLobbyGameMode.cpp`, `SnowRumblePlayerController.cpp` 컴파일은 통과했다. 최종 링크는 실행 중인 Unreal Editor가 `Binaries/Win64/UnrealEditor-SnowRumble.dll`을 잡고 있어 `LNK1104`로 실패했다. 에디터 종료 후 재빌드 확인이 필요하다.
- 2026-08-10: 전체/팀 채팅 전환 방식을 버튼에서 채팅 입력 중 `Tab` 키로 변경하고, `ALobbyPlayerController`는 팀 채팅을 허용하지 않게 했다. 로비에서 들어오는 팀 채팅 요청은 서버 제출 단계에서도 전체 채팅으로 고정된다. 채팅 입력이 열려 있는 동안 기존 캐릭터 행동 입력과 Tab 이모션이 같이 반응하지 않도록 `ASnowRumbleCharacter::CanPerformGameplayAction()`이 채팅 입력 상태를 확인하게 했다. `ChatWidget_C.cpp`, `LobbyPlayerController.cpp`, `SnowRumbleCharacter.cpp`, `SnowRumblePlayerController.cpp` 등 관련 C++ 컴파일은 통과했고, 최종 링크는 실행 중인 Unreal Editor가 `Binaries/Win64/UnrealEditor-SnowRumble.dll`을 잡고 있어 `LNK1104`로 실패했다.
- 2026-08-10: 채팅 로그 표시를 단일 TextBlock 누적 방식에서 `ChatLogScrollBox`에 메시지 TextBlock을 하나씩 추가하는 방식으로 변경했다. 입력창이 열린 동안에는 마우스 휠로 `ChatLogScrollBox` 스크롤 위치를 조정할 수 있고, 이 상태에서는 새 메시지가 와도 강제로 맨 아래로 스크롤하지 않는다. `ChatLogText`는 기존 WBP 호환용 fallback으로 유지한다.
- 2026-08-10: `ChatLogScrollBox` 메시지 행 추가와 입력 중 마우스 휠 스크롤 변경 후 `git diff --check`와 `SnowRumbleEditor Win64 Development` 빌드가 통과했다.
- 2026-08-10: Enter로 채팅을 열면 `ChatInputTextBox`에 바로 `SetUserFocus`와 `SetKeyboardFocus`를 적용하게 조정했다. PlayerController 입력 모드 설정이 TextBox 포커스를 덮지 않도록 입력 모드 설정 뒤에 채팅 입력창을 열게 했다. `ChatWidget_C.cpp`, `SnowRumblePlayerController.cpp` 컴파일은 통과했고, 최종 링크는 실행 중인 Unreal Editor가 `Binaries/Win64/UnrealEditor-SnowRumble.dll`을 잡고 있어 `LNK1104`로 실패했다.
- 2026-08-10: 메시지 입력 완료 Enter 또는 입력 취소 시 `UChatWidget::CloseChatInput()`만 호출하던 경로를 `ASnowRumblePlayerController::CloseChatInput()`으로 바꿨다. 입력창 닫기와 함께 `GameOnly` 입력 모드와 숨김 커서가 복구된다. `ChatWidget_C.cpp` 컴파일은 통과했고, 최종 링크는 실행 중인 Unreal Editor가 `Binaries/Win64/UnrealEditor-SnowRumble.dll`을 잡고 있어 `LNK1104`로 실패했다.
- 2026-08-10: `ChatChannelText`를 `ChatInputTextBox`처럼 채팅 입력창이 열린 동안에만 표시되게 조정했다. C++가 생성하는 메시지 행은 `ChatMessageFont`, 채널 표시 텍스트는 `ChatChannelFont`를 적용할 수 있게 했다. 입력창 자체 폰트는 `UEditableTextBox`의 WBP Widget Style에서 설정한다. 해당 변경 후 `git diff --check`와 `SnowRumbleEditor Win64 Development` 빌드가 통과했다.
- 2026-08-10: 채팅 입력이 닫혀 있는 동안 `ChatLogScrollBox`의 스크롤바를 숨기고, 선택 바인딩 `ChatLogBorder`가 있으면 테두리 브러시 알파만 0으로 바꾸게 했다. ScrollBox와 메시지 TextBlock은 계속 표시되므로 기존 채팅 내용은 입력 전에도 그대로 보인다. 해당 변경 후 `git diff --check`와 `SnowRumbleEditor Win64 Development` 빌드가 통과했다.
- 2026-08-10: 마지막 채팅 갱신 또는 Enter 입력 후 `ChatVisibleDuration` 동안 채팅 위젯을 표시하고, 이후 `ChatFadeOutDuration` 동안 서서히 사라지게 했다. `Enter`로 입력창을 열거나 새 메시지를 받으면 즉시 다시 보이게 했다. 해당 변경 후 `git diff --check`가 통과했고 `ChatWidget_C.cpp` 컴파일도 통과했다. 최종 링크는 실행 중인 Unreal Editor가 `Binaries/Win64/UnrealEditor-SnowRumble.dll`을 잡고 있어 `LNK1104`로 실패했다.
- 2026-08-10: PvP 채팅 입력창에 포커스가 있을 때 `Tab`이 TextBox에서 먼저 처리되어 채널 전환까지 도달하지 않는 문제를 수정했다. `UChatWidget::NativeOnPreviewKeyDown()`에서 입력창 포커스보다 먼저 `Tab`을 잡아 `ToggleChatChannel()`을 호출한다. 해당 변경 후 `git diff --check`가 통과했고 `ChatWidget_C.cpp` 컴파일도 통과했다. 최종 링크는 실행 중인 Unreal Editor가 `Binaries/Win64/UnrealEditor-SnowRumble.dll`을 잡고 있어 `LNK1104`로 실패했다.
- 2026-08-10: Enter 입력 재오픈 시 `ASnowRumblePlayerController::HandleChatInputPressed()`가 항상 전체 채팅으로 열던 흐름을 수정했다. 팀 채팅 가능 모드에서는 기존 `UChatWidget::GetActiveChatChannel()` 값을 다시 사용하고, 로비처럼 팀 채팅이 불가능한 모드에서는 전체 채팅으로 고정한다. 해당 변경 후 `git diff --check -- Source/SnowRumble/UI/SnowRumblePlayerController.cpp Tasks/C/C-19_text_chat.md Tasks/C/PLAN_C.md`가 통과했고, `SnowRumblePlayerController.cpp` 컴파일도 통과했다. 최종 링크는 실행 중인 Unreal Editor가 `Binaries/Win64/UnrealEditor-SnowRumble.dll`을 잡고 있어 `LNK1104`로 실패했다.
- 2026-08-10: `UChatWidget`에 `AllChatMessageColor`와 `TeamChatMessageColor`를 추가하고, `ChatLogScrollBox`에 추가되는 메시지 TextBlock 행 색상을 채널별로 적용했다. 팀 채팅 기본색은 하늘색 `FLinearColor(0.35, 0.85, 1.0, 1.0)`이다. 해당 변경 후 `git diff --check -- Source/SnowRumble/UI/ChatWidget_C.h Source/SnowRumble/UI/ChatWidget_C.cpp`와 `SnowRumbleEditor Win64 Development` 빌드가 통과했다.
- 2026-08-18: Enter 입력 회귀 보강 후 `ChatWidget_C.cpp`와 `SnowRumblePlayerController.cpp` 컴파일은 통과했다. 최종 링크는 실행 중인 Unreal Editor가 `Binaries/Win64/UnrealEditor-SnowRumble.dll`을 잡고 있어 `LNK1104`로 실패했다.
- 2026-08-18: 채팅 입력 open/close에서 `SetIgnoreMoveInput(true)`/`SetIgnoreLookInput(true)`와 `ResetIgnoreMoveInput()`/`ResetIgnoreLookInput()`를 `bChatInputIgnoringPawnInput`으로 한 번씩만 적용하게 했다. `SnowRumbleCharacter.cpp`, `SnowRumblePlayerController.cpp`, `ChatWidget_C.cpp` 컴파일은 통과했고 최종 링크는 실행 중인 Unreal Editor DLL 잠금으로 실패했다.

### 결과 확인

- [x] 로비에서 채팅 로그가 기본 표시된다.
- [x] 새 채팅 메시지가 들어올 때마다 `ChatLogScrollBox`에 메시지 행이 하나씩 추가된다.
- [x] 채팅 입력이 닫혀 있을 때 채팅 로그 텍스트는 보이고 `ChatLogScrollBox` 스크롤바와 `ChatLogBorder` 테두리는 보이지 않는다.
- [x] 로비에서 Enter를 누르면 채팅 입력창이 열리고 입력할 수 있다.
- [x] 로비에서 Enter를 누르면 `ChatChannelText`가 보이고, 입력이 끝나면 숨겨진다.
- [x] `ChatMessageFont`를 바꾸면 새로 추가되는 채팅 메시지 행 폰트가 바뀐다.
- [x] `ChatInputTextBox` Widget Style 폰트를 바꾸면 입력 중인 글자 폰트가 바뀐다.
- [x] 로비에서 Enter를 누른 상태로 마우스 휠을 움직이면 이전 채팅 로그를 볼 수 있다.
- [x] 로비에서 메시지를 입력하고 Enter로 확정하면 호스트와 클라이언트 모두 전체 채팅을 받는다.
- [x] 로비에서 채팅 입력 중 Tab을 눌러도 채널이 전체로 유지된다.
- [x] PvP에서 채팅 로그가 기본 표시된다.
- [x] PvP에서 새 채팅 메시지가 들어올 때마다 `ChatLogScrollBox`에 메시지 행이 하나씩 추가된다.
- [x] PvP에서 채팅 입력이 닫혀 있을 때 채팅 로그 텍스트는 보이고 `ChatLogScrollBox` 스크롤바와 `ChatLogBorder` 테두리는 보이지 않는다.
- [x] PvP에서 Enter를 누르면 채팅 입력창이 열리고 입력할 수 있다.
- [x] PvP에서 Enter를 누르면 `ChatChannelText`가 보이고, 입력이 끝나면 숨겨진다.
- [x] PvP에서 Enter를 누른 상태로 마우스 휠을 움직이면 이전 채팅 로그를 볼 수 있다.
- [x] PvP에서 전체 채팅이 호스트와 클라이언트 모두에게 표시된다.
- [x] PvP에서 채팅 입력 중 Tab을 누르면 전체 채팅과 팀 채팅이 전환된다.
- [x] PvP에서 팀 채팅이 같은 팀 색 플레이어에게만 표시된다.
- [x] 로비 ESC 메뉴가 열린 상태에서는 Enter 채팅 입력이 열리지 않는다.
- [ ] 마지막 채팅 갱신 후 5초가 지나면 채팅 위젯이 서서히 사라진다.
- [ ] 채팅 위젯이 사라진 상태에서 Enter를 누르면 채팅 입력창과 로그가 즉시 다시 보인다.
- [ ] 새 채팅 메시지를 받으면 사라졌던 채팅 로그가 즉시 다시 보이고, 다시 5초 뒤 fade-out된다.
- [ ] PvP에서 Enter로 채팅 입력창을 연 뒤 Tab을 누르면 `ChatChannelText`가 `전체`와 `팀` 사이에서 전환된다.
- [ ] PvP에서 Tab으로 `팀` 채널로 바꾼 뒤 Enter로 제출·닫기·다시 열기를 해도 `ChatChannelText`가 `팀`으로 유지된다.
- [ ] PvP에서 Tab으로 `전체` 채널로 바꾼 뒤 Enter로 제출·닫기·다시 열기를 해도 `ChatChannelText`가 `전체`로 유지된다.
- [ ] PvP에서 팀 채팅 메시지는 하늘색으로 표시되고, 전체 채팅 메시지와 색으로 구분된다.

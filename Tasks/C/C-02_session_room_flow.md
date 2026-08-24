# Task C-02 - 세션과 방 설정

## 설명

최대 8명의 LAN 방 만들기·검색·참가, 빠른 게임과 방 코드 참가를 새 메인 로비 흐름에 맞춘다.

## 상태 전이 기준
- 시작 가능: C-01 완료와 방 코드 참가 기준 확정
- 완료 가능: 호스트·클라이언트 세션 결과와 S-02 UI 계약 인계 확인

## 구현 항목
- [x] 방 이름, 최대 8명과 팀 PvP 방 정보를 서버 세션 흐름에 반영한다.
- [x] 빠른 게임이 빈자리가 있는 LAN 방 중 참가 가능한 방을 찾아 자동 참가하는 결과를 제공한다.
- [x] 방 생성 시 방 코드를 생성·광고하고, 대기방 UI가 읽을 수 있는 계약을 제공한다.
- [x] 호스트 로비 UI가 세션 생성 타이밍과 무관하게 pending 방 코드를 fallback으로 읽을 수 있게 한다.
- [x] 방 찾기에서 입력한 방 코드와 일치하는 검색 결과에만 참가하는 요청을 제공한다.
- [x] 에디터 확인 중 세션 호출 여부와 검색 결과를 추적할 수 있도록 `LogSnowRumbleSession` 로그를 추가한다.

## 작업 배정
- 담당자·기능·계약 소유자: 최재원(C)
- 자산 수정자·생성·변경 후보: 구현 승인 전 확정
- 공유 확인 대상: S-02
- 병합 순서: C-01 후, S-02 전

## 공용 계약과 인계
- 제공받을 계약: C-01 기존 세션 연결
- 제공할 계약:
  - `USnowRumbleSessionSubsystem::HostLanSession(int32 MaxPlayers, const FString& RoomName)`: 호스트가 최대 8명으로 제한된 LAN 방을 만들고 6자리 방 코드를 생성해 세션 설정에 광고한다.
  - `USnowRumbleSessionSubsystem::QuickJoinLanSession()`: LAN 검색 후 `CurrentPlayers < MaxPlayers`인 첫 번째 방에 자동 참가한다.
  - `USnowRumbleSessionSubsystem::JoinLanSessionByRoomCode(const FString& RoomCode)`: 입력 방 코드와 검색 결과의 `FSnowRumbleSessionInfo::RoomCode`가 일치하는 방에 참가한다.
  - `USnowRumbleSessionSubsystem::GetCurrentRoomCode()`: 호스트 또는 참가 요청이 들고 있는 현재 방 코드를 반환한다.
  - `FSnowRumbleSessionInfo::RoomName`, `FSnowRumbleSessionInfo::RoomCode`, `FSnowRumbleSessionInfo::GameModeName`: 검색 결과에서 UI가 표시하거나 필터링할 방 정보다.
  - `UMainMenuWidget::QuickJoinLanGame()`, `UMainMenuWidget::JoinLanGameByRoomCode(const FString& RoomCode)`: S-02 시작화면 WBP가 버튼과 입력창에 연결할 함수다.
  - `UMainMenuWidget`의 선택 바인딩 위젯 `RoomCodeJoinPanel`, `RoomCodeTextBox`, `ConfirmRoomCodeJoinButton`, `CancelRoomCodeJoinButton`: 이름을 맞춰 배치하면 C++ 부모가 방 코드 입력 패널 표시, 확인 참가 요청, 취소 닫기를 처리한다.
  - `UMainMenuWidget::OnRoomCodeJoinPromptRequested()`: 방 코드 입력 패널이 열릴 때 S-02 WBP가 추가 연출을 연결할 수 있는 Blueprint 이벤트다.
  - `ULobbyWidget::GetCurrentRoomCode()`: S-02 대기방 WBP가 오른쪽 상단에 표시할 방 코드 조회 함수다.
  - `ULobbyWidget`의 선택 바인딩 위젯 `RoomCodeTextBlock`: 이름을 맞춰 배치하면 C++ 부모가 현재 방 코드를 자동 표시한다.
  - `ULocalPlayerIdentitySubsystem`: 메인메뉴에서 입력한 로컬 닉네임을 GameInstance 수명 동안 저장한다. 현재는 닉네임만 저장하며, 추후 커스터마이징 데이터는 같은 로컬 정체성 책임 안에서 확장한다.
  - `UMainMenuWidget`의 선택 바인딩 위젯 `PlayerNameTextBox`: C++ 부모가 최초 표시 시 랜덤 기본 닉네임을 채우고, Host, 빠른 참여, 방 코드 참가 전에 입력값을 로컬 닉네임으로 저장한다.
  - `ALobbyPlayerController::RequestApplyLobbyPlayerName(const FString& NewName)`: 소유 클라이언트가 서버의 자기 `PlayerState` 닉네임 변경을 요청하는 RPC 경로다.
  - `ULobbyWidget`: 로비 입장 후 저장된 로컬 닉네임을 `ALobbyPlayerController::RequestApplyLobbyPlayerName()`으로 서버에 적용한다.
- 인계 대상: S-02, C-03

## 범위 밖
- Steam과 전용 서버. Steam 세션, Steam Overlay 친구 초대와 초대 수락은 C-18 최종 통합 Task에서 처리한다.
- UI 레이아웃과 그래픽
- 경기 결과 뒤 대기방 복귀와 방 나가기

## 사전 전제
- C-01

## 기존 구현 인수
- `USnowRumbleSessionSubsystem`에 NULL LAN Host, 검색, Join 요청과 `FSnowRumbleSessionInfo` 검색 결과 구조체가 이미 있다.
- `OnSessionStateChanged`, `OnSessionSearchCompleted`는 S-02 UI가 구독할 수 있는 기존 Blueprint 이벤트다.
- 현재 Host 흐름은 `L_Lobby`로 이동한 뒤 Listen Server 세션을 생성한다.
- 새 기획 기준으로 최대 인원은 16명 기록에서 8명으로 줄이고, 빠른 게임과 방 코드 참가 흐름은 아직 없다.

## 결정 필요
- 없음

## 확정된 방 참가 기준
- 빠른 게임은 검색된 LAN 방 중 현재 인원이 최대 인원보다 적은 첫 번째 방에 자동 참가한다.
- 방 생성 시 C++ 세션 계약이 방 코드를 만들고 LAN 세션 설정에 광고한다.
- 대기방 UI는 방 오른쪽 상단에 현재 방 코드를 표시한다. 실제 위젯 배치와 그래픽은 S-02 범위다.
- 방 찾기는 사용자가 입력한 방 코드와 일치하는 LAN 검색 결과를 찾아 참가한다.
- 방 코드가 없거나 일치하는 방이 없거나 방이 가득 찬 경우, C++는 실패 상태와 메시지를 UI로 전달한다.
- 현재 개발과 테스트는 LAN/NULL 세션을 유지한다.
- 이후 새 UI·로비·PvP 기능은 `USnowRumbleSessionSubsystem`의 공개 함수와 이벤트를 통해 세션 기능을 사용하고, LAN/Steam 구현 세부사항에 직접 의존하지 않는다.
- Steam 출시용 세션 전환은 C-18에서 `USnowRumbleSessionSubsystem` 내부 구현을 확장하는 방식으로 처리한다.

## 수동 작업 (구현 후 구체화)
- S-02에서 `WBP_MainMenu` 또는 새 시작화면 WBP의 빠른 게임 버튼을 `UMainMenuWidget::QuickJoinLanGame()`에 연결한다.
- 빠른 참여 버튼은 `QuickJoinButton` 이름으로 바인딩하면 C++ 부모가 자동으로 `QuickJoinLanGame()`을 호출한다.
- S-02에서 메인메뉴 닉네임 입력창을 만들고 `PlayerNameTextBox` 이름으로 바인딩한다. C++ 부모가 최초 랜덤 닉네임을 표시하고 Host, 빠른 참여, 방 코드 참가 전에 입력값을 저장한다.
- S-02에서 `WBP_MainMenu`에 방 코드 입력 패널을 만들고 `RoomCodeJoinPanel` 이름으로 바인딩한다. 기본 표시는 C++ 부모가 `Collapsed`로 설정한다.
- S-02에서 방 코드 입력창은 `RoomCodeTextBox`, 확인 버튼은 `ConfirmRoomCodeJoinButton`, 취소 버튼은 `CancelRoomCodeJoinButton` 이름으로 바인딩한다.
- 방 찾기 버튼은 `FindButton` 이름을 유지하면 C++ 부모가 `RoomCodeJoinPanel`을 열고, 확인 버튼은 `RoomCodeTextBox` 값을 읽어 `JoinLanGameByRoomCode(RoomCode)`를 호출한다.
- S-02에서 방 만들기 UI가 방 이름을 받는 경우 `UMainMenuWidget::HostLanGame(8, RoomName)`으로 전달한다.
- S-02에서 `WBP_Lobby` 오른쪽 상단에 TextBlock을 만들고 `RoomCodeTextBlock` 이름으로 바인딩한다. C++ 부모가 `ULobbyWidget::GetCurrentRoomCode()` 결과를 자동 표시한다.
- C-03은 C-02의 방 입장 완료 후 기존 `ASnowRumbleLobbyGameState`와 `ASnowRumblePlayerState` 대기방 상태를 확장한다.

## 완료 조건
### 에이전트 확인
- [x] 관련 코드·계약 변경 완료
- [x] 서버 검증과 소유권 규칙 확인
- [x] S-02 인계 기록 완료
- [x] `git diff --check` 공백 점검 통과
- [x] `SnowRumbleEditor Win64 Development` 최종 빌드 확인

### 검증 메모
- 2026-08-08: 최종 빌드는 MSVC PCH 가상 메모리 부족 `C3859/C1076`, system code `1455`로 실패했다. 현재 오류는 코드 진단 전에 발생한 환경 리소스 문제이며, page file 여유 확보 후 재실행이 필요하다.
- 2026-08-08: page file 여유 확보 후 `SnowRumbleEditor Win64 Development` 재빌드가 `Result: Succeeded`로 완료됐다.
- 2026-08-08: 빠른 참가·코드 참가 확인을 위해 `LogSnowRumbleSession` 로그와 이미 연결된 클라이언트의 방 만들기 방어 메시지를 추가했고, 재빌드가 `Result: Succeeded`로 완료됐다.
- 2026-08-08: 방 코드 입력 UI 표시와 확인·취소 동작을 `UMainMenuWidget` C++ 부모의 선택 바인딩 위젯으로 처리하도록 보강했고, 재빌드가 `Result: Succeeded`로 완료됐다.
- 2026-08-08: 빠른 참여 버튼을 `QuickJoinButton` 선택 바인딩으로 정리했고, 재빌드가 `Result: Succeeded`로 완료됐다.
- 2026-08-08: `WBP_Lobby`가 `RoomCodeTextBlock` 이름만 맞추면 방 코드를 자동 표시하도록 `ULobbyWidget` C++ 부모를 보강했다.
- 2026-08-08: 추후 커스터마이징 확장을 고려해 로컬 정체성 저장 책임을 `ULocalPlayerIdentitySubsystem`으로 분리하고, 현재는 `PlayerNameTextBox` 닉네임만 로비 `PlayerState`에 적용하도록 구현했다.
- 2026-08-08: 닉네임 정체성 데이터 흐름 추가 후 `SnowRumbleEditor Win64 Development` 재빌드가 `Result: Succeeded`로 완료됐다.
- 2026-08-08: 최초 닉네임이 없을 때 `ULocalPlayerIdentitySubsystem`이 귀엽고 가벼운 후보 목록에서 랜덤 기본 닉네임을 만들고, `PlayerNameTextBox`에 자동 표시하도록 보강했다.
- 2026-08-08: 랜덤 기본 닉네임 흐름 추가 후 `SnowRumbleEditor Win64 Development` 재빌드가 `Result: Succeeded`로 완료됐다.
- 2026-08-08: 클라이언트 닉네임이 서버 화면에서 기본 PC 이름으로 남는 문제를 막기 위해 닉네임 적용 경로를 소유 클라이언트의 `ALobbyPlayerController` 서버 RPC로 변경했다.
- 2026-08-08: 서버 `PostLogin` 후 `ALobbyPlayerController::ClientRequestApplySavedLobbyPlayerName()`으로 클라이언트 저장 닉네임 제출을 요청하는 핸드셰이크를 추가했고, 재빌드가 `Result: Succeeded`로 완료됐다.
- 2026-08-08: 서버가 닉네임을 적용할 때 `LobbyPlayerName`과 기본 `APlayerState::PlayerName`을 함께 갱신하도록 보강했고, 재빌드가 `Result: Succeeded`로 완료됐다.
- 2026-08-21: Hamachi 테스트 중 방 코드 참가 재시도에서 NULL OSS named session이 남아 `can't join twice`가 발생하는 로그를 확인했다. 참가 시작 전이나 참가 실패 완료 시 로컬 named session을 정리해 다음 참가 요청이 막히지 않도록 보강했다. C++ 컴파일과 `.lib` 생성은 통과했고, 최종 DLL 링크는 실행 중인 Unreal Editor DLL 잠금으로 보류됐다.
- 2026-08-21: 클라이언트가 참가 중 호스트 연결을 잃으면 DemoMap 등 기본 맵으로 빠지는 문제에 대응했다. 네트워크 실패 처리에서 메인메뉴 travel URL에 `BP_MainMenuGameMode`를 강제하고, 메인메뉴 진입 알림을 `호스트의 연결이 해제되었습니다.`로 표시하게 했다. C++ 컴파일과 `.lib` 생성은 통과했고, 최종 DLL 링크는 실행 중인 Unreal Editor DLL 잠금으로 보류됐다.
- 2026-08-21: 접속 실패 일부가 `NetworkFailure`가 아닌 `TravelFailure` 경로로 처리되고, 프로젝트 기본 맵이 PvP 테스트용 DemoMap이면 엔진 fallback이 DemoMap으로 이동하는 원인을 확인했다. `TravelFailure`도 세션 실패로 처리하고, `GameDefaultMap`과 `ServerDefaultMap`을 `L_MainMenu`로 고정해 실패 fallback도 메인메뉴가 되게 했다. `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-23: 호스트가 로비에 들어갔을 때 간헐적으로 방 코드가 비어 보이는 문제를 보강했다. `GetCurrentRoomCode()`는 `CurrentRoomCode`가 비었더라도 호스트 생성 중이면 `PendingHostRoomCode`를 반환하고, `CreateLanSession()`과 생성 성공 처리에서도 `CurrentRoomCode`를 pending 코드로 복구한다.

### 결과 확인
- [ ] 호스트가 방을 만들면 `L_Lobby`로 이동하고 대기방 UI에서 6자리 방 코드를 확인할 수 있다.
- [ ] 호스트가 세션 생성 완료 전 로비 UI를 먼저 보더라도 방 코드가 비어 있지 않다.
- [ ] 다른 클라이언트가 빠른 게임을 누르면 빈자리가 있는 LAN 방에 자동 참가한다.
- [ ] 다른 클라이언트가 방 찾기에서 같은 방 코드를 입력하면 해당 LAN 방에 참가한다.
- [ ] 존재하지 않는 코드 또는 가득 찬 방을 입력하면 클라이언트가 로비에 남고 실패 메시지를 받는다.
- [ ] 최대 인원은 8명으로 제한된다.

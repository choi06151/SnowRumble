# Task 03-2 - 대기방 팀 선택과 준비

## 설명

LAN 세션 참가자들이 시작화면에서 Host/Join을 통해 대기방에 들어오고, 대기방에서 서로의 이름·팀·준비 상태를 공유한 뒤 호스트가 게임방으로 이동을 시작할 수 있게 한다. 이번 구현 단위는 C++ 서버 권한·복제 기본틀과 Blueprint UI 부모 연결 지점이다.

## 구현 항목

- [x] Host LAN Session이 게임방이 아니라 대기방 맵으로 이동하도록 한다.
- [x] 접속한 플레이어 목록 상태를 서버가 관리하고 복제한다.
- [x] 플레이어가 이름과 팀 색상 변경을 서버에 요청할 수 있다.
- [x] 플레이어가 준비와 준비 해제를 서버에 요청할 수 있다.
- [x] 대기방 상태가 모든 참가자에게 동일하게 복제된다.
- [x] 호스트의 경기 시작 가능 여부를 서버가 판정한다.
- [x] 호스트가 시작을 요청하면 게임방 맵으로 ServerTravel할 수 있다.
- [x] 시작화면과 대기방 UI가 상속할 C++ 부모 위젯을 제공한다.
- [x] 시작화면과 대기방 진입 시 C++ PlayerController가 담당 WBP를 자동 생성해 표시한다.
- [x] 시작화면 전용 C++ GameMode를 제공해 메인 메뉴 맵 설정을 통일한다.
- [x] 시작화면 WBP의 임시 Host/Search/Join 버튼을 이름 기준으로 C++에서 자동 바인딩한다.

## 메인 프로그래머 책임

- 접속자, 이름, 팀, 준비 상태와 경기 시작 가능 조건을 서버 권한 및 복제 상태로 완성한다.
- 시작화면과 대기방 WBP가 호출할 C++ 부모 함수와 이벤트를 제공한다.

## SUB UI 인계

- 연계 SUB Task: `Tasks/S03-2_lobby_team_ready_ui.md`
- 실제 연결 지점:
  - `UMainMenuWidget`: `WBP_MainMenu`가 상속할 C++ 부모 위젯이다.
  - `ASnowRumbleMainMenuGameMode`: 시작화면 맵에서 사용할 C++ GameMode이며 기본 `PlayerControllerClass`는 `AMainMenuPlayerController`다.
  - `AMainMenuPlayerController`: 시작화면 맵에서 `WBP_MainMenu`를 자동 생성하고 UI 입력 모드로 전환하는 C++ PlayerController다.
  - `MainMenuWidgetClass`: `BP_MainMenuPlayerController`에서 지정할 실제 `WBP_MainMenu` 클래스다.
  - `HostButton`: `WBP_MainMenu`에 같은 이름으로 만들면 `HostLanGame(16)`에 자동 연결된다.
  - `FindButton`: `WBP_MainMenu`에 같은 이름으로 만들면 `FindLanGames()`에 자동 연결된다.
  - `JoinFirstButton`: `WBP_MainMenu`에 같은 이름으로 만들면 `JoinLanGame(0)`에 자동 연결된다.
  - `StatusTextBlock`: 선택 사항이며, 같은 이름으로 만들면 세션 상태와 검색 결과 개수가 자동 표시된다.
  - `HostLanGame(int32 MaxPlayers)`: 시작화면 Host 버튼에서 호출한다.
  - `FindLanGames()`: 시작화면 세션 검색 버튼에서 호출한다.
  - `JoinLanGame(int32 ResultIndex)`: 검색 결과 선택 Join 버튼에서 호출한다.
  - `GetLanSearchResults()`: 마지막 LAN 검색 결과 목록을 반환한다.
  - `OnMainMenuSessionStateChanged(Operation, State, Message)`: Host/Search/Join 진행 상태를 Blueprint UI에 전달한다.
  - `OnMainMenuSearchCompleted(Results)`: 검색 완료 결과를 Blueprint UI에 전달한다.
  - `ULobbyWidget`: `WBP_Lobby`가 상속할 C++ 부모 위젯이다.
  - `ALobbyPlayerController`: 대기방 맵에서 `WBP_Lobby`를 자동 생성하고 UI 입력 모드로 전환하는 C++ PlayerController다.
  - `LobbyWidgetClass`: `BP_LobbyPlayerController`에서 지정할 실제 `WBP_Lobby` 클래스다.
  - `GetLobbyPlayers()`: 현재 대기방 플레이어 목록을 반환한다.
  - `RequestSetLocalPlayerName(FString)`: 로컬 플레이어 이름 변경을 서버에 요청한다.
  - `RequestSetLocalPlayerTeam(ESnowRumbleTeam)`: 로컬 플레이어 팀 변경을 서버에 요청한다.
  - `RequestSetLocalPlayerReady(bool)`: 로컬 플레이어 준비 상태 변경을 서버에 요청한다.
  - `RequestStartMatch()`: 호스트의 게임 시작 요청을 서버에 전달한다.
  - `IsLocalPlayerHost()`: 현재 로컬 플레이어가 Listen Server 호스트인지 반환한다.
  - `CanStartMatch()`: 현재 서버 기준 시작 가능 여부를 반환한다.
  - `OnLobbyStateChanged()`: 대기방 목록 또는 플레이어 상태 변경 시 Blueprint UI에 전달된다.
  - `ASnowRumblePlayerState::GetLobbyPlayerName()`, `GetLobbyTeam()`, `IsLobbyReady()`: 목록 행에서 표시할 값이다.

## 작업 배정

- 담당자: 메인 프로그래머
- 메인 프로그래머 선점 파일: `Source/SnowRumble/Game/SnowRumblePlayerState.h`, `Source/SnowRumble/Game/SnowRumblePlayerState.cpp`, `Source/SnowRumble/Game/SnowRumbleLobbyGameState.h`, `Source/SnowRumble/Game/SnowRumbleLobbyGameState.cpp`, `Source/SnowRumble/Game/SnowRumbleLobbyGameMode.h`, `Source/SnowRumble/Game/SnowRumbleLobbyGameMode.cpp`, `Source/SnowRumble/Game/SnowRumbleMainMenuGameMode.h`, `Source/SnowRumble/Game/SnowRumbleMainMenuGameMode.cpp`, `Source/SnowRumble/UI/MainMenuWidget.h`, `Source/SnowRumble/UI/MainMenuWidget.cpp`, `Source/SnowRumble/UI/MainMenuPlayerController.h`, `Source/SnowRumble/UI/MainMenuPlayerController.cpp`, `Source/SnowRumble/UI/LobbyWidget.h`, `Source/SnowRumble/UI/LobbyWidget.cpp`, `Source/SnowRumble/UI/LobbyPlayerController.h`, `Source/SnowRumble/UI/LobbyPlayerController.cpp`, `Source/SnowRumble/Online/SnowRumbleSessionSubsystem.cpp`, `Tasks/03-2_lobby_team_ready.md`, `Tasks/S03-2_lobby_team_ready_ui.md`, `docs/ARCHITECTURE.md`, `docs/PLANS.md`
- 공유 확인 대상: Host/Join 후 대기방 진입, 대기방 목록 복제, 팀/준비 변경 복제, 호스트 시작 요청과 게임방 이동
- 반영 순서: 메인 C++ 연결 지점 반영 후 사용자가 `WBP_MainMenu`, `WBP_Lobby`, `L_Lobby`를 구성한다.

## 범위 밖

- 상세 캐릭터 커스터마이징
- 여러 게임 모드의 세부 옵션
- 경기 맵 랜덤 선택
- 최종 메뉴 디자인
- 전용 서버 또는 Steam 세션

## 사전 전제

- Task 03-1의 LAN 세션 Host와 Join

## 현재 구현 결과

- `USnowRumbleSessionSubsystem`의 Host 이동 경로는 `/Game/Maps/L_Lobby?listen`이며 세션 광고 맵 이름도 `L_Lobby`다. 메뉴 맵에서 Host를 누르면 Standalone은 `OpenLevel(..., listen)`, 이미 ListenServer인 PIE는 `ServerTravel`로 로비에 진입한 뒤 LAN 세션을 생성한다.
- `ASnowRumbleMainMenuGameMode`의 기본 `PlayerControllerClass`는 `AMainMenuPlayerController`이며, 시작화면 진입 시 로컬 클라이언트의 `MainMenuWidgetClass`를 자동 생성한다.
- `ASnowRumbleLobbyGameMode`는 `ASnowRumbleLobbyGameState`와 `ASnowRumblePlayerState`를 기본 클래스로 사용한다.
- `ASnowRumblePlayerState`는 `LobbyPlayerName`, `LobbyTeam`, `bLobbyReady`를 복제하고 소유 클라이언트 요청 함수로 서버 변경을 요청한다.
- `ASnowRumbleLobbyGameState::CanStartLobbyMatch()`는 기본 조건을 `2명 이상`, `모든 플레이어 Ready`, `Red/Blue 양 팀 최소 1명`으로 판정한다.
- `ASnowRumbleLobbyGameMode::RequestStartMatch()`는 Listen Server 호스트의 요청만 받아들이고 조건이 맞으면 기본 `/Game/Maps/L_Prototype?listen`으로 `ServerTravel`한다.
- `ASnowRumbleLobbyGameMode`의 기본 `PlayerControllerClass`는 `ALobbyPlayerController`이며, 로비 진입 시 로컬 클라이언트마다 `LobbyWidgetClass`를 자동 생성한다.
- `UMainMenuWidget`은 기존 LAN 세션 서브시스템 호출과 상태 이벤트를 시작화면 WBP에 전달한다.
- `AMainMenuPlayerController`는 시작화면 진입 시 로컬 클라이언트의 `MainMenuWidgetClass`를 자동 생성하고 마우스 커서와 UI 입력 모드를 적용한다.
- `ULobbyWidget`은 로컬 PlayerState 요청 함수와 대기방 목록/시작 가능 여부 조회 함수를 대기방 WBP에 제공한다.
- `UMainMenuWidget`은 `HostButton`, `FindButton`, `JoinFirstButton`, `StatusTextBlock`을 선택적으로 자동 바인딩한다.

## 수동 작업

1. 실행 중인 Unreal Editor를 종료한다.
2. `SnowRumbleEditor`를 `Development Editor`, `Win64` 구성으로 빌드하고 프로젝트를 연다.
3. `Content/Maps/`에 `L_Lobby` 맵을 만든다.
4. `BP_LobbyGameMode`를 만들고 부모 클래스를 `SnowRumbleLobbyGameMode`로 설정한다.
5. `L_Lobby`의 World Settings에서 GameMode Override를 `BP_LobbyGameMode`로 설정한다.
6. `BP_LobbyGameMode`에서 `Match Travel Url`이 기본 `/Game/Maps/L_Prototype?listen`인지 확인하고, 실제 게임방 맵을 따로 만들면 해당 경로로 바꾼다.
7. `WBP_MainMenu`를 만들고 부모 클래스를 `MainMenuWidget`으로 설정한다.
8. `WBP_MainMenu`에 버튼을 만들고 이름을 각각 `HostButton`, `FindButton`, `JoinFirstButton`으로 지정한다. 상태 표시가 필요하면 Text Block 이름을 `StatusTextBlock`으로 지정한다.
9. `BP_MainMenuPlayerController`를 만들고 부모 클래스를 `MainMenuPlayerController`로 설정한 뒤 `Main Menu Widget Class`에 `WBP_MainMenu`를 지정한다.
10. `BP_MainMenuGameMode`를 만들고 부모 클래스를 `SnowRumbleMainMenuGameMode`로 설정한다.
11. `BP_MainMenuGameMode`의 Player Controller Class를 `BP_MainMenuPlayerController`로 설정한다. C++ 기본값은 `MainMenuPlayerController`지만, WBP 클래스 지정을 위해 BP 컨트롤러를 사용하는 것이 좋다.
12. 시작화면 맵 `L_MainMenu`의 World Settings에서 GameMode Override를 `BP_MainMenuGameMode`로 설정한다.
13. `WBP_Lobby`를 만들고 부모 클래스를 `LobbyWidget`으로 설정한다.
14. `WBP_Lobby`에서 `On Lobby State Changed` 이벤트를 사용해 `Get Lobby Players` 목록을 다시 읽고 플레이어 이름·팀·준비 상태를 표시한다.
15. `WBP_Lobby`에서 이름 입력, 팀 버튼, 준비 버튼은 각각 `Request Set Local Player Name`, `Request Set Local Player Team`, `Request Set Local Player Ready`를 호출한다.
16. `WBP_Lobby`에서 시작 버튼은 `Is Local Player Host`가 true일 때만 표시하고, 클릭 시 `Request Start Match`를 호출한다.
17. 시작 버튼 활성화 여부는 `Can Start Match` 결과를 사용한다.
18. `BP_LobbyPlayerController`를 만들고 부모 클래스를 `LobbyPlayerController`로 설정한 뒤 `Lobby Widget Class`에 `WBP_Lobby`를 지정한다.
19. `BP_LobbyGameMode`의 Player Controller Class를 `BP_LobbyPlayerController`로 설정한다. C++ 기본값은 `LobbyPlayerController`지만, WBP 클래스 지정을 위해 BP 컨트롤러를 사용하는 것이 좋다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드 수정 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료 — `git diff --check`와 `SnowRumbleEditor` 빌드 성공
- [x] Unreal C++/Blueprint/에셋 규칙 위반 없음
- [x] 현재 task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인

- [ ] 시작화면에서 Host를 누르면 `L_Lobby` Listen Server로 이동하는지 확인한다.
- [ ] 다른 클라이언트가 검색 후 Join하면 같은 `L_Lobby`에 입장하는지 확인한다.
- [ ] 시작화면 진입 시 Level Blueprint 없이 `WBP_MainMenu`가 자동 표시되는지 확인한다.
- [ ] `L_Lobby` 진입 시 Level Blueprint 없이 `WBP_Lobby`가 자동 표시되는지 확인한다.
- [ ] 호스트와 클라이언트의 `WBP_Lobby`에서 같은 플레이어 목록이 보이는지 확인한다.
- [ ] 각 플레이어가 이름을 바꾸면 모든 화면의 대기방 목록에 같은 이름이 표시되는지 확인한다.
- [ ] 각 플레이어가 Red/Blue 팀을 선택하면 모든 화면에 같은 팀이 표시되는지 확인한다.
- [ ] 각 플레이어가 준비/준비 해제를 하면 모든 화면에 같은 준비 상태가 표시되는지 확인한다.
- [ ] 준비 인원 부족, 한 팀만 존재, 준비하지 않은 플레이어가 있을 때 `Can Start Match`가 false인지 확인한다.
- [ ] 2명 이상이 Red/Blue 양 팀에 나뉘고 모두 Ready일 때 `Can Start Match`가 true인지 확인한다.
- [ ] 클라이언트 화면에는 시작 버튼이 표시되지 않거나 눌러도 게임이 시작되지 않는지 확인한다.
- [ ] 호스트가 시작 버튼을 누르면 모든 참가자가 게임방 맵으로 이동하는지 확인한다.

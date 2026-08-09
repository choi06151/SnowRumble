# Task C-15 - 로비 게시판 상호작용

## 설명

로비 게시판 Blueprint가 기존 눈덩이처럼 가까이 가면 outline이 표시되고, `E` 입력으로 서버 검증된 상호작용 이벤트를 받을 수 있게 한다.

## 상태 전이 기준

- 시작 가능: 기본 캐릭터 E 상호작용과 outline 컴포넌트 재사용 가능
- 완료 가능: 게시판 C++ 부모, 캐릭터 후보 탐색, 서버 상호작용 요청, Blueprint 인계와 수동 확인 절차 기록 완료

## 구현 항목

- [x] 게시판 Blueprint가 상속할 C++ 부모 액터를 추가한다.
- [x] 로컬 플레이어가 게시판 근처에 붙으면 기존 outline 경로로 표시되게 한다.
- [x] `E` 입력 시 서버가 거리와 캐릭터 상태를 검증해 게시판 상호작용을 확정한다.
- [x] 게시판 Blueprint가 상호작용 결과를 연결할 이벤트를 제공한다.
- [x] outline으로 잡힌 게시판이 있을 때만 `E` 게시판 상호작용을 요청한다.
- [x] 게시판 상호작용 성공 시 소유 클라이언트 카메라가 게시판을 바라보게 한다.
- [x] 게시판 포커스 중 `E`를 다시 누르면 포커스를 해제하고 캐릭터 카메라로 돌아오게 한다.
- [x] 게시판 포커스 중에는 outline을 숨기고, 포커스 해제 후 근처 대상 outline이 기존 후보 탐색으로 다시 표시되게 한다.
- [x] 게시판 Blueprint 안에서 포커스 구도를 조정할 수 있는 카메라 컴포넌트를 제공한다.
- [x] 게시판 Blueprint 안에 붙일 월드 UI 컴포넌트와 WBP 부모를 제공한다.
- [x] 게시판 포커스 중 마우스 커서를 표시하고 이동·시점 입력을 차단한다.
- [x] 게시판 UI 버튼 클릭을 서버 검증된 게시판 액션 이벤트로 전달한다.
- [x] 게시판 자식 Blueprint에 추가한 여러 WidgetComponent를 클릭 대상으로 지원한다.
- [x] 팀 색 버튼 8개와 로비 모드 버튼 2개의 WBP 자동 바인딩을 제공한다.
- [x] 팀 색 버튼 클릭 시 서버 검증 후 PlayerState 팀 색을 변경한다.
- [x] 팀 색 변경을 이름표 글자색과 배경색에 반영한다.
- [x] 색별 현재 선택 인원 수 TextBlock 자동 표시를 제공한다.
- [x] 같은 준비/시작 버튼을 로컬 플레이어가 호스트면 `게임 시작`, 클라이언트면 `준비 완료` 또는 `준비 취소`로 표시하게 한다.
- [x] 준비/시작 버튼 클릭 시 호스트는 경기 시작을 요청하고, 클라이언트는 ready 상태를 토글하게 한다.
- [x] 기존 `WBP_Lobby`가 쓰는 `ULobbyWidget`에 준비 완료 인원 수, 현재 게임모드, 내 이름, 내 팀색, 내 준비 상태 자동 표시를 제공한다.
- [x] 머리 위 이름표 WBP에 준비 완료 이미지와 호스트 이미지 표시 바인딩을 제공한다.
- [x] 로비에서 공용 캐릭터의 로컬 snow VFX가 보이지 않고 PvP 맵에서만 보이게 제한한다.
- [x] 비호스트 참가자 전원이 준비 완료한 상태에서 호스트가 게임 시작을 누르면 PvP 전용 레벨로 이동한다.
- [x] 호스트가 게임 시작을 누르면 모든 참여자에게 로딩창을 즉시 띄우고, PvP 레벨에서 예상 인원이 모두 접속되면 로딩창을 닫는다.
- [x] 로딩창 WBP에 접속 완료 인원 기준 진행률 ProgressBar와 상태 TextBlock 바인딩을 제공한다.
- [x] PvP 레벨 이동 후에도 로비에서 확정한 플레이어 이름과 팀 색이 유지되게 한다.
- [x] PvP 레벨에서 모든 PlayerStart 중 매번 랜덤한 지점에 스폰되게 한다.
- [x] 현재 PvP 이동 기본 맵을 `L_Prototype`에서 겨울 환경 `DemoMap`으로 변경한다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: C++·문서: 최재원(C), 게시판 Blueprint·맵 배치: 사용자 또는 S 인계
- 생성·변경 후보: `Source/SnowRumble/Interaction/LobbyInteractionBoard_C.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Source/SnowRumble/UI/LobbyBoardWidget_C.*`, `Tasks/C/C-15_lobby_board_interaction.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`
- 공유 확인 대상: S-03 또는 사용자
- 병합 순서: C-03 대기방 상호작용 표현 전에 인계

## 공용 계약과 인계

- 제공받을 계약: 기존 `UOutlineComponent`, `ASnowRumbleCharacter`의 `InteractAction`
- 제공할 계약:
  - `ALobbyInteractionBoard`: 게시판 Blueprint가 상속할 C++ 부모 액터다.
  - `ALobbyInteractionBoard::BoardMeshComponent`: 게시판 모델과 Custom Depth outline이 적용될 기본 Static Mesh 컴포넌트다.
  - `ALobbyInteractionBoard::FocusCameraComponent`: 게시판 포커스 시 소유 클라이언트가 보는 카메라다. 게시판 Blueprint 안에서 위치와 회전을 조정해 포커스 구도를 정한다.
  - `ALobbyInteractionBoard::BoardWidgetComponent`: 게시판 표면에 붙일 월드 UI 컴포넌트다. 게시판 WBP 클래스를 에디터에서 연결한다.
  - `ALobbyInteractionBoard::GetBoardWidgetComponents(TArray<UWidgetComponent*>& OutWidgetComponents)`: 기본 `BoardWidgetComponent`와 자식 Blueprint에 추가한 `ULobbyBoardWidget` 기반 WidgetComponent들을 클릭 후보로 반환한다.
  - `ALobbyInteractionBoard::InteractionRadius`: 로컬 outline 후보 탐색과 서버 상호작용 검증에 공통으로 쓰는 거리다. 기본값은 320cm다.
  - `ALobbyInteractionBoard::CanInteractWith(const ASnowRumbleCharacter* Character)`: 캐릭터와 게시판 mesh bounds의 가장 가까운 점 사이 거리로 조건을 확인한다.
  - `ALobbyInteractionBoard::GetFocusLocation()`: 카메라가 바라볼 게시판 mesh bounds 중심을 반환한다.
  - `ALobbyInteractionBoard::HandleBoardAction(ASnowRumbleCharacter* Character, ELobbyBoardAction BoardAction)`: 서버가 게시판 UI 버튼 액션을 확정하고 Blueprint 이벤트로 전달한다.
  - `ALobbyInteractionBoard::OnBoardInteracted(ASnowRumbleCharacter* Character)`: 서버가 상호작용을 확정했을 때 Blueprint에서 UI나 연출을 연결할 이벤트다.
  - `ALobbyInteractionBoard::OnBoardActionRequested(ASnowRumbleCharacter* Character, ELobbyBoardAction BoardAction)`: 서버가 게시판 UI 버튼 액션을 확정했을 때 Blueprint에서 실제 동작을 연결할 이벤트다.
  - `ULobbyBoardWidget`: 게시판 WBP가 상속할 C++ 부모 위젯이다.
  - `ULobbyBoardWidget::BoardActionButton0~3`: WBP에서 같은 이름의 Button을 만들면 자동으로 `ELobbyBoardAction::Action0~3` 서버 요청에 연결된다.
  - `ULobbyBoardWidget::CloseFocusButton`: WBP에서 같은 이름의 Button을 만들면 게시판 포커스 해제에 연결된다.
  - `ULobbyBoardWidget::RedTeamButton`, `SkyTeamButton`, `GreenTeamButton`, `YellowTeamButton`, `PurpleTeamButton`, `PinkTeamButton`, `BlueTeamButton`, `WhiteTeamButton`: WBP에서 같은 이름의 Button을 만들면 팀 색 선택 이벤트에 자동 연결된다.
  - `ASnowRumbleLobbyGameState::GetReadyPlayerCount()`: 현재 준비 완료한 플레이어 수를 반환한다.
  - `ASnowRumbleLobbyGameState::GetReadyRequiredPlayerCount()`: 호스트를 제외한 준비 필요 플레이어 수를 반환한다.
  - `ASnowRumbleLobbyGameState::CanStartLobbyMatch()`: 참가자 수와 팀 선택을 검사하고, 호스트를 제외한 모든 참가자가 준비 완료했을 때만 true를 반환한다.
  - `ASnowRumbleLobbyGameState::GetLobbyMode()`: 현재 선택된 로비 게임 모드를 반환한다.
  - `ASnowRumbleLobbyGameState::SetLobbyModeFromServer(ESnowRumbleLobbyMode NewLobbyMode)`: 서버가 로비 게임 모드를 변경하고 UI 갱신 이벤트를 알린다.
  - `ULobbyBoardWidget::PvpModeButton`, `SnowmanModeButton`: WBP에서 같은 이름의 Button을 만들면 호스트 화면에서 현재 로비 모드 상태 변경에 자동 연결된다.
  - `ULobbyBoardWidget::RedTeamCountText`, `SkyTeamCountText`, `GreenTeamCountText`, `YellowTeamCountText`, `PurpleTeamCountText`, `PinkTeamCountText`, `BlueTeamCountText`, `WhiteTeamCountText`: WBP에서 같은 이름의 TextBlock을 만들면 현재 색별 선택 인원 수를 숫자로 자동 표시한다.
  - `ULobbyBoardWidget::ReadyStartButton`: WBP에서 같은 이름의 Button을 만들면 로컬 호스트는 게임 시작, 클라이언트는 준비 토글에 자동 연결된다.
  - `ULobbyBoardWidget::ReadyStartButtonText`: WBP에서 같은 이름의 TextBlock을 만들면 로컬 호스트는 `게임 시작`, 준비 전 클라이언트는 `준비 완료`, 준비 후 클라이언트는 `준비 취소`로 자동 표시된다.
  - `ULobbyWidget::ReadyPlayerCountText`: 기존 `WBP_Lobby`에 있으면 호스트를 제외한 준비 완료 인원 수와 준비 필요 인원 수를 `0 / 1` 형식으로 자동 표시한다.
  - `ULobbyWidget::CurrentGameModeText`: 기존 `WBP_Lobby`에 있으면 현재 로비 게임 모드를 자동 표시한다.
  - `ULobbyWidget::LocalPlayerNameText`: 기존 `WBP_Lobby`에 있으면 로컬 플레이어 이름을 자동 표시한다.
  - `ULobbyWidget::LocalTeamColorText`: 기존 `WBP_Lobby`에 있으면 로컬 플레이어 팀 색 이름을 자동 표시한다.
  - `ULobbyWidget::LocalReadyStateText`: 기존 `WBP_Lobby`에 있으면 로컬 플레이어 준비 상태를 자동 표시한다.
  - `ULobbyWidget::LocalTeamColorBorder`: 기존 `WBP_Lobby`에 있으면 로컬 플레이어 팀 색을 swatch 배경색으로 자동 표시한다.
  - `UOverheadNameplateWidget::ReadyStateImage`: 있으면 관찰 대상이 호스트가 아니고 준비 완료 상태일 때만 표시한다.
  - `UOverheadNameplateWidget::HostStateImage`: 있으면 관찰 대상이 로비 호스트일 때만 표시한다.
  - `ULobbyBoardWidget::OnTeamColorButtonClicked(ELobbyBoardTeamColor TeamColor)`: 팀 색 버튼 클릭 시 WBP가 표시 반응을 처리하는 이벤트다. 실제 팀 변경 요청은 C++ 부모가 서버 검증 경로로 보낸다.
  - `ULobbyBoardWidget::OnLobbyModeButtonClicked(ELobbyBoardGameMode GameMode)`: 로비 모드 버튼 클릭 시 WBP가 표시 반응이나 이후 기능 연결을 처리하는 이벤트다.
  - `ULobbyBoardWidget::SetFocusedPlayerController(ALobbyPlayerController* NewPlayerController)`: 게시판 포커스 중 월드 위젯이 listen 환경에서도 소유 로컬 컨트롤러 RPC 경로를 잃지 않게 한다.
  - `ASnowRumbleCharacter::FindClosestLobbyBoardCandidate()`: 로컬 outline과 `E` 입력 대상 게시판을 찾는다.
  - `ASnowRumbleCharacter::ServerTryInteractWithLobbyBoard(ALobbyInteractionBoard* Board)`: 서버가 캐릭터 상태와 게시판 거리를 다시 검사해 상호작용을 확정한다.
  - `ASnowRumbleCharacter::ClientFocusLobbyBoard(ALobbyInteractionBoard* Board)`: 서버가 상호작용을 확정한 뒤 소유 클라이언트의 ViewTarget을 게시판으로 전환한다. 같은 게시판이 이미 포커스 중이면 포커스를 해제한다.
  - `ASnowRumbleCharacter::RequestLobbyBoardAction(ELobbyBoardAction BoardAction)`: 포커스 중인 게시판 UI 버튼 액션을 서버 검증 요청으로 전달한다.
  - `ASnowRumbleCharacter::RequestLobbyTeamSelection(ESnowRumbleTeam NewTeam)`: 포커스 중인 게시판 팀 색 선택을 캐릭터 소유 서버 RPC로 전달한다.
  - `ASnowRumbleCharacter::ShouldShowLocalSnowEffect()`: 로컬 플레이어가 PvP 맵에 있을 때만 카메라 snow VFX 표시를 허용한다.
  - `ULoadingScreenWidget`: 로딩창 WBP가 상속할 C++ 부모다. WBP가 없을 때는 기본 `Loading...` 화면을 표시한다.
  - `ULoadingScreenWidget::LoadingProgressBar`: 있으면 현재 PvP 레벨 접속 완료 인원 / 예상 인원 비율을 자동 표시한다.
  - `ULoadingScreenWidget::LoadingStatusText`: 있으면 현재 접속 완료 인원과 예상 인원을 `1 / 2` 형식으로 자동 표시한다.
  - `ULoadingScreenWidget::LoadingMessageText`: 있으면 기본 로딩 문구를 자동 표시한다.
  - `ULoadingScreenWidget::GetLoadingProgress()`: WBP 바인딩에서 사용할 0~1 진행률을 반환한다.
  - `ULoadingScreenWidget::GetLoadingStatusText()`: WBP 바인딩에서 사용할 접속 인원 텍스트를 반환한다.
  - `ULoadingScreenSubsystem`: 로컬 GameInstance에 로딩 위젯을 유지해 레벨 이동 중에도 로딩창 표시 상태를 보존한다.
  - `ASnowRumblePlayerController::ClientShowLoadingScreen()`: 서버가 매치 이동 직전에 소유 클라이언트 로딩창 표시를 요청한다.
  - `ASnowRumblePlayerController::ClientUpdateLoadingProgress(int32 LoadedPlayers, int32 ExpectedPlayers)`: 서버가 PvP 레벨 접속 진행률을 소유 클라이언트에 전달한다.
  - `ASnowRumblePlayerController::ClientHideLoadingScreen()`: PvP GameMode가 예상 인원 접속 완료 후 소유 클라이언트 로딩창 제거를 요청한다.
  - `ASnowRumbleGameMode`: PvP 레벨에서 `ExpectedPlayers` URL 옵션을 읽고, 해당 인원이 모두 `PostLogin` 되면 로딩창을 닫는다.
  - `ASnowRumblePlayerState::CopyProperties(APlayerState* PlayerState)`: seamless travel 시 로비 이름, 팀 색, 준비 상태, 호스트 상태를 새 PlayerState로 복사한다.
  - `ASnowRumblePlayerState::OverrideWith(APlayerState* PlayerState)`: seamless travel 복구 경로에서 기존 로비 이름과 팀 색 상태를 현재 PlayerState에 반영한다.
  - `ASnowRumbleGameMode::ChoosePlayerStart_Implementation(AController* Player)`: PvP 레벨에서 전체 PlayerStart 중 아직 사용하지 않은 지점을 랜덤 선택하고, 후보를 모두 사용하면 전체 후보 중 다시 랜덤 선택한다.
  - `ASnowRumbleCharacter::ServerRequestLobbyTeamSelection(ALobbyInteractionBoard* Board, ESnowRumbleTeam NewTeam)`: 서버가 게시판 거리와 캐릭터 상태를 다시 검사한 뒤 해당 캐릭터의 PlayerState 팀을 변경한다.
  - `ALobbyPlayerController::RequestApplyLobbyTeam(ESnowRumbleTeam NewTeam)`: 소유 클라이언트가 로비 팀 색을 서버 PlayerState에 적용하도록 요청한다.
  - `ASnowRumbleCharacter::ServerRequestLobbyBoardAction(ALobbyInteractionBoard* Board, ELobbyBoardAction BoardAction)`: 서버가 캐릭터 상태와 게시판 거리를 다시 검사해 UI 버튼 액션을 확정한다.
  - `ASnowRumblePlayerState::GetLobbyTeamColor()`: 현재 팀 색을 이름표 글자색과 배경색에 사용할 `FLinearColor`로 반환한다.
  - `ASnowRumblePlayerState::IsLobbyHost()`: 복제된 대기방 호스트 여부를 반환한다. 준비 인원 수 표시에서 호스트를 제외하는 기준으로 사용한다.
  - `UOverheadNameplateWidget::PlayerNameBackgroundBorder`: 있으면 C++ 부모가 팀 색을 배경에 자동 적용한다.
  - `ASnowRumbleCharacter::ClearLobbyBoardFocus()`: 로컬 플레이어 화면의 게시판 포커스를 해제하고 캐릭터 카메라로 복귀한다.
- 인계 대상: S-03 또는 사용자

## 범위 밖

- 없음
- 게시판 UI 레이아웃, 모델, 머티리얼과 맵 배치
- 상호작용 안내 UI

## 사전 전제

- 기존 캐릭터 `InteractAction`과 `UOutlineComponent`

## 결정 필요

- 없음

## 변경 기록

- 2026-08-08: 사용자가 기존 눈덩이처럼 동작하는 게시판 outline과 `E` 상호작용을 요청해 C-15를 추가하고 구현했다.
- 2026-08-08: 게시판 크기 때문에 중심점 거리 기준으로 outline 후보가 잘 잡히지 않는 문제에 대응해 mesh bounds 기준 거리 검사로 바꾸고 기본 상호작용 반경을 320cm로 늘렸다.
- 2026-08-08: 눈덩이 상호작용 거리는 기존 `USnowballEquipmentComponent::PickupRadius` 180cm를 유지하고, 게시판은 `ALobbyInteractionBoard::InteractionRadius`에서 개별 조절하게 정리했다. `E` 입력은 현재 outline으로 잡힌 게시판에만 적용하며, 성공 시 카메라를 게시판으로 돌린다.
- 2026-08-09: 연속 `E` 입력으로 포커스 구도가 계속 바뀌는 문제에 대응해 게시판 포커스를 토글 방식으로 바꾸고, `ALobbyInteractionBoard::FocusCameraComponent`를 추가해 게시판 Blueprint 안에서 포커스 구도를 조정할 수 있게 했다.
- 2026-08-09: 게시판 포커스 중 마우스 커서와 UI 입력 모드를 켜고 이동·시점 입력을 차단하도록 보강했다. 게시판 WBP 부모 `ULobbyBoardWidget`과 `BoardWidgetComponent`를 추가해 버튼 클릭을 서버 검증된 `OnBoardActionRequested` 이벤트로 전달하게 했다.
- 2026-08-09: 게시판 클릭 후보를 단일 `BoardWidgetComponent`에서 `ULobbyBoardWidget` 기반 WidgetComponent 전체로 확장했다. 자식 Blueprint에 WidgetComponent를 추가하고 같은 부모 WBP를 연결하면 마우스 ray가 맞는 가장 가까운 위젯에 클릭을 전달한다.

## 수동 작업

- Unreal Editor에서 `ALobbyInteractionBoard`를 부모로 하는 `BP_LobbyInteractionBoard_C` Blueprint를 만든다.
- `BP_LobbyInteractionBoard_C`의 `BoardMeshComponent`에 게시판 Static Mesh와 머티리얼을 연결한다.
- `BP_LobbyInteractionBoard_C`의 `FocusCameraComponent` 위치와 회전을 원하는 게시판 포커스 구도로 조정한다.
- Unreal Editor에서 `ULobbyBoardWidget`을 부모로 하는 게시판 WBP를 만든다.
- 게시판 WBP 안에 필요한 Button을 만들고 이름을 `BoardActionButton0`, `BoardActionButton1`, `BoardActionButton2`, `BoardActionButton3`, `CloseFocusButton` 중 필요한 이름으로 맞춘다.
- 팀 색 버튼은 `RedTeamButton`, `SkyTeamButton`, `GreenTeamButton`, `YellowTeamButton`, `PurpleTeamButton`, `PinkTeamButton`, `BlueTeamButton`, `WhiteTeamButton` 이름으로 만든다.
- 팀 색 인원 수 텍스트는 `RedTeamCountText`, `SkyTeamCountText`, `GreenTeamCountText`, `YellowTeamCountText`, `PurpleTeamCountText`, `PinkTeamCountText`, `BlueTeamCountText`, `WhiteTeamCountText` 이름으로 만든다.
- 로비 모드 버튼은 `PvpModeButton`, `SnowmanModeButton` 이름으로 만든다.
- 준비/시작 버튼은 `ReadyStartButton` 이름으로 만들고, 버튼 안의 텍스트는 `ReadyStartButtonText` 이름으로 만든다.
- 기존 `WBP_Lobby`에는 필요한 TextBlock 또는 Border를 `ReadyPlayerCountText`, `CurrentGameModeText`, `LocalPlayerNameText`, `LocalTeamColorText`, `LocalReadyStateText`, `LocalTeamColorBorder` 이름으로 만든다.
- 로딩창 커스텀이 필요하면 `ULoadingScreenWidget`을 부모로 하는 WBP를 만들고, 로비 PlayerController Blueprint의 `LoadingScreenWidgetClass`에 연결한다.
- 로딩창 WBP에 ProgressBar를 추가하면 이름을 `LoadingProgressBar`로 맞춘다.
- 로딩창 WBP에 접속 인원 텍스트가 필요하면 TextBlock 이름을 `LoadingStatusText`로 맞춘다.
- 로딩창 WBP에 기본 문구 텍스트가 필요하면 TextBlock 이름을 `LoadingMessageText`로 맞춘다.
- `L_Prototype` World Settings의 GameMode Override가 `ASnowRumbleGameMode` 또는 그 Blueprint 자식인지 확인한다. 이 GameMode가 적용되어야 예상 인원 접속 완료 후 로딩창 닫기 RPC가 실행된다.
- 현재 기본 PvP 이동 대상은 `/Game/LowpolyStyle/WinterEnvironment/Maps/DemoMap?listen`이다. 다른 PvP 맵이 준비되면 `ASnowRumbleLobbyGameMode::MatchTravelUrl` 또는 로비 GameMode Blueprint의 `MatchTravelUrl`을 새 맵 경로로 바꾼다.
- PvP 대상 레벨의 World Settings GameMode Override가 `ASnowRumbleGameMode` 또는 그 Blueprint 자식인지 확인한다.
- PvP 대상 레벨에 PlayerStart를 원하는 수만큼 배치한다. `Player Start Tag`는 스폰 규칙에 사용하지 않는다.
- `BP_LobbyInteractionBoard_C`의 `BoardWidgetComponent`에 게시판 WBP 클래스를 연결하고 게시판 표면에 맞게 위치·회전·크기를 조정한다.
- 게시판에 두 번째 이상의 WidgetComponent가 필요하면 `BP_LobbyInteractionBoard_C` 자식 Blueprint에 WidgetComponent를 추가하고, Widget Class를 `ULobbyBoardWidget` 기반 WBP로 설정한 뒤 위치·회전·크기를 조정한다.
- `BP_LobbyInteractionBoard_C`를 로비 맵에 배치한다.
- 상호작용 확인을 위해 `OnBoardInteracted` 이벤트에 임시 Print String 또는 열릴 UI 호출을 연결한다.
- 버튼 상호작용 확인을 위해 `OnBoardActionRequested` 이벤트에 임시 Print String 또는 실제 게시판 액션 호출을 연결한다.
- 이름표 WBP에 배경색 자동 적용이 필요하면 Border를 추가하고 이름을 `PlayerNameBackgroundBorder`로 맞춘다.
- 이름표 WBP에 준비 완료 표시 Image를 추가하고 이름을 `ReadyStateImage`로 맞춘다.
- 이름표 WBP에 호스트 표시 Image를 추가하고 이름을 `HostStateImage`로 맞춘다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드 변경 완료
- [x] `git diff --check` 공백 점검 통과
- [x] `SnowRumbleEditor Win64 Development` 빌드 성공
- [x] 게시판 bounds 거리 판정 변경 후 최종 링크 확인
- [x] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [x] 공용 계약과 캡슐화 규칙 위반 없음
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨

### 검증 메모

- 2026-08-08: 게시판 bounds 거리 판정 변경 후 `LobbyInteractionBoard_C.cpp`, `Module.SnowRumble.cpp`, `SnowRumbleCharacter.cpp` 컴파일은 통과했지만, 실행 중인 Unreal Editor가 `Binaries/Win64/UnrealEditor-SnowRumble.dll`을 잡고 있어 링크가 `LNK1104`로 실패했다. 에디터 종료 후 재빌드 확인이 필요하다.
- 2026-08-08: outline 대상 기준 `E` 상호작용과 `ClientFocusLobbyBoard` 카메라 focus RPC 추가 후 `LobbyInteractionBoard_C.cpp`, `Module.SnowRumble.cpp`, `OverheadNameplateWidget_C.cpp`, `SnowRumbleCharacter.cpp`, `SnowRumbleLobbyGameMode.cpp` 컴파일은 통과했다. 링크는 실행 중인 Unreal Editor의 DLL 잠금으로 다시 `LNK1104` 실패했다.
- 2026-08-08: Unreal Editor 종료 상태에서 `SnowRumbleEditor Win64 Development` 최종 빌드가 `Result: Succeeded`로 완료됐다.
- 2026-08-09: 게시판 포커스 토글과 `FocusCameraComponent` 추가 후 `git diff --check`와 `SnowRumbleEditor Win64 Development` 빌드가 `Result: Succeeded`로 완료됐다.
- 2026-08-09: 게시판 월드 UI와 `ULobbyBoardWidget`, `LobbyBoardWidgetInteractionComponent` 추가 후 `git diff --check`는 통과했다. `LobbyBoardWidget_C.cpp`, `LobbyInteractionBoard_C.cpp`, `Module.SnowRumble.cpp`, `SnowRumbleCharacter.cpp` 컴파일은 통과했지만, 실행 중인 Unreal Editor 프로세스가 `Binaries/Win64/UnrealEditor-SnowRumble.dll`을 잡고 있어 링크가 `LNK1104`로 실패했다. 에디터 종료 후 재빌드 확인이 필요하다.
- 2026-08-09: 게시판 WBP 버튼 클릭이 안 되는 문제에 대응해 `BoardWidgetComponent`가 `Visibility` trace를 block하도록 보강하고, 포커스 중 좌클릭 감지를 입력 바인딩 대신 Tick 기반 `WasInputKeyJustPressed/Released`로 처리하게 했다. `git diff --check`와 관련 C++ 컴파일은 통과했으며, 링크는 실행 중인 Unreal Editor DLL 잠금으로 `LNK1104` 실패했다.
- 2026-08-09: 게시판 WBP 클릭 판정을 마우스 화면 좌표 deproject 기반으로 바꿨다. 포커스 중 마우스 좌표에서 월드 ray를 만들고, 현재 게시판의 `BoardWidgetComponent`에 대해 직접 `LineTraceComponent`를 수행한 hit result를 `LobbyBoardWidgetInteractionComponent`의 custom hit result로 넣은 뒤 pointer press/release를 전달한다. `git diff --check`와 `SnowRumbleEditor Win64 Development` 빌드가 `Result: Succeeded`로 완료됐다.
- 2026-08-09: `BoardWidgetComponent`의 collision trace 의존을 제거하고, 마우스 ray와 위젯 컴포넌트 평면의 교차점을 직접 계산해 위젯 사각형 안일 때 custom hit result를 구성하도록 바꿨다. `git diff --check`와 `SnowRumbleCharacter.cpp` 컴파일은 통과했으며, 링크는 실행 중인 Unreal Editor DLL 잠금으로 `LNK1104` 실패했다.
- 2026-08-09: 다중 게시판 WidgetComponent 클릭 후보 지원 후 `git diff --check`와 `SnowRumbleEditor Win64 Development` 빌드가 `Result: Succeeded`로 완료됐다.
- 2026-08-09: 기능 구현 전 단계로 `ULobbyBoardWidget`에 팀 색 버튼 8개와 로비 모드 버튼 2개의 자동 바인딩을 추가했다. 현재는 WBP 이벤트까지만 전달하고 실제 팀 색·모드 변경 서버 기능은 범위 밖으로 둔다.
- 2026-08-09: 팀 색·로비 모드 버튼 바인딩 추가 후 `git diff --check`와 `LobbyBoardWidget_C.cpp`, `LobbyInteractionBoard_C.cpp`, `Module.SnowRumble.cpp` 컴파일은 통과했다. 링크는 실행 중인 Unreal Editor DLL 잠금으로 `LNK1104` 실패했다.
- 2026-08-09: 사용자가 기존 Red/Blue 2팀이 아니라 8개 색 각각이 팀이 될 수 있게 요청해 로비 팀 모델을 8색으로 확장했다. 게시판 팀 색 버튼 클릭은 서버 검증 후 `ASnowRumblePlayerState::LobbyTeam`을 바꾸며, 이름표 글자색과 배경색은 복제된 팀 색을 따른다.
- 2026-08-09: 8색 팀 선택, 이름표 색 반영, 다색 팀 시작 조건 변경 후 `git diff --check`와 `SnowRumbleEditor Win64 Development` 빌드가 `Result: Succeeded`로 완료됐다.
- 2026-08-09: `ULobbyBoardWidget`에 색별 팀 인원 수 TextBlock 자동 바인딩을 추가했다. 현재 로비 PlayerState의 팀 색 선택 수를 읽어 `0`, `1`, `2` 같은 숫자로 표시한다.
- 2026-08-09: 색별 팀 인원 수 TextBlock 추가 후 `git diff --check`와 `LobbyBoardWidget_C.cpp`, `LobbyInteractionBoard_C.cpp`, `Module.SnowRumble.cpp` 컴파일은 통과했다. 링크는 실행 중인 Unreal Editor DLL 잠금으로 `LNK1104` 실패했다.
- 2026-08-09: Listen 클라이언트에서 팀 변경이 적용되지 않는 문제에 대응해 팀 색 선택 경로를 캐릭터 서버 거리 재검증 RPC에서 소유 `ASnowRumblePlayerState::RequestSetLobbyTeam` 서버 RPC로 변경했다. 게시판 포커스 상태에서만 요청을 보내는 로컬 게이트는 유지한다.
- 2026-08-09: Listen 팀 변경 요청 경로 수정 후 `git diff --check`와 관련 C++ 컴파일은 통과했다. 링크는 실행 중인 Unreal Editor DLL 잠금으로 `LNK1104` 실패했다.
- 2026-08-09: Listen 환경에서 서버와 클라이언트 모두 팀 변경이 적용되지 않는 문제에 대응해 팀 색 변경을 닉네임 적용과 같은 `ALobbyPlayerController` 소유 RPC 경로로 옮겼다. 월드 위젯이 `FocusedCharacter`를 못 잡는 경우에는 로컬 LobbyPlayerController fallback으로 요청한다.
- 2026-08-09: LobbyPlayerController 팀 색 RPC 경로 변경 후 `git diff --check`와 `SnowRumbleEditor Win64 Development` 빌드가 `Result: Succeeded`로 완료됐다.
- 2026-08-09: Listen 환경에서 게시판 월드 위젯이 잘못된 로컬 컨트롤러를 fallback으로 잡을 수 있어, 포커스 시점의 소유 `ALobbyPlayerController`를 `ULobbyBoardWidget`에 직접 저장하고 팀 색 요청은 이 컨트롤러 RPC로 우선 전달하게 보강했다.
- 2026-08-09: 포커스 컨트롤러 보강 후 `git diff --check`와 `SnowRumbleEditor Win64 Development` 빌드가 `Result: Succeeded`로 완료됐다.
- 2026-08-09: Listen 환경에서 여전히 팀 변경이 적용되지 않아 팀 색 버튼 경로를 캐릭터 소유 서버 RPC로 직접화했다. 버튼 클릭 시 로컬 포커스 게시판 포인터와 팀 색을 `ServerRequestLobbyTeamSelection`에 보내고, 서버가 거리 검증 후 해당 캐릭터의 PlayerState 팀을 변경한다.
- 2026-08-09: 캐릭터 소유 팀 선택 RPC 추가 후 `git diff --check`는 통과했고 관련 C++ 컴파일도 통과했다. 링크는 실행 중인 Unreal Editor 프로세스가 `Binaries/Win64/UnrealEditor-SnowRumble.dll`을 잡고 있어 `LNK1104`로 실패했다. 에디터 종료 후 재빌드 확인이 필요하다.
- 2026-08-09: Standalone에서는 로그와 팀 변경이 정상이고 listen 2창에서는 버튼 로그 자체가 없다는 결과를 바탕으로, 멀티 PIE/listen의 `UWidgetInteractionComponent` Slate 입력 충돌 가능성에 대응했다. 게시판 포커스 시 로컬 컨트롤러와 캐릭터의 고유 ID로 `VirtualUserIndex`와 `PointerIndex`를 설정하고, 포인터 press/release 전달 로그를 추가했다.
- 2026-08-09: listen 2창 월드 UI 포인터 보강 후 `git diff --check`와 관련 C++ 컴파일은 통과했다. 링크는 실행 중인 Unreal Editor 프로세스가 `Binaries/Win64/UnrealEditor-SnowRumble.dll`을 잡고 있어 `LNK1104`로 실패했다. 에디터 종료 후 재빌드 확인이 필요하다.
- 2026-08-09: 사용자가 listen 2창에서 팀 변경 정상 동작을 확인했다. 추적용 로그와 화면 디버그 메시지를 제거하고, 고유 `VirtualUserIndex`/`PointerIndex` 설정은 유지했다.
- 2026-08-09: 게시판 포커스 중에는 outline 후보 탐색을 멈춰 현재 outline을 숨기고, 포커스 해제 후에는 기존 로컬 후보 탐색으로 근처 게시판 outline이 다시 표시되게 했다.
- 2026-08-09: 게시판 WBP의 `ReadyStartButton`과 `ReadyStartButtonText` 자동 바인딩을 추가했다. 같은 버튼이 로컬 호스트에게는 `게임 시작`으로 보이고 클릭 시 시작을 요청하며, 클라이언트에게는 ready 상태에 따라 `준비 완료` 또는 `준비 취소`로 보이고 클릭 시 ready를 토글한다.
- 2026-08-09: 준비/시작 버튼 자동 바인딩 추가 후 `git diff --check`와 관련 C++ 컴파일은 통과했다. 링크는 실행 중인 Unreal Editor 프로세스가 `Binaries/Win64/UnrealEditor-SnowRumble.dll`을 잡고 있어 `LNK1104`로 실패했다. 에디터 종료 후 재빌드 확인이 필요하다.
- 2026-08-09: 기존 `WBP_Lobby`가 쓰는 `ULobbyWidget`에 준비 완료 인원 수·현재 게임모드·내 이름·내 팀색·내 준비 상태 자동 표시 바인딩을 추가했다. 준비 완료 인원 수는 호스트를 제외한 클라이언트 기준으로 표시하고, 로비 GameState에는 복제 로비 모드 상태를 유지한다.
- 2026-08-09: `ReadyPlayerCountText`를 호스트 제외 기준으로 조정했다. 2인 listen 대기방에서는 클라이언트가 준비 전이면 `0 / 1`, 준비 후면 `1 / 1`로 표시된다.
- 2026-08-09: 클라이언트 화면에서 호스트 제외 카운트가 `0 / 2`로 보이는 문제를 확인했다. 원격 클라이언트는 호스트 PlayerState의 Owner/Controller 정보를 안정적으로 판정할 수 없으므로, `ASnowRumblePlayerState::bLobbyHost` 복제 플래그를 추가하고 서버 `PostLogin`에서 listen 호스트만 true로 지정하게 했다.
- 2026-08-09: 이름표 WBP에 `ReadyStateImage`, `HostStateImage` 선택 바인딩을 추가했다. 준비 이미지는 비호스트 플레이어가 ready일 때만 표시하고, 호스트 이미지는 로비 호스트에게 항상 표시한다.
- 2026-08-09: 로비와 PvP가 같은 캐릭터 클래스를 공유하면서 로비에서도 `LocalSnowEffect`가 보이는 문제를 막았다. 로컬 snow VFX는 GameState가 존재하고 로비 GameState가 아닌 PvP 맵에서만 활성화한다.
- 2026-08-09: 게임 시작 조건을 호스트 제외 전원 ready 기준으로 정리했다. 호스트가 `게임 시작`을 누르면 현재 로비 모드가 PvP이고 시작 조건을 만족할 때 `/Game/Maps/L_Prototype?listen`으로 ServerTravel한다.
- 2026-08-09: 매치 시작 로딩창 흐름을 추가했다. 서버는 시작 직전 모든 `ASnowRumblePlayerController`에 로딩창 표시 RPC를 보내고, PvP 이동 URL에 `ExpectedPlayers`를 붙인다. PvP `ASnowRumbleGameMode`는 예상 인원이 모두 `PostLogin` 되면 전체 로딩창 닫기 RPC를 보낸다.
- 2026-08-09: 로딩창 진행률을 접속 완료 인원 기준으로 추가했다. 로비에서는 `0 / 예상 인원`으로 시작하고, PvP GameMode는 `PostLogin`마다 모든 접속 완료 클라이언트에 현재 인원 / 예상 인원을 RPC로 갱신한다.
- 2026-08-09: PvP 레벨 이동 후 이름표가 `DESKTOP-...`와 흰색으로 되돌아가는 문제를 막기 위해 로비 매치 이동을 seamless travel로 전환하고, `ASnowRumblePlayerState`의 로비 이름·팀 색 상태를 새 PlayerState로 복사하게 했다. 로딩창 표시 시 기존 로비 UI도 함께 숨긴다.
- 2026-08-09: PvP 스폰을 전체 PlayerStart 랜덤 선택으로 변경했다. 먼저 아직 사용하지 않은 PlayerStart 중 랜덤 선택하고, 후보를 모두 사용하면 전체 PlayerStart 중 다시 랜덤 선택한다. 기본 PvP 이동 대상은 겨울 환경 `DemoMap`으로 변경했다.

### 결과 확인

- [ ] 로비 맵에서 플레이어가 게시판 가까이 가면 게시판 mesh에 outline이 표시된다.
- [ ] 게시판에서 멀어지면 outline이 사라진다.
- [ ] 게시판 가까이에서 `E`를 눌렀다 떼면 서버가 `OnBoardInteracted`를 실행한다.
- [ ] 게시판 거리 밖에서 `E`를 눌러도 `OnBoardInteracted`가 실행되지 않는다.
- [ ] 게시판 outline이 표시된 상태에서만 `E` 입력으로 게시판 상호작용이 실행된다.
- [ ] 게시판 상호작용 성공 후 로컬 카메라가 `FocusCameraComponent` 구도로 전환된다.
- [ ] 게시판 포커스 중 마우스 커서가 표시되고 버튼 클릭이 가능하다.
- [ ] 게시판 포커스 중 이동과 시점 입력이 동작하지 않는다.
- [ ] 게시판 포커스 중 게시판 outline이 보이지 않는다.
- [ ] 포커스 해제 후 플레이어가 게시판 근처에 있으면 게시판 outline이 다시 보인다.
- [ ] `BoardActionButton0~3` 클릭 시 서버가 `OnBoardActionRequested`를 실행한다.
- [ ] 자식 Blueprint에 추가한 두 번째 WidgetComponent의 버튼도 같은 방식으로 클릭된다.
- [ ] 팀 색 버튼 8개 클릭 시 `OnTeamColorButtonClicked`가 맞는 enum 값으로 실행된다.
- [ ] 팀 색 버튼 8개 클릭 시 현재 플레이어의 이름표 글자색과 배경색이 해당 색으로 바뀐다.
- [ ] 팀 색 변경이 호스트와 클라이언트 화면 모두에서 같은 이름표 색으로 보인다.
- [ ] 색별 인원 수 TextBlock이 현재 선택 인원 수를 숫자로 표시한다.
- [ ] `PvpModeButton`, `SnowmanModeButton` 클릭 시 `OnLobbyModeButtonClicked`가 맞는 enum 값으로 실행된다.
- [ ] 호스트 화면에서는 `ReadyStartButtonText`가 `게임 시작`으로 보인다.
- [ ] 클라이언트 화면에서는 준비 전 `ReadyStartButtonText`가 `준비 완료`로 보이고, 클릭 후 `준비 취소`로 바뀐다.
- [ ] 클라이언트가 준비 완료한 뒤 호스트가 `게임 시작`을 클릭하면 시작 조건이 맞을 때 경기 맵 이동이 요청된다.
- [ ] 비호스트 참가자 전원이 준비 완료하고 호스트가 `게임 시작`을 누르면 `L_Prototype` PvP 레벨로 이동한다.
- [ ] 호스트가 `게임 시작`을 누르면 호스트와 클라이언트 화면에 로딩창이 즉시 표시된다.
- [ ] 로딩창의 `LoadingProgressBar`가 PvP 레벨 접속 완료 인원 기준으로 증가한다.
- [ ] 로딩창의 `LoadingStatusText`가 `0 / 2`, `1 / 2`, `2 / 2`처럼 갱신된다.
- [ ] PvP 레벨에서 예상 참여 인원이 모두 접속되면 모든 화면의 로딩창이 사라진다.
- [ ] PvP 레벨 이동 후에도 각 플레이어 이름표가 로비에서 설정한 이름과 팀 색으로 유지된다.
- [ ] PvP 레벨에서 플레이어들은 전체 PlayerStart 중 랜덤한 지점에 스폰된다.
- [ ] 로비 GameState에서는 인게임 `MainHUDWidget`이 보이지 않고 기존 `WBP_Lobby`가 보인다.
- [ ] 기존 `WBP_Lobby`의 준비 완료 인원 수는 호스트를 제외해 표시되고, 2인 listen 대기방에서 클라이언트 준비 전 `0 / 1`, 준비 후 `1 / 1`로 보인다.
- [ ] 기존 `WBP_Lobby`의 현재 게임모드, 내 이름, 내 팀색, 내 준비 상태가 호스트와 클라이언트 각각 자신의 로컬 상태에 맞게 표시된다.
- [ ] 비호스트 플레이어가 준비 완료하면 해당 플레이어의 이름표 `ReadyStateImage`가 보이고, 준비 취소하면 숨겨진다.
- [ ] 로비 호스트 플레이어의 이름표 `HostStateImage`는 로비에서 항상 보인다.
- [ ] 로비에서는 로컬 플레이어 카메라 snow VFX가 보이지 않는다.
- [ ] PvP 맵으로 이동한 뒤에는 로컬 플레이어 카메라 snow VFX가 보인다.
- [ ] 거리 밖이거나 포커스 대상이 아닌 게시판 버튼 액션은 실행되지 않는다.
- [ ] 게시판 포커스 중 `E`를 다시 누르면 캐릭터 카메라로 돌아온다.
- [ ] `CloseFocusButton` 클릭 시 캐릭터 카메라로 돌아온다.
- [ ] 게시판 포커스와 해제를 반복해도 카메라가 더 확대되거나 구도가 누적 변경되지 않는다.
- [ ] 포커스 해제 후 마우스 커서가 사라지고 이동·시점 입력이 복구된다.
- [ ] 근처에 게시판이 없으면 기존 눈덩이 획득 상호작용이 기존처럼 동작한다.

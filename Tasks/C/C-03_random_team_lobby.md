# Task C-03 - 랜덤 팀 대기방

## 설명

대기방 참가자를 서버가 두 팀에 무작위로 배정하고 양 팀 인원이 같을 때만 준비 완료 후 시작할 수 있게 한다.

## 상태 전이 기준
- 시작 가능: C-02 완료
- 완료 가능: 1vs1~4vs4 동수 검사와 S-03 인계 확인

## 구현 항목
- [x] 로비 입장 시 지정된 `PlayerStart`에서 기본 캐릭터 Pawn으로 스폰되게 한다.
- [x] 로비 UI가 표시된 상태에서도 플레이어가 기본 이동 입력을 사용할 수 있게 한다.
- [x] 서버가 참가자를 두 팀 중 하나로 무작위 배정한다.
- [x] 팀당 최대 4명과 양 팀 동수 조건을 검사한다.
- [x] 모든 참가자의 준비 상태와 호스트 시작 권한을 서버가 확정한다.
- [x] 직접 팀 선택 기능은 MVP에서 비활성화한다.
- [x] 호스트가 방 설정을 실제로 변경하면 비호스트 클라이언트에 개인 텍스트 알림을 표시한다.

## 작업 배정
- 담당자·기능·계약 소유자: 최재원(C)
- 자산 수정자·생성·변경 후보: 구현 승인 전 확정
- 공유 확인 대상: S-03, C-08
- 병합 순서: C-02 후, S-03·C-04 전

## 공용 계약과 인계
- 제공받을 계약: C-02 참가자 세션 상태
- 제공할 계약:
  - `ASnowRumbleLobbyGameMode::AssignLobbyTeam(APlayerController* NewPlayer)`: 서버 `PostLogin` 이후 새 참가자를 팀당 최대 4명 안에서 인원이 적은 팀에 배정하고, 양 팀 인원이 같으면 Red 또는 Blue 중 무작위로 배정한다.
  - `ASnowRumblePlayerState::AssignLobbyTeamFromServer(ESnowRumbleTeam NewTeam)`: 서버 전용 팀 배정 적용 경로다. 배정 시 준비 상태는 false로 초기화한다.
  - `ASnowRumblePlayerState::RequestSetLobbyTeam(ESnowRumbleTeam NewTeam)`: MVP에서는 직접 팀 선택을 제공하지 않으므로 호출해도 서버가 팀을 바꾸지 않는다.
  - `ASnowRumblePlayerState::RequestSetLobbyReady(bool bNewReady)`: 서버가 팀 미배정 플레이어의 준비 상태를 false로 유지한다.
  - `ASnowRumbleLobbyGameState::CanStartLobbyMatch()`: 서버 기준으로 총 2~8명, Red와 Blue 동수, 팀당 1~4명, 전원 준비 상태일 때만 true를 반환한다.
  - `ASnowRumbleLobbyGameState::GetLobbyTeamPlayerCount(ESnowRumbleTeam Team)`: S-03 UI가 팀별 인원 표시를 구성할 때 읽을 수 있는 팀 인원 조회 함수다.
  - `ASnowRumblePlayerState::GetLobbyTeam()`, `ASnowRumblePlayerState::IsLobbyReady()`: S-03 UI가 각 참가자의 팀과 준비 상태를 읽는 원본 복제 상태다.
  - `ASnowRumbleLobbyGameState`는 로비 모드, 라운드 수, 게임 속도가 서버에서 실제 변경될 때 비호스트 클라이언트가 포커스한 게시판의 기존 예외행동 피드백 UI에 `방장이 방설정을 변경하였습니다`를 표시한다.
- 인계 대상: S-03, C-04, C-08

## 범위 밖
- 게시판 직접 팀 선택과 팀 색 변경

## 사전 전제
- C-02

## 기존 구현 인수
- `ASnowRumblePlayerState`에 대기방 이름, `ESnowRumbleTeam`, 준비 상태 복제와 서버 요청 함수가 이미 있다.
- `ASnowRumbleLobbyGameState::GetLobbyPlayers`, `CanStartLobbyMatch`와 `OnLobbyStateChanged`가 기존 로비 UI 데이터 원본이다.
- `ASnowRumbleLobbyGameMode::RequestStartMatch`가 호스트 시작 요청과 ServerTravel을 담당한다.
- 기존 수동 팀 선택은 MVP 기준에서 대체 대상이며, 팀 배정은 서버 랜덤 배정으로 바꾼다.
- C-03 첫 구현으로 `ASnowRumbleLobbyGameMode`의 `DefaultPawnClass`를 `ASnowRumbleCharacter`로 지정하고, `ALobbyPlayerController` 입력 모드를 `GameAndUI`로 변경했다.
- 기본 로비 이동 상태는 `ALobbyPlayerController::EnableLobbyGameInput()`을 사용해 `GameOnly` 입력과 숨김 커서로 둔다. 추후 설정 UI를 열 때는 `EnableLobbyUiInput()`, 닫을 때는 `EnableLobbyGameInput()`을 WBP에서 호출한다.
- 서버 `PostLogin` 이후 `ASnowRumbleLobbyGameMode::AssignLobbyTeam()`이 `ASnowRumblePlayerState::AssignLobbyTeamFromServer()`로 팀 배정 결과를 적용한다.
- `ASnowRumbleLobbyGameState::CanStartLobbyMatch()`는 모든 플레이어 준비 상태와 1vs1~4vs4 동수 조건을 함께 검사한다.

## 결정 필요
- 없음

## 수동 작업 (구현 후 구체화)
- 로비 맵 `L_Lobby`에 `PlayerStart`를 배치한다.
- 기본 C++ 캐릭터 대신 현재 프로젝트의 기본 캐릭터 Blueprint를 써야 하면, 로비 GameMode Blueprint 또는 맵 World Settings에서 기본 Pawn을 `BP_SnowRumbleCharacter`로 지정한다.
- S-03에서 직접 팀 선택 버튼이나 게시판 팀 변경 UI는 MVP에서는 노출하지 않는다.
- S-03에서 참가자 목록은 `ULobbyWidget::GetLobbyPlayers()`로 받고, 각 행은 `ASnowRumblePlayerState::GetLobbyTeam()`, `GetLobbyPlayerName()`, `IsLobbyReady()` 값을 읽어 표시한다.
- S-03에서 팀별 인원 표시는 `ASnowRumbleLobbyGameState::GetLobbyTeamPlayerCount(ESnowRumbleTeam::Red/Blue)`를 사용한다.
- S-03에서 시작 버튼 활성화는 호스트 화면에서 `ULobbyWidget::IsLocalPlayerHost()`와 `ULobbyWidget::CanStartMatch()`가 모두 true일 때로 연결한다.
- S-03에서 별도 UI 자산 변경은 필요 없다. 방 설정 버튼에서 사용하는 게시판 `InvalidActionReasonText`와 `InvalidActionAnimation` 연결을 그대로 재사용한다.

## 완료 조건
### 에이전트 확인
- [x] 로비 기본 Pawn 스폰과 게임 입력 가능 상태 C++ 변경 완료
- [x] 로비 스폰·이동 확인용 수동 작업 기록 완료
- [x] `SnowRumbleEditor Win64 Development` 빌드 성공
- [x] 기본 로비 이동 상태에서 마우스 커서를 숨기도록 입력 모드 조정 완료
- [x] 기본 로비 이동 상태를 `GameOnly` 입력으로 조정하고 UI 입력 전환 함수를 추가함
- [x] 서버 랜덤 배정과 동수 검사 완료
- [x] 대기방 계약 인계 완료
- [x] 호스트·클라이언트 정적 점검 완료
- [x] `SnowRumbleEditor Win64 Development` 재빌드 성공
- [x] 방 설정 변경 게시판 피드백 C++ 보강 완료
- [x] 방 설정 변경 게시판 피드백 `git diff --check` 통과
- [x] 방 설정 변경 게시판 피드백 `SnowRumbleEditor Win64 Development` 빌드 성공

### 검증 메모
- 2026-08-24: 호스트가 로비 모드, 라운드 수, 게임 속도를 실제로 변경하면 비호스트 클라이언트가 포커스한 게시판에 `방장이 방설정을 변경하였습니다` 피드백을 표시하도록 보강했다. `git diff --check`, 충돌 표식 검색, `SnowRumbleEditor Win64 Development` 빌드가 성공했다.

### 결과 확인
- [x] 호스트가 방을 만들면 `L_Lobby`의 `PlayerStart` 위치에 기본 캐릭터로 스폰된다.
- [ ] 클라이언트가 빠른 참여 또는 방 코드 참여를 하면 각자 `PlayerStart` 위치에 기본 캐릭터로 스폰된다.
- [x] 로비 UI가 떠 있는 상태에서도 WASD와 마우스 시점 조작이 동작한다.
- [ ] 로비 UI 버튼과 텍스트 입력이 계속 동작한다.
- [ ] 두 플레이어가 같은 위치에 겹쳐 스폰되지 않도록 `PlayerStart` 배치를 확인한다.
- [ ] 기본 로비 이동 상태에서는 마우스 커서가 보이지 않는다.
- [ ] 호스트와 클라이언트가 로비에 들어오면 서버가 각 플레이어를 Red 또는 Blue로 자동 배정한다.
- [ ] 1vs1처럼 Red와 Blue 인원이 같고 모든 참가자가 준비하면 호스트 화면에서 시작 가능 상태가 된다.
- [ ] 1vs2처럼 Red와 Blue 인원이 다르면 모든 참가자가 준비해도 시작 가능 상태가 되지 않는다.
- [ ] 팀당 4명을 초과하는 시작 가능 상태가 만들어지지 않는다.
- [ ] S-03 UI에서 직접 팀 선택 버튼이나 게시판 팀 변경 기능을 연결하지 않아도 진행이 막히지 않는다.
- [ ] 클라이언트가 준비 완료한 뒤 호스트가 로비 게시판에서 모드, 라운드 수, 게임 속도 중 하나를 실제로 변경하면 클라이언트 준비 상태가 초기화되고 포커스 중인 게시판에 `방장이 방설정을 변경하였습니다` 피드백이 표시된다.
- [ ] 호스트가 현재 값과 같은 방 설정 버튼을 다시 누르면 클라이언트에 방 설정 변경 알림이 새로 뜨지 않는다.

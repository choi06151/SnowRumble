# SUB UI Task S03-2 - 대기방 팀 선택과 준비 UI

## 설명

복제된 대기방 상태를 표시하고 로컬 플레이어가 이름, 팀과 준비 상태 변경을 요청할 수 있는 대기방 UI를 구성한다.

## 구현 항목

- [ ] 접속한 플레이어 목록과 각 플레이어의 이름·팀·준비 상태를 표시한다.
- [ ] 로컬 플레이어의 이름과 팀 변경 요청 UI를 제공한다.
- [ ] 준비와 준비 해제 요청 UI를 제공한다.
- [ ] 호스트에게만 경기 시작 가능 여부와 시작 요청 UI를 표시한다.
- [ ] UI 로컬 값으로 팀·준비·시작 가능 상태를 확정하지 않는다.

## 범위 밖

- 대기방 복제 상태와 서버 검사 구현
- 캐릭터 커스터마이징과 최종 대기방 아트
- C++·Config·맵·비UI Blueprint 수정

## 사전 전제

- Main Task 03-2의 대기방 상태, 변경 요청 함수와 갱신 이벤트
- `WBP_MainMenu` 부모 클래스: `MainMenuWidget`
- `BP_MainMenuGameMode` 부모 클래스: `SnowRumbleMainMenuGameMode`
- `BP_MainMenuPlayerController` 부모 클래스: `MainMenuPlayerController`
- `BP_MainMenuPlayerController.Main Menu Widget Class`: `WBP_MainMenu`
- `WBP_MainMenu.HostButton`: 있으면 `Host Lan Game(16)`에 자동 연결
- `WBP_MainMenu.FindButton`: 있으면 `Find Lan Games`에 자동 연결
- `WBP_MainMenu.JoinFirstButton`: 있으면 `Join Lan Game(0)`에 자동 연결
- `WBP_MainMenu.StatusTextBlock`: 선택 사항이며 세션 상태와 검색 결과 개수 자동 표시
- `WBP_Lobby` 부모 클래스: `LobbyWidget`
- `BP_LobbyPlayerController` 부모 클래스: `LobbyPlayerController`
- `BP_LobbyPlayerController.Lobby Widget Class`: `WBP_Lobby`
- 시작화면 함수:
  - `Host Lan Game`
  - `Find Lan Games`
  - `Join Lan Game`
  - `Get Lan Search Results`
  - `On Main Menu Session State Changed`
  - `On Main Menu Search Completed`
- 대기방 함수:
  - `Get Lobby Players`
  - `Request Set Local Player Name`
  - `Request Set Local Player Team`
  - `Request Set Local Player Ready`
  - `Request Start Match`
  - `Is Local Player Host`
  - `Can Start Match`
  - `On Lobby State Changed`
- 플레이어 목록 행 표시용 값:
  - `Get Lobby Player Name`
  - `Get Lobby Team`
  - `Is Lobby Ready`

## 작업 배정

- 담당자: SUB 프로그래머
- 선점 UI 자산: Main Task 인계 후 확정
- 공유 확인 대상: 호스트와 클라이언트의 동일한 대기방 상태 표시
- 반영 순서: Main Task 03-2 완료 후 시작

## 수동 작업 (구현 후 구체화)

## 완료 조건

### 에이전트 확인

- [ ] 관련 UI 자산 수정 완료
- [ ] Blueprint/에셋 규칙 위반 없음
- [ ] C++·Config·맵·비UI Blueprint를 수정하지 않음
- [ ] 현재 Task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인 (구현 후 구체화)

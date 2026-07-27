# SUB UI Task S03-1 - LAN Host와 Join UI

## 설명

메인 프로그래머가 제공한 세션 요청과 결과 이벤트를 사용해 LAN 세션을 만들고 검색·참가할 수 있는 최소 메뉴를 구성한다.

## 구현 항목

- [ ] Host 요청을 실행할 버튼과 진행 상태를 표시한다.
- [ ] 검색 요청과 검색된 세션 목록을 표시한다.
- [ ] 선택한 세션에 Join을 요청할 수 있다.
- [ ] Host·검색·Join의 진행, 성공과 실패를 구분해 표시한다.
- [ ] UI는 세션 결과를 자체 판정하지 않고 메인 Task가 제공한 결과만 표시한다.

## 범위 밖

- C++와 OnlineSubsystem 설정 수정
- 세션 생성·검색·참가 규칙 구현
- Steam, 인터넷 전용 서버와 최종 메뉴 아트

## 사전 전제

- Main Task 03-1의 LAN 세션 요청 함수와 결과 이벤트

## 작업 배정

- 담당자: SUB 프로그래머
- 선점 UI 자산: Main Task 인계 후 확정
- 공유 확인 대상: Host·검색·Join의 각 상태 표시
- 반영 순서: Main Task 03-1 완료 후 시작

## Main Task 03-1 인계

- `Get Snow Rumble Session Subsystem` 노드로 서브시스템을 가져와 로컬 UI 요청에 사용한다.
- `Host Lan Session(MaxPlayers)`는 2~16 범위로 보정된 인원의 NULL LAN Listen Server 세션을 생성한다.
- `Find Lan Sessions()`는 LAN 검색을 시작하고 완료 시 검색 결과를 갱신한다.
- `Join Lan Session(ResultIndex)`는 마지막 검색 결과의 `ResultIndex`로 참가를 요청한다.
- `Get Search Results()`는 `SnowRumbleSessionInfo` 배열을 반환한다.
- `SnowRumbleSessionInfo`에는 `ResultIndex`, `HostName`, `CurrentPlayers`, `MaxPlayers`, `PingMilliseconds`가 있다.
- `On Session State Changed(Operation, State, Message)` 이벤트로 요청별 진행·성공·실패를 표시한다.
- `Operation`은 `None`, `Host`, `Search`, `Join`, `State`는 `Idle`, `InProgress`, `Succeeded`, `Failed`로 구분된다.
- `On Session Search Completed(Results)` 이벤트로 검색 목록을 다시 만든다.
- UI는 `State`와 이벤트 결과만 표시하고 세션 성공 여부나 연결 주소를 자체 판정하지 않는다.

## 수동 작업 (구현 후 구체화)

## 완료 조건

### 에이전트 확인

- [ ] 관련 UI 자산 수정 완료
- [ ] Blueprint/에셋 규칙 위반 없음
- [ ] C++·Config·맵·비UI Blueprint를 수정하지 않음
- [ ] 현재 Task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인 (구현 후 구체화)

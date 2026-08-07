# Task 03-1 - LAN 세션 Host와 Join

## 설명

OnlineSubsystem NULL을 사용하는 LAN 환경에서 한 플레이어가 Listen Server 세션을 만들고 다른 플레이어가 찾아 참가할 수 있게 한다.

> 03-2 이후 현재 Host 목적지는 게임방 `L_Prototype`이 아니라 대기방 `L_Lobby`다. 이 문서의 결과 확인 중 `L_Prototype` 이동 표현은 03-1 당시의 검증 기록이다.

## 구현 항목

- [x] 플레이어가 LAN 세션을 생성할 수 있다.
- [x] 다른 플레이어가 참가 가능한 LAN 세션을 검색할 수 있다.
- [x] 검색된 세션에 참가할 수 있다.
- [x] Host와 Join의 진행·성공·실패 결과가 구분된 상태와 이벤트로 제공된다.

## 메인 프로그래머 책임

- OnlineSubsystem NULL 기반 세션 생성·검색·참가 흐름과 성공·실패 결과를 C++로 제공한다.

## SUB UI 인계

- 연계 SUB Task: `Tasks/S03-1_lan_host_join_ui.md`
- Host·검색·Join 요청과 진행·성공·실패 결과를 UI가 호출하고 받을 수 있게 제공한다.

## 작업 배정

- 담당자: 메인 프로그래머
- SUB UI 담당자: 없음
- 메인 프로그래머 선점 파일·자산: `Source/SnowRumble/Online/SnowRumbleSessionSubsystem.h`, `Source/SnowRumble/Online/SnowRumbleSessionSubsystem.cpp`, `Source/SnowRumble/SnowRumble.Build.cs`, `Config/DefaultEngine.ini`, `Content/Maps/L_Prototype`, `Tasks/03-1_lan_host_join.md`, `Tasks/S03-1_lan_host_join_ui.md`, `docs/ARCHITECTURE.md`, `docs/PENDING_ISSUES.md`, `docs/PLANS.md`
- SUB 프로그래머 선점 UI 자산: 없음
- 공유 확인 대상: Host·검색·Join 요청의 진행·성공·실패 상태, 검색 결과와 Join 이동 결과
- 인계 조건: 실제 Blueprint 함수, 상태 열거형, 검색 결과 구조체와 이벤트 이름을 `Tasks/S03-1_lan_host_join_ui.md`에 기록한 뒤 SUB UI 작업에 인계한다.
- 반영 순서: C++ 세션 서브시스템과 NULL 설정 구현 후 메인 프로그래머가 임시 Blueprint 호출로 동작을 확인하고, 완료 후 SUB 프로그래머가 별도 UI Task를 시작한다.

## 범위 밖

- Steam 세션
- 인터넷 전용 서버
- 대기방 팀 선택과 준비 상태
- 최종 메뉴 디자인

## 사전 전제

- Task 02-2의 2인 네트워크 전투 세로 단면

## 수동 작업

1. 실행 중인 Unreal Editor를 종료한다.
2. Visual Studio에서 `SnowRumbleEditor`를 `Development Editor`, `Win64` 구성으로 빌드한 뒤 `SnowRumble.uproject`를 연다.
3. `L_Prototype`의 Level Blueprint를 열고 `Event BeginPlay`에서 `Get Snow Rumble Session Subsystem` 노드를 추가한 뒤 반환값을 `SessionSubsystem` 변수로 저장한다. UE 5.8에서는 범용 `Get Game Instance Subsystem` 노드가 검색 목록에 직접 표시되지 않는다.
4. `SessionSubsystem`의 `On Session State Changed`에 Custom Event를 바인딩하고 `Operation`, `State`, `Message` 입력을 만든다. `Print String`은 연결하지 않고 Custom Event 노드에 Blueprint Breakpoint만 설정한다.
5. `SessionSubsystem`의 `On Session Search Completed`에 Custom Event를 바인딩하고 `Results` 입력을 만든다. 이 Custom Event에도 Blueprint Breakpoint를 설정한다.
6. 키보드 `H` 입력에서 `SessionSubsystem`의 `Host Lan Session`을 `Max Players = 16`으로 호출한다.
7. 키보드 `F` 입력에서 `SessionSubsystem`의 `Find Lan Sessions`를 호출한다.
8. 키보드 `J` 입력에서 `SessionSubsystem`의 `Join Lan Session`을 `Result Index = 0`으로 호출한다.
9. Level Blueprint를 컴파일하고 임시 테스트를 위해 `L_Prototype`을 저장한다.
10. `SnowRumble.uproject`를 두 번째 Unreal Editor 프로세스로 한 번 더 연다. 두 번째 Editor에서는 자산을 수정하거나 저장하지 않는다.
11. 두 Editor 모두 플레이어 수를 `1`, Net Mode를 `Play Standalone`으로 설정하고 `New Editor Window (PIE)`로 각각 실행해 서로 다른 Editor 프로세스의 PIE 창을 준비한다.
12. 첫 번째 Editor의 PIE 창을 `Host 창`, 두 번째 Editor의 PIE 창을 `Join 창`으로 정한다. `H`는 Host 창에서만 누르고, Join 창에서는 절대로 `H`를 누르지 않는다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드 수정 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료 — 세션 서브시스템 C++와 UHT 컴파일 성공, 실행 중인 Editor의 DLL 점유로 최종 링크는 수동 빌드 필요
- [x] Unreal C++/Blueprint/에셋 규칙 위반 없음
- [x] 현재 task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인

- [x] 첫 번째 Editor의 PIE 창에서 `H`를 눌렀을 때 Host 작업이 `In Progress`에서 `Succeeded`로 바뀌고 `L_Prototype` Listen Server로 이동하는지 Breakpoint 입력값으로 확인한다.
- [x] 두 번째 Editor의 PIE 창에서 `F`를 눌렀을 때 Search 작업이 `In Progress`에서 `Succeeded`로 바뀌는지 확인한다.
- [x] 검색 완료 이벤트의 `Results` 배열에 첫 번째 Host 세션이 있고 `Result Index`, `Host Name`, 현재·최대 인원과 Ping 값이 제공되는지 확인한다.
- [x] 두 번째 Editor의 PIE 창에서 `J`를 눌렀을 때 Join 작업이 `In Progress`에서 `Succeeded`로 바뀌고 Host의 `L_Prototype`으로 이동하는지 확인한다.
- [x] 참가 후 Host와 Join 플레이어가 같은 맵에서 서로의 이동을 볼 수 있는지 확인한다.
- [x] Host 요청 직후 검색 등 다른 요청을 연속 호출했을 때 중복 요청이 `Failed` 상태와 실패 메시지로 구분되는지 확인한다.
- [x] 유효한 검색 결과 없이 `Join Lan Session`을 호출했을 때 `Failed` 상태와 실패 메시지가 제공되는지 확인한다.
- [x] 검증을 마친 뒤 `L_Prototype` Level Blueprint에서 이번 Task에 추가한 BeginPlay 바인딩, `H`, `F`, `J` 테스트 노드와 변수·Custom Event를 모두 제거하고 저장한다.

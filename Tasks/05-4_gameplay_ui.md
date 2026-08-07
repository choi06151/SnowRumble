# Task 05-4 - 게임플레이 UI 데이터 연결

## 설명

SUB 프로그래머가 MVP HUD를 만들 수 있도록 현재 HP, 행동 진행도, 들고 있는 대상과 상호작용 상태를 로컬 플레이어 기준의 데이터와 이벤트로 제공한다. 현재 구현 단위는 메인 HUD가 체력 바, 조준점, 투척 충전 바, 머리 위 행동 진행 바를 한 화면 루트에서 배치할 수 있는 UI 통합이다.

## 구현 항목

- [x] 자신의 현재 HP를 UI가 읽거나 변경 알림을 받을 수 있다.
- [x] 메인 HUD가 내 체력 바 위치와 다른 플레이어 체력 바 목록 위치를 Blueprint에서 배치할 수 있다.
- [x] 같은 체력 바 WBP를 여러 플레이어 대상에 재사용할 수 있다.
- [x] 투척 충전 진행도를 메인 HUD의 Progress Bar로 표시할 수 있다.
- [x] 조준 상태를 메인 HUD의 조준점 표시 여부로 연결할 수 있다.
- [x] 투척을 제외한 머리 위 행동 진행 바를 메인 HUD 안에 배치할 수 있다.
- [ ] 눈 제작 진행도를 독립 HUD UI로 받을 수 있다.
- [ ] 가까운 상호작용 대상과 가능한 행동을 UI가 받을 수 있다.
- [ ] 현재 들고 있거나 착용한 MVP 대상의 상태를 UI가 받을 수 있다.
- [ ] 얼기와 관전 상태를 UI가 구분할 수 있는 데이터를 제공한다.
- [x] 체력 UI 데이터가 로컬 플레이어 소유 상태를 기준으로 제공된다.

## 메인 프로그래머 책임

- UI용 데이터의 소유자, 초기값 조회와 갱신 이벤트를 C++ 또는 메인 소유 게임플레이 Blueprint에서 제공한다.

## SUB UI 인계

- 연계 SUB Task: `Tasks/S05-3_gameplay_hud.md`
- 실제 연결 지점:
  - `UMainHUDWidget`: `WBP_MainHUD`가 상속할 C++ 부모 위젯이다.
  - `MainHUDWidgetClass`: `BP_SnowRumbleCharacter` 디테일에서 지정하면 로컬 플레이어 화면에 `WBP_MainHUD`를 자동 생성한다.
  - `LocalHealthBar`: `WBP_MainHUD` 안에 같은 이름으로 직접 배치한 내 체력 바 위젯에 자동 연결된다.
  - `OtherPlayersHealthPanel`: `WBP_MainHUD` 안에 같은 이름으로 배치한 Panel에 다른 플레이어 체력 바들이 자동 추가된다.
  - `OtherPlayerHealthBarWidgetClass`: `WBP_MainHUD` 디테일에서 다른 플레이어마다 생성할 체력 바 WBP 클래스를 지정한다.
  - `AimChargeProgressBar`: `WBP_MainHUD` 안에 같은 이름으로 배치한 Progress Bar에 투척 충전량이 자동 표시된다.
  - `AimCrosshair`: `WBP_MainHUD` 안에 같은 이름으로 배치한 조준점 위젯이며, 조준 중에만 표시된다.
  - `OverheadTimedActionBar`: `WBP_MainHUD` 안에 같은 이름으로 배치한 `WBP_OverheadActionProgress` 위젯이며, 기존 `OverheadTimedActionWidget` 부모 로직이 머리 위 위치와 진행도를 자동 갱신한다.
  - `UHealthBarWidget`: `WBP_HealthBar`가 상속할 C++ 부모 위젯이다.
  - `HealthProgressBar`: `WBP_HealthBar` 안에 같은 이름으로 만든 Progress Bar에 자동 연결된다.
  - `HealthTextBlock`: 선택 항목이며, 같은 이름으로 Text Block을 만들면 `현재 HP / 최대 HP`를 자동 표시한다.
  - `SetObservedActor(AActor*)`: 같은 체력 바 WBP가 표시할 캐릭터를 지정한다.
  - `SetObservedHealthComponent(USnowRumbleHealthComponent*)`: 같은 체력 바 WBP가 표시할 HP 컴포넌트를 직접 지정한다.

## 범위 밖

- 최종 아트 스타일
- 전체 옵션 화면
- 이모션 선택 UI
- 모든 장기 아이템 전용 UI

## 사전 전제

- Task 04-1의 눈 제작과 상호작용
- Task 04-2의 투척 충전
- Task 02-2의 HP와 얼기
- Task 05-3의 MVP 기본 아이템

## 수동 작업 (구현 후 구체화)

1. 실행 중인 Unreal Editor를 종료한다.
2. `SnowRumbleEditor`를 `Development Editor`, `Win64` 구성으로 빌드하고 프로젝트를 연다.
3. `WBP_HealthBar`를 만들고 부모 클래스를 `HealthBarWidget`으로 설정한다.
4. `WBP_HealthBar` 안에 Progress Bar를 만들고 변수 이름을 `HealthProgressBar`로 지정한다.
5. HP 숫자 표시가 필요하면 Text Block을 만들고 변수 이름을 `HealthTextBlock`으로 지정한다.
6. `WBP_MainHUD`를 만들고 부모 클래스를 `MainHUDWidget`으로 설정한다.
7. `WBP_MainHUD` 안에 내 체력 표시용 `WBP_HealthBar`를 원하는 위치에 직접 배치하고 변수 이름을 `LocalHealthBar`로 지정한다.
8. `WBP_MainHUD` 안에 다른 플레이어 체력 바 목록을 넣을 Panel을 좌측 상단 등 원하는 위치에 배치하고 변수 이름을 `OtherPlayersHealthPanel`로 지정한다.
9. `WBP_MainHUD` 디테일에서 `Other Player Health Bar Widget Class`에 `WBP_HealthBar`를 지정한다.
10. `WBP_MainHUD` 안에 투척 충전 Progress Bar를 원하는 위치에 배치하고 변수 이름을 `AimChargeProgressBar`로 지정한다.
11. `WBP_MainHUD` 안에 조준점 이미지 또는 위젯을 원하는 위치에 배치하고 변수 이름을 `AimCrosshair`로 지정한다.
12. `WBP_MainHUD` 안에 `WBP_OverheadActionProgress`를 원하는 계층에 배치하고 변수 이름을 `OverheadTimedActionBar`로 지정한다.
13. 기존 `WBP_AimChargeDebug`에서 만들던 조준점, 충전 바, 머리 위 진행 바 생성·배치 연결은 사용하지 않는다.
14. `BP_SnowRumbleCharacter` 디테일에서 `Main HUD Widget Class`에 `WBP_MainHUD`를 지정한다.
15. C++가 로컬 플레이어 화면에 `WBP_MainHUD`를 자동 생성하므로 Blueprint에서 별도 생성·화면 추가 로직을 만들지 않는다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드 수정 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료
- [x] Unreal C++/Blueprint/에셋 규칙 위반 없음
- [x] 현재 task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인

- [ ] 호스트 화면에 `WBP_MainHUD`가 표시되고 직접 배치한 `LocalHealthBar`가 시작 HP 100%로 보이는지 확인한다.
- [ ] 클라이언트 화면에 자기 캐릭터 기준 `WBP_MainHUD`와 `LocalHealthBar`가 시작 HP 100%로 보이는지 확인한다.
- [ ] 다른 플레이어 체력 바가 `OtherPlayersHealthPanel`에 같은 `WBP_HealthBar` 모양으로 추가되는지 확인한다.
- [ ] 호스트가 피해를 받으면 호스트 화면의 체력 바가 감소하는지 확인한다.
- [ ] 클라이언트가 피해를 받으면 클라이언트 화면의 체력 바가 감소하는지 확인한다.
- [ ] 다른 플레이어가 피해를 받아도 자기 체력 UI가 다른 플레이어 HP로 바뀌지 않는지 확인한다.
- [ ] 조준하지 않을 때 `AimCrosshair`와 `AimChargeProgressBar`가 보이지 않는지 확인한다.
- [ ] 우클릭 조준 중 `AimCrosshair`가 보이는지 확인한다.
- [ ] 조준 중 좌클릭을 누르면 `AimChargeProgressBar`가 보이고 충전량에 따라 0~1로 차오르는지 확인한다.
- [ ] 좌클릭을 떼거나 충전이 끝나면 `AimChargeProgressBar`가 숨겨지는지 확인한다.
- [ ] 눈 제작 또는 눈 굴리기 중 `OverheadTimedActionBar`가 기존처럼 캐릭터 머리 위 위치로 이동하며 진행도를 표시하는지 확인한다.

# SUB UI Task S04-3 - 머리 위 행동 진행도 UI

## 설명

로컬 플레이어가 눈덩이 투척 충전을 제외한 진행형 행동을 수행할 때, 자기 캐릭터 머리 위에 작은 진행 바를 표시한다. 현재 구현 단위는 눈 제작과 눈덩이 굴리기 성장이다.

## 구현 항목

- [x] C++에서 현재 시간제 행동 종류와 통합 `0~1` 진행도를 UI에 제공한다.
- [x] C++ 부모 위젯이 로컬 캐릭터, 진행도, 표시 여부와 머리 위 화면 위치를 자동 갱신한다.
- [x] 로컬 플레이어가 눈을 제작하는 동안 머리 위에 정규화된 제작 진행도를 표시한다.
- [ ] 로컬 플레이어가 눈덩이를 굴리는 동안 최대 성장까지 정규화된 성장 진행도를 표시한다.
- [x] 기존 조준·투척 차지바와 다른 모양의 별도 Progress Bar를 사용한다.
- [x] 진행형 행동이 없거나 행동이 취소·완료되면 진행 바를 숨기고 값을 초기화한다.
- [x] 캐릭터 이동과 카메라 이동 중에도 진행 바가 자기 캐릭터 머리 위 화면 위치를 따라간다.
- [x] 다른 플레이어의 머리 위에는 진행 바를 표시하지 않는다.
- [x] UI가 제작, 충전, 투척과 서버 판정 상태를 직접 변경하지 않는다.

## 범위 밖

- 기존 화면 중앙 조준·충전 Progress Bar와 그 스타일의 수정 또는 재사용
- 작은 눈·큰 눈의 투척 충전 진행도를 머리 위 바에 표시
- 다른 플레이어의 행동 진행도 표시
- 아이템 획득 애니메이션과 향후 핫팩 부활 진행도
- 승인된 시간제 행동 UI 데이터 연결 외의 C++ 코드와 Config 수정
- 서버 판정, RPC와 복제 상태 변경
- 맵, 캐릭터와 게임플레이 Blueprint 수정
- 현재 UI와 관계없는 애니메이션, 이펙트, 사운드와 모델

## 사전 전제

- 완료된 Main Task 04-1의 눈 제작 상태와 진행도
- Main Task 04-3의 눈 굴리기 상태와 복제 성장 진행도

## Main Task 연결 지점

- 새 WBP의 부모 클래스를 `OverheadTimedActionWidget`으로 지정한다.
- 새 Progress Bar의 이름을 정확히 `TimedActionProgressBar`로 지정하면 `BindWidget`으로 자동 연결된다.
- C++ 부모 위젯이 현재 화면의 로컬 플레이어 Pawn만 가져와 `SnowRumbleCharacter`로 확인한다.
- 캐릭터의 `Get Timed Action State()`는 현재 `None`, `Creating Snowball`, `Rolling Snowball`을 반환한다.
- 캐릭터의 `Get Timed Action Progress()`는 현재 행동의 진행도를 `0~1`로 반환하며 `None`이면 `0`이다.
- 굴리기 중에는 `Get Rolling Snowball()`의 복제된 `Get Growth Progress()`를 통합 진행도로 반환한다.
- 기존 `Is Charging Snowball()`과 `Get Snowball Charge Progress()`는 중앙 투척 차지바 전용으로 유지하며 머리 위 위젯에서는 사용하지 않는다.
- 필요하면 기존 `Is Creating Snowball()`과 `Get Snowball Creation Progress()`를 개별 표시 보조 데이터로 사용할 수 있다.
- C++ 부모 위젯이 로컬 Pawn 위치에 `Overhead World Offset` 기본 `(0, 0, 130)`을 더해 화면 위치로 투영한다.
- C++ 부모 위젯이 전체 화면 Canvas 안의 자기 Canvas Slot 위치를 자동 갱신한다.
- 다른 PlayerState나 다른 캐릭터를 순회하지 않으며 로컬 Pawn 이외의 진행도를 조회하지 않는다.
- `Content/WBP/WBP_MainHUD`는 새 머리 위 행동 위젯을 생성·배치하는 컨테이너로 사용한다.
- `WBP_AimChargeDebug`는 Main HUD 통합 뒤 더 이상 새 UI 배치 컨테이너로 사용하지 않는다.
- 기존 투척 차지바와 Progress Bar·진행도 데이터는 공유하지 않는다.
- 머리 위 행동 진행도는 이 Task 전용 새 Widget Blueprint와 별도 Progress Bar로 구성한다.

## 작업 배정

- 담당자: 메인 프로그래머
- 역할 분담: Codex는 C++ 데이터 연결을 구현하고, 사용자는 Unreal Editor에서 WBP와 별도 Progress Bar를 직접 제작한다.
- 메인 프로그래머 선점 UI 자산: 사용자가 만들 `Content/WBP/WBP_OverheadActionProgress.uasset`, 컨테이너 연결에 사용할 `Content/WBP/WBP_MainHUD.uasset`, 새 위젯 전용 UI 자산
- SUB 프로그래머 선점 UI 자산: 없음
- 메인 프로그래머 선점 파일: `Source/SnowRumble/SnowRumble.Build.cs`, `Source/SnowRumble/Player/SnowRumbleCharacter.h`, `Source/SnowRumble/Player/SnowRumbleCharacter.cpp`, `Source/SnowRumble/UI/OverheadTimedActionWidget.h`, `Source/SnowRumble/UI/OverheadTimedActionWidget.cpp`, `Tasks/S04-3_overhead_timed_action_ui.md`, `docs/ARCHITECTURE.md`, `docs/PLANS.md`
- 공유 확인 대상: 호스트와 클라이언트가 각자 자기 캐릭터 머리 위 진행 바만 보는지, 제작·충전·취소·완료에 따라 값과 표시가 전환되는지
- 반영 순서: 메인 프로그래머가 현재 Main 변경과 승인된 S04-3 UI 변경을 같은 작업 흐름에서 관리하고 함께 결과를 확인한다.

## 구현 승인 범위

- 사용자 승인일: 2026-08-05
- C++ 담당: `ESnowRumbleTimedActionState`, `GetTimedActionState()`, `GetTimedActionProgress()`와 WBP 자동 바인딩 부모 클래스를 제공한다.
- 사용자 수동 담당: 새 `WBP_OverheadActionProgress`에 기존 차지바와 다른 별도 Progress Bar를 구성한다.
- 사용자 수동 담당: `WBP_MainHUD`에 새 위젯을 배치하고 변수 이름을 연결한다.
- 기존 조준·충전 Progress Bar, 스타일과 동작은 변경하지 않는다.
- Config, 맵과 `BP_SnowRumbleCharacter`는 변경하지 않는다.

## 현재 C++ 구현 결과

- `ESnowRumbleTimedActionState`는 현재 `None`, `CreatingSnowball`, `RollingSnowball`을 Blueprint Enum으로 제공한다.
- `GetTimedActionState()`는 굴리기를 우선하고 그다음 눈 제작 상태를 머리 위 진행형 행동으로 반환한다.
- `GetTimedActionProgress()`는 선택된 행동의 기존 서버 기준 진행도를 `0~1`로 제한해 반환하며 행동이 없으면 `0`이다.
- 굴리기 상태에서는 현재 굴리는 `ASnowballItem`의 복제 성장률을 반환한다.
- `UOverheadTimedActionWidget`은 `TimedActionProgressBar`를 `BindWidget`으로 자동 연결한다.
- 위젯 Tick은 로컬 소유 캐릭터만 조회해 Progress Bar의 `Percent`와 표시 여부를 갱신한다.
- 위젯 Tick은 캐릭터 위치에 `OverheadWorldOffset`을 더해 화면에 투영하고 자신의 Canvas Slot 위치를 갱신한다.
- 자기 전용 UI이므로 별도 RPC나 복제 프로퍼티는 추가하지 않았다.

## 수동 작업

1. Unreal Editor가 종료된 상태에서 `SnowRumbleEditor`를 `Development Editor`, `Win64` 구성으로 빌드하고 프로젝트를 연다.
2. `Content/WBP/`에 `WBP_OverheadActionProgress` Widget Blueprint를 만든다.
3. 새 위젯의 부모 클래스를 `OverheadTimedActionWidget`으로 지정한다.
4. 새 위젯에 기존 조준·투척 차지바와 다른 모양의 별도 Progress Bar를 추가하고 이름을 정확히 `TimedActionProgressBar`로 지정한다.
5. `TimedActionProgressBar`의 `Is Variable`을 활성화한다.
6. `WBP_MainHUD`의 전체 화면 Canvas에 `WBP_OverheadActionProgress`를 자식으로 추가하고 변수 이름을 `OverheadTimedActionBar`로 지정한다.
7. 새 위젯의 Canvas Slot 크기를 원하는 작은 로딩바 크기로 설정한다.
8. 머리 위 높이가 맞지 않으면 `WBP_OverheadActionProgress`의 Class Defaults에서 `Overhead World Offset` 기본 `(0, 0, 130)`과 `Screen Position Offset`을 조정한다.
9. 새 WBP에는 Tick, `Get Owning Player Pawn`, `Set Percent`, `Set Visibility`, 월드 위치 투영 노드를 따로 만들지 않는다.

## 완료 조건

### 에이전트 확인

- [x] 승인된 C++ UI 데이터 연결 수정 완료
- [x] WBP 자동 바인딩과 로컬 머리 위 위치 갱신 C++ 기본 클래스 구현 완료
- [x] 로컬 정적 점검 완료 — `git diff --check` 통과
- [x] Unreal C++ 규칙 위반 없음
- [x] Config·맵·비UI Blueprint를 수정하지 않음
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨
- [x] 사용자 WBP와 별도 Progress Bar 제작 완료

### 결과 확인

- [x] 행동이 없을 때 머리 위 Progress Bar가 보이지 않는지 확인한다.
- [x] 눈 제작 중 자기 캐릭터 머리 위 바가 `0~1`로 차고 완료·취소 시 숨는지 확인한다.
- [x] 작은 눈과 큰 눈을 충전해도 머리 위 바는 나타나지 않고 기존 중앙 차지바만 동작하는지 확인한다.
- [x] 기존 조준·투척 차지바가 기존 모양과 동작을 유지하는지 확인한다.
- [x] 캐릭터와 카메라를 움직여도 새 바가 자기 캐릭터 머리 위를 따라가는지 확인한다.
- [x] 호스트와 클라이언트 화면에서 각자 자기 캐릭터의 바만 보이는지 확인한다.
- [ ] 눈덩이를 굴리면 머리 위 바가 현재 성장률에서 시작해 최대 크기에서 `1`까지 차는지 확인한다.
- [ ] 굴리기를 중간에 끝내거나 최대 성장 뒤 종료하면 머리 위 바가 숨는지 확인한다.
- [ ] 호스트와 클라이언트가 각각 굴릴 때 각자 자기 화면의 머리 위 바만 표시되는지 확인한다.

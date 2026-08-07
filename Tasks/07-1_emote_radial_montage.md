# Task 07-1 - 임시 이모션 원형 선택과 몽타주 실행

## 설명

기존 `Tab` 이모션 입력만 사용해 원형 선택 UI를 열고, 선택된 8개 칸 중 하나에 연결된 몽타주를 호스트와 클라이언트 화면에서 동일하게 재생할 수 있게 한다. 이 task는 MVP 이후 기능인 이모션을 임시로 앞당긴 작업이며, 최종 UI 연출과 아이콘 구성은 포함하지 않는다.

## 구현 항목

- [x] 기존 이모션 입력으로 Blueprint가 원형 선택 UI를 열고 닫을 수 있는 입력 이벤트를 유지한다.
- [x] Blueprint가 선택한 이모션 인덱스를 C++에 요청할 수 있다.
- [x] 원형 UI Blueprint가 상속할 C++ 부모 위젯에서 8개 버튼을 `0~7` 순서로 자동 연결한다.
- [x] 원형 UI Blueprint가 현재 마우스 호버 중인 버튼을 기억하고 `Tab` 해제 시 해당 이모션을 실행한다.
- [x] 캐릭터 C++가 로컬 플레이어용 원형 UI 위젯을 생성하고 `Tab` 입력으로 열고 닫는다.
- [x] 원형 UI가 열린 동안 마우스 커서를 표시하고 카메라 회전 입력을 차단한다.
- [x] 서버가 이모션 인덱스와 현재 행동 가능 상태를 검사한다.
- [x] 서버가 확정한 이모션 몽타주가 모든 참가자 화면에서 재생된다.

## SUB UI 인계

- 연계 SUB Task: 없음
- 제공할 UI 데이터·이벤트:
  - `OnEmoteInput(bool bPressed)`: 기존 `Tab` 입력 누름/해제를 Blueprint에서 받아 원형 선택 UI 표시 상태로 사용한다.
  - `RequestPlayEmote(int32 EmoteIndex)`: UI에서 선택한 `0~7` 인덱스를 캐릭터에 요청한다.
  - `EmoteMontages`: `BP_SnowRumbleCharacter`에서 `0~7` 칸에 대응하는 `AnimMontage`를 연결한다.
  - `EmoteRadialMenuWidgetClass`: `BP_SnowRumbleCharacter`에서 로컬 플레이어에게 생성할 `WBP_EmoteRadialMenu` 클래스를 연결한다.
  - `UEmoteRadialMenuWidget`: `EmoteButton0`~`EmoteButton7` 버튼 클릭을 `RequestPlayEmote(0~7)`로 자동 전달하는 UI 부모 클래스다.

## 작업 배정

- 담당자: 메인 프로그래머
- 메인 프로그래머 선점 파일: `Source/SnowRumble/Player/SnowRumbleCharacter.h`, `Source/SnowRumble/Player/SnowRumbleCharacter.cpp`, `Source/SnowRumble/UI/EmoteRadialMenuWidget.h`, `Source/SnowRumble/UI/EmoteRadialMenuWidget.cpp`, `Tasks/07-1_emote_radial_montage.md`, `docs/ARCHITECTURE.md`, `docs/PLANS.md`
- 공유 확인 대상: 호스트와 클라이언트가 각각 이모션을 선택했을 때 모든 화면에서 같은 몽타주가 보이는지 확인한다.
- 반영 순서: 메인 C++ 연결 지점 반영 후 사용자가 Blueprint에서 임시 원형 UI와 몽타주를 연결한다.

## 범위 밖

- 최종 원형 UI 디자인, 아이콘, 사운드, UI 애니메이션
- 이모션 쿨다운, 전투 취소, 이동 잠금, 상호작용 차단 같은 최종 규칙
- 신규 이모션 입력 키 추가

## 사전 전제

- `BP_SnowRumbleCharacter`가 기존 `EmoteAction`을 `Tab`에 연결해 사용한다.

## 수동 작업

1. 실행 중인 Unreal Editor를 종료한다.
2. `SnowRumbleEditor`를 `Development Editor`, `Win64` 구성으로 빌드하고 프로젝트를 연다.
3. `BP_SnowRumbleCharacter`에서 `Emote Montages` 배열에 8개 이하의 임시 `AnimMontage`를 `0~7` 순서로 연결한다.
4. `WBP_EmoteRadialMenu`를 만들고 부모 클래스를 `EmoteRadialMenuWidget`으로 설정한다.
5. `WBP_EmoteRadialMenu` 안에 버튼 8개를 만들고 변수 이름을 `EmoteButton0`, `EmoteButton1`, `EmoteButton2`, `EmoteButton3`, `EmoteButton4`, `EmoteButton5`, `EmoteButton6`, `EmoteButton7`로 지정한다.
6. `BP_SnowRumbleCharacter`에서 `Emote Radial Menu Widget Class`에 `WBP_EmoteRadialMenu`를 연결한다.
7. `Tab` 입력 시 캐릭터 C++가 `WBP_EmoteRadialMenu`를 자동 생성하고 `Open Emote Menu`와 `Close Emote Menu`를 호출하며 마우스 커서와 입력 모드를 전환하므로, `On Emote Input`에서 별도 열기·닫기 연결을 만들지 않는다.
8. 버튼 클릭 시 C++가 자동으로 `Request Play Emote(0~7)`을 호출하므로 버튼별 클릭 이벤트를 따로 만들지 않는다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드 수정 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료
- [x] Unreal C++/Blueprint/에셋 규칙 위반 없음
- [x] 현재 task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인

- [ ] 호스트가 `Tab`을 누르면 임시 원형 UI가 열리고, 떼면 닫히는지 확인한다.
- [ ] 클라이언트가 `Tab`을 누르면 자기 화면에만 임시 원형 UI가 열리고, 떼면 닫히는지 확인한다.
- [ ] 원형 UI가 열린 동안 마우스 커서가 보이고, 마우스를 움직여도 캐릭터 카메라가 회전하지 않는지 확인한다.
- [ ] 원형 UI가 닫힌 뒤 마우스 커서가 숨겨지고 기존 카메라 회전 입력이 다시 동작하는지 확인한다.
- [ ] 호스트가 `Tab`을 누른 상태에서 0~7 중 연결된 칸에 마우스를 올리고 `Tab`을 떼면 해당 몽타주가 호스트와 클라이언트 화면 모두에서 재생되는지 확인한다.
- [ ] 클라이언트가 `Tab`을 누른 상태에서 0~7 중 연결된 칸에 마우스를 올리고 `Tab`을 떼면 해당 몽타주가 호스트와 클라이언트 화면 모두에서 재생되는지 확인한다.
- [ ] 마우스가 어떤 칸에도 올라가지 않은 상태에서 `Tab`을 떼면 이모션 없이 UI만 닫히는지 확인한다.
- [ ] 몽타주가 연결되지 않은 칸을 선택해도 오류 없이 아무 동작도 하지 않는지 확인한다.
- [ ] 캐릭터가 얼어있거나 눈덩이를 굴리거나 눈을 제작 중이면 이모션 선택 요청이 실행되지 않는지 확인한다.

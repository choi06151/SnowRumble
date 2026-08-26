# Task C-22 - 상호작용 안내 UI

## 설명

로컬 플레이어가 게시판, 바닥 눈덩이처럼 상호작용 가능한 대상 가까이에 있을 때 대상 옆 화면 위치에 `E - 게시판`, `E - 눈덩이` 형식의 안내 위젯을 표시한다.

## 상태 전이 기준

- 시작 가능: C-15 게시판 상호작용 후보 탐색과 기존 눈덩이 획득 후보 탐색 재사용 가능
- 완료 가능: 로컬 후보 대상명 계산, 안내 WBP 부모 계약, 게시판·눈덩이 표시 확인 절차 기록 완료

## 구현 항목

- [x] 로컬 플레이어의 현재 상호작용 후보를 게시판 우선, 눈덩이 후순위로 계산한다.
- [x] 후보가 있으면 `E - 대상명` 안내를 로컬 화면에 표시하고, 후보가 없거나 게시판 포커스 중이면 숨긴다.
- [x] 게시판과 눈덩이가 겹칠 때 실제 `E` 처리 우선순위와 안내 텍스트가 일치하게 한다.
- [x] 플레이어가 눈덩이를 굴리는 중에는 굴리는 눈덩이의 `E - 눈덩이` 안내를 숨긴다.
- [x] WBP가 상속할 상호작용 안내 위젯 부모를 제공한다.
- [x] 이후 아이템·상호작용 대상이 같은 안내 위젯을 재사용할 수 있는 표시 함수와 텍스트 계약을 제공한다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: C++·문서 최재원(C), 실제 안내 WBP 배치·스타일은 사용자 또는 S 인계
- 생성 파일: `Source/SnowRumble/UI/InteractionPromptWidget_C.*`
- 변경 파일: `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Tasks/C/C-22_interaction_prompt_ui.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Tasks/C/ROLE_C.md`
- 공유 확인 대상: S-03 또는 사용자
- 병합 순서: C-15 상호작용 후보 탐색 기준 위에 후속 UI 계약으로 병합

## 공용 계약과 인계

- 제공받을 계약: C-15 `ASnowRumbleCharacter::FindClosestLobbyBoardCandidate()`, `USnowballEquipmentComponent::FindClosestPickupCandidate()`
- 제공할 계약:
  - `UInteractionPromptWidget`: 상호작용 안내 WBP가 상속할 C++ 부모
  - `UInteractionPromptWidget::PromptText`: 있으면 `E - 게시판` 같은 안내 문구를 자동 표시하는 TextBlock
  - `UInteractionPromptWidget::SetPromptText(FText)`: 안내 문구를 갱신하고 표시한다.
  - `UInteractionPromptWidget::ClearPrompt()`: 안내 문구를 숨긴다.
  - `ASnowRumbleCharacter::InteractionPromptWidgetClass`: 로컬 플레이어 화면에 만들 안내 WBP 클래스
  - `ASnowRumbleCharacter::InteractionPromptScreenOffset`: 대상 화면 좌표에서 안내 위젯을 옮길 픽셀 오프셋
  - `ASnowRumbleCharacter::InteractionPromptWorldHeightOffset`: 대상 bounds 위쪽에서 안내 기준점을 얼마나 더 올릴지 정하는 월드 높이
  - `ASnowRumbleCharacter::GetCurrentInteractionPromptText()`: 현재 로컬 상호작용 후보 기준 안내 문구를 반환한다.
- 인계 대상: S-03 또는 사용자

## 범위 밖

- 안내 UI의 최종 그래픽 스타일, 애니메이션, 아이콘 제작
- 게시판과 눈덩이 외 새로운 상호작용 대상 구현
- 서버 상호작용 판정, RPC, 복제 규칙 변경

## 사전 전제

- C-15 게시판 상호작용
- 기존 눈덩이 획득 상호작용

## 결정 필요

- 없음

## 변경 기록

- 2026-08-11: 사용자가 상호작용 가능한 물건 옆에 `E - 게시판` 형식의 안내 위젯 표시를 요청해 C-22를 추가했다.
- 2026-08-11: `UInteractionPromptWidget`과 캐릭터 로컬 안내 갱신 경로를 추가했다. 게시판 후보를 눈덩이보다 우선 표시하고, 대상 bounds를 화면 좌표로 투영해 안내 위젯을 대상 옆에 배치한다. `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-23: 눈덩이를 굴리는 동안에는 굴리는 눈덩이의 outline과 `E - 눈덩이` 안내가 보이지 않게 했다. 굴리기가 끝난 뒤 바닥 눈덩이는 기존처럼 다시 후보가 된다.

## 수동 작업

- Unreal Editor에서 `UInteractionPromptWidget`을 부모로 하는 안내 WBP를 만든다.
- 안내 WBP 안에 TextBlock을 만들고 이름을 `PromptText`로 맞춘다.
- `PromptText`에는 `E - 게시판`, `E - 눈덩이` 같은 문구가 C++에서 자동 입력된다.
- 플레이어 캐릭터 BP의 `InteractionPromptWidgetClass`에 안내 WBP를 지정한다.
- 안내 위치가 대상과 너무 붙거나 멀면 플레이어 캐릭터 BP의 `InteractionPromptScreenOffset` 값을 조정한다.
- 안내 높이가 대상보다 너무 높거나 낮으면 플레이어 캐릭터 BP의 `InteractionPromptWorldHeightOffset` 값을 조정한다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드 변경 완료
- [x] `git diff --check` 공백 점검 통과
- [x] `SnowRumbleEditor Win64 Development` 빌드 성공
- [x] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [x] 공용 계약과 캡슐화 규칙 위반 없음
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인

- [ ] 로비에서 게시판 가까이 가면 게시판 옆 화면 위치에 `E - 게시판` 안내가 보인다.
- [ ] 게시판에서 멀어지면 상호작용 안내가 사라진다.
- [ ] 게시판과 눈덩이 후보가 동시에 있을 때 안내 문구가 `E - 게시판`으로 보이고, `E` 입력도 게시판 상호작용을 우선 실행한다.
- [ ] 바닥 눈덩이 가까이 가면 눈덩이 옆 화면 위치에 `E - 눈덩이` 안내가 보인다.
- [ ] 눈덩이를 들고 있으면 바닥 눈덩이 안내가 보이지 않는다.
- [ ] 눈덩이를 굴리는 동안에는 굴리는 눈덩이의 `E - 눈덩이` 안내가 보이지 않는다.
- [ ] 게시판 포커스 중에는 상호작용 안내가 보이지 않는다.
- [ ] 안내 WBP를 지정하지 않은 캐릭터 BP에서는 기존 상호작용 기능이 깨지지 않는다.

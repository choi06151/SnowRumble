# Task C-21 - 옵션 메뉴

## 설명

메인메뉴와 로비에서 공통으로 사용할 옵션 메뉴 계약을 정의하고 구현한다. 옵션 WBP는 진입 위치와 관계없이 같은 부모와 같은 WBP를 사용한다. PvP와 눈사람 모드에서는 현재 MVP 기준으로 ESC 옵션 메뉴를 열지 않는다.

## 상태 전이 기준

- 시작 가능: 메인메뉴 WBP, 로비 ESC 메뉴, 로컬 플레이어 설정 저장 위치 사용 가능
- 완료 가능: 옵션 WBP 부모 계약, 메인메뉴/로비 진입 경로, 로컬 설정 저장·적용, 수동 WBP 연결 절차 기록 완료

## 기능 정의

- 옵션 메뉴 진입:
  - 메인메뉴에서 옵션을 열 수 있다.
  - 로비에서 ESC 메뉴의 설정 버튼으로 옵션을 열 수 있다.
  - 메인메뉴과 로비에서 열리는 옵션 WBP는 동일한 WBP를 사용한다.
  - PvP와 눈사람 모드에서는 ESC 자체 또는 ESC 옵션 메뉴를 열지 않는다.
- 옵션 메뉴 닫기:
  - 옵션 메뉴에서 뒤로가기 또는 닫기 버튼으로 이전 화면 흐름으로 돌아간다.
  - 로비에서 옵션을 닫으면 로비 ESC 메뉴 또는 로비 UI 흐름으로 복귀한다.
  - 메인메뉴에서 옵션을 닫으면 메인메뉴 흐름으로 복귀한다.
- 설정 저장:
  - 옵션 값은 로컬 사용자 설정으로 저장한다.
  - 네트워크로 복제하지 않는다.
  - 서버 판정, 경기 결과, 다른 플레이어 상태에 영향을 주지 않는다.

## 옵션 항목

### 1차 구현 후보

- 감도 조절:
  - 마우스/카메라 감도를 조정한다.
  - 로컬 캐릭터의 `Look()` 입력에 적용한다.
  - 기본값과 최소/최대값을 C++ 기본값으로 제공하고 WBP에서 슬라이더로 조정한다.
- 사운드 조절:
  - 마스터 볼륨을 우선 제공한다.
  - BGM/SFX/보이스 볼륨은 사운드 자산 구조가 정리된 뒤 분리한다.

### 후속 구현 후보

- 키 설정:
  - 이동, 점프, 상호작용, 조준, 행동, 장비 내려놓기, 이모션, 채팅 입력 등 현재 입력 액션을 변경할 수 있게 한다.
  - Enhanced Input 리매핑 저장 방식 확인 후 별도 하위 범위로 구현한다.
- 마이크 설정:
  - 마이크 입력 장치, 입력 감도, 음소거 여부를 설정한다.
  - 실제 음성 채팅 또는 Steam/플랫폼 음성 기능을 붙일지 결정한 뒤 구현한다.

## 구현 항목

- [x] 옵션 메뉴 공통 WBP 부모를 만든다.
- [x] 상단 카테고리 버튼 4개와 하단 `OptionsContentSwitcher` 구조를 제공한다.
- [x] 메인메뉴에서 옵션 메뉴를 열 수 있는 C++ 연결 지점을 만든다.
- [x] 로비 ESC 메뉴의 `SettingsButton`으로 같은 옵션 메뉴를 열 수 있게 한다.
- [x] 키바인딩 패널 표시용 행 위젯 부모와 기본 조작 목록 계약을 제공한다.
- [x] 키 변경 버튼 클릭 후 다음 키/마우스 버튼을 캡처해 UI 할당값을 갱신한다.
- [x] 키 변경값을 로컬 사용자 설정에 저장하고 캐릭터 Enhanced Input 매핑에 실제 적용한다.
- [ ] PvP와 눈사람 모드에서는 ESC 옵션 메뉴를 열지 않는 정책을 유지한다.
- [ ] 감도 값을 로컬 저장하고 캐릭터 카메라 입력에 적용한다.
- [ ] 마스터 사운드 볼륨 값을 로컬 저장하고 적용한다.
- [ ] 키 설정은 결정 필요 또는 후속 구현으로 분리한다.
- [ ] 마이크 설정은 결정 필요 또는 후속 구현으로 분리한다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: C++·문서 최재원(C), 옵션 WBP 배치와 스타일은 사용자 또는 S 인계
- 생성 파일: `Source/SnowRumble/UI/OptionsWidget_C.h`, `Source/SnowRumble/UI/OptionsWidget_C.cpp`, `Tasks/C/C-21_options_menu.md`
- 변경 파일: `Source/SnowRumble/UI/MainMenuWidget.*`, `Source/SnowRumble/UI/MainMenuPlayerController.*`, `Source/SnowRumble/UI/LobbyEscapeMenuWidget.*`, `Source/SnowRumble/UI/LobbyPlayerController.*`, `Source/SnowRumble/UI/SnowRumblePlayerController.*`, `Source/SnowRumble/UI/OptionsWidget_C.*`, `Source/SnowRumble/UI/OptionsKeyBindingRowWidget_C.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Source/SnowRumble/Player/SnowRumbleUserSettingsSubsystem_C.*`, `Tasks/C/C-21_options_menu.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`
- 변경 파일 후보: 없음
- 공유 확인 대상: 사용자, S UI
- 병합 순서: C++ 옵션 계약 선행, WBP 배치·스타일 후속

## 공용 계약과 인계

- 제공할 계약 후보:
  - `UOptionsWidget`: 메인메뉴와 로비에서 공통으로 띄울 옵션 WBP 부모
  - `ESnowRumbleOptionsCategory`: 감도, 사운드, 키 설정, 마이크 카테고리 enum
  - `UOptionsWidget::OptionsContentSwitcher`: 하단 내용 패널을 전환하는 WidgetSwitcher
  - `UOptionsWidget::SensitivityCategoryButton`: 감도 카테고리 버튼
  - `UOptionsWidget::AudioCategoryButton`: 사운드 카테고리 버튼
  - `UOptionsWidget::KeyBindingCategoryButton`: 키 설정 카테고리 버튼
  - `UOptionsWidget::MicrophoneCategoryButton`: 마이크 카테고리 버튼
  - `UOptionsWidget::CloseButton`: 옵션 닫기 버튼
  - `UOptionsWidget::ApplyButton`: 옵션 적용 버튼
  - `UOptionsWidget::ResetButton`: 옵션 초기화 버튼
  - `UOptionsWidget::SetOptionsCategory(...)`: 카테고리 선택과 WidgetSwitcher 전환 함수
  - `UOptionsWidget::OnOptionsCategoryChanged(...)`: Blueprint가 카테고리 선택 표시를 갱신할 이벤트
  - `UOptionsWidget::OnOptionsCloseRequested()`: Blueprint 또는 소유 메뉴가 닫기 흐름을 처리할 이벤트
  - `UOptionsWidget::OnOptionsApplyRequested()`: Blueprint가 적용 버튼 표시 흐름을 처리할 이벤트
  - `UOptionsWidget::OnOptionsResetRequested()`: Blueprint가 초기화 버튼 표시 흐름을 처리할 이벤트
  - `UOptionsWidget::KeyBindingListBox`: 키 설정 행을 자동 생성할 VerticalBox
  - `UOptionsWidget::KeyBindingStatusText`: 키 변경 대기 상태를 표시할 선택 TextBlock
  - `UOptionsWidget::KeyBindingRowWidgetClass`: 키 설정 행 WBP 클래스
  - `UOptionsKeyBindingRowWidget`: 키 설정 행 WBP 부모
  - `UOptionsKeyBindingRowWidget::ActionNameText`: 조작 이름 TextBlock
  - `UOptionsKeyBindingRowWidget::CurrentKeyText`: 현재 키 TextBlock
  - `UOptionsKeyBindingRowWidget::RebindButton`: 키 변경 요청 버튼
  - `UOptionsKeyBindingRowWidget::ResetButton`: 키 기본값 복원 요청 버튼
  - `UOptionsWidget::OnKeyRebindRequested(...)`: 키 입력 대기 상태 진입 이벤트
  - `UOptionsWidget::OnKeyBindingChanged(...)`: 키 할당 UI 값 변경 이벤트
  - `UOptionsWidget::OnKeyRebindCanceled(...)`: 키 입력 대기 취소 이벤트
  - `USnowRumbleUserSettingsSubsystem`: 로컬 옵션 설정 저장·조회 담당
  - `ASnowRumbleCharacter::ApplyInputMappingContext()`: 저장된 키 설정을 반영한 런타임 `IMC_Player` 복제본을 적용
  - `AMainMenuPlayerController::OptionsWidgetClass`: 메인메뉴에서 열 공통 옵션 WBP 클래스
  - `AMainMenuPlayerController::ShowOptionsMenu()`: 메인메뉴에서 옵션 WBP를 연다.
  - `AMainMenuPlayerController::HideOptionsMenu()`: 옵션 WBP를 닫고 메인메뉴 입력으로 돌아간다.
  - `UMainMenuWidget::SettingsButton`: 메인메뉴에서 옵션 WBP를 여는 버튼
  - `ALobbyPlayerController::OptionsWidgetClass`: 로비에서 열 공통 옵션 WBP 클래스
  - `ALobbyPlayerController::ShowOptionsMenu()`: 로비 ESC 메뉴 위에서 옵션 WBP를 연다.
  - `ALobbyPlayerController::HideOptionsMenu()`: 옵션 WBP를 닫고 로비 ESC 메뉴 입력으로 돌아간다.
  - `ULobbyEscapeMenuWidget::SettingsButton`: 로비 ESC 메뉴에서 옵션 WBP를 여는 버튼
  - `USnowRumbleUserSettingsSubsystem`: 로컬 사용자 설정 저장·조회·적용 담당
  - 감도 조정 함수와 현재 감도 Getter
  - 마스터 볼륨 조정 함수와 현재 볼륨 Getter
  - 옵션 메뉴 닫기 이벤트 또는 호출 함수
- 인계 대상: 사용자, S UI

## 범위 밖

- PvP/눈사람 모드 ESC 메뉴
- 온라인 음성 채팅 최종 구현
- Steam Overlay 음성/친구 설정
- 모든 키 리매핑의 최종 UX
- 사운드 클래스가 확정되지 않은 BGM/SFX/보이스 세부 믹싱

## 사전 전제

- 메인메뉴와 로비 ESC 메뉴가 옵션 메뉴를 열 수 있는 UI 버튼 또는 이벤트를 가진다.
- 옵션 값은 로컬 클라이언트 설정으로만 다룬다.

## 결정 필요

- 감도 범위와 기본값
- 마스터 볼륨 외 BGM/SFX/보이스 분리 시점
- 키 설정을 MVP에 포함할지, 포함한다면 어떤 입력부터 허용할지
- 마이크 설정이 실제 음성 채팅 구현 전 임시 UI인지, 플랫폼 음성 기능과 함께 구현할지

## 변경 기록

- 2026-08-10: 사용자가 옵션 기능 정의를 먼저 정리하길 원해 C-21을 추가했다. 옵션은 메인메뉴와 로비에서 같은 WBP로 열고, PvP와 눈사람 모드에서는 ESC 옵션 메뉴를 막는 정책으로 정리했다.
- 2026-08-10: 옵션 WBP 틀을 상단 카테고리와 하단 WidgetSwitcher 구조로 확정하고 `UOptionsWidget` 부모를 추가했다.
- 2026-08-11: 메인메뉴 `SettingsButton`과 로비 ESC 메뉴 `SettingsButton`에서 같은 옵션 WBP를 열 수 있도록 PlayerController 연결을 추가했다.
- 2026-08-11: 옵션 진입 연결 변경분은 C++ 컴파일 단계까지 통과했으나, 실행 중인 에디터가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크가 `LNK1104`로 중단됐다.
- 2026-08-11: 옵션 WBP 포커스 경고를 막기 위해 `UOptionsWidget` 기본 포커스를 생성자와 런타임 구성 단계에서 활성화했다. 키바인딩 패널용 `UOptionsKeyBindingRowWidget` 부모와 기본 조작 표시 목록을 추가했고, 해당 변경분도 C++ 컴파일 단계까지 통과했으나 최종 링크는 같은 DLL 잠금으로 중단됐다.
- 2026-08-11: 키바인딩 행의 변경 버튼을 누르면 다음 키보드/마우스 버튼 입력을 캡처해 UI 할당값을 변경하고, `Esc` 입력으로 대기 상태를 취소하게 했다. 같은 키를 다른 행이 이미 쓰고 있으면 기존 행은 `미할당`으로 비운다.
- 2026-08-11: `SnowRumbleEditor Win64 Development` 전체 빌드가 성공했다.
- 2026-08-11: 키바인딩 변경값을 `USnowRumbleUserSettingsSubsystem`에 저장하고, 캐릭터는 원본 `IMC_Player`를 런타임 복제한 뒤 저장된 키로 교체해 실제 Enhanced Input 매핑에 적용하게 했다. 채팅 열기 키도 PlayerController 직접 키 바인딩을 로컬 설정 기준으로 재구성한다.

## 수동 작업

- 구현 후 `UOptionsWidget`을 부모로 하는 옵션 WBP를 만든다.
- 옵션 WBP 상단에 버튼 4개를 배치하고 이름을 `SensitivityCategoryButton`, `AudioCategoryButton`, `KeyBindingCategoryButton`, `MicrophoneCategoryButton`으로 맞춘다.
- 옵션 WBP 하단에 WidgetSwitcher를 배치하고 이름을 `OptionsContentSwitcher`로 맞춘다.
- `OptionsContentSwitcher`의 자식 순서는 0 감도, 1 사운드, 2 키 설정, 3 마이크로 맞춘다.
- 필요하면 `CloseButton`, `ApplyButton`, `ResetButton`을 배치한다.
- 메인메뉴 WBP에 옵션 버튼이 있으면 이름을 `SettingsButton`으로 맞춘다.
- 메인메뉴 PlayerController Blueprint의 `OptionsWidgetClass`에 `WBP_OptionWidget`을 지정한다.
- 로비 PlayerController Blueprint의 `OptionsWidgetClass`에 같은 `WBP_OptionWidget`을 지정한다.
- 키 설정 패널에 자동 행 목록을 쓰려면 VerticalBox 이름을 `KeyBindingListBox`로 맞춘다.
- 키 설정 패널 안내 텍스트가 필요하면 TextBlock 이름을 `KeyBindingStatusText`로 맞춘다.
- 키 설정 행 WBP를 만들 때 부모를 `UOptionsKeyBindingRowWidget`으로 두고, `ActionNameText`, `CurrentKeyText`, `RebindButton`, `ResetButton` 이름을 맞춘다.
- 옵션 WBP의 `KeyBindingRowWidgetClass`에 키 설정 행 WBP를 지정한다.
- 감도, 사운드, 키 설정, 마이크 설정 UI는 C++에서 제공한 바인딩 이름과 함수 기준으로 배치한다.

## 완료 조건

### 에이전트 확인

- [ ] 관련 코드 변경 완료
- [x] `UOptionsWidget` 부모 틀 코드 변경 완료
- [x] 메인메뉴/로비 옵션 진입 C++ 연결 완료
- [x] 키바인딩 패널 표시 계약 코드 변경 완료
- [x] 키바인딩 UI 입력 캡처 코드 변경 완료
- [x] 키바인딩 로컬 저장과 실제 입력 매핑 적용 코드 변경 완료
- [x] `git diff --check` 공백 점검 통과
- [x] `SnowRumbleEditor Win64 Development` 빌드 통과
- [ ] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [ ] 공용 계약과 캡슐화 규칙 위반 없음
- [x] 현재 Task 문서가 기능 정의 기준으로 생성됨

### 결과 확인

- [ ] 메인메뉴에서 옵션 메뉴가 열린다.
- [ ] 로비 ESC 메뉴에서 같은 옵션 메뉴가 열린다.
- [ ] PvP와 눈사람 모드에서는 ESC 옵션 메뉴가 열리지 않는다.
- [ ] 키바인딩 변경 후 캐릭터 입력과 채팅 열기 키가 변경된 키로 동작한다.
- [ ] 감도 조절 값이 저장되고 카메라 입력에 적용된다.
- [ ] 사운드 볼륨 값이 저장되고 적용된다.

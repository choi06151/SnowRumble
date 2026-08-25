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
- 로비 ESC 메뉴와 로비 옵션 메뉴는 공용 UI 입력 전환 경로를 사용해 마우스 커서와 클릭/호버 이벤트를 함께 켠다.
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
- [x] 이동 키 변경 후 다른 이동 매핑이 연쇄 변경되어 캐릭터가 움직이지 않는 문제를 수정한다.
- [x] 옵션 메뉴 종료 후 로비 게임 입력이 막힌 채 남는 문제를 수정한다.
- [x] `ResetButton`이 현재 선택된 옵션 패널 항목만 초기화하게 한다.
- [x] 키 설정 변경은 `ApplyButton`을 누르기 전까지 UI 임시값으로만 유지하고, 옵션을 닫으면 폐기되게 한다.
- [x] 저장되지 않은 변경이 있을 때만 `ApplyButton`을 클릭할 수 있게 한다.
- [ ] PvP와 눈사람 모드에서는 ESC 옵션 메뉴를 열지 않는 정책을 유지한다.
- [x] 감도 값을 로컬 저장하고 캐릭터 카메라 입력에 적용한다.
- [x] 감도 패널에 슬라이더와 퍼센트 텍스트 표시용 C++ 바인딩을 제공한다.
- [x] 배경음악과 효과음 볼륨 값을 로컬 저장하고 적용한다.
- [x] 소리 패널에 배경음악/효과음 슬라이더와 퍼센트 텍스트 표시용 C++ 바인딩을 제공한다.
- [ ] 키 설정은 결정 필요 또는 후속 구현으로 분리한다.
- [x] 마이크 음량 슬라이더와 퍼센트 텍스트 표시용 C++ 바인딩을 제공한다.
- [x] 마이크 방식을 눌러서 말하기와 항상 말하기 중 하나로 선택하게 한다.
- [x] 마이크 입력 기본 키를 `K`로 추가하고 키 설정에서 변경 가능하게 한다.
- [x] 마이크 입력 상태를 엔진 네트워크 음성 송출 시작/중지로 연결한다.
- [x] 캐릭터 Blueprint가 연결할 `MicrophonePushToTalkAction` Enhanced Input 슬롯을 제공한다.
- [x] 마이크 송출 중인 플레이어 이름을 로비와 인게임 HUD TextBlock에 표시한다.
- [x] `N` 키로 마이크 채널을 전체/팀 말하기로 전환하고 personal alarm으로 현재 상태를 표시한다.
- [x] `M` 키를 누르면 플레이어 지정 음소거 WBP를 토글로 열 수 있게 한다.
- [x] 현재 인게임 플레이어 목록 기준으로 음소거 대상 행을 동적 생성한다.
- [x] 각 플레이어 행의 버튼으로 로컬 수동 음소거 상태를 토글한다.
- [x] 옵션 메뉴의 카테고리와 마이크 방식 버튼은 현재 선택값이면 눌린 상태처럼 표시한다.
- [x] 로비 ESC/F10 메뉴와 로비 옵션 메뉴를 열 때 마우스 커서가 보이게 한다.
- [x] 기본 `T` 키로 누르는 동안만 키 가이드 WBP를 표시하는 캐릭터 입력·UI 슬롯을 제공한다.
- [x] 옵션 마이크 패널에서 선택 장치의 실제 PCM 입력 레벨을 테스트하고 표시한다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: C++·문서 최재원(C), 옵션 WBP 배치와 스타일은 사용자 또는 S 인계
- 생성 파일: `Source/SnowRumble/UI/OptionsWidget_C.h`, `Source/SnowRumble/UI/OptionsWidget_C.cpp`, `Source/SnowRumble/UI/KeyGuideWidget_C.h`, `Source/SnowRumble/UI/KeyGuideWidget_C.cpp`, `Tasks/C/C-21_options_menu.md`
- 변경 파일: `Config/DefaultEngine.ini`, `Config/DefaultGame.ini`, `Source/SnowRumble/Game/SnowRumblePlayerState.*`, `Source/SnowRumble/UI/MainMenuWidget.*`, `Source/SnowRumble/UI/MainMenuPlayerController.*`, `Source/SnowRumble/UI/LobbyEscapeMenuWidget.*`, `Source/SnowRumble/UI/LobbyPlayerController.*`, `Source/SnowRumble/UI/LobbyBoardWidget_C.*`, `Source/SnowRumble/UI/LobbyWidget.*`, `Source/SnowRumble/UI/MainHUDWidget.*`, `Source/SnowRumble/UI/SnowRumblePlayerController.*`, `Source/SnowRumble/UI/OptionsWidget_C.*`, `Source/SnowRumble/UI/OptionsKeyBindingRowWidget_C.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Source/SnowRumble/Player/SnowRumbleUserSettingsSubsystem_C.*`, `Tasks/C/C-21_options_menu.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`
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
  - `UOptionsWidget::OnOptionsCategoryResetRequested(...)`: 현재 선택된 카테고리 패널만 Blueprint에서 초기화할 이벤트
  - `UOptionsWidget::SetHasPendingOptionChanges(...)`: WBP가 저장되지 않은 임시 변경 여부를 알려 `ApplyButton` 활성 상태를 갱신하는 함수
  - `UOptionsWidget::DiscardPendingOptionChanges()`: 옵션 닫기 또는 ESC 복귀 시 적용하지 않은 임시 변경을 폐기하는 함수
  - `UOptionsWidget::SensitivitySlider`: 감도 패널의 슬라이더 선택 바인딩
  - `UOptionsWidget::SensitivityValueText`: 현재 감도를 퍼센트로 표시하는 TextBlock 선택 바인딩
  - `UOptionsWidget::BgmVolumeSlider`: 배경음악 볼륨 슬라이더 선택 바인딩
  - `UOptionsWidget::BgmVolumeValueText`: 배경음악 볼륨 퍼센트 TextBlock 선택 바인딩
  - `UOptionsWidget::SfxVolumeSlider`: 효과음 볼륨 슬라이더 선택 바인딩
  - `UOptionsWidget::SfxVolumeValueText`: 효과음 볼륨 퍼센트 TextBlock 선택 바인딩
  - `UOptionsWidget::BgmSoundClass`: 적용 시 배경음악 볼륨을 반영할 선택 SoundClass
  - `UOptionsWidget::SfxSoundClass`: 적용 시 효과음 볼륨을 반영할 선택 SoundClass
  - `USnowRumbleUserSettingsSubsystem::SetMouseSensitivity(...)`: 로컬 감도 저장 함수
  - `USnowRumbleUserSettingsSubsystem::GetMouseSensitivity()`: 캐릭터 카메라 입력이 읽는 로컬 감도 함수
  - `USnowRumbleUserSettingsSubsystem::SetBgmVolume(...)`: 로컬 배경음악 볼륨 저장 함수
  - `USnowRumbleUserSettingsSubsystem::SetSfxVolume(...)`: 로컬 효과음 볼륨 저장 함수
  - `UOptionsWidget::MicrophoneVolumeSlider`: 마이크 음량 슬라이더 선택 바인딩
  - `UOptionsWidget::MicrophoneVolumeValueText`: 마이크 음량 퍼센트 TextBlock 선택 바인딩
  - `UOptionsWidget::MicrophonePushToTalkButton`: 눌러서 말하기 방식 선택 버튼
  - `UOptionsWidget::MicrophoneAlwaysOnButton`: 항상 말하기 방식 선택 버튼
  - `UOptionsWidget::OnMicrophoneModeChanged(...)`: WBP가 마이크 방식 선택 표시를 갱신할 이벤트
  - `UOptionsWidget::MicrophoneTestButton`: 로컬 마이크 입력 테스트 시작/중지 버튼
  - `UOptionsWidget::MicrophoneTestStatusText`: 입력 감지·대기·실패 상태 텍스트 선택 바인딩
  - `UOptionsWidget::MicrophoneInputLevelProgressBar`: 0~1 입력 레벨 선택 바인딩
  - `UOptionsWidget::OnMicrophoneTestStateChanged(...)`: 테스트 상태와 입력 레벨을 WBP가 갱신하는 이벤트
  - `USnowRumbleUserSettingsSubsystem::SetMicrophoneVolume(...)`: 로컬 마이크 음량 저장 함수
  - `USnowRumbleUserSettingsSubsystem::SetMicrophoneMode(...)`: 로컬 마이크 방식 저장 함수
  - `ASnowRumblePlayerController::OnMicrophoneInputStateChanged(...)`: 실제 음성 시스템 또는 WBP가 마이크 입력 상태 변화에 반응할 이벤트
  - `ASnowRumblePlayerController::OnMicrophoneSettingsApplied(...)`: 실제 음성 시스템 또는 WBP가 적용된 마이크 방식과 음량에 반응할 이벤트
  - `ASnowRumblePlayerController::RequestVoiceChannelToggle()`: 기본 `N` 키 또는 캐릭터 Enhanced Input으로 전체/팀 말하기 채널을 전환하는 함수
  - `ASnowRumblePlayerController::RequestVoiceTargetMute()`: 기본 `M` 키 또는 캐릭터 Enhanced Input으로 플레이어 지정 음소거 메뉴를 토글하는 함수
  - `ASnowRumblePlayerController::OnVoiceTargetMuteRequested()`: 플레이어 지정 음소거 입력을 Blueprint가 받을 수 있는 이벤트
  - `ASnowRumblePlayerController::VoiceMuteMenuWidgetClass`: `M` 키로 열 플레이어 지정 음소거 WBP 클래스
  - `UVoiceMuteMenuWidget`: 플레이어 지정 음소거 메뉴 WBP 부모
  - `UVoiceMuteMenuWidget::PlayerListBox`: 플레이어 행들이 동적으로 들어갈 Panel 바인딩
  - `UVoiceMuteMenuWidget::CloseButton`: 선택 닫기 버튼 바인딩
  - `UVoiceMuteMenuWidget::PlayerRowWidgetClass`: 플레이어 행 WBP 클래스
  - `UVoiceMutePlayerRowWidget`: 플레이어 지정 음소거 행 WBP 부모
  - `UVoiceMutePlayerRowWidget::PlayerNameText`: 플레이어 이름 TextBlock
  - `UVoiceMutePlayerRowWidget::MuteButton`: 음소거 토글 Button
  - `UVoiceMutePlayerRowWidget::MuteButtonText`: 버튼 라벨 TextBlock
  - `UOptionsWidget::KeyBindingListBox`: 키 설정 행을 자동 생성할 ScrollBox 또는 VerticalBox
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
  - `ASnowRumbleCharacter::KeyGuideAction`: 기본 `T` 또는 옵션 키 설정의 `KeyGuide` 저장값으로 키 가이드 표시 입력을 받는 Enhanced Input 슬롯
  - `ASnowRumbleCharacter::KeyGuideWidgetClass`: 로컬 플레이어 화면에 표시할 `UKeyGuideWidget` 기반 키 가이드 WBP 클래스 슬롯
  - `ASnowRumbleCharacter::CloseKeyGuideWidget()`: 키 가이드 WBP를 숨기고 게임 입력으로 복구하는 함수
  - `UKeyGuideWidget`: 키 가이드 WBP 부모. 현재 로컬 키 설정을 읽어 키 표시 TextBlock만 갱신한다.
  - `UKeyGuideWidget::MoveKeyText`: 이동 키 표시. 기본 `WASD`
  - `UKeyGuideWidget::SnowCreateKeyText`: 눈 만들기 키 표시. 기본 `좌클릭`
  - `UKeyGuideWidget::SnowPickupKeyText`: 눈 잡기/획득 키 표시. 기본 `E`
  - `UKeyGuideWidget::SnowRollKeyText`: 눈 굴리기 키 표시. 기본 `E`
  - `UKeyGuideWidget::AimKeyText`: 조준 키 표시. 기본 `우클릭`
  - `UKeyGuideWidget::ThrowKeyText`: 눈 던지기 키 표시. 기본 `좌클릭`
  - `UKeyGuideWidget::EmoteKeyText`: 이모션 키 표시. 기본 `Tab`
  - `UKeyGuideWidget::VoiceMuteKeyText`: 음소거 키 표시. 기본 `M`
  - `UKeyGuideWidget::VoiceChannelKeyText`: 음성 채널 변경 키 표시. 기본 `N`
  - `UKeyGuideWidget::KeyGuideKeyText`: 키 가이드 표시 키. 기본 `T`
  - `UKeyGuideWidget::ChatKeyText`: 채팅창 키 표시. 기본 `Enter`
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
- 2026-08-11: 이동 키 변경 후 플레이어가 움직이지 않는 문제를 수정했다. 런타임 입력 매핑을 수정할 때 이미 바뀐 키를 다시 기본 키로 오인하지 않도록 원본 매핑 스냅샷 기준으로만 저장 키를 적용했고, `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-11: 옵션 또는 로비 ESC 메뉴가 입력 차단을 중첩한 뒤 한 번만 해제해 게임 복귀 후 이동이 막히는 문제를 수정했다. 로비 메뉴 입력 차단은 열 때와 닫을 때 `ResetIgnoreMoveInput()`과 `ResetIgnoreLookInput()`으로 정리한다.
- 2026-08-11: 하단 `ResetButton`은 현재 선택된 WidgetSwitcher 카테고리만 초기화하게 했다. 키 설정 카테고리에서는 모든 키 저장값을 기본값으로 되돌리고, 감도·사운드·마이크 카테고리는 `OnOptionsCategoryResetRequested(...)` 이벤트로 WBP가 해당 패널만 초기화할 수 있다. `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-11: 키 설정 변경은 `ApplyButton`을 누르기 전까지 UI 임시값으로만 유지하고, `ApplyButton`을 누를 때만 로컬 설정 저장과 실제 입력 매핑 적용이 실행되게 했다. 옵션 닫기 또는 ESC 복귀는 `DiscardPendingOptionChanges()`로 임시 변경을 폐기한다. 저장되지 않은 변경이 없으면 `ApplyButton`은 비활성화된다. `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-11: 감도 설정을 추가했다. 옵션 WBP는 `SensitivitySlider`와 `SensitivityValueText`를 배치하면 감도 임시값과 퍼센트 텍스트를 자동 갱신하고, `ApplyButton`을 누를 때만 `USnowRumbleUserSettingsSubsystem`에 저장한다. 캐릭터 `Look()` 입력은 저장된 `GetMouseSensitivity()` 값을 곱해 카메라 회전에 적용한다. `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-11: 배경음악과 효과음 볼륨 설정을 추가했다. 옵션 WBP는 `BgmVolumeSlider`/`BgmVolumeValueText`, `SfxVolumeSlider`/`SfxVolumeValueText`를 배치하면 임시값과 퍼센트 텍스트를 자동 갱신하고, `ApplyButton`을 누를 때만 로컬 설정에 저장한다. WBP에서 `BgmSoundClass`와 `SfxSoundClass`를 지정하면 적용 시 해당 SoundClass 볼륨에도 반영한다. C++ 컴파일은 통과했으나 실행 중인 에디터가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크가 `LNK1104`로 중단됐다.
- 2026-08-11: 마이크 설정을 추가했다. 옵션 WBP는 `MicrophoneVolumeSlider`/`MicrophoneVolumeValueText`로 마이크 음량 임시값과 퍼센트 텍스트를 갱신하고, `MicrophonePushToTalkButton`/`MicrophoneAlwaysOnButton`으로 눌러서 말하기 또는 항상 말하기 방식을 선택한다. 키 설정 목록에는 기본 `K`인 `MicrophonePushToTalk` 행을 추가했고, `ASnowRumblePlayerController::OnMicrophoneInputStateChanged(...)`와 `OnMicrophoneSettingsApplied(...)` 이벤트로 WBP 표시나 후속 음성 UI가 반응할 수 있다. C++ 컴파일은 통과했으나 실행 중인 에디터가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크가 `LNK1104`로 중단됐다.
- 2026-08-11: 옵션 메뉴의 선택형 버튼 표시를 보강했다. 현재 카테고리 버튼과 마이크 방식 버튼은 각 버튼의 기존 `Pressed` 스타일을 선택 상태의 `Normal`/`Hovered` 스타일로 재사용해, 선택된 항목이 계속 눌린 상태처럼 보인다. 같은 선택 표시 방식은 로비 게시판의 현재 팀 색, 모드, 라운드 수, 게임 속도, 비호스트 ready 버튼에도 적용했다.
- 2026-08-11: 마이크 입력 상태를 실제 엔진 네트워크 음성 송출에 연결했다. 로비와 인게임에서 기본 `K` 또는 변경된 `MicrophonePushToTalk` 키를 누르면 `StartTalking()`, 키를 떼거나 설정이 꺼지면 `StopTalking()`을 호출한다. 항상 말하기 모드는 적용 즉시 송출 상태를 켠다. 캐릭터 Blueprint에는 `MicrophonePushToTalkAction` Enhanced Input 슬롯을 추가했으며, 프로젝트 설정에는 OnlineSubsystem NULL voice와 GameSession push-to-talk 설정을 명시했다. `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-11: 마이크 송출 중인 플레이어 이름 표시를 추가했다. `ASnowRumblePlayerState`는 음성 송출 중 여부를 복제하고, `ULobbyWidget`과 `UMainHUDWidget`은 `VoiceSpeakingContainer` 선택 바인딩 위젯 안의 `VoiceSpeakingIcon` Image와 `VoiceSpeakingNamesText` TextBlock으로 현재 말하는 플레이어를 표시한다. 말하는 플레이어가 없으면 표시 묶음을 숨긴다. C++ 컴파일은 통과했으나 실행 중인 에디터가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크가 `LNK1104`로 중단됐다.
- 2026-08-11: 마이크 채널 전환을 추가했다. 키 설정 목록에는 기본 `M`인 `MicrophoneChannelToggle` 행을 추가했고, PlayerController는 `M` 입력마다 전체 말하기와 팀 말하기를 전환한다. 현재 채널은 PlayerState에 복제되며 팀 말하기 중인 플레이어는 같은 팀에게만 말하는 사람 표시가 보이고, 다른 팀 클라이언트에서는 gameplay mute 처리된다. 로컬 상태 변경은 `PersonalAlarmText`와 `PersonalAlarmAnimation` 선택 바인딩으로 "전체로 말하기"/"팀으로 말하기" 알림을 재생한다. `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-11: 마이크 채널 전환 기본 키를 `M`에서 `N`으로 옮기고, `M`은 플레이어 지정 음소거 입력으로 분리했다. 캐릭터 Blueprint에는 `MicrophoneChannelToggleAction`과 `VoiceTargetMuteAction` 슬롯을 추가했고, PlayerController에는 `RequestVoiceChannelToggle()`, `RequestVoiceTargetMute()`, `OnVoiceTargetMuteRequested()` 진입점을 추가했다.
- 2026-08-11: `M` 플레이어 지정 음소거 메뉴를 추가했다. `UVoiceMuteMenuWidget`은 현재 `GameState->PlayerArray` 기준으로 로컬 플레이어를 제외한 플레이어 행을 만들고, `UVoiceMutePlayerRowWidget`의 `MuteButton`은 로컬 수동 음소거 목록과 `GameplayMutePlayer()`/`GameplayUnmutePlayer()`를 토글한다. `FUniqueNetIdRepl::ToString()` 링크 오류를 피하도록 로컬 수동 음소거 키를 PlayerId와 이름 기반으로 바꾸고, `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-11: 키 설정 목록이 길어져 `KeyBindingListBox` 바인딩 타입을 `UPanelWidget`으로 일반화했다. 기존 VerticalBox와 새 ScrollBox 모두 같은 이름으로 바인딩해 키 설정 행을 동적으로 추가할 수 있다.
- 2026-08-19: 로비에서 ESC/F10 메뉴가 열려도 마우스 커서가 보이지 않고 화면이 움직이는 문제를 수정했다. 로비 ESC 메뉴와 로비 옵션 메뉴는 하드웨어 기본 커서를 명시적으로 쓰고 `UIOnly` 입력 모드로 열린다. `ASnowRumblePlayerController::IsGameplayUiInputOpen()` 공용 조회를 추가하고, `ALobbyPlayerController`는 로비 ESC/옵션 메뉴가 열려 있으면 true를 반환해 캐릭터 Move/Look/Zoom/GameplayAction 경로를 차단한다.
- 2026-08-19: 로비 ESC/F10 커서와 카메라 입력 차단 변경은 `git diff --check`, UHT, C++ 컴파일을 통과했다. 최종 DLL 링크는 실행 중인 Unreal Editor의 `UnrealEditor-SnowRumble.dll` 잠금 `LNK1104`로 보류됐다.
- 2026-08-23: 캐릭터에 키 가이드 WBP용 `KeyGuideAction`/`KeyGuideWidgetClass` 슬롯을 추가했다. 기본 키 설정 항목은 `KeyGuide`/`T`이며, 입력을 누르는 동안만 로컬 WBP를 표시하고 해제 시 게임 입력으로 복구한다.
- 2026-08-23: `UKeyGuideWidget` 부모를 추가해 키 가이드 WBP의 키 부분 TextBlock만 현재 로컬 키 설정에 맞춰 자동 갱신하게 했다. 이모션 기본 키 설정도 실제 사용 기준과 맞게 `Tab`으로 정리했다.
- 2026-08-23: 마이크 입력 점검을 위해 `K` 눌러서 말하기 시작 시 OnlineSubsystem VoiceInterface의 로컬 토커 등록을 보장하고, 등록·헤드셋 감지·StartTalking/StopTalking 흐름을 로그로 남기게 했다. 계산만 하고 직접 바인딩하지 않았던 `N` 채널 전환과 `M` 플레이어 음소거 키도 PlayerController 직접 바인딩에 추가했다.
- 2026-08-25: 옵션 마이크 패널에 선택형 `MicrophoneDeviceComboBox` 계약을 추가했다. `AudioCaptureCore`에서 장치 이름·ID를 열거하고, 선택한 장치 ID를 로컬 설정에 저장하며 `OnMicrophoneDeviceChanged` 이벤트로 WBP 표시를 갱신한다. 현재 NULL `IOnlineVoice`는 엔진 내부에서 기본 입력 장치를 직접 생성하므로, 저장된 선택값을 네트워크 음성 캡처에 실제 적용하는 후속 엔진 음성 계층 연결이 필요하다.
- 2026-08-25: 옵션 마이크 패널에 `MicrophoneTestButton`, `MicrophoneTestStatusText`, `MicrophoneInputLevelProgressBar`를 추가했다. `AudioCaptureCore` PCM 콜백으로 선택 장치의 RMS 입력 레벨을 계산하고, `OnMicrophoneTestStateChanged` 이벤트로 입력 대기·감지·실패 상태와 레벨을 WBP에 전달한다. `NativeDestruct`에서 캡처 스트림을 중지하며 `SnowRumbleEditor Win64 Development` 빌드가 성공했다.

## 수동 작업

- 구현 후 `UOptionsWidget`을 부모로 하는 옵션 WBP를 만든다.
- 옵션 WBP 상단에 버튼 4개를 배치하고 이름을 `SensitivityCategoryButton`, `AudioCategoryButton`, `KeyBindingCategoryButton`, `MicrophoneCategoryButton`으로 맞춘다.
- 옵션 WBP 하단에 WidgetSwitcher를 배치하고 이름을 `OptionsContentSwitcher`로 맞춘다.
- `OptionsContentSwitcher`의 자식 순서는 0 감도, 1 사운드, 2 키 설정, 3 마이크로 맞춘다.
- 필요하면 `CloseButton`, `ApplyButton`, `ResetButton`을 배치한다.
- 메인메뉴 WBP에 옵션 버튼이 있으면 이름을 `SettingsButton`으로 맞춘다.
- 메인메뉴 PlayerController Blueprint의 `OptionsWidgetClass`에 `WBP_OptionWidget`을 지정한다.
- 로비 PlayerController Blueprint의 `OptionsWidgetClass`에 같은 `WBP_OptionWidget`을 지정한다.
- 키 설정 패널에 자동 행 목록을 쓰려면 ScrollBox 또는 VerticalBox 이름을 `KeyBindingListBox`로 맞춘다. 키 항목이 많아졌으므로 ScrollBox 사용을 우선한다.
- 키 설정 패널 안내 텍스트가 필요하면 TextBlock 이름을 `KeyBindingStatusText`로 맞춘다.
- 키 설정 행 WBP를 만들 때 부모를 `UOptionsKeyBindingRowWidget`으로 두고, `ActionNameText`, `CurrentKeyText`, `RebindButton`, `ResetButton` 이름을 맞춘다.
- 옵션 WBP의 `KeyBindingRowWidgetClass`에 키 설정 행 WBP를 지정한다.
- 감도, 사운드, 키 설정, 마이크 설정 UI는 C++에서 제공한 바인딩 이름과 함수 기준으로 배치한다.
- 마이크 패널에 선택형 ComboBoxString 이름을 `MicrophoneDeviceComboBox`로 배치하면 운영체제의 입력 장치 목록이 자동으로 채워진다.
- 마이크 패널에 Button 이름을 `MicrophoneTestButton`으로 배치하면 선택 장치의 로컬 PCM 입력 테스트를 시작·중지한다.
- 마이크 패널에 TextBlock 이름을 `MicrophoneTestStatusText`로 배치하면 입력 대기·감지·실패 상태가 표시된다.
- 마이크 패널에 ProgressBar 이름을 `MicrophoneInputLevelProgressBar`로 배치하면 현재 입력 레벨이 0~1로 표시된다.
- 감도 패널에 Slider 이름을 `SensitivitySlider`로 배치한다.
- 감도 패널에 TextBlock 이름을 `SensitivityValueText`로 배치한다.
- 사운드 패널에 배경음악 Slider 이름을 `BgmVolumeSlider`로 배치한다.
- 사운드 패널에 배경음악 TextBlock 이름을 `BgmVolumeValueText`로 배치한다.
- 사운드 패널에 효과음 Slider 이름을 `SfxVolumeSlider`로 배치한다.
- 사운드 패널에 효과음 TextBlock 이름을 `SfxVolumeValueText`로 배치한다.
- 실제 소리 자산에 볼륨을 반영하려면 옵션 WBP 기본값에서 `BgmSoundClass`와 `SfxSoundClass`에 프로젝트 SoundClass 자산을 지정한다.
- 마이크 패널에 Slider 이름을 `MicrophoneVolumeSlider`로 배치한다.
- 마이크 패널에 TextBlock 이름을 `MicrophoneVolumeValueText`로 배치한다.
- 마이크 패널에 버튼 두 개를 배치하고 이름을 `MicrophonePushToTalkButton`, `MicrophoneAlwaysOnButton`으로 맞춘다.
- 마이크 방식 버튼은 C++ 부모가 현재 임시 선택값에 맞춰 눌린 표시를 갱신한다. WBP 추가 연출이 필요하면 `OnMicrophoneModeChanged(...)` 이벤트에서 보강한다.
- 캐릭터 Blueprint의 `MicrophonePushToTalkAction` 슬롯에 마이크 입력 액션을 지정한다. Mapping Context에는 기본 `K` 매핑을 넣고, 옵션 키 설정 변경은 C++이 저장값 기준으로 런타임 매핑을 교체한다.
- 캐릭터 Blueprint의 `MicrophoneChannelToggleAction` 슬롯에 마이크 채널 전환 입력 액션을 지정한다. Mapping Context에는 기본 `N` 매핑을 넣는다.
- 캐릭터 Blueprint의 `VoiceTargetMuteAction` 슬롯에 플레이어 지정 음소거 입력 액션을 지정한다. Mapping Context에는 기본 `M` 매핑을 넣는다.
- 키 가이드용 InputAction을 만들고 캐릭터 Blueprint의 `KeyGuideAction` 슬롯에 지정한다.
- `UKeyGuideWidget`을 부모로 하는 키 가이드용 WBP를 만들고 캐릭터 Blueprint의 `KeyGuideWidgetClass` 슬롯에 지정한다.
- `IMC_Player`에는 키 가이드 InputAction의 기본 키를 `T`로 배치한다. 옵션 키 설정에서는 `KeyGuide` 항목이 기본 `T`로 표시된다.
- 키 가이드 WBP에서 설명 문구는 자유롭게 배치하고, 키 부분만 별도 TextBlock으로 분리해 이름을 `MoveKeyText`, `SnowCreateKeyText`, `SnowPickupKeyText`, `SnowRollKeyText`, `AimKeyText`, `ThrowKeyText`, `EmoteKeyText`, `VoiceMuteKeyText`, `VoiceChannelKeyText`, `KeyGuideKeyText`, `ChatKeyText` 중 필요한 이름으로 맞춘다.
- `UVoiceMuteMenuWidget`을 부모로 하는 mute 전용 WBP를 만든다.
- mute 메뉴 WBP에 플레이어 행들이 들어갈 패널을 배치하고 이름을 `PlayerListBox`로 맞춘다.
- mute 메뉴 WBP에 닫기 버튼이 필요하면 이름을 `CloseButton`으로 맞춘다.
- `UVoiceMutePlayerRowWidget`을 부모로 하는 행 WBP를 만든다.
- 행 WBP에는 HorizontalBox를 배치하고, 그 안에 플레이어 이름 TextBlock `PlayerNameText`와 Button `MuteButton`을 둔다.
- `MuteButton` 안 라벨 TextBlock을 쓰면 이름을 `MuteButtonText`로 맞춘다.
- mute 메뉴 WBP의 `PlayerRowWidgetClass`에 행 WBP 클래스를 지정한다.
- PvP PlayerController Blueprint의 `VoiceMuteMenuWidgetClass`에 mute 메뉴 WBP 클래스를 지정한다.
- 로비 WBP와 인게임 HUD WBP의 좌측 중단에 HorizontalBox 같은 컨테이너를 배치하고 이름을 `VoiceSpeakingContainer`로 맞춘다. 그 안에 스피커 아이콘 Image `VoiceSpeakingIcon`과 TextBlock `VoiceSpeakingNamesText`를 나란히 배치한다. 말하는 플레이어가 없으면 C++이 표시 묶음을 자동으로 숨긴다.
- 로비 WBP와 인게임 HUD WBP에 로컬 상태 알림 TextBlock `PersonalAlarmText`를 배치한다. 알림 애니메이션을 쓰면 이름을 `PersonalAlarmAnimation`으로 맞춘다. 로비 WBP는 없을 경우 기존 `InvalidActionReasonText`와 `InvalidActionAnimation`을 fallback으로 사용한다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드 변경 완료
- [x] `UOptionsWidget` 부모 틀 코드 변경 완료
- [x] 메인메뉴/로비 옵션 진입 C++ 연결 완료
- [x] 키바인딩 패널 표시 계약 코드 변경 완료
- [x] 키바인딩 UI 입력 캡처 코드 변경 완료
- [x] 키바인딩 로컬 저장과 실제 입력 매핑 적용 코드 변경 완료
- [x] 키바인딩 변경 후 이동 매핑 연쇄 변경 오류 수정 완료
- [x] 옵션 메뉴 복귀 후 로비 게임 입력 차단 잔류 오류 수정 완료
- [x] 현재 선택 패널 기준 ResetButton 초기화 코드 변경 완료
- [x] 적용 전 키 설정 임시 변경과 닫기 시 폐기 코드 변경 완료
- [x] 변경사항 유무 기준 ApplyButton 활성화 코드 변경 완료
- [x] 감도 슬라이더와 퍼센트 텍스트 바인딩 코드 변경 완료
- [x] 감도 로컬 저장과 캐릭터 카메라 입력 적용 코드 변경 완료
- [x] 배경음악/효과음 볼륨 슬라이더와 퍼센트 텍스트 바인딩 코드 변경 완료
- [x] 배경음악/효과음 로컬 저장과 선택 SoundClass 볼륨 적용 코드 변경 완료
- [x] 마이크 음량 슬라이더와 퍼센트 텍스트 바인딩 코드 변경 완료
- [x] 마이크 방식 선택 버튼과 로컬 저장 코드 변경 완료
- [x] 기본 `K` 마이크 입력 키바인딩과 PlayerController 상태 이벤트 코드 변경 완료
- [x] 마이크 입력 상태와 엔진 네트워크 음성 송출 연결 완료
- [x] 캐릭터 `MicrophonePushToTalkAction` Enhanced Input 슬롯 추가 완료
- [x] 마이크 송출 중 플레이어 이름 TextBlock 표시 코드 변경 완료
- [x] 마이크 송출 중 스피커 아이콘 Image 표시 코드 변경 완료
- [x] 기본 `N` 마이크 채널 전환 키바인딩 코드 변경 완료
- [x] 기본 `M` 플레이어 지정 음소거 키바인딩과 Blueprint 이벤트 진입점 코드 변경 완료
- [x] 플레이어 지정 음소거 메뉴와 행 WBP 부모 코드 변경 완료
- [x] 수동 음소거 목록과 gameplay mute 적용 코드 변경 완료
- [x] 전체/팀 말하기 상태 복제와 팀 채널 gameplay mute 코드 변경 완료
- [x] 마이크 채널 전환 personal alarm 코드 변경 완료
- [x] 옵션 메뉴 선택형 버튼 눌림 유지 표시 코드 변경 완료
- [x] 선택 마이크 장치의 로컬 PCM 입력 테스트와 UI 상태 이벤트 코드 변경 완료
- [x] `git diff --check` 공백 점검 통과
- [x] `SnowRumbleEditor Win64 Development` 최종 링크 통과
- [ ] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [ ] 공용 계약과 캡슐화 규칙 위반 없음
- [x] 현재 Task 문서가 기능 정의 기준으로 생성됨

### 결과 확인

- [ ] 메인메뉴에서 옵션 메뉴가 열린다.
- [ ] 로비 ESC 메뉴에서 같은 옵션 메뉴가 열린다.
- [ ] PvP와 눈사람 모드에서는 ESC 옵션 메뉴가 열리지 않는다.
- [ ] 키바인딩 변경 후 캐릭터 입력과 채팅 열기 키가 변경된 키로 동작한다.
- [ ] 이동 키를 다른 키로 바꾼 뒤 앞으로/뒤로/좌/우 이동이 계속 동작한다.
- [ ] 옵션 또는 로비 ESC 메뉴를 닫고 게임으로 복귀하면 이동과 시점 입력이 다시 동작한다.
- [ ] 키 설정 패널에서 `ResetButton`을 누르면 키 설정만 기본값으로 복원된다.
- [ ] 감도·사운드·마이크 패널에서 `ResetButton`을 누르면 해당 패널의 WBP 초기화 이벤트만 실행된다.
- [ ] 키 설정을 바꿔도 `ApplyButton`을 누르기 전에는 실제 캐릭터 입력이 바뀌지 않는다.
- [ ] 저장되지 않은 키 설정 변경 후 옵션을 닫으면 이전 저장 상태가 유지된다.
- [ ] 저장되지 않은 변경이 없으면 `ApplyButton`이 비활성화되고, 변경이 생기면 활성화된다.
- [ ] 감도 슬라이더를 움직이면 `SensitivityValueText`가 퍼센트로 갱신된다.
- [ ] 감도 슬라이더를 움직여도 `ApplyButton`을 누르기 전에는 실제 카메라 감도가 바뀌지 않는다.
- [ ] 감도 변경 후 `ApplyButton`을 누르면 카메라 회전 감도가 바뀐다.
- [ ] 감도 변경 후 옵션을 닫으면 이전 저장 상태가 유지된다.
- [ ] 감도 패널에서 `ResetButton`을 누르면 감도 임시값이 100%로 돌아간다.
- [ ] 배경음악/효과음 슬라이더를 움직이면 각 퍼센트 텍스트가 갱신된다.
- [ ] 배경음악/효과음 슬라이더를 움직여도 `ApplyButton`을 누르기 전에는 저장값과 실제 SoundClass 볼륨이 바뀌지 않는다.
- [ ] 배경음악/효과음 변경 후 `ApplyButton`을 누르면 로컬 설정에 저장되고 지정된 SoundClass 볼륨에 반영된다.
- [ ] 배경음악/효과음 변경 후 옵션을 닫으면 이전 저장 상태가 유지된다.
- [ ] 소리 패널에서 `ResetButton`을 누르면 배경음악과 효과음 임시값이 100%로 돌아간다.
- [ ] 마이크 음량 슬라이더를 움직이면 퍼센트 텍스트가 갱신된다.
- [ ] 마이크 방식 버튼을 누르면 WBP 선택 표시 이벤트가 호출되고 `ApplyButton`이 활성화된다.
- [ ] 마이크 설정 변경 후 `ApplyButton`을 누르면 로컬 설정에 저장된다.
- [ ] 눌러서 말하기 방식에서는 기본 `K` 키를 누르는 동안 마이크 입력 상태 이벤트가 켜지고, 키를 떼면 꺼진다.
- [ ] 항상 말하기 방식에서는 적용 후 마이크 입력 상태 이벤트가 켜진 상태로 유지된다.
- [ ] 키 설정에서 `마이크 입력` 키를 바꾸면 변경된 키로 눌러서 말하기가 동작한다.
- [ ] LAN 로비 또는 인게임에서 마이크 입력 상태가 켜진 플레이어의 음성이 다른 플레이어에게 들린다.
- [ ] 마이크 입력 상태가 켜진 동안 로비와 인게임 HUD의 `VoiceSpeakingContainer` 안에서 `VoiceSpeakingIcon`과 `VoiceSpeakingNamesText`가 나란히 표시된다.
- [ ] `N` 키를 누르면 `PersonalAlarmText`에 "전체로 말하기" 또는 "팀으로 말하기"가 표시되고 애니메이션이 재생된다.
- [ ] `M` 키를 누르면 플레이어 지정 음소거 메뉴가 열리고, 다시 누르면 닫힌다.
- [ ] `K` 키를 누르면 로그에 `Microphone input ON`, `Voice talker ready check`, `StartTalking requested`가 출력되고, 키를 떼면 `Microphone input OFF`, `StopTalking requested`가 출력된다.
- [ ] 음소거 메뉴에는 로컬 플레이어를 제외한 현재 인게임 플레이어 수만큼 행이 생성된다.
- [ ] 행의 `MuteButton`을 누르면 해당 플레이어 음성이 로컬에서 음소거되고 버튼 표시가 해제로 바뀐다.
- [ ] 다시 같은 행의 버튼을 누르면 수동 음소거가 해제되고 팀 채널 규칙상 들을 수 있는 플레이어 음성이 다시 들린다.
- [ ] 팀 말하기 상태에서 송출하면 같은 팀에게만 음성 표시가 보이고 다른 팀 클라이언트는 해당 플레이어 음성이 gameplay mute 처리된다.
- [ ] 옵션 메뉴의 현재 카테고리와 마이크 방식 버튼이 선택 상태처럼 눌려 보이고, 다른 선택지는 원래 스타일로 돌아간다.
- [ ] 마이크 패널 테스트 버튼으로 선택 장치 캡처가 시작되고 입력 대기 상태가 표시된다.
- [ ] 마이크에 소리를 입력하면 Level 표시가 올라가고 입력 감지 상태가 표시된다.
- [ ] 테스트를 중지하거나 옵션을 닫으면 캡처가 중지되고 Level이 0으로 돌아간다.
- [ ] 로비, PvP, 눈사람 모드에서 `T`를 누르고 있는 동안 키 가이드 WBP가 보이고, 키를 떼면 사라진다.
- [ ] 키 가이드 WBP가 열려 있는 동안 카메라 시점과 일반 행동 입력이 실행되지 않는다.
- [ ] 캐릭터 BP에 `KeyGuideAction` 또는 `KeyGuideWidgetClass`를 지정하지 않은 상태에서는 기존 입력이 깨지지 않는다.

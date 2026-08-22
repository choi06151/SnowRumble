# Task C-30 - 오디오 피드백과 보이스 믹싱 계약

## 설명

UI 버튼, 눈덩이 투척과 폭발, 피해 피드백, 아이템 및 게시판 상호작용, 보이스 상태에 대한 사운드 계약을 정리한다. 옵션의 소리 설정을 기준으로 전체 볼륨, 배경음악, 효과음, 보이스가 서로 충돌하지 않게 라우팅하고, Blueprint가 연결할 실제 사운드 자산과 재생 지점을 제공한다.
맵별 배경음악은 메인메뉴, 커스터마이징, 로비, PvP, 눈사람 모드, 포디움에서 각 GameMode 또는 PlayerController가 시작한다.

## 상태 전이 기준

- 시작 가능: C-21 옵션 메뉴의 사운드·마이크 설정과 현재 UI/전투/상호작용/보이스 이벤트 구조
- 완료 가능: 공용 오디오 계약, 볼륨 라우팅, 행동별 사운드 트리거, 수동 연결 절차가 정리됨

## 구현 항목

- [x] UI 버튼 클릭과 메뉴 상호작용에 사용할 공용 사운드 트리거 지점을 정리한다.
- [x] 눈덩이 제작, 투척, 충돌, 폭발, 피해 피드백에 사용할 사운드 트리거 지점을 정리한다.
- [x] 아이템 획득, 놓기, 게시판 상호작용, 상호작용 안내 확인에 사용할 사운드 트리거 지점을 정리한다.
- [x] 옵션의 소리 설정이 전체 볼륨과 BGM, 효과음, 보이스에 일관되게 적용되도록 라우팅한다.
- [x] 배경음악은 재생 종료 시 같은 음원을 다시 시작해 사운드 자산의 loop 설정이 빠져도 반복되게 한다.
- [x] 보이스 채널 전환과 음소거 상태가 음성 표시와 소리 반영에 일관되게 연결되도록 정리한다.
- [x] 각 맵의 배경음악 시작 지점을 GameMode 또는 PlayerController에 연결한다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: C++·문서 최재원(C), 사운드 자산·연출은 사용자 또는 서유정(S)
- 생성 파일: `Tasks/C/C-30_audio_feedback_and_voice_mix.md`
- 변경 파일: `Source/SnowRumble/Audio/SnowRumbleAudioHelpers.*`, `Source/SnowRumble/Player/SnowRumbleUserSettingsSubsystem_C.*`, `Source/SnowRumble/UI/OptionsWidget_C.*`, `Source/SnowRumble/UI/MainMenuWidget.*`, `Source/SnowRumble/UI/MainMenuPlayerController.*`, `Source/SnowRumble/UI/CustomizationPlayerController_C.*`, `Source/SnowRumble/UI/LobbyEscapeMenuWidget.*`, `Source/SnowRumble/UI/LobbyBoardWidget_C.*`, `Source/SnowRumble/UI/MainHUDWidget.*`, `Source/SnowRumble/UI/VoiceMuteMenuWidget_C.*`, `Source/SnowRumble/UI/VoiceMutePlayerRowWidget_C.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Source/SnowRumble/Player/SnowRumblePlayerController.*`, `Source/SnowRumble/Interaction/LobbyInteractionBoard_C.*`, `Source/SnowRumble/Item/*`, `Source/SnowRumble/Snowball/*`, `Source/SnowRumble/Game/SnowRumbleGameMode.*`, `Source/SnowRumble/Game/SnowRumbleLobbyGameMode.*`, `Source/SnowRumble/Game/SnowmanModeGameMode_K.*`, `Source/SnowRumble/Game/SnowRumbleMainMenuGameMode.*`, `Source/SnowRumble/Game/SnowRumbleCustomizationGameMode_C.*`, `Source/SnowRumble/Game/PodiumGameMode.*`, `Source/SnowRumble/Game/PodiumPlayerController.*`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`
- 공유 확인 대상: 사용자, 서유정(S)
- 병합 순서: C++ 오디오 계약 선행, 사운드 자산과 UI 표현은 후속 연결

## 공용 계약과 인계

- 제공받을 계약:
  - C-21의 BGM/SFX/보이스 옵션과 로컬 저장 구조
  - C-24/C-25/C-26/C-28의 기존 전투·상호작용·잡기·발걸음 이벤트
  - `ASnowRumblePlayerState::IsVoiceSpeaking()`와 `GetVoiceChannel()`
- 제공할 계약:
  - UI 버튼, 전투, 상호작용, 보이스 상태에 대해 Blueprint가 연결할 수 있는 재생 지점
  - 옵션의 소리 설정이 전체 볼륨과 개별 카테고리에 반영되는 공용 적용 경로
- 인계 대상: 사용자 또는 서유정(S)이 UI 버튼, 전투, 상호작용, 보이스용 사운드 자산과 연출을 연결

## 범위 밖

- 실제 SoundCue, MetaSound, Wave, Music 자산 제작
- 믹싱용 사운드 효과의 최종 디자인 튜닝
- Steam Voice, 플랫폼 음성, 외부 음성 장치 지원 확장
- `.uasset` 직접 수정

## 사전 전제

- C-21 옵션 메뉴의 소리·마이크 설정이 유지된다.
- UI 버튼과 게임 행동의 기존 입력·이벤트 경로가 유지된다.

## 결정 필요

- 전체 볼륨을 별도 master volume으로 둘지, 현재 BGM/SFX/보이스 조합으로 충분한지
- UI 클릭음, 전투 효과음, 상호작용 효과음, 보이스 알림음의 우선순위와 반복 재생 규칙
- 보이스 음량을 엔진 음성 채널에 직접 연결할지, 별도 attenuation 경로를 둘지

## 변경 기록

- 2026-08-21: 사용자 요청으로 오디오 피드백과 보이스 믹싱 계약 Task를 추가했다. 현재 코드에는 사운드 재생 헬퍼가 거의 없어서, 기존 옵션의 BGM/SFX/보이스 설정을 유지하면서 UI 버튼, 전투, 상호작용, 보이스 상태를 한 번에 다루는 공용 계약이 필요하다.
- 2026-08-22: 맵별 배경음악 시작 지점을 추가했다. 메인메뉴와 커스터마이징은 로컬 PlayerController가 시작하고, 로비·PvP·눈사람 모드·포디움은 GameMode가 각 클라이언트의 PlayerController에 배경음악 재생 RPC를 보낸다.
- 2026-08-22: PvP 전환 중 배경음악이 끊기는 문제를 해결하기 위해 배경음악 재생을 `USnowRumbleBackgroundMusicSubsystem`으로 이관했다. 각 컨트롤러의 EndPlay 정지 호출을 제거해 travel 중에도 음악이 이어지고, 옵션의 BGM 슬라이더는 같은 subsystem 오디오 컴포넌트에 즉시 반영한다.
- 2026-08-22: 배경음악이 사운드 자산의 loop 설정에만 의존하지 않도록 `USnowRumbleBackgroundMusicSubsystem`이 재생 종료 시 같은 음원을 다시 시작하게 했다. `UAudioComponent`의 loop 상태에 기대지 않고 `OnAudioFinished`에서 재생을 재개한다.
- 2026-08-22: 배경음악이 한 번 재생된 뒤 끊기는 회귀를 보강했다. `OnAudioFinished` 콜백 프레임에서 기존 컴포넌트가 아직 재생 중으로 보일 수 있어 조기 반환하던 경로를 막고, 종료 콜백에서는 이전 컴포넌트 참조를 먼저 비운 뒤 같은 음원을 새 컴포넌트로 재시작한다. `git diff --check`, 충돌 표식 검색, UHT와 C++ 컴파일 및 `.lib` 생성은 통과했고, 최종 DLL 링크는 실행 중인 Unreal Editor의 DLL 잠금 `LNK1104`로 보류됐다.

## 수동 작업

- `WBP_MainMenu`, `WBP_Lobby`, `WBP_MainHUD`, `WBP_Options`, `WBP_LobbyBoard`, `WBP_VoiceMuteMenu`에서 버튼/패널 클릭 사운드를 재생할 WBP 이벤트를 연결한다.
- `BP_MainMenuGameMode`, `BP_LobbyGameMode`, `BP_SnowRumblePVPGameMode`, `BP_SnowmanModeGameMode`, `BP_SnowRumblePodiumGameMode`, `BP_CustomizationGameMode`에서 `BackgroundMusicSound` 또는 해당 컨트롤러 배경음악 자산을 지정한다.
- `BP_SnowRumbleCharacter`와 관련 AnimBP/Blueprint에서 눈덩이 투척, 폭발, 피해, 상호작용, 잡기, 발걸음 사운드 자산을 연결한다.
- `BgmSoundClass`, `SfxSoundClass`, 마이크 관련 음성 설정이 실제 SoundClass와 음성 채널에 맞게 연결되도록 확인한다.
- 필요하면 master volume을 SoundClass 계층 또는 별도 AudioSettings UI로 배치한다.

## 완료 조건

### 에이전트 확인

- [ ] 관련 코드·문서 변경 완료
- [ ] 오디오 계약과 볼륨 라우팅 기준이 정리됨
- [ ] 현재 Task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인

- [ ] UI 버튼 클릭 시 지정한 사운드가 재생된다.
- [ ] 눈덩이 투척과 폭발에 사운드가 연결된다.
- [ ] 피해 피드백과 아이템/게시판 상호작용 사운드가 연결된다.
- [ ] 옵션 소리 설정 변경이 전체 오디오 출력에 반영된다.
- [ ] 보이스 송출과 음소거 상태가 음향과 표시 둘 다에서 일관되게 반영된다.
- [ ] `BackgroundMusicSound`에 loop 설정이 없어도 한 곡 종료 후 같은 배경음악이 다시 시작된다.

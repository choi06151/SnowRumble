# Task C-14 - 팀 스폰과 시작 연출

## 설명

무작위 맵에서 팀별 시작 지점에 플레이어를 생성하고 라운드 시작 5초 동안 이동·전투 행동을 제한하며 팀 소개 카메라 상태를 제공한다.

## 상태 전이 기준

- 시작 가능: C-03, C-04 완료와 맵별 팀 시작 지점 준비
- 완료 가능: 서버 스폰·행동 제한과 S·J 맵 담당 인계 확인

## 구현 항목

- [ ] 서버가 팀에 맞는 시작 지점을 선택한다.
- [x] PlayerStart 주변 분산 스폰이 바닥과 캡슐 충돌 검증을 통과한 위치만 사용한다.
- [ ] 라운드 시작 5초 동안 이동과 전투 행동을 제한한다.
- [x] 로컬 카메라가 팀원을 보여줄 시작·종료 상태를 제공한다.
- [ ] 맵 담당자가 배치할 팀 시작 지점 요구사항을 제공한다.

## 작업 배정

- 담당자·기능·계약 소유자: 최재원(C)
- 자산 수정자·생성·변경 후보: 구현 승인 전 확정
- 공유 확인 대상: S-10, J-02, J-04
- 병합 순서: C-03·C-04 후, 맵 게임플레이 배치 전

## 공용 계약과 인계

- 제공받을 계약: C-03 팀 상태, C-04 선택 맵
- 제공할 계약: 팀 스폰 결과, 시작 제한, 팀 소개 상태와 시작 지점 요구사항
  - `ASnowRumbleGameMode::MatchIntroTeamShotSeconds`: PvP 시작 전 한 팀을 보여줄 시간이다. 기본값은 2.5초다.
  - `ASnowRumbleGameMode`: 로딩창 종료와 기존 카운트다운 사이에 현재 참가 팀을 Red, Blue, Sky, Green, Yellow, Purple, Pink, White 순서로 소개한다.
  - `ASnowRumblePlayerController::ClientPlayPvpTeamIntroShot(Team, ShotDurationSeconds)`: 클라이언트가 로컬 월드의 해당 팀 Pawn들을 찾아 임시 CameraActor로 팀 소개 샷을 보여준다.
  - `ASnowRumblePlayerController::ClientFinishPvpTeamIntro()`: 팀 소개 종료 후 로컬 view target을 자기 Pawn으로 되돌린다.
  - `ASnowRumblePlayerController::OnPvpTeamIntroShot(Team, TeamDisplayText, ShotDurationSeconds)`: WBP에서 팀명 UI나 연출을 붙일 수 있는 Blueprint 이벤트다.
- 인계 대상: S-10, J-02, J-04, C-05

## 범위 밖

- 이름표 UI 레이아웃
- 맵별 시작 지점 배치
- 팀 소개 카메라 연출 디테일

## 사전 전제

- C-03
- C-04

## 결정 필요

- 팀 소개 카메라의 최종 샷 길이, 스킵 여부, UI 애니메이션 디테일

## 변경 기록

- 2026-08-14: 사용자가 랜덤 PlayerStart 스폰 때문에 CameraRig를 미리 정확히 지정할 수 있는지 질문해, 고정 CameraRig 의존 대신 스폰된 Pawn 위치를 기준으로 런타임 팀 소개 카메라를 계산하는 C++ 기본틀을 추가했다.
- 2026-08-14: `ASnowRumbleGameMode`는 로딩창 종료 후 기존 `3, 2, 1, 시작!` 카운트다운 전에 참가 팀별 인트로 RPC를 순차 호출한다. `ASnowRumblePlayerController`는 해당 팀 Pawn bounds를 로컬에서 수집해 임시 `ACameraActor`를 만들고 `SetViewTargetWithBlend()`로 보여준다.
- 2026-08-14: WBP는 `OnPvpTeamIntroShot` Blueprint 이벤트에서 `빨간팀`, `파란팀` 같은 팀명 UI를 표시하면 된다. C++ 기본 동작은 기존 personal alarm에도 팀명을 보낸다.
- 2026-08-14: 팀 소개 카메라가 즉시 튀는 느낌을 줄이기 위해 현재 view point에서 샷 시작 위치까지 Tick 보간하고, 이후 팀을 바라보며 좌우 dolly 이동하게 조정했다. 기본 카메라 거리는 950cm에서 650cm로 당겼고 `PvpIntroCameraDollyDistance`로 좌우 이동폭을 조정할 수 있다.
- 2026-08-14: 모든 플레이어 접속 후 로딩창이 닫히고 팀 소개 카메라가 시작되기 전 의미 없는 화면이 보이는 구간을 검정 화면에서 페이드 아웃되게 조정했다. `ASnowRumbleGameMode`가 로딩창 종료 직후 `ClientStartPvpIntroFadeOut()`을 호출하고, 클라이언트는 `PlayerCameraManager->StartCameraFade(1 -> 0)`로 처리한다.
- 2026-08-14: 팀 소개 카메라가 영화처럼 위아래 letterbox를 갖도록 인트로용 임시 `ACameraActor`의 `UCameraComponent`에 `ConstrainAspectRatio`를 켜고 `PvpIntroCinematicAspectRatio` 기본값 2.39를 적용했다. 일반 플레이 카메라는 인트로 카메라 종료 후 자기 Pawn으로 돌아가므로 영향받지 않는다.
- 2026-08-14: 시작 시퀀스 재생 조건을 첫 정규 PvP 진입과 단판 승부전 진입으로 제한했다. `ShouldPlayMatchIntroSequence()`는 활성 매치의 `CurrentRoundNumber <= 1` 또는 `IsTiebreakerActive()`일 때만 true를 반환하며, 일반 2라운드/3라운드 맵 이동에서는 팀 소개와 검정 페이드를 건너뛰고 기존 카운트다운만 시작한다.
- 2026-08-18: PvP 맵 이동 후 건물 내부나 빈 공간에 스폰되는 원인을 `ASnowRumbleGameMode::BuildScatteredPlayerStartTransform()`가 `PlayerStart` 주변 900cm 랜덤 오프셋을 바닥·충돌 검증 없이 사용하는 경로로 확인했다.
- 2026-08-18: PvP 분산 스폰 후보는 이제 월드 정적 바닥 trace로 캡슐 높이를 보정하고, Pawn 캡슐 overlap 검증과 기존 스폰 위치 간격 검증을 모두 통과할 때만 사용한다. 유효 후보가 없으면 더 이상 임의 후보를 쓰지 않고 원래 `PlayerStart` 위치로 fallback한다.
- 2026-08-18: PvP 분산 스폰 안전성 변경은 `git diff --check`, UHT, `SnowRumbleGameMode.cpp` 컴파일을 통과했다. 최종 DLL 링크는 실행 중인 Unreal Editor의 `UnrealEditor-SnowRumble.dll` 잠금 `LNK1104`로 보류됐다.

## 수동 작업 (구현 후 구체화)

## 완료 조건

### 에이전트 확인
- [ ] 서버 팀 스폰과 행동 제한 완료
- [ ] 맵 담당 시작 지점 요구 인계 완료
- [x] 로컬 전용 카메라 상태 분리 확인
- [x] `git diff --check` 공백 점검 통과
- [x] 관련 C++ 컴파일과 `.lib` 생성 통과
- [ ] Unreal Editor 종료 후 `SnowRumbleEditor Win64 Development` 최종 링크 확인

### 검증 메모

- 2026-08-14: `git diff --check`는 통과했다. `SnowRumbleEditor Win64 Development` 빌드는 UHT, `SnowRumbleGameMode.cpp`, `SnowRumblePlayerController.cpp`, `.lib` 생성까지 통과했지만 실행 중인 Unreal Editor가 `Binaries/Win64/UnrealEditor-SnowRumble.dll`을 사용 중이라 최종 DLL 링크는 `LNK1104`로 실패했다.
- 2026-08-14: 카메라 보간과 dolly 이동 조정 후 다시 빌드했다. UHT, C++ 컴파일, `.lib` 생성까지 통과했고 최종 DLL 링크만 실행 중인 Unreal Editor DLL 잠금 `LNK1104`로 보류됐다.
- 2026-08-14: 인트로 전 검정 페이드 아웃 추가 후 다시 빌드했다. 첫 빌드는 `PlayerCameraManager` include 경로를 수정했고, 재빌드는 UHT, C++ 컴파일, `.lib` 생성까지 통과했다. 최종 DLL 링크만 실행 중인 Unreal Editor DLL 잠금 `LNK1104`로 보류됐다.
- 2026-08-14: 팀 소개 카메라 letterbox 설정 추가 후 다시 빌드했다. UHT, C++ 컴파일, `.lib` 생성까지 통과했고 최종 DLL 링크만 실행 중인 Unreal Editor DLL 잠금 `LNK1104`로 보류됐다.
- 2026-08-14: 시작 시퀀스 재생 조건 제한 후 다시 빌드했다. UHT, C++ 컴파일, `.lib` 생성까지 통과했고 최종 DLL 링크만 실행 중인 Unreal Editor DLL 잠금 `LNK1104`로 보류됐다.
- 2026-08-18: PvP 분산 스폰 위치 안전성 보강 후 다시 빌드했다. UHT와 `SnowRumbleGameMode.cpp` 컴파일은 통과했고, 최종 DLL 링크만 실행 중인 Unreal Editor DLL 잠금 `LNK1104`로 보류됐다.

### 결과 확인 (구현 후 구체화)

- [ ] PvP 로딩창이 닫힌 뒤 빨간팀/파란팀 등 참가 팀 소개 카메라가 순서대로 보인다.
- [ ] 팀 소개 중 이동·시점·공격 입력이 반응하지 않는다.
- [ ] 팀 소개가 끝난 뒤 기존 `3`, `2`, `1`, `시작!` 카운트다운이 표시된다.
- [ ] `시작!` 이후 view target이 자기 Pawn으로 돌아오고 조작이 풀린다.
- [ ] PvP 맵에서 플레이어가 PlayerStart 주변 건물 내부·공중·벽 안이 아니라 바닥 위의 충돌 없는 위치에 스폰된다.
- [ ] 유효한 분산 후보가 없을 때는 이상한 랜덤 위치 대신 원래 PlayerStart 위치에서 스폰된다.

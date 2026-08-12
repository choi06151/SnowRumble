# Task C-23 - 마우스 휠 카메라 줌

## 설명

로비와 PvP 인게임에서 로컬 플레이어가 마우스 휠을 올리거나 내리면 3인칭 카메라 SpringArm 길이가 가까워지거나 멀어지게 한다.

## 상태 전이 기준

- 시작 가능: 기존 `ASnowRumbleCharacter` 카메라 SpringArm 보간과 로컬 입력 처리 재사용 가능
- 완료 가능: 로컬 전용 휠 입력 처리, 카메라 길이 최소·최대·스텝 조정값, 로비/PvP 결과 확인 절차 기록 완료

## 구현 항목

- [x] 마우스 휠 업 입력으로 로컬 카메라 SpringArm 목표 길이를 줄인다.
- [x] 마우스 휠 다운 입력으로 로컬 카메라 SpringArm 목표 길이를 늘린다.
- [x] 줌 길이는 최소·최대값 사이로 제한한다.
- [x] 로비와 PvP에서 같은 캐릭터 카메라 경로로 동작하게 한다.
- [x] 게시판 포커스, UI 전용 입력 상태처럼 캐릭터 카메라를 직접 조작하지 않는 상황에서는 줌 처리를 막는다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: C++·문서 최재원(C), 캐릭터 BP 조정값 확인은 사용자/S 인계
- 변경 파일: `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Tasks/C/C-23_camera_wheel_zoom.md`, `Tasks/C/PLAN_C.md`, `Tasks/C/ROLE_C.md`, `docs/PLANS.md`
- 공유 확인 대상: 사용자 또는 S
- 병합 순서: 기존 플레이어 카메라 입력 경로 위에 후속 기능으로 병합

## 공용 계약과 인계

- 제공받을 계약: 기존 `ASnowRumbleCharacter::CameraBoom`, 로컬 카메라 보간 경로
- 제공할 계약:
  - `ASnowRumbleCharacter::CameraZoomStep`: 휠 한 칸당 SpringArm 길이 변화량
  - `ASnowRumbleCharacter::MinimumCameraArmLength`: 카메라 줌 최소 SpringArm 길이
  - `ASnowRumbleCharacter::MaximumCameraArmLength`: 카메라 줌 최대 SpringArm 길이
  - `ASnowRumbleCharacter::UpdateCameraZoomInput()`: 로컬 휠 업/다운 입력으로 기본 카메라 SpringArm 목표 길이를 갱신한다.
- 인계 대상: 사용자 또는 S

## 범위 밖

- 줌 UI 표시
- 줌 설정 저장
- 조준 카메라 규칙 변경
- 새 Enhanced Input Action 자산 생성

## 사전 전제

- 기존 `ASnowRumbleCharacter` 카메라

## 결정 필요

- 없음

## 변경 기록

- 2026-08-11: 사용자가 로비와 인게임에서 마우스 휠 업/다운으로 카메라 SpringArm 길이를 줄이고 늘리는 기능을 요청해 C-23을 추가했다.
- 2026-08-11: `ASnowRumbleCharacter` 로컬 Tick에서 `MouseScrollUp`/`MouseScrollDown`을 감지해 `DesiredCameraArmLength`를 최소·최대 범위 안에서 갱신하게 했다. 조준 중에는 기존 조준 카메라 길이를 유지하고, 일반 카메라로 돌아오면 휠 줌 값이 적용된다.
- 2026-08-11: `git diff --check`는 통과했고 `SnowRumbleCharacter.cpp` 컴파일도 통과했으나, 실행 중인 Unreal Editor가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크는 `LNK1104`로 보류됐다. 에디터 종료 후 재빌드 확인이 필요하다.

## 수동 작업

- 캐릭터 BP에서 휠 한 칸당 변화량은 `CameraZoomStep`으로 조정한다.
- 가장 가까운 카메라 거리는 `MinimumCameraArmLength`로 조정한다.
- 가장 먼 카메라 거리는 `MaximumCameraArmLength`로 조정한다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드 변경 완료
- [x] `git diff --check` 공백 점검 통과
- [ ] `SnowRumbleEditor Win64 Development` 빌드 성공
- [x] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [x] 공용 계약과 캡슐화 규칙 위반 없음
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인

- [ ] 로비에서 마우스 휠 업을 하면 카메라가 캐릭터에 가까워진다.
- [ ] 로비에서 마우스 휠 다운을 하면 카메라가 캐릭터에서 멀어진다.
- [ ] PvP 인게임에서 마우스 휠 업/다운을 하면 같은 방식으로 카메라 거리가 바뀐다.
- [ ] 카메라 거리는 `MinimumCameraArmLength`보다 가까워지지 않는다.
- [ ] 카메라 거리는 `MaximumCameraArmLength`보다 멀어지지 않는다.
- [ ] 게시판 포커스 중에는 마우스 휠을 움직여도 캐릭터 카메라 줌이 바뀌지 않는다.
- [ ] 조준 중에는 기존 조준 카메라 거리가 유지되고, 조준 해제 후 일반 카메라 줌 값으로 돌아온다.

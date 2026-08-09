# Task C-16 - 팀원 HP HUD

## 설명

PvP 맵에서 로컬 플레이어와 같은 팀 색을 가진 다른 플레이어들의 HP 바를 좌측 상단 HUD 영역에 동적으로 표시한다.

## 상태 전이 기준

- 시작 가능: C-03 팀 색 상태와 기존 `UHealthBarWidget`, `UMainHUDWidget` 사용 가능
- 완료 가능: 같은 팀원만 표시, 팀원 입장·퇴장·팀 변경·HP 변경 대응, WBP 인계와 결과 확인 절차 기록 완료

## 구현 항목

- [x] 자기 HP 바는 기존 `LocalHealthBar` 경로를 유지한다.
- [x] 다른 플레이어 HP 바는 로컬 플레이어와 같은 `ESnowRumbleTeam`일 때만 생성한다.
- [x] 팀원이 아니거나 팀 정보가 없거나 Pawn이 사라진 플레이어의 HP 바를 제거한다.
- [x] 팀원 HP 바는 기존 `UHealthBarWidget`을 재사용해 복제 HP 변경을 실시간 표시한다.
- [x] 팀원 HP 바 클래스가 비어 있으면 기존 로컬 HP 바 클래스 fallback을 사용한다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: C++·문서: 최재원(C), HUD WBP 배치: 사용자 또는 S 인계
- 생성·변경 후보: `Tasks/C/C-16_teammate_health_hud.md`
- 변경 파일: `Source/SnowRumble/UI/MainHUDWidget.h`, `Source/SnowRumble/UI/MainHUDWidget.cpp`, `Tasks/C/PLAN_C.md`, `Tasks/C/ROLE_C.md`, `docs/PLANS.md`
- 공유 확인 대상: 사용자 또는 S
- 병합 순서: C-03 팀 상태와 기존 HP 컴포넌트 후, HUD WBP 최종 배치 전

## 공용 계약과 인계

- 제공받을 계약:
  - `ASnowRumblePlayerState::GetLobbyTeam()`: 로컬 플레이어와 다른 플레이어의 팀 색 비교에 사용한다.
  - `USnowRumbleHealthComponent`: 각 캐릭터의 복제 HP 원본이다.
  - `UHealthBarWidget::SetObservedActor(AActor* NewObservedActor)`: 팀원 캐릭터의 HP 컴포넌트에 HP 바를 연결한다.
- 제공할 계약:
  - `UMainHUDWidget::OtherPlayersHealthPanel`: WBP에서 좌측 상단에 배치할 팀원 HP 바 목록 패널이다.
  - `UMainHUDWidget::OtherPlayerHealthBarWidgetClass`: 팀원마다 동적으로 생성할 HP 바 WBP 클래스다. 비어 있으면 `LocalHealthBar`의 클래스가 fallback으로 사용된다.
  - `UMainHUDWidget::LocalHealthBar`: 기존 자기 HP 바이며 좌측 하단 표시를 유지한다.
- 인계 대상: 사용자 또는 S

## 범위 밖

- 팀원 HP 바의 최종 그래픽 디자인
- 팀원 닉네임, 아이콘, 거리, 상태 이상 표시 추가
- 얼음·사망·관전 전용 HUD 상태

## 사전 전제

- C-03 팀 색 상태
- 기존 `UHealthBarWidget`

## 결정 필요

- 없음

## 변경 기록

- 2026-08-09: 사용자가 PvP 맵에서 같은 색 팀원이 있을 때 좌측 상단에 팀원 HP 바를 동적으로 표시하길 요청해 C-16을 추가하고 구현했다.

## 수동 작업

- HUD WBP에서 기존 자기 HP 바는 `LocalHealthBar` 이름으로 좌측 하단에 유지한다.
- HUD WBP 좌측 상단에 Vertical Box 또는 Panel을 만들고 이름을 `OtherPlayersHealthPanel`로 맞춘다.
- 팀원 HP 바 전용 WBP를 따로 쓰려면 `OtherPlayerHealthBarWidgetClass`에 `UHealthBarWidget` 기반 WBP를 연결한다.
- `OtherPlayerHealthBarWidgetClass`를 비워 두면 C++ 부모가 `LocalHealthBar`와 같은 WBP 클래스를 팀원 HP 바에도 사용한다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드 변경 완료
- [x] `git diff --check` 공백 점검 통과
- [x] 관련 C++ 컴파일 통과
- [x] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [x] 공용 계약과 캡슐화 규칙 위반 없음
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨

### 검증 메모

- 2026-08-09: `UMainHUDWidget`의 다른 플레이어 HP 바 후보를 같은 팀 색 플레이어로 제한하고, 팀원 HP 바 WBP 클래스가 비어 있으면 `LocalHealthBar` 클래스를 fallback으로 쓰게 했다. `git diff --check`는 통과했고 `MainHUDWidget.cpp`와 `Module.SnowRumble.cpp` 컴파일도 통과했다. 링크는 실행 중인 Unreal Editor 프로세스가 `Binaries/Win64/UnrealEditor-SnowRumble.dll`을 잡고 있어 `LNK1104`로 실패했다.
- 2026-08-09: 사용자가 팀원 HP HUD 동작을 확인했다.

### 결과 확인

- [x] PvP 맵에서 자기 HP 바는 기존처럼 좌측 하단에 표시된다.
- [x] 같은 팀 색 플레이어가 1명 이상 있으면 좌측 상단 `OtherPlayersHealthPanel`에 팀원 HP 바가 인원 수만큼 생성된다.
- [x] 다른 팀 색 플레이어의 HP 바는 좌측 상단에 표시되지 않는다.
- [x] 팀원이 피해를 받으면 좌측 상단 팀원 HP 바가 실시간으로 감소한다.
- [x] 팀원이 나가거나 Pawn이 사라지면 해당 HP 바가 제거된다.
- [x] 로컬 플레이어의 팀 색이 없으면 팀원 HP 바가 생성되지 않는다.

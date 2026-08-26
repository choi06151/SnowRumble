# Task J-02 - 얼음 빙하 게임플레이 배치

## 설명

얼음 빙하에 팀 시작 지점, 기본 눈덩이, 상자·모닥불 후보와 수위 상승 환경 압박 로직을 배치해 플레이 가능한 맵으로 완성한다.

경기 진행 시간에 따라 빙하 지형을 단계적으로 축소하기 위해 `SnowRumbleIceGlacierCollapseActor_J`를 사용한다. 현재 1차 구현은 Fracture, Geometry Collection, Chaos 물리를 쓰지 않고, C++ Actor가 지정된 빙판 조각 Actor의 Transform을 직접 제어하는 방식이다.

## 상태 전이 기준
- 시작 가능: J-01, C-04, C-05, C-07, C-14, K-02, K-08 완료
- 완료 가능: 수위 로직·피해·스폰·아이템 배치의 호스트·클라이언트 결과 확인

## 구현 항목
- [ ] 팀별 시작 지점과 5초 소개에 필요한 공간을 배치한다.
- [ ] 기본 눈덩이, 선물상자 Spawn Point와 기본 모닥불 후보를 배치한다.
- [ ] 수위 상승 단계와 10초 전 경고가 공간 변화와 일치하게 한다.
- [ ] 수위 상승과 얼음물 지속 피해를 맵 전용 Actor·Component·Blueprint로 구현한다.
- [ ] 위험 지역의 균열·물결·경고 색상과 안전 복귀 동선을 제공한다.
- [ ] 주요 판정을 Level Blueprint에 중복 구현하지 않는다.

## 작업 배정
- 담당자·기능·자산 수정자: 정다영(J)
- 계약 소유자: 레벨·수위 환경 로직은 J, 공용 피해·경기 계약은 C, 아이템 배치는 K
- 생성·변경 후보: 구현 승인 전 확정
- 공유 확인 대상: C-04, C-05, C-07, C-14, K-02, K-08, S-04
- 병합 순서: J-01 → C·K 계약 → 배치 → J-05

## 공용 계약과 인계
- 제공받을 계약: C 랜덤 맵·경기 시간·피해·스폰, K 상자·모닥불
- 제공할 계약: 완성 맵, 침수 로직·기본 눈덩이·배치·로딩 이미지 요구
- 인계 대상: C-04, C-12, K-10, S-04, J-05

## 범위 밖
- 공용 플레이어·경기 상태 직접 변경
- Fracture, Geometry Collection, Chaos 물리 기반 빙판 파괴
- 공용 hazard framework 신규 추가

## 사전 전제
- J-01
- C-04, C-05, C-07, C-14
- K-02, K-08

## 결정 필요
- 얼음물에 빠진 플레이어의 수영 복귀 지점과 경계 처리

## 변경 기록

- 2026-08-19 `0d8dace`: `Source/SnowRumble/Map/SnowRumbleIceGlacierCollapseActor_J.h`, `Source/SnowRumble/Map/SnowRumbleIceGlacierCollapseActor_J.cpp`를 추가하고 `L_IceGlacier_J.umap`에 연결할 빙판 침몰 Actor의 1차 구조를 만들었다.
- 2026-08-19 `0d8dace`: 1차 구조는 경기 경과 시간 기준으로 Group 1은 240~300초, Group 2는 300~360초에 침몰하도록 하고, Final Core는 침몰 대상에서 제외하는 방향으로 기록한다.
- 2026-08-21 `e2dee52`: 기존 `CollapseGroup1`, `CollapseGroup2` 단순 Actor 배열을 `Group1Pieces`, `Group2Pieces` 구조체 배열로 바꾸고, 조각별 `StartDelaySeconds`, `WarningDurationSeconds`, `ShakeAmplitude`, `ShakeFrequency`, `FallDurationSeconds`, `SinkDistance`를 설정할 수 있게 보강했다.
- 2026-08-21 `e2dee52`: 각 빙판 조각의 상태를 `Idle -> Warning -> Falling -> Done`으로 계산하고, Warning 흔들림과 Falling 하강을 `InitialTransform` 기준 절대 Transform 계산으로 처리하도록 정리했다.
- 2026-08-21 `e2dee52`: 완전히 침몰한 빙판 조각은 `bDisableCollisionAfterFullySunk` 값에 따라 기존 Collision 상태를 저장한 뒤 비활성화할 수 있게 했다.
- 2026-08-24 `c916bd9`: 원격 J 브랜치의 `L_IceGlacier_J.umap` 최신 수정분을 반영했다. 해당 커밋은 맵 자산만 수정하므로 C++ 기능 상태 변경으로 보지 않는다.

## 현재 C++ 구현 기록

### SnowRumbleIceGlacierCollapseActor_J

- 목적: 경기 진행 시간에 따라 얼음 빙하의 외곽 빙판을 단계적으로 축소한다.
- 기존 `CollapseGroup1`, `CollapseGroup2` 단순 Actor 배열을 `Group1Pieces`, `Group2Pieces` 구조체 배열로 대체했다.
- 각 조각은 `Idle → Warning → Falling → Done` 순서로 동작한다.
- 조각마다 `StartDelaySeconds`를 다르게 주어 여러 빙판이 동시에 떨어지지 않고 약간의 시간차를 두고 하강하게 한다.
- 기존 전역 `SinkDistance`는 제거하고 각 조각별 `SinkDistance`를 사용한다.
- Runtime에서는 숫자 설정을 별도 Runtime Settings에 복사하고, `TargetActor`는 `TWeakObjectPtr<AActor>`로 관리한다.

### 경기 시간

- Group 1: 240 ~ 300초
- Group 2: 300 ~ 360초
- Final Core: 붕괴 대상에서 제외
- 빙하 Actor 자체 로컬 타이머가 아니라 기존 SnowRumble GameState의 경기 경과 시간을 사용한다.
- Group Start = 그룹 전체 붕괴의 기준 시작 시각
- Start Delay = 해당 그룹 안에서 각 조각이 얼마나 늦게 시작할지 결정하는 값
- 실제 조각 Warning 시작 시간은 `GroupStartSeconds + StartDelaySeconds`다. 예: `Group1StartSeconds = 240`, `StartDelaySeconds = 0.5`이면 경기시간 240.5초부터 Warning이 시작된다.

### 조각 상태 계산

- `WarningStart = GroupStartSeconds + StartDelaySeconds`
- `WarningEnd = WarningStart + WarningDurationSeconds`
- `FallEnd = WarningEnd + FallDurationSeconds`
- Idle: 아직 해당 조각의 시작 시간이 되지 않은 상태
- Warning: 하강 직전 흔들리는 상태
- Falling: 아래 방향으로 하강 중인 상태
- Done: 하강 완료 상태
- `RoundElapsed`가 `GroupEndSeconds` 이상이면 해당 그룹 조각은 최종 Done 상태로 맞춘다.
- [확인 필요] `StartDelaySeconds + WarningDurationSeconds + FallDurationSeconds`가 그룹 시간 안에 끝나도록 Editor에서 조정한다. 그룹 종료 시간을 넘으면 종료 시점에 최종 위치로 보정될 수 있다.

### Transform 처리

- Warning과 Falling은 이전 프레임 값에 더하는 누적 방식이 아니라 각 조각의 `InitialTransform` 기준 절대 계산이다.
- Warning은 Initial 기준 X/Y에 작은 Sin/Cos 흔들림을 주고, Pitch/Roll에 최대 약 1.5도 수준의 작은 회전을 적용한다.
- Warning 시작과 끝에서 갑자기 튀지 않도록 흔들림 강도가 자연스럽게 올라갔다 내려가도록 처리한다.
- `ShakeAmplitude` 또는 `ShakeFrequency`가 0이면 기본 `InitialTransform`을 유지한다.
- Falling은 Initial X/Y, Rotation, Scale을 유지하고 Z만 `Initial Z - SinkDistance` 방향으로 하강한다.
- 현재 1차 구현에서는 Falling 중 추가 회전이나 실제 물리 파괴를 사용하지 않는다.

### 멀티플레이

- 기존 서버 권한 구조를 유지한다.
- 서버가 경기 시간을 기준으로 `CurrentCollapsePhase`를 계산하고 복제한다.
- 클라이언트는 복제된 Phase와 GameState의 `GetRoundElapsedSeconds()`를 사용해 동일한 조각 상태와 Transform을 계산한다.
- 공용 GameMode, Character, Health 로직은 수정하지 않는다.
- [확인 필요] 클라이언트는 Phase 복제가 도착하기 전 잠깐 이전 Phase 상태를 볼 수 있다.

## Editor 설정값

### Group1Pieces / Group2Pieces

각 그룹에서 실제로 흔들리고 하강할 빙판 조각 목록이다. 기존 `CollapseGroup1`, `CollapseGroup2` 단순 Actor 배열을 대체하며, 각 배열 항목마다 개별 조각 설정을 가진다. Final Core는 배열에 등록하지 않는다.

### FIceGlacierCollapsePiece

- `TargetActor`: 해당 설정을 적용할 실제 빙판 조각 Actor.
- `StartDelaySeconds`: Group Start 이후 해당 조각이 Warning을 시작하기까지 기다리는 시간. 조각마다 서로 다른 값을 주어 모든 빙판이 동시에 떨어지는 느낌을 방지한다.
- `WarningDurationSeconds`: 실제 하강 전에 빙판이 흔들리는 시간.
- `ShakeAmplitude`: Warning 구간의 흔들림 크기. 값이 클수록 X/Y 이동 흔들림이 커진다.
- `ShakeFrequency`: Warning 구간의 흔들림 빠르기. Amplitude는 크기, Frequency는 속도로 이해한다.
- `FallDurationSeconds`: Warning 종료 이후 빙판이 최종 침몰 위치까지 이동하는 데 걸리는 시간. 값이 작을수록 빠르게 떨어지고, 값이 클수록 천천히 내려간다.
- `SinkDistance`: 빙판이 아래 방향으로 이동하는 총 거리. `Initial Z - SinkDistance` 위치까지 내려간다. 같은 FallDuration이면 SinkDistance가 클수록 이동 속도도 빨라진다.
- `Group1StartSeconds / Group2StartSeconds`: 각 그룹 전체 붕괴의 기준 시간이며, 조각별 StartDelay와 역할이 다르다.

## 현재 구현 상태

### 구현 완료

- `Group1Pieces`, `Group2Pieces` 구조체 배열 적용
- 조각별 `StartDelaySeconds` 구조
- Warning 흔들림 구조
- 조각별 `FallDurationSeconds`
- 조각별 `SinkDistance`
- `InitialTransform` 기준 절대 계산
- 기존 GameState 경기 시간 사용
- 기존 서버 Phase 구조 유지
- 공용 GameMode, Character, Health 수정 없음

### 확인 필요

- [확인 필요] 실제 빌드 성공
- [확인 필요] UHT 검증 완료
- [확인 필요] Unreal Editor PIE 테스트 완료
- [확인 필요] Host / Client 동기화 테스트 완료
- [확인 필요] 실제 흔들림 수치 최종 확정
- [확인 필요] 실제 StartDelay 최종 확정

### 정적 검토 메모

- 변경 C++ 파일: `Source/SnowRumble/Map/SnowRumbleIceGlacierCollapseActor_J.h`, `Source/SnowRumble/Map/SnowRumbleIceGlacierCollapseActor_J.cpp`
- `git diff --check` 통과
- 공용 파일 수정 없음
- `.umap`은 이번 C++ 구현 작업에서 수정하지 않음

## 수동 작업

1. `L_IceGlacier_J`에 배치된 `SnowRumbleIceGlacierCollapseActor_J` 인스턴스를 확인한다.
2. 기존 `CollapseGroup1`, `CollapseGroup2` 할당은 재사용하지 않고 `Group1Pieces`, `Group2Pieces`를 다시 설정한다.
3. 각 조각마다 `TargetActor`, `StartDelaySeconds`, `WarningDurationSeconds`, `ShakeAmplitude`, `ShakeFrequency`, `FallDurationSeconds`, `SinkDistance`를 지정한다.
4. Final Core는 `Group1Pieces`, `Group2Pieces`에 등록하지 않는다.
5. [확인 필요] Group 1은 240~300초, Group 2는 300~360초 안에서 Warning과 Falling이 끝나도록 조각별 시간을 조정한다.

## 완료 조건
### 에이전트 확인
- [ ] 게임플레이 배치 완료
- [ ] Level Blueprint 중복 판정 없음
- [ ] C·K·S 인계 완료
- [ ] [확인 필요] `SnowRumbleIceGlacierCollapseActor_J` 빌드와 UHT 검증 완료
### 결과 확인
- [ ] [확인 필요] PIE에서 Group 1 조각이 240~300초 구간에 순차적으로 Warning/Falling/Done 처리된다.
- [ ] [확인 필요] PIE에서 Group 2 조각이 300~360초 구간에 순차적으로 Warning/Falling/Done 처리된다.
- [ ] [확인 필요] Host / Client에서 붕괴 Phase와 조각 Transform이 같은 흐름으로 보인다.
- [ ] [확인 필요] Final Core가 붕괴 대상에서 제외되어 유지된다.

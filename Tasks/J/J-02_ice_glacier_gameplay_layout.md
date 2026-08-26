# Task J-02 - 얼음 빙하 게임플레이 배치

## 설명

얼음 빙하에 팀 시작 지점, 기본 눈덩이, 상자·모닥불 후보와 빙판 붕괴 환경 압박 로직을 배치해 플레이 가능한 맵으로 완성한다.

경기 진행 시간에 따라 빙하 지형을 단계적으로 축소하기 위해 `SnowRumbleIceGlacierCollapseActor_J`를 사용한다. 현재 구현은 Fracture, Geometry Collection, Chaos 물리를 쓰지 않고, C++ Actor가 지정된 빙판 조각 Actor의 Transform을 직접 제어하는 방식이다.

## 상태 전이 기준
- 시작 가능: J-01, C-04, C-05, C-07, C-14, K-02, K-08 완료
- 완료 가능: 빙판 붕괴·스폰·아이템 배치의 호스트·클라이언트 결과 확인

## 구현 항목
- [ ] 팀별 시작 지점과 5초 소개에 필요한 공간을 배치한다.
- [ ] 기본 눈덩이, 선물상자 Spawn Point와 기본 모닥불 후보를 배치한다.
- [ ] 빙판 조각 붕괴 단계와 Warning 경고가 공간 변화와 일치하게 한다.
- [ ] 빙판 조각 하강과 Falling Runtime Actor Carry를 맵 전용 Actor·Component·Blueprint로 구현한다.
- [ ] 위험 지역의 균열·물결·경고 색상과 안전 복귀 동선을 제공한다.
- [ ] 주요 판정을 Level Blueprint에 중복 구현하지 않는다.

## 작업 배정
- 담당자·기능·자산 수정자: 정다영(J)
- 계약 소유자: 레벨·빙판 붕괴 환경 로직은 J, 공용 피해·경기 계약은 C, 아이템 배치는 K
- 생성·변경 후보: 구현 승인 전 확정
- 공유 확인 대상: C-04, C-05, C-07, C-14, K-02, K-08, S-04
- 병합 순서: J-01 → C·K 계약 → 배치 → J-05

## 공용 계약과 인계
- 제공받을 계약: C 랜덤 맵·경기 시간·피해·스폰, K 상자·모닥불
- 제공할 계약: 완성 맵, 빙판 붕괴 로직·기본 눈덩이·배치·로딩 이미지 요구
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

- 2026-08-19: `Source/SnowRumble/Map/SnowRumbleIceGlacierCollapseActor_J.h`, `Source/SnowRumble/Map/SnowRumbleIceGlacierCollapseActor_J.cpp`를 추가하고 `L_IceGlacier_J.umap`에 연결할 빙판 침몰 Actor의 1차 구조를 만들었다.
- 2026-08-19: 1차 구조는 경기 경과 시간 기준으로 Group 1은 240~300초, Group 2는 300~360초에 침몰하도록 하고, Final Core는 침몰 대상에서 제외하는 방향으로 기록한다.
- 2026-08-21: 기존 `CollapseGroup1`, `CollapseGroup2` 단순 Actor 배열을 `Group1Pieces`, `Group2Pieces` 구조체 배열로 바꾸고, 조각별 `StartDelaySeconds`, `WarningDurationSeconds`, `ShakeAmplitude`, `ShakeFrequency`, `FallDurationSeconds`, `SinkDistance`를 설정할 수 있게 보강했다.
- 2026-08-21: 각 빙판 조각의 상태를 `Idle -> Warning -> Falling -> Done`으로 계산하고, Warning 흔들림과 Falling 하강을 `InitialTransform` 기준 절대 Transform 계산으로 처리하도록 정리했다.
- 2026-08-21: 완전히 침몰한 빙판 조각은 `bDisableCollisionAfterFullySunk` 값에 따라 기존 Collision 상태를 저장한 뒤 비활성화할 수 있게 했다.
- 2026-08-24: 원격 J 브랜치의 `L_IceGlacier_J.umap` 최신 수정분을 반영했다. 해당 커밋은 맵 자산만 수정하므로 C++ 기능 상태 변경으로 보지 않는다.
- 2026-08-26: Warning 중 로컬 플레이어 Camera Shake를 추가하고, 현재 밟고 있는 Warning Piece의 `WarningAlpha`와 실행 중 Camera Shake `ShakeScale`을 동기화하도록 C++ 구현을 보강했다. 기본 Start/Stop과 WarningAlpha 강도 증가 PIE 확인까지 완료했다.
- 2026-08-26: Warning 빙판 진동은 `InitialTransform` 기준 절대 계산과 `WarningAlpha^2` 강도 증가 방식으로 PIE 정상 동작을 확인했다.
- 2026-08-26: Falling 진입 순간 빙판 위 runtime gameplay Actor를 `LineTraceMultiByChannel`로 감지하고, Falling 동안 Piece 하강 Delta만큼 함께 이동시키는 Carry 구조를 C++에 추가했다. C4800, C4458 컴파일 오류 보정은 완료했으며, 보정 후 재빌드와 PIE 검증은 아직 확인 필요 상태다.

## 현재 C++ 구현 기록

### SnowRumbleIceGlacierCollapseActor_J

- 목적: 경기 진행 시간에 따라 얼음 빙하의 외곽 빙판을 단계적으로 축소한다.
- 기존 `CollapseGroup1`, `CollapseGroup2` 단순 Actor 배열을 `Group1Pieces`, `Group2Pieces` 구조체 배열로 대체했다.
- 각 조각은 `Idle → Warning → Falling → Done` 순서로 동작한다.
- Core는 최종 안전 지형이며 `Group1Pieces`, `Group2Pieces`에 등록하지 않는 붕괴 제외 대상이다.
- 조각마다 `StartDelaySeconds`를 다르게 주어 여러 빙판이 동시에 떨어지지 않고 약간의 시간차를 두고 하강하게 한다.
- 기존 전역 `SinkDistance`는 제거하고 각 조각별 `SinkDistance`를 사용한다.
- Runtime에서는 숫자 설정을 별도 Runtime Settings에 복사하고, `TargetActor`는 `TWeakObjectPtr<AActor>`로 관리한다.

### 경기 시간

- 0 ~ 240초: 붕괴 없음
- 240 ~ 300초: Group 1 붕괴 구간
- 300 ~ 360초: Group 2 붕괴 구간
- 360초 이후: Core만 남는 최종 구간
- 빙하 Actor 자체 로컬 타이머가 아니라 기존 SnowRumble GameState의 경기 경과 시간을 사용한다.
- `GroupStartSeconds`는 그룹 전체 붕괴의 기준 시작 시각이다.
- 실제 조각 Warning 시작 시간은 `GroupStartSeconds + StartDelaySeconds`다. 예: `Group1StartSeconds = 240`, `StartDelaySeconds = 0.5`이면 경기시간 240.5초부터 Warning이 시작된다.

### 조각 상태 계산

- `WarningStart = GroupStartSeconds + StartDelaySeconds`
- `WarningEnd = WarningStart + WarningDurationSeconds`
- `FallEnd = WarningEnd + FallDurationSeconds`
- Idle: 아직 해당 조각의 시작 시간이 되지 않은 상태
- Warning: 하강 직전 흔들리는 상태
- Falling: 아래 방향으로 하강 중인 상태
- Done: 하강 완료 상태
- `WarningAlpha`는 Warning 시작에서 0.0, Falling 직전에서 1.0이다.
- `RoundElapsed`가 `GroupEndSeconds` 이상이면 해당 그룹 조각은 최종 Done 상태로 맞춘다.
- [확인 필요] `StartDelaySeconds + WarningDurationSeconds + FallDurationSeconds`가 그룹 시간 안에 끝나도록 Editor에서 조정한다. 그룹 종료 시간을 넘으면 종료 시점에 최종 위치로 보정될 수 있다.

### Transform 처리

- Warning과 Falling은 이전 프레임 값에 더하는 누적 방식이 아니라 각 조각의 `InitialTransform` 기준 절대 계산이다.
- Warning은 Initial 기준 X/Y에 작은 Sin/Cos 흔들림을 주고, Pitch/Roll에 최대 약 1.5도 수준의 작은 회전을 적용한다.
- Warning 흔들림 강도는 `ShakeStrength = WarningAlpha * WarningAlpha`를 사용한다.
- 흔들림은 약하게 시작해 점점 강해지고, Falling 직전에 최대가 된다.
- Warning 빙판 진동은 PIE에서 정상 동작을 확인했다.
- 기존 `Sin(WarningAlpha * PI)`처럼 약함 → 강함 → 다시 약함 형태의 강도 곡선은 사용하지 않는다.
- `ShakeAmplitude` 또는 `ShakeFrequency`가 0이면 기본 `InitialTransform`을 유지한다.
- Falling은 Initial X/Y, Rotation, Scale을 유지하고 Z만 `Initial Z - SinkDistance` 방향으로 하강한다.
- 현재 구현에서는 Falling 중 추가 회전이나 실제 물리 파괴를 사용하지 않는다.
- 떨어지면서 산산조각 나는 Fracture, Geometry Collection, Chaos 기반 연출은 향후 추가 검토 항목이며 현재 구현 완료 기능이 아니다.

### Falling Runtime Actor Carry

- 목적: Ice Piece가 Falling할 때 해당 빙판 위에 있는 runtime gameplay Actor가 공중에 남지 않고 빙판과 함께 하강하게 한다.
- Editor에서 Actor를 Piece에 수동 Attach하거나 Piece마다 Item을 수동 등록하지 않는다.
- 런타임에 랜덤 Spawn된 대상도 Falling 진입 순간 자동 감지하는 구조다.
- Carry 후보 클래스는 현재 최소 구현 기준 `AGiftBox`, `AGiftBoxItemPickup`, `ACampfire`, `ASnowballItem`이다.
- `ACharacter`, `APawn`, 플레이어, Ice `TargetActor` 자기 자신, `SnowRumbleIceGlacierCollapseActor_J`, invalid 또는 Destroy 상태 Actor, 다른 Actor에 Attach된 Actor, Static Mobility Actor, 이동 대상으로 적절하지 않은 Actor는 제외한다.
- Snowball은 `ASnowballItem::CanBePickedUp()` 기준의 일반 Ground Pickup 상태만 Carry한다. Held, Rolling, Thrown, Holder가 존재하는 상태, Attach된 상태는 제외한다.
- 별도 Trigger, Box Collision, Overlap Volume은 추가하지 않는다.
- `BoxOverlap`, `OverlapMulti` 방식은 사용하지 않는다.
- Falling 진입 순간 서버에서 `TActorIterator`로 Carry 후보 클래스만 순회한다.
- `TargetActor`의 기존 `PrimitiveComponent` Bounds는 후보 Actor가 빙판 XY 범위와 명백하게 떨어져 있는지 빠르게 거르는 좌표 비교 용도로만 사용한다.
- XY 필터를 통과한 후보 Actor는 Root 또는 Primitive Collision Bounds 기준 하단 부근에서 아래 방향 `LineTraceMultiByChannel`을 수행한다.
- 첫 유효 바닥 Hit Actor가 현재 Falling 중인 `TargetActor`인 경우에만 현재 Ice Piece 위 Actor로 확정해 Carry 목록에 저장한다.
- 다른 빙판, 다른 지형이 먼저 Hit되거나 Hit가 없으면 Carry 대상에서 제외한다.
- Carry Actor 탐색은 Warning에서 Falling으로 진입하는 순간 한 번만 수행하고, Falling 중 매 Tick Trace를 반복하지 않는다.
- Piece별 runtime 상태로 `CarriedActors`, `PreviousCarryPieceLocation`, `PreviousPieceState`, `bCarryInitializedForFall`을 관리한다.
- Warning에서 Falling으로 처음 진입한 프레임에는 Carry 대상 탐색, `CarriedActors` 저장, `PreviousCarryPieceLocation` 초기화까지만 수행하고 Carry Actor를 이동시키지 않는다.
- 첫 Falling 프레임에 이동 Delta를 적용하지 않는 이유는 Warning 마지막 X/Y 흔들림이 `InitialTransform` 기준으로 복귀하며 생기는 횡방향 Delta가 Item에 전달되어 옆으로 순간 이동하는 현상을 방지하기 위해서다.
- 다음 Tick부터 `CurrentPieceLocation - PreviousCarryPieceLocation`으로 `DeltaLocation`을 계산하고, Carry Actor에 동일한 Delta만 적용한다.
- Actor의 절대 위치를 Piece 위치로 덮어쓰지 않고, Actor가 기존에 가지고 있던 빙판 위 상대 위치를 유지한다.
- 현재 Ice Falling은 Z축 하강 중심이므로 불필요한 회전 또는 복잡한 DeltaTransform Carry는 추가하지 않는다.
- `CarriedActors`는 `TArray<TWeakObjectPtr<AActor>>`로 관리한다.
- Falling 도중 Actor가 Pickup, Destroy, invalid, Attach, Carry 부적합 상태가 되면 안전하게 목록에서 제거하거나 무시한다.
- Piece가 Falling 상태를 벗어나거나 Done 상태가 되면 해당 Piece의 Carry 목록과 runtime 상태를 초기화한다.
- Carry 기능은 `Simulate Physics`에 의존하지 않는다.
- Chaos, Geometry Collection, Fracture는 이번 Carry 구현 범위에 포함하지 않으며 향후 빙판 파쇄 연출과 별도 기능으로 취급한다.

### Warning Camera Shake

- Warning 상태인 빙판 위에 서 있는 로컬 플레이어에게만 Camera Shake를 적용한다.
- Camera Shake Class는 C++ Content 경로에 하드코딩하지 않고 `TSubclassOf<UCameraShakeBase> WarningCameraShakeClass`로 Editor에 노출한다.
- Camera Shake Blueprint Asset은 Unreal Editor에서 수동 지정한다.
- 플레이어가 현재 Piece 위에 있는지는 `Character -> CharacterMovement -> MovementBase -> MovementBase Owner` 경로로 확인한다.
- `MovementBase Owner == TargetActor`이면 해당 Warning Piece 위에 있다고 판단한다.
- Warning Piece 위에 올라가면 Camera Shake를 시작한다.
- 같은 Warning Piece 위에 계속 있으면 기존 Camera Shake Instance를 유지하고 매 Tick 새 Camera Shake를 만들지 않는다.
- Warning이 아닌 다른 빙판, Core, 일반 지형으로 이동하거나 점프 또는 Piece 이탈이 발생하면 Camera Shake를 정지한다.
- Warning이 끝나 Falling으로 전환되면 Camera Shake를 정지한다.
- 다른 Warning Piece로 이동하면 기존 Shake를 정지한 뒤 새 Piece 기준으로 Camera Shake를 시작한다.
- `EndPlay`에서는 활성 Camera Shake를 안전하게 정지한다.

### WarningAlpha - Camera Shake 강도 동기화

- Camera Shake Blueprint는 흔들림 방향, Amplitude, Frequency, Duration 등 흔들림 자체의 모양을 담당한다.
- C++은 현재 Warning Piece의 `WarningAlpha`를 사용해 실행 중인 Camera Shake의 전체 Scale을 조절한다.
- 최종 Camera Shake Scale은 아래 흐름으로 계산한다.

```text
ClampedWarningAlpha = Clamp(WarningAlpha, 0.0f, 1.0f)
CameraShakeStrength = Lerp(0.3f, 1.0f, ClampedWarningAlpha * ClampedWarningAlpha)
FinalScale = WarningCameraShakeScale * CameraShakeStrength
```
- Warning 시작 시 Camera Shake는 기본 약 30% 수준으로 약하게 시작하고, Warning 진행 중 점점 강해지며, Falling 직전 최대 강도가 된다.
- Falling 시작 시 Camera Shake는 정지한다.
- 초기 구현에서는 `WarningAlpha^2`만 사용했으나, 짧은 Warning에서 초반 Camera Shake가 너무 약하게 느껴져 최소 기본 진동을 유지하도록 보정했다.
- 빙판 Warning 자체와 Camera Shake는 `WarningAlpha^2` 기반 Ease-In 진행도를 공유하고, Camera Shake는 최소 체감 강도만 추가 보정한다.
- 실행 중에는 새 Shake를 매 Tick 생성하지 않고 기존 `ActiveWarningCameraShakeInstance`의 `ShakeScale`만 갱신한다.
- 같은 TargetActor 위에 있더라도 `ActiveWarningCameraShakeInstance`가 invalid이면 현재 Warning 상태 기준으로 다시 시작할 수 있게 처리한다.
- 기본 Camera Shake Start/Stop과 WarningAlpha에 따른 강도 증가는 PIE에서 동작 확인을 완료했다.

### 멀티플레이

- 기존 서버 권한 구조를 유지한다.
- 서버가 경기 시간을 기준으로 `CurrentCollapsePhase`를 계산하고 복제한다.
- 클라이언트는 복제된 Phase와 GameState의 `GetRoundElapsedSeconds()`를 사용해 동일한 조각 상태와 Transform을 계산한다.
- Camera Shake는 멀티플레이 환경에서 각 로컬 플레이어 전용 연출로 동작한다.
- Camera Shake 자체 네트워크 복제는 하지 않는다.
- Camera Shake용 서버 RPC와 Multicast는 추가하지 않는다.
- 전체 플레이어 Camera를 일괄적으로 흔들지 않는다.
- Host는 Host 자신의 화면, Client는 해당 Client 자신의 화면만 처리한다.
- Falling Runtime Actor Carry는 실제 gameplay Actor Transform을 변경하므로 `HasAuthority()`인 서버에서만 Carry 대상 탐색과 이동을 수행한다.
- 클라이언트는 Carry Actor 위치를 독립 계산하지 않고 기존 Actor movement replication 결과를 받는다.
- Carry는 서버 권한 기반 gameplay Actor Carry, 클라이언트 독립 Transform 계산 없음, 기존 Actor movement replication 활용 구조로 정리한다.
- 공용 GameMode, Character, Health 로직은 수정하지 않는다.
- [확인 필요] 클라이언트는 Phase 복제가 도착하기 전 잠깐 이전 Phase 상태를 볼 수 있다.

## Editor 설정값

### Group1Pieces / Group2Pieces

각 그룹에서 실제로 흔들리고 하강할 빙판 조각 목록이다. 기존 `CollapseGroup1`, `CollapseGroup2` 단순 Actor 배열을 대체하며, 각 배열 항목마다 개별 조각 설정을 가진다. Final Core는 배열에 등록하지 않는다.

### FIceGlacierCollapsePiece

- `TargetActor`: 해당 설정을 적용할 실제 빙판 조각 Actor.
- `StartDelaySeconds`: Group Start 이후 해당 조각이 Warning을 시작하기까지 기다리는 시간. 조각마다 서로 다른 값을 주어 모든 빙판이 동시에 떨어지는 느낌을 방지한다.
- `WarningDurationSeconds`: 실제 하강 전에 빙판이 흔들리는 시간.
- `ShakeAmplitude`: Warning 구간의 빙판 흔들림 크기. 값이 클수록 X/Y 이동 흔들림이 커진다.
- `ShakeFrequency`: Warning 구간의 빙판 흔들림 빠르기. Amplitude는 크기, Frequency는 속도로 이해한다.
- `FallDurationSeconds`: Warning 종료 이후 빙판이 최종 침몰 위치까지 이동하는 데 걸리는 시간. 값이 작을수록 빠르게 떨어지고, 값이 클수록 천천히 내려간다.
- `SinkDistance`: 빙판이 아래 방향으로 이동하는 총 거리. `Initial Z - SinkDistance` 위치까지 내려간다. 같은 FallDuration이면 SinkDistance가 클수록 이동 속도도 빨라진다.
- `Group1StartSeconds / Group2StartSeconds`: 각 그룹 전체 붕괴의 기준 시간이며, 조각별 StartDelay와 역할이 다르다.


### Warning Camera Shake

- `WarningCameraShakeClass`: Warning 중 로컬 플레이어에게 재생할 Camera Shake Blueprint Class다. 실제 Asset은 Editor에서 수동 지정한다.
- `WarningCameraShakeScale`: C++이 WarningAlpha 기반 강도와 곱해 최종 Camera Shake Scale을 만들 때 사용하는 기본 배율이다.
- Camera Shake Blueprint의 Amplitude, Frequency, Duration 숫자 값은 현재 Editor 튜닝 항목이며 이 문서에서 확정 사양으로 기록하지 않는다.

## 현재 구현 상태

### 구현 완료

- `Group1Pieces`, `Group2Pieces` 구조체 배열 적용
- Core 붕괴 제외 구조
- 조각별 `StartDelaySeconds` 구조
- 조각별 `WarningDurationSeconds`
- Warning 흔들림 구조
- Warning 흔들림 `WarningAlpha^2` Ease-In 강도 적용
- Warning 빙판 진동 PIE 정상 동작 확인 완료
- 조각별 `FallDurationSeconds`
- 조각별 `SinkDistance`
- `InitialTransform` 기준 절대 계산
- 기존 GameState 경기 시간 사용
- 기존 서버 Phase 구조 유지
- 로컬 플레이어 전용 Warning Camera Shake C++ 구조
- Warning Camera Shake 기본 Start/Stop PIE 동작 확인 완료
- 현재 Warning Piece의 `WarningAlpha` 기반 Camera Shake Scale 동기화 C++ 구현 및 PIE 동작 확인 완료
- Warning Camera Shake 최소 기본 진동 0.3 보정 완료
- Warning Camera Shake가 시간이 지날수록 강해지는 동작 PIE 확인 완료
- 같은 Warning Piece 위에서 실행 중 Camera Shake Instance 유지와 `ShakeScale` 갱신
- Warning 종료, Falling 전환, Piece 이탈, Core 또는 일반 지형 이동 시 Camera Shake 정지 구조
- Falling Piece runtime Actor Carry C++ 구현
- Piece별 Carry runtime 상태
- `TActorIterator` 후보 탐색
- XY Bounds 사전 필터
- 아래 방향 `LineTraceMultiByChannel` 판정
- Character/Pawn Carry 제외
- Snowball Ground Pickup 상태 필터
- Falling 진입 1회 Carry Capture
- 다음 Tick부터 `DeltaLocation` Carry 이동
- `TWeakObjectPtr<AActor>` 기반 Carry 수명 처리
- Carry 대상 탐색과 이동의 Server Authority 처리
- C4800 `FBox::IsValid` 명시적 bool 판정 보정
- C4458 `RootComponent` 지역 변수 이름 가림 보정
- 공용 GameMode, Character, Health 수정 없음
- Camera Shake용 서버 RPC, Multicast 추가 없음

### 확인 필요

- [확인 필요] Carry 컴파일 오류 보정 후 최종 Build 성공
- [확인 필요] UHT 검증 완료
- [확인 필요] PIE에서 GiftBox Carry 확인
- [확인 필요] PIE에서 GiftBoxItemPickup Carry 확인
- [확인 필요] PIE에서 Campfire Carry 확인
- [확인 필요] PIE에서 Ground Snowball Carry 확인
- [확인 필요] PIE에서 Held, Rolling, Thrown Snowball 제외 확인
- [확인 필요] PIE에서 Character/Pawn 비대상 확인
- [확인 필요] PIE에서 Random Spawn Actor Carry 대응 확인
- [확인 필요] Host / Client에서 Carry 대상 Actor의 ReplicateMovement 결과 확인
- [확인 필요] Host / Client 동기화 테스트 완료
- [확인 필요] 실제 빙판 흔들림 수치 최종 확정
- [확인 필요] 실제 StartDelay 최종 확정
- [확인 필요] Camera Shake Blueprint의 Amplitude, Frequency, Duration 최종 튜닝

### 정적 검토 메모

- 변경 C++ 파일: `Source/SnowRumble/Map/SnowRumbleIceGlacierCollapseActor_J.h`, `Source/SnowRumble/Map/SnowRumbleIceGlacierCollapseActor_J.cpp`
- Warning 빙판 진동, Warning Camera Shake 기본 동작과 WarningAlpha 강도 연동은 C++ 구현과 PIE 동작 확인을 완료했다.
- Falling Runtime Actor Carry는 C++ 구현과 C4800/C4458 컴파일 오류 보정까지 완료했다.
- Carry 보정 후 Build, UHT, PIE 검증은 아직 완료하지 않았다.
- 공용 파일 수정 없음
- `.umap`은 이번 C++ 구현 작업에서 수정하지 않음

## 수동 작업

1. `L_IceGlacier_J`에 배치된 `SnowRumbleIceGlacierCollapseActor_J` 인스턴스를 확인한다.
2. 기존 `CollapseGroup1`, `CollapseGroup2` 할당은 재사용하지 않고 `Group1Pieces`, `Group2Pieces`를 다시 설정한다.
3. 각 조각마다 `TargetActor`, `StartDelaySeconds`, `WarningDurationSeconds`, `ShakeAmplitude`, `ShakeFrequency`, `FallDurationSeconds`, `SinkDistance`를 지정한다.
4. Final Core는 `Group1Pieces`, `Group2Pieces`에 등록하지 않는다.
5. `WarningCameraShakeClass`에 Camera Shake Blueprint Asset을 지정한다.
6. `WarningCameraShakeScale`과 Camera Shake Blueprint의 Amplitude, Frequency, Duration은 PIE 확인 후에도 연출 튜닝 값으로 관리한다.
7. [확인 필요] Group 1은 240~300초, Group 2는 300~360초 안에서 Warning과 Falling이 끝나도록 조각별 시간을 조정한다.

## 완료 조건
### 에이전트 확인
- [ ] 게임플레이 배치 완료
- [ ] Level Blueprint 중복 판정 없음
- [ ] C·K·S 인계 완료
- [ ] [확인 필요] `SnowRumbleIceGlacierCollapseActor_J` Carry 보정 후 빌드와 UHT 검증 완료
### 결과 확인
- [ ] [확인 필요] PIE에서 Group 1 조각이 240~300초 구간에 순차적으로 Warning/Falling/Done 처리된다.
- [ ] [확인 필요] PIE에서 Group 2 조각이 300~360초 구간에 순차적으로 Warning/Falling/Done 처리된다.
- [x] PIE에서 Warning 빙판 진동이 시간이 지날수록 강해지고 Falling 직전 최대가 된다.
- [x] PIE에서 Warning 중인 Piece 위 로컬 플레이어에게 Camera Shake가 시작된다.
- [x] PIE에서 Warning Camera Shake가 WarningAlpha 진행도에 맞춰 약함 → 강함으로 증가한다.
- [x] PIE에서 Warning Piece 이탈, 정상 빙판, Core, Falling 전환 시 Camera Shake가 정지된다.
- [ ] [확인 필요] PIE에서 GiftBox가 Falling Piece와 함께 하강한다.
- [ ] [확인 필요] PIE에서 GiftBoxItemPickup이 Falling Piece와 함께 하강한다.
- [ ] [확인 필요] PIE에서 Campfire가 Falling Piece와 함께 하강한다.
- [ ] [확인 필요] PIE에서 Ground Snowball이 Falling Piece와 함께 하강한다.
- [ ] [확인 필요] PIE에서 Held, Rolling, Thrown Snowball은 Carry 대상에서 제외된다.
- [ ] [확인 필요] PIE에서 Character/Pawn은 Carry 대상에서 제외된다.
- [ ] [확인 필요] PIE에서 Random Spawn Actor가 Falling 진입 순간 자동 Carry 후보로 처리된다.
- [ ] [확인 필요] Host / Client에서 붕괴 Phase와 조각 Transform이 같은 흐름으로 보인다.
- [ ] [확인 필요] Host / Client에서 Camera Shake가 각자 로컬 화면에만 적용된다.
- [ ] [확인 필요] Host / Client에서 Carry 대상 Actor 위치가 기존 movement replication으로 반영된다.
- [ ] [확인 필요] Final Core가 붕괴 대상에서 제외되어 유지된다.

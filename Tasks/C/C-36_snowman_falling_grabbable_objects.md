# Task C-36 - 눈사람 모드 랜덤 낙하 Grab 물체

## 설명

눈사람 모드가 실제로 시작된 뒤 종료될 때까지, 맵에 배치한 전용 스포너 Actor에 등록한 후보 Static Mesh 중 하나를 서버가 무작위 선택해 NavMesh 위 랜덤 위치의 하늘에서 낙하시킨다. 생성된 물체는 기존 `AGrabbablePhysicsObject` 계약을 사용해 호스트와 클라이언트가 동일하게 보고 잡을 수 있어야 한다.

## 상태 전이 기준

- 시작 가능: C-34 `AGrabbablePhysicsObject` 및 Static Mesh 변환 계약 완료, 사용자 구현 승인
- 완료 가능: 눈사람 모드 시작·반복·종료에 연결된 서버 권한 낙하 구현, Blueprint 인계, 수동 멀티플레이 확인

## 구현 항목

- [ ] C 소유 전용 스포너 Actor에 후보 Static Mesh 배열과 낙하 주기·높이·NavMesh 검색 반경·물리 상호작용 조정값을 노출한다.
- [ ] NavMesh 위 랜덤 위치를 서버에서 선택하고 후보 Mesh를 가진 `AGrabbablePhysicsObject`를 하늘에서 반복 스폰한다.
- [ ] 눈사람 GameState의 실제 경기 시작·종료 상태를 감지해 모드 종료 또는 맵 전환 시 낙하 타이머와 남은 생성 작업을 중단한다.
- [ ] 호스트·클라이언트가 동일한 복제 물체를 Grab할 수 있도록 기존 물리 Actor 계약을 재사용한다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: 사용자/S (후보 Static Mesh 연결 및 필요 시 물체 Blueprint)
- 생성·변경 후보: 구현 승인 후 `Source/SnowRumble/Item/SnowmanFallingGrabbableSpawner_C.*`, 필요 시 `Source/SnowRumble/Item/`의 기존 공개 계약, 이 Task 문서와 `Tasks/C/PLAN_C.md`, `docs/PLANS.md`
- 공유 확인 대상: C-34 Grab 물리 물건, K 눈사람 모드, S 후보 Mesh·표현 자산
- 병합 순서: C-34 선행 계약 후 C-36 구현, 후보 Mesh 연결 후 수동 멀티플레이 확인

## 공용 계약과 인계

- 제공받을 계약: `AGrabbablePhysicsObject::ConfigureReplicatedVisuals()`, `ConfigureInteractionSettings()`, `GetPhysicsComponent()`
- 제공할 계약: `ASnowmanFallingGrabbableSpawner`의 후보 Mesh·낙하 조정값 Blueprint 슬롯과 서버 스폰 동작
- 인계 대상: 사용자/S

## 범위 밖

- 새 Mesh·Material·VFX·Sound 제작
- 눈사람 모드 승패·감염 규칙 변경
- 기존 맵에 후보 Mesh를 직접 배치하거나 Unreal 자산을 코드로 생성

## 사전 전제

- C-34의 물리 Grab, 복제, 눈덩이 충돌 계약을 그대로 사용한다.
- 낙하 위치는 NavMesh가 유효한 지점으로 제한한다.

## 결정 필요

- 없음. 기본값은 시작 후 일정 지연 뒤 첫 생성, 이후 일정 주기 반복, 한 번에 1개 생성으로 둔다.

## 변경 기록

- 2026-08-29: 사용자 요청으로 눈사람 모드 NavMesh 랜덤 위치 기반 낙하 Grab 물체 Task를 추가했다. 눈사람 GameMode K 소유권을 침범하지 않도록 독립 스포너 Actor 방식으로 분리했다.

## 수동 작업 (구현 후 구체화)

- `ASnowmanFallingGrabbableSpawner`를 부모로 하는 Blueprint를 만든다.
- 눈사람 모드로 사용하는 각 맵에 스포너 Blueprint를 1개 배치하고, Actor 위치를 NavMesh 검색 중심으로 둔다.
- `CandidateStaticMeshes`에 낙하시킬 Static Mesh를 1개 이상 지정한다. 비어 있으면 스폰하지 않는다.
- 필요하면 `FirstDropDelaySeconds`, `DropIntervalSeconds`, `NavMeshSearchRadius`, `DropHeightOffset`와 상호작용 값을 조정한다.
- 스포너 위치 주변에 NavMesh가 생성되어 있는지 확인한다. 실제 스폰은 서버에서만 수행되며 생성된 물체는 자동 복제된다.

## 완료 조건

### 에이전트 확인

- [x] 관련 C++ 코드와 Task 문서 갱신 완료
- [x] 정적 점검 및 프로젝트 구조 기준 확인 완료
- [x] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [x] 공용 계약과 캡슐화 규칙 위반 없음

### 결과 확인 (구현 후 구체화)

1. 눈사람 모드 시작 카운트다운과 제한시간이 시작된 뒤 후보 Mesh가 하늘에서 NavMesh 위 랜덤 위치로 떨어진다.
2. 등록한 후보가 여러 개면 생성마다 후보 Mesh가 무작위로 선택된다.
3. 호스트와 클라이언트 화면에서 같은 물체가 같은 위치로 복제되고, 양쪽 플레이어가 물체를 Grab할 수 있다.
4. 눈사람 모드가 종료되면 추가 낙하가 발생하지 않는다.

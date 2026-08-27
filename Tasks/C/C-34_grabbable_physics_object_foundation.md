# Task C-34 - Grab 물리 상호작용 물건 베이스

## 설명

여러 Blueprint 물건이 상속할 수 있는 서버 권한 물리 Actor를 제공한다. 플레이어가 잡아 손을 따라 이동시키고, 물건이 플레이어를 밀치며, 투척된 작은 눈과 큰 눈을 부술 수 있는 기반을 만든다.

## 상태 전이 기준

- 시작 가능: C-28 Grab 계약 완료 및 사용자 승인
- 완료 가능: C++ 베이스 Actor와 Grab·눈덩이 연결 구현, 정적 점검 및 Blueprint 인계 완료

## 구현 항목

- [x] `AGrabbablePhysicsObject` 기반 Actor와 물리 조정값 추가
- [x] 기존 Grab 후보 탐색·Constraint에 물리 Actor 연결 추가
- [x] 눈덩이 충돌 시 물리 Actor 파괴 처리 추가
- [x] Grab 물건 자식 기믹 확장 훅과 탬버린 사운드 기믹 베이스 추가
- [x] 멀티플레이 수동 확인 절차와 Blueprint 연결 정보 기록

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: 없음(후속 Blueprint는 별도 배정)
- 생성·변경 후보: `Source/SnowRumble/Item/GrabbablePhysicsObject_C.*`, `Source/SnowRumble/Item/TambourineGrabbableObject_C.*`, 기존 Grab·Snowball 코드
- 공유 확인 대상: S의 물건 Blueprint 제작, C-28 Grab, C-09 Snowball
- 병합 순서: C-34 구현 후 물건 Blueprint 연결

## 공용 계약과 인계

- 제공할 계약: `AGrabbablePhysicsObject`의 Root Primitive, 힘 조정값, Grab 가능 상태, Grab 중 확장 훅, `ATambourineGrabbableObject` 사운드 슬롯
- 인계 대상: 후속 물건 Blueprint 제작 담당자

## 범위 밖

- 실제 물건별 Blueprint·Mesh·Material·Sound 제작
- 새로운 피해량 규칙이나 게임 승패 규칙

## 사전 전제

- 기존 C-28 Grab의 서버 권한과 손 Constraint를 재사용한다.

## 결정 필요

- 없음

## 변경 기록

- 2026-08-27: Grab 물체의 카메라 Pitch 추종 방향이 상하 반대로 적용되던 문제를 수정했다. 물리 상대 회전에 적용하는 Pitch Delta 부호를 반전해 카메라를 올리면 물체도 위로, 내리면 아래로 이동하도록 보정했다.

- 2026-08-27: 잡히지 않은 물리 물체의 일반 충돌이 플레이어를 밀치지 않도록 Grab 상태 검사를 추가했다. Grab 중 별도 overlap 기반 플레이어 밀침은 유지한다.
- 2026-08-27: 물리 물건을 잡은 뒤 플레이어가 바라보는 Pitch 변화도 물건의 상대 위치·회전에 반영하도록 확장했다. 잡기 시작 시점의 Pitch를 기준으로 변화량만 적용해 Grab 순간의 물건 방향은 유지한다.
- 2026-08-27: 사용자의 물리 상호작용 물건 베이스 요청으로 신규 Task를 추가했다.
- 2026-08-27: 물리 시뮬레이션 Blueprint가 `PhysicsBody` Object Type을 사용할 때 Grab trace에서 누락되던 오류를 수정했다.
- 2026-08-27: 물리 물건을 잡는 동안 Grab 게이지가 줄거나 자동 해제되지 않도록 예외 처리했다.
- 2026-08-27: 물리 물건이 손 위치를 더 강하게 따라오도록 선형 Constraint 위치 드라이브와 강도·감쇠 조정값을 추가했다.
- 2026-08-27: 물리 물건 Grab 중 외부 충돌 토크로 나풀거리는 현상을 막기 위해 Constraint 회전 자유도를 잠갔다.
- 2026-08-27: Grab한 플레이어와 물리 물건이 서로 충돌하지 않도록 물건 Component에 소유 Actor 무시를 적용하고 해제 시 복원했다.
- 2026-08-27: Actor 무시만으로 물리 접촉이 남을 수 있어 플레이어 Capsule-물건 사이에 충돌만 비활성화하는 자유 Constraint를 추가했다.
- 2026-08-27: 물리 물건 Constraint가 손 bone을 직접 당겨 팔이 늘어나는 회귀를 막기 위해 숨김 PhysicsOnly 앵커를 손 위치에 갱신하고 물건은 해당 앵커에 연결하도록 변경했다.
- 2026-08-27: 회전 중 물리 물건이 손 기준점에서 점점 멀어지는 현상을 줄이기 위해 물리 물건 전용 작은 slack과 서버 속도 보정을 추가했다.
- 2026-08-27: 물리 물건 Grab 중에는 플레이어 Pawn이 카메라 Yaw를 따라 회전하고, 해제 시 이전 회전 설정으로 복원되도록 했다.
- 2026-08-27: 잡힌 물리 물건은 손 위치에서 밀리지 않도록 Grab 중 물리 시뮬레이션을 잠시 끄고 직접 위치 고정으로 전환했다. 상대 플레이어 밀침은 별도 overlap 기반 처리로 유지한다.
- 2026-08-27: 물리 물건 Grab 시작 시 손 목표점으로 순간 이동하지 않고, 잡힌 순간의 캐릭터 상대 Transform을 유지하며 따라오도록 변경했다.
- 2026-08-27: 잡힌 물리 물건의 상대 플레이어 밀침 판정을 Sphere 범위가 아니라 실제 PrimitiveComponent collision shape overlap으로 변경했다.
- 2026-08-27: 눈덩이가 Grab 물리 물건에 맞으면 반사되지 않고 충돌 이펙트 후 부서지도록 변경했다.
- 2026-08-27: `AGrabbablePhysicsObject`에 Grab 시작·해제·Tick 확장 훅을 추가하고, `ATambourineGrabbableObject`가 잡힌 상태에서 플레이어 이동 시 위치 기반 찰랑 사운드를 멀티캐스트로 재생하도록 추가했다.

## 수동 작업

- 물건 Blueprint가 `AGrabbablePhysicsObject`를 부모로 상속한다.
- 탬버린 Blueprint는 `ATambourineGrabbableObject`를 부모로 상속한다.
- Root Primitive 또는 `PhysicsComponent`에 Static Mesh를 연결하고 Simulate Physics와 충돌을 활성화한다.
- 물건별 `PlayerPushStrength`를 조정한다.
- 탬버린 Blueprint에서 `JingleSound`, 필요 시 `JingleSoundAttenuation`, 이동 속도·거리·쿨다운 값을 조정한다.

## 완료 조건

### 에이전트 확인

- [x] 관련 C++ 코드와 Task 문서 갱신 완료
- [x] 정적 점검 완료 (`git diff --check`, 충돌 표식 검색)
- [x] 공용 계약과 캡슐화 규칙 위반 없음

### 결과 확인

- [ ] 호스트가 물건을 잡아 이동할 때 물건이 모든 클라이언트에서 따라온다.
- [ ] 클라이언트가 물건을 잡아 이동할 때 서버가 상태를 확정한다.
- [ ] 물건 충돌로 플레이어가 밀려난다.
- [ ] 작은 눈과 큰 눈이 물건에 맞으면 충돌 이펙트 후 부서진다.
- [ ] 탬버린을 잡고 이동하면 모든 클라이언트에서 위치 기반 찰랑 소리가 들린다.

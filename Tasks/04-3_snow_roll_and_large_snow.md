# Task 04-3 - 눈 굴리기와 큰 눈

## 설명

플레이어가 눈덩이를 굴려 최대 크기까지 성장시키고, 커진 눈덩이를 느리게 운반해 포물선 투척과 광역 충돌에 사용할 수 있게 한다.

## 구현 항목

- [x] 눈덩이 앞에서 굴리기 행동을 시작할 수 있다.
- [x] 굴리는 동안 눈덩이가 정해진 최대 크기까지 성장한다.
- [x] 크기에 따라 굴리기와 운반 중 이동속도가 감소한다. — 현재 단위에서는 굴리기 이동속도 감소를 구현함
- [ ] 큰 눈덩이를 양손 운반 상태로 구분할 수 있다.
- [ ] 큰 눈덩이는 작은 눈덩이보다 오래 충전해 포물선으로 던진다.
- [ ] 큰 눈덩이가 충돌하면 정해진 범위에 영향을 준다.
- [ ] 성장, 운반, 투척, 충돌 결과가 모든 참가자에게 동일하게 보인다.

## 메인 프로그래머 책임

- 굴리기, 성장, 이동속도 변화, 양손 운반, 큰 눈 투척·광역 충돌과 모든 비UI 표현을 완성한다.

## SUB UI 인계

- 연계 SUB Task: `Tasks/S04-2_aim_charge_ui.md`
- 큰 눈 조준 시 충전 UI가 구분해야 하는 상태와 정규화된 진행도를 제공한다.

## 범위 밖

- 최종 피해 수치와 밸런스
- 눈사람 만들기 게임 모드
- 최종 애니메이션과 폭발 이펙트

## 사전 전제

- Task 04-2의 작은 눈덩이 투척

## 현재 구현 단위

- 첫 번째 단위는 바닥 눈덩이를 대상으로 `E 탭 획득`과 `E 유지 + WASD 굴리기`를 구분한다.
- `E`를 누른 뒤 이동 입력 없이 놓으면 기존 획득을 요청한다.
- `E`를 유지한 상태에서 WASD를 입력하면 획득 대신 굴리기를 시작하고, `E`를 놓으면 굴리기를 종료한다.
- 굴리기 대상과 상태은 서버가 거리·아이템 상태를 다시 검사해 확정하고 모든 참가자에게 복제한다.
- 크기 성장은 다음 단위에서 굴린 실제 거리를 기준으로 `1배 → 최대 3배`, 기본 최대 성장 거리 `10m`를 Blueprint 조정값으로 구현한다.

## 현재 성장 구현 단위

- 서버가 캐릭터 이동 방향 앞의 목표 지점까지 눈덩이를 충돌 Sweep으로 이동한다.
- 서버가 굴리기 중 눈덩이의 실제 이동 거리를 누적해 기본 `10m`에서 최대 성장에 도달하도록 한다.
- 성장값은 `0~1`로 복제하고 눈덩이 Actor Scale과 충돌 구체를 `1배 → 최대 3배` 범위로 함께 키운다.
- 눈덩이가 커질수록 굴리는 캐릭터 이동속도를 감소시킨다.

## 현재 성장 구현 결과

- `ASnowballItem`의 `GrowthProgress`는 서버가 실제 수평 이동 거리를 누적해 계산하고 `0~1`로 복제한다.
- `Distance For Maximum Growth` 기본값은 `1000cm`, `Maximum Scale Multiplier` 기본값은 `3`이다.
- `OnRep_GrowthProgress()`는 초기 Actor Scale을 기준으로 눈덩이 Actor 전체 크기를 적용해 메시와 충돌 구체를 함께 키운다.
- 서버는 성장으로 증가한 충돌 반지름만큼 눈덩이 중심을 위로 보정하고, 굴리기 한 번의 최초 성장에만 기본 `2cm`의 지면 여유를 추가해 다음 Sweep을 막지 않으면서 여유 높이가 누적되지 않게 한다.
- 굴리기 상태에서는 물리 시뮬레이션과 중력을 잠시 끄고 서버가 `Rolling Distance` 기본 `90cm`의 목표 지점까지 충돌 Sweep으로 이동한다.
- 굴리는 캐릭터의 최대 이동속도는 성장률에 따라 `300 → 150`으로 감소하고 굴리기 종료 시 기존 이동 규칙으로 복구된다.
- 로컬 소유 플레이어가 굴리는 동안에는 복제된 `RollingSnowball`을 기존 `OutlineComponent`의 대상으로 사용해 해당 눈덩이의 아웃라인을 유지한다.
- `GetRollingSnowball()`로 현재 굴리기 대상을 Blueprint에서 읽을 수 있다.
- `GetGrowthProgress()`와 `IsRollingSnowball()`을 Blueprint에서 읽을 수 있다.
- Animation Blueprint용 `ESnowballCarryState`는 `Normal`, `SmallSnowball`, `LargeSnowball`을 제공한다.
- 캐릭터의 `GetSnowballCarryState()`는 획득 연출 중이거나 빈손이면 `Normal`, 최대 성장 전 보유 눈덩이는 `SmallSnowball`, 성장률 `1`에 도달한 보유 눈덩이는 `LargeSnowball`을 반환한다.
- 별도 `ESnowballActionState`는 우선 `None`, `RollingSnowball`을 제공하며 `GetSnowballActionState()`로 읽는다.
- 굴리기 중 AnimBP 상태 조합은 `CarryState = Normal`, `ActionState = RollingSnowball`이다.
- 기존 `IsHoldingSnowball()`은 기존 Blueprint 호환성을 위해 유지한다.

## 현재 구현 결과

- `HandleInteractStarted()`는 E 입력 유지 상태만 기록하며 즉시 획득하지 않는다.
- E를 유지한 채 첫 이동 입력이 들어오면 `StartRollingSnowball()`이 서버에 굴리기를 요청한다.
- 서버는 기존 획득 반경 `180cm` 안의 가장 가까운 `Ground` 눈덩이를 `Rolling` 상태로 전환한다.
- 서버의 `SnowballEquipmentComponent` Tick이 캐릭터의 서버 이동 방향을 기준으로 눈덩이를 앞쪽 `90cm` 지점까지 Sweep 이동한다.
- 굴리기 중에는 눈덩이 Sweep과 캐릭터 캡슐 이동 양쪽에서 서로만 충돌 대상에서 제외하고, 월드 장애물과 다른 액터 충돌은 유지하며 종료 시 제외 설정을 복구한다.
- 굴리기 중 눈덩이는 물리 시뮬레이션과 중력을 끄고 충돌 Sweep과 Actor 이동 복제를 사용한다.
- 이동한 거리와 충돌 구체 반지름을 기준으로 눈덩이 Actor 회전을 적용한다.
- E를 놓거나 캐릭터가 얼거나 대상과 `250cm` 이상 멀어지면 `Ground` 상태와 바닥 물리를 복구한다.
- 이동 입력 없이 E를 놓으면 기존 `TryPickupSnowball()`을 호출한다.

## 수동 작업

1. 실행 중인 Unreal Editor를 종료한다.
2. `SnowRumbleEditor`를 `Development Editor`, `Win64` 구성으로 빌드하고 프로젝트를 연다.
3. `BP_SnowRumbleCharacter`의 `SnowballEquipmentComponent`에서 필요하면 `Rolling Distance` 기본 `90`, `Maximum Rolling Separation` 기본 `250`, 굴리기 이동속도 `300~150`을 조정한다.
4. `BP_SnowballItem`에서 필요하면 `Distance For Maximum Growth` 기본 `1000`, `Maximum Scale Multiplier` 기본 `3`, `Rolling Ground Clearance` 기본 `2`를 조정한다.
5. Animation Blueprint에서 기존 `Is Holding Snowball`만으로 상태를 나누는 대신 캐릭터의 `Get Snowball Carry State` 결과를 사용해 `Normal`, `Small Snowball`, `Large Snowball` 상태를 분기한다.
6. 굴리기 애니메이션은 `Get Snowball Action State`가 `Rolling Snowball`인지 확인해 분기하고, 운반 Enum의 `Normal`과 조합한다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드 수정 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료 — `git diff --check`와 `SnowRumbleEditor` 최종 링크 빌드 성공
- [x] Unreal C++/Blueprint/에셋 규칙 위반 없음
- [x] 현재 task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인

- [ ] 호스트가 가까운 바닥 눈덩이 앞에서 E를 눌렀다가 이동 없이 놓으면 기존처럼 획득하는지 확인한다.
- [ ] 클라이언트도 E를 눌렀다가 이동 없이 놓으면 정상 획득하는지 확인한다.
- [ ] 호스트가 E를 유지하며 WASD를 누르면 획득하지 않고 눈덩이가 이동 방향 앞에서 굴러가는지 확인한다.
- [ ] 클라이언트가 E를 유지하며 WASD를 눌렀을 때도 동일하게 굴러가는지 확인한다.
- [ ] E를 놓으면 눈덩이가 굴리기를 멈추고 중력·물리가 적용되는 바닥 상태로 돌아가는지 확인한다.
- [ ] 굴리는 플레이어가 얼거나 눈덩이와 너무 멀어지면 굴리기가 자동 종료되는지 확인한다.
- [ ] 다른 플레이어가 굴리는 중인 눈덩이를 E 탭으로 획득할 수 없는지 확인한다.
- [ ] 굴리기 상태와 눈덩이 위치·회전이 호스트와 클라이언트 화면에 동일하게 보이는지 확인한다.
- [ ] 굴리기 전에는 기존 `Ground` 후보에 아웃라인이 표시되고, 굴리기 중에는 해당 눈덩이에 아웃라인이 계속 유지되며 종료 후 다시 바닥 후보 판정으로 전환되는지 확인한다.
- [ ] 굴리는 눈덩이 아웃라인이 굴리는 플레이어의 로컬 화면에만 표시되고 다른 참가자의 화면에는 강제로 표시되지 않는지 확인한다.
- [ ] E+WASD 굴리기 중 눈덩이가 이동 방향 앞에서 장애물을 통과하지 않고 안정적으로 Sweep 이동하는지 확인한다.
- [ ] 첫 이동 이후에도 굴리는 플레이어 자신의 캡슐에 막히지 않고 WASD 이동을 계속 따라오는지 확인한다.
- [ ] 캐릭터 캡슐도 굴리는 눈덩이에 막히지 않아 서버 이동속도와 굴리기 방향이 계속 유지되는지 확인한다.
- [ ] 첫 성장 직후 커진 충돌 구체가 바닥에 파고들어 다음 Sweep이 멈추지 않는지 확인한다.
- [ ] 눈덩이를 약 `10m` 굴리면 초기 크기의 최대 `3배`까지 점진적으로 커지는지 확인한다.
- [ ] 성장한 눈덩이의 메시와 충돌 범위가 함께 커지는지 확인한다.
- [ ] 성장 크기가 호스트와 클라이언트 화면에 동일하게 보이는지 확인한다.
- [ ] 눈덩이가 커질수록 굴리는 캐릭터가 `300 → 150` 범위에서 느려지는지 확인한다.
- [ ] E를 놓아 굴리기를 종료하면 캐릭터 이동속도가 기존 걷기 속도로 복구되는지 확인한다.
- [ ] 성장한 눈덩이를 놓거나 다시 굴려도 기존 성장 크기가 유지되는지 확인한다.
- [ ] Animation Blueprint에서 `Get Snowball Carry State`와 `ESnowballCarryState`의 세 값을 사용할 수 있는지 확인한다.
- [ ] 빈손·획득 연출 중에는 `Normal`, 작은 눈 보유 중에는 `Small Snowball`, 최대 성장 눈 보유 중에는 `Large Snowball`이 반환되는지 확인한다.
- [ ] 굴리기 중 `Get Snowball Action State`가 `Rolling Snowball`, 종료 후 `None`을 반환하는지 확인한다.

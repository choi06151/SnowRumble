# Task 04-1 - 눈 만들기와 상호작용

## 설명

플레이어가 눈이 있는 바닥에서 작은 눈덩이를 만들고, 가까운 눈덩이를 식별해 잡거나 내려놓을 수 있게 한다.

## 구현 항목

- [x] 눈이 있는 바닥을 바라보며 행동 입력을 유지하면 제작이 진행된다.
- [x] 제작이 완료되면 작은 눈덩이가 생성된다.
- [x] 가까이 있는 상호작용 가능한 눈덩이를 식별할 수 있다.
- [x] 플레이어가 작은 눈덩이를 잡고 운반할 수 있다.
- [x] 들고 있는 눈덩이를 내려놓을 수 있다.
- [x] 눈덩이 생성과 소유 상태가 호스트와 클라이언트에 동일하게 보인다. — 현재 단위에서는 맵에 배치된 눈덩이와 획득·장착 소유 상태를 확인함

## 메인 프로그래머 책임

- 눈 바닥 판정, 제작 입력·진행·생성, 잡기·내려놓기, 소유 상태와 비UI 제작·운반 표현을 완성한다.

## SUB UI 인계

- 연계 SUB Task: `Tasks/S04-1_interaction_ui.md`
- 로컬 플레이어의 상호작용 대상, 가능한 행동과 제작 진행도를 UI가 읽거나 변경 알림을 받을 수 있게 제공한다.

## 범위 밖

- 투척과 피해
- 눈 굴리기와 크기 성장
- 최종 아웃라인·손 애니메이션

## 사전 전제

- Task 02-1의 최소 네트워크 눈덩이
- Task 01-2의 상호작용 입력

## 현재 구현 단위

- 첫 번째 단위로 맵에 놓인 작은 눈덩이를 가까이에서 식별하고 `E`로 획득·장착하는 흐름을 구현했다.
- 두 번째 단위는 눈덩이를 들지 않은 상태에서 눈 바닥을 바라보고 좌클릭을 유지해 제작한 뒤 바닥 상태의 `SnowballItem`을 서버가 생성하는 흐름이다.
- 제작 애니메이션은 복제된 `IsCreatingSnowball()`, 아이템 획득 애니메이션은 서버 획득 성공 후 기본 `0.6초` 유지되는 `IsPickingUpItem()`을 사용한다.
- 실제 소유권과 소켓 장착은 획득 성공 즉시 적용하지만 `IsHoldingSnowball()`은 `IsPickingUpItem()`이 false로 돌아온 뒤 true가 된다.
- `IsPickingUpItem()`이 true인 동안 수평 이동속도는 0이며 이동·점프·스프린트와 다른 게임플레이 행동을 차단한다.
- 좌클릭 입력은 애니메이션용 `IsHoldingSnowball()`로 기능을 미리 분기하지 않고, 서버가 실제 장비 상태를 검사해 빈손이면 제작하고 보유·조준 중이면 투척 충전한다.
- 들고 있는 눈덩이 내려놓기는 제작 단위 확인 후 같은 Task의 후속 단위로 진행한다.

## 현재 추가 구현 단위

- 작업명: 획득 가능 아이템 범위 아웃라인
- 로컬 플레이어가 획득 범위 안의 가장 적합한 상호작용 후보를 감지하면 해당 액터의 모든 Primitive Component에 아웃라인을 활성화한다.
- 기존 후보가 범위를 벗어나거나 획득 불가 상태가 되거나 다른 후보로 바뀌면 이전 아웃라인을 반드시 해제한다.
- 표현은 사용자가 보유한 Post Process 아웃라인 Material을 재사용하고 `Set Render Custom Depth`와 `Set Custom Depth Stencil Value`로 제어한다.
- 아웃라인 선택은 로컬 표현이며 복제하지 않는다. 서버는 기존과 동일하게 실제 거리·상태·소유권을 다시 검사한다.
- 기본 상호작용 Stencil 값은 `1`로 두고 컴포넌트 기본값에서 조정 가능하게 한다.
- 범용 `UOutlineComponent`는 전달받은 Actor의 모든 Primitive Component에 Custom Depth 표현을 적용하며 특정 아이템 클래스에 의존하지 않는다.
- 캐릭터는 로컬 소유 화면에서만 기존 `FindClosestPickupCandidate()` 결과를 `OutlineComponent`에 전달한다.
- 첫 연결 대상은 `ASnowballItem`의 획득 후보로 제한하며 실제 획득은 기존 서버 판정을 그대로 사용한다.

## 현재 내려놓기 구현 단위

- 로컬 플레이어의 `G` 입력은 `DropHeldSnowball()`을 통해 서버에 내려놓기를 요청한다.
- 서버는 얼기·획득 연출·실제 보유 상태를 다시 검사하고 클라이언트가 전달한 위치를 사용하지 않는다.
- 서버가 눈덩이의 손 장착 월드 위치를 유지한 채 `Held → Ground` 상태로 전환한다.
- 내려놓기 전 조준과 충전 상태를 취소하며, 눈덩이는 장착 해제 후 `BlockAllDynamic` 충돌·중력·물리를 명시적으로 복구하고 물리 바디를 깨운다.
- 내려놓은 눈덩이의 위치와 바닥 상태는 기존 Actor 이동·상태 복제를 통해 모든 참가자에게 전달된다.

## 현재 제작 위치 보정 단위

- 제작 가능 여부는 기존처럼 카메라 중앙에서 `Creation Trace Distance` 안의 `SnowSurface` 태그 지면을 바라보는지 서버가 판정한다.
- 소유 클라이언트는 `FollowCamera`의 실제 월드 위치와 Forward Vector를 서버 RPC에 전달하고, 서버는 카메라 원점이 캐릭터의 `Max Camera Origin Distance` 안인지 확인한 뒤 직접 Line Trace한다.
- `Creation Trace Distance`는 플레이어 기준 제작 가능 거리로 유지하며, 실제 카메라 Trace에는 3인칭 카메라와 플레이어 사이 거리를 더해 카메라 오프셋 때문에 지면 도달 전에 선이 끝나지 않게 한다.
- 실제 생성 위치만 캐릭터 전방 `Creation Forward Distance`의 지면을 기준으로 한다.
- 서버는 카메라로 확인한 눈 바닥 평면에 캐릭터 전방 위치를 투영해 실제 생성 좌표를 계산한다.
- 기본 전방 거리는 `100cm`이며 `BP_SnowRumbleCharacter`의 `SnowballCreationComponent`에서 조정할 수 있다.
- 제작 완료에는 시작 시 서버가 확정한 눈 바닥 액터와 평면을 사용하므로 원격 클라이언트의 서버 Pawn 시점 차이 때문에 제작이 취소되지 않는다.
- Blueprint 재인스턴싱으로 캐릭터의 제작 컴포넌트 멤버 참조가 비어 있어도, 실제 소유 중인 `SnowballCreationComponent`를 다시 찾아 제작 요청을 전달한다.
- 생성 오류 확인에 사용한 로컬·서버 Trace 선과 좌클릭 화면 진단 출력은 결과 확인 후 제거했다.
- 서버는 이미 `bIsCreating`이 true이면 반복된 제작 시작 RPC를 무시해 입력 이벤트 반복 호출이 1.5초 완료 타이머를 계속 초기화하지 못하게 한다.
- `Ground` 상태 눈덩이는 서버에서 위쪽 지면 충돌을 확인하면 복제된 `bIsSettledOnGround`를 켜고 물리 시뮬레이션과 중력을 꺼 현재 위치에 고정한다.
- 획득·투척·굴리기 전환과 내려놓기·굴리기 종료 시 지면 고정값을 초기화하므로, 다시 바닥 상태가 된 눈덩이는 낙하한 뒤 새 지면에 닿으면 고정된다.

## 작업 배정

- 담당자: 메인 프로그래머
- SUB UI 담당자: 없음
- 메인 프로그래머 선점 파일·자산: `Source/SnowRumble/Interaction/OutlineComponent.h`, `Source/SnowRumble/Interaction/OutlineComponent.cpp`, `Source/SnowRumble/Player/SnowRumbleCharacter.h`, `Source/SnowRumble/Player/SnowRumbleCharacter.cpp`, `Content/Characters/BP_SnowRumbleCharacter`, `Content/Snowball/BP_SnowballItem`, `Tasks/04-1_snow_creation_interaction.md`, `Tasks/S04-1_interaction_ui.md`, `docs/ARCHITECTURE.md`, `docs/PLANS.md`
- SUB 프로그래머 선점 UI 자산: 없음
- 공유 확인 대상: 호스트와 클라이언트의 눈 바닥 판정, 제작 진행도, 서버 생성, 바닥 눈덩이 식별, 획득·장착과 소유 상태
- 인계 조건: 실제 상호작용 대상·보유 상태 프로퍼티와 변경 이벤트를 `Tasks/S04-1_interaction_ui.md`에 기록한 뒤 SUB UI 작업에 인계한다.
- 반영 순서: C++ 눈덩이 아이템·장비 컴포넌트 구현 후 메인 프로그래머가 눈덩이 Blueprint와 손 장착 위치를 연결하고 2인 PIE를 확인한다.

## 수동 작업

1. 실행 중인 Unreal Editor를 종료한다.
2. Visual Studio에서 `SnowRumbleEditor`를 `Development Editor`, `Win64` 구성으로 빌드한 뒤 `SnowRumble.uproject`를 연다.
3. `Content/Snowball/BP_SnowballItem` Blueprint를 만들고 부모 클래스를 `SnowballItem`으로 지정한다.
4. `BP_SnowballItem`에 Static Mesh Component를 추가하고 상속받은 `CollisionComponent`에 연결한다.
5. Static Mesh Component에 엔진 기본 구체 등 구분 가능한 임시 메시를 지정하고 메시 자체의 충돌은 `NoCollision`로 설정한다.
6. 임시 메시가 상속받은 충돌 구체의 반지름 18cm에 맞도록 크기를 조정한 뒤 Blueprint를 컴파일하고 저장한다.
   - 바닥의 눈덩이는 상속받은 `CollisionComponent`가 물리와 중력을 담당하므로 Static Mesh Component의 `Simulate Physics`는 켜지 않는다.
7. 캐릭터가 사용하는 Skeleton에 `SnowballSocket` 소켓이 존재하고 손 본 아래에 배치되어 있는지 확인한다.
8. `Content/Characters/BP_SnowRumbleCharacter`를 열고 상속받은 `SnowballHoldPoint`가 Mesh의 `SnowballSocket`에 부착되어 있는지 확인한다.
9. 필요하면 `SnowballHoldPoint`의 Relative Location과 Rotation을 미세 조정한 뒤 컴파일하고 저장한다.
10. `L_Prototype`에 `BP_SnowballItem`을 여러 개 배치한다. 최소 두 개는 플레이어 시작 위치에서 접근할 수 있게 두고 서로 약간 떨어뜨린다.
11. `L_Prototype`을 저장한다.
12. `BP_SnowRumbleCharacter`의 상속받은 `SnowballCreationComponent`를 선택하고 `Snowball Item Class`를 `BP_SnowballItem`으로 지정한다.
13. `L_Prototype`에서 제작을 허용할 바닥 액터를 선택하고 Details의 `Actor > Tags`에 `SnowSurface`를 추가한다.
    - 해당 바닥은 `Visibility` 채널을 Block해야 카메라 중앙 Line Trace가 감지할 수 있다.
    - 제작 위치가 너무 가깝거나 멀면 `BP_SnowRumbleCharacter`의 `SnowballCreationComponent`에서 `Creation Forward Distance`를 조정한다.
14. 에디터를 종료한 상태에서 `SnowRumbleEditor`를 다시 빌드한 뒤 프로젝트를 연다.
15. Animation Blueprint에서 캐릭터의 `Is Creating Snowball`을 제작 애니메이션 전환 조건으로 연결한다.
16. 같은 Animation Blueprint에서 `Is Picking Up Item`을 아이템 획득 애니메이션 전환 조건으로 연결한다.
    - 획득 상태 기본 유지시간은 `BP_SnowRumbleCharacter`의 `Pickup Animation State Duration`에서 조정할 수 있으며 기본값은 `0.6초`다.
17. Project Settings의 Rendering에서 `Custom Depth-Stencil Pass`를 `Enabled with Stencil`로 설정한다.
18. 기존 아웃라인 Post Process Material이 사용하는 Post Process Volume 또는 카메라에 연결되어 있고 Stencil 값 `1`을 표시하는지 확인한다.
    - 다른 Stencil 값이 필요하면 `BP_SnowRumbleCharacter`의 상속받은 `OutlineComponent`에서 `Custom Depth Stencil Value`를 조정한다.
19. `BP_SnowRumbleCharacter`의 `Drop Equipment Action`이 `IA_DropEquipment`으로 설정되어 있는지 확인한다.
20. `IMC_Player`에서 `IA_DropEquipment`이 `G` 키에 연결되어 있는지 확인한다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드 수정 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료 — 플레이어 전방 지면 제작 위치 보정 코드를 포함해 `git diff --check` 통과, 현재 수동 실행 확인 모드에 따라 컴파일·PIE는 사용자 확인 필요
- [x] Unreal C++/Blueprint/에셋 규칙 위반 없음
- [x] 현재 task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인

- [x] 2인 PIE를 `Play As Listen Server`로 실행했을 때 바닥의 `BP_SnowballItem`이 호스트와 클라이언트 화면에 같은 위치로 보이는지 확인한다.
- [x] 바닥의 `BP_SnowballItem`이 중력과 물리 충돌에 따라 떨어지고 밀리는지 확인한다.
- [x] 호스트가 눈덩이의 180cm 안에서 `E`를 눌렀을 때 가장 가까운 눈덩이 하나가 `SnowballHoldPoint`에 장착되는지 확인한다.
- [x] 호스트가 획득한 눈덩이의 이동과 장착 상태가 클라이언트 화면에도 동일하게 보이는지 확인한다.
- [x] 클라이언트가 눈덩이의 180cm 안에서 `E`를 눌렀을 때 가장 가까운 눈덩이 하나가 해당 클라이언트의 `SnowballHoldPoint`에 장착되는지 확인한다.
- [x] 클라이언트가 획득한 눈덩이의 이동과 장착 상태가 호스트 화면에도 동일하게 보이는지 확인한다.
- [x] 눈덩이가 장착될 때 위치와 회전은 `SnowballHoldPoint`에 맞고 장착 전 월드 스케일은 유지되는지 확인한다.
- [x] 이미 한 플레이어가 획득한 눈덩이를 다른 플레이어가 `E`로 빼앗을 수 없는지 확인한다.
- [x] 눈덩이를 든 플레이어가 다른 바닥 눈덩이 근처에서 `E`를 눌러도 두 번째 눈덩이를 획득하지 않는지 확인한다.
- [x] 획득 범위 밖에서 `E`를 눌렀을 때 아무 눈덩이도 획득하지 않는지 확인한다.
- [x] 얼어있는 플레이어가 바닥 눈덩이 근처에서 `E`를 눌러도 획득하지 않는지 확인한다.
- [x] `Is Holding Snowball`, `Has Held Snowball`, `Get Held Snowball`, `Find Closest Pickup Candidate`를 Blueprint에서 호출할 수 있는지 확인한다.
- [x] 좌클릭만 눌렀을 때 기존 프로토타입 눈덩이가 새로 생성·투척되지 않는지 확인한다.
- [x] 빈손으로 `SnowSurface` 바닥을 화면 중앙에 두고 좌클릭을 `1.5초` 유지하면 카메라 시선 위치가 아닌 캐릭터 코앞 지면에 `BP_SnowballItem` 하나가 생성되는지 확인한다.
- [x] 좌클릭을 `1.5초` 전에 놓으면 눈덩이가 생성되지 않는지 확인한다.
- [x] 카메라가 `SnowSurface` 태그가 없는 바닥이나 제작 거리 `600cm` 밖을 바라보면 생성되지 않는지 확인한다.
- [x] 눈덩이를 들고 있거나 얼어 있거나 조준 중일 때 제작되지 않는지 확인한다.
- [x] 호스트가 코앞 지면에 제작한 눈덩이가 클라이언트에, 클라이언트가 코앞 지면에 제작한 눈덩이가 호스트에 같은 위치로 보이는지 확인한다.
- [ ] 제작하거나 내려놓은 눈덩이가 지면에 닿으면 움직임이 멈추고 물리 시뮬레이션과 중력이 꺼져 그 위치에 고정되는지 확인한다.
- [ ] 고정된 눈덩이를 획득하거나 굴린 뒤 다시 바닥 상태로 만들면 낙하 후 새 지면에서 다시 고정되는지 확인한다.
- [x] `Is Creating Snowball`, `Get Snowball Creation Progress`와 `On Creating Changed`를 Blueprint에서 사용할 수 있는지 확인한다.
- [x] 제작 중 `Is Creating Snowball`이 true이고 제작 완료·취소 시 false로 돌아오는지 확인한다.
- [x] 실제 눈덩이 획득 성공 시에만 `Is Picking Up Item`이 약 `0.6초` 동안 true인지 확인한다.
- [x] 획득 중에는 `Is Holding Snowball`이 false이고 획득 시간이 끝난 직후 true로 전환되는지 확인한다.
- [x] 획득 중에는 우클릭을 눌러도 조준 상태로 전환되지 않는지 확인한다.
- [x] 획득 중 기존 수평 이동이 즉시 멈추고 WASD·점프·스프린트가 동작하지 않는지 확인한다.
- [x] 획득 시간이 끝나면 걷기 속도가 정상으로 복구되는지 확인한다.
- [x] 다른 참가자 화면의 Animation Blueprint에서도 제작·획득 상태를 동일하게 읽을 수 있는지 확인한다.
- [x] 빈손인 로컬 플레이어가 획득 가능한 눈덩이의 180cm 안으로 접근하면 해당 눈덩이에만 아웃라인이 표시되는지 확인한다.
- [x] 서로 떨어진 두 눈덩이 사이를 이동하면 가장 가까운 후보로 아웃라인이 전환되고 이전 후보에서는 해제되는지 확인한다.
- [x] 범위를 벗어나거나 눈덩이를 획득하거나 얼기 상태가 되면 기존 아웃라인이 즉시 해제되는지 확인한다.
- [x] 호스트 화면의 로컬 후보와 클라이언트 화면의 로컬 후보가 서로 간섭하지 않고 각 화면에만 표시되는지 확인한다.
- [x] 눈덩이에 Static Mesh와 Collision처럼 여러 Primitive Component가 있으면 모든 시각 Primitive에 아웃라인 설정이 적용되는지 확인한다.
- [x] 호스트가 눈덩이를 든 상태에서 `G`를 누르면 눈덩이가 손의 현재 위치에서 풀리고 바닥에 충돌하는지 확인한다.
- [x] 클라이언트가 눈덩이를 든 상태에서 `G`를 눌렀을 때도 동일하게 내려놓아지는지 확인한다.
- [x] 내려놓은 위치와 바닥 상태가 호스트와 클라이언트 화면에 동일하게 보이는지 확인한다.
- [x] 내려놓은 눈덩이를 호스트와 클라이언트가 다시 획득할 수 있는지 확인한다.
- [x] 빈손이거나 얼기·획득 연출 중 `G`를 눌러도 상태가 바뀌지 않는지 확인한다.
- [x] 조준 또는 충전 중 `G`를 누르면 조준·충전이 취소되고 눈덩이가 투척되지 않은 채 내려놓아지는지 확인한다.
- [x] 내려놓기 후 `Has Held Snowball`과 `Is Holding Snowball`이 false이고 `On Held Snowball Changed`가 `None`을 전달하는지 확인한다.

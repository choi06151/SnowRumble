# Task 01-2 - 플레이어 이동, 카메라, 2인 PIE

## 설명

호스트 1명과 클라이언트 1명의 PIE 환경에서 플레이어가 3인칭 시점으로 이동하고 카메라를 조작하며 이후 눈싸움 행동에 사용할 기본 입력을 받을 수 있게 한다.

## 구현 항목

- [x] 플레이어가 `WASD`로 이동할 수 있다.
- [x] 마우스로 3인칭 카메라를 조작할 수 있다.
- [x] 점프 입력이 동작한다.
- [x] `Left Shift`를 누르는 동안 서버 기준 스프린트 속도로 이동할 수 있다.
- [x] Animation Blueprint에서 이동, 공중, 스프린트와 얼기 상태를 조회할 수 있다.
- [x] 상호작용, 조준, 행동, 장비 내려놓기, 이모션 입력을 받을 기반을 준비한다.
- [x] Listen Server 호스트 1명과 클라이언트 1명의 PIE 테스트 환경을 준비한다.
- [x] 호스트와 클라이언트에서 플레이어 이동이 서로 보인다.

## 메인 프로그래머 책임

- 이동·점프의 네트워크 기본 동작, 입력 액션, 캐릭터 Blueprint, 3인칭 카메라와 2인 PIE 설정을 완성한다.
- 이 단계에는 SUB UI 작업이 없다.

## 작업 배정

- 담당자: 메인 프로그래머
- SUB UI 담당자: 없음
- 메인 프로그래머 선점 파일·자산: `Source/SnowRumble/Player/SnowRumbleCharacter.h`, `Source/SnowRumble/Player/SnowRumbleCharacter.cpp`, `Source/SnowRumble/Game/SnowRumbleGameMode.h`, `Source/SnowRumble/Game/SnowRumbleGameMode.cpp`, `Content/Characters/BP_SnowRumbleCharacter`, `Content/Input/IMC_Player`, `Content/Input/IA_Move`, `Content/Input/IA_Look`, `Content/Input/IA_Jump`, `Content/Input/IA_Interact`, `Content/Input/IA_Aim`, `Content/Input/IA_Action`, `Content/Input/IA_DropEquipment`, `Content/Input/IA_Emote`, `Content/Game/BP_SnowRumbleGameMode`, `Content/Maps/L_Prototype`, `Tasks/01-2_player_movement_camera_input.md`, `docs/ARCHITECTURE.md`, `docs/PENDING_ISSUES.md`, `docs/PLANS.md`
- SUB 프로그래머 선점 UI 자산: 없음
- 공유 확인 대상: 호스트와 클라이언트의 이동·점프 복제 결과 및 각 로컬 플레이어의 카메라·입력
- 인계 조건: 없음
- 반영 순서: C++ 캐릭터·게임 모드 기본틀 구현 후 메인 프로그래머가 Blueprint·입력 자산과 맵을 연결하고 2인 PIE를 확인한다.

## 범위 밖

- 각 행동 입력에 연결될 실제 눈덩이·아이템 기능
- 캐릭터 커스터마이징
- 최종 캐릭터 모델과 애니메이션

## 사전 전제

- Task 01-1의 프로젝트 기반

## 수동 작업

1. 실행 중인 Unreal Editor를 종료한다.
2. Visual Studio에서 `SnowRumbleEditor`를 `Development Editor`, `Win64` 구성으로 빌드한 뒤 `SnowRumble.uproject`를 연다.
3. Content Browser에 `Characters`, `Game`, `Input` 폴더를 만든다.
4. `Content/Input`에 아래 Enhanced Input 액션을 만든다.
   - `IA_Move`: Value Type을 `Axis2D (Vector2D)`로 설정한다.
   - `IA_Look`: Value Type을 `Axis2D (Vector2D)`로 설정한다.
   - `IA_Jump`, `IA_Interact`, `IA_Aim`, `IA_Action`, `IA_DropEquipment`, `IA_Emote`: Value Type을 `Digital (bool)`로 설정한다.
   - `IA_Sprint`: Value Type을 `Digital (bool)`로 설정한다.
5. `Content/Input/IMC_Player` 입력 매핑 컨텍스트를 만들고 아래 키를 연결한다.
   - `IA_Move`: `W`, `A`, `S`, `D`
   - `W`: `Swizzle Input Axis Values`를 `YXZ`로 설정한다.
   - `S`: `Negate`와 `Swizzle Input Axis Values(YXZ)`를 설정한다.
   - `A`: `Negate`를 설정한다.
   - `IA_Look`: `Mouse XY 2D-Axis`
   - `IA_Jump`: `Space Bar`
   - `IA_Sprint`: `Left Shift`
   - `IA_Interact`: `E`
   - `IA_Aim`: `Right Mouse Button`
   - `IA_Action`: `Left Mouse Button`
   - `IA_DropEquipment`: `G`
   - `IA_Emote`: `Tab`
6. `Content/Characters/BP_SnowRumbleCharacter` Blueprint를 만들고 부모 클래스를 `SnowRumbleCharacter`로 지정한다.
7. `BP_SnowRumbleCharacter`의 `SnowRumble|Input` 프로퍼티에 `IMC_Player`와 각 `IA_` 자산을 같은 이름의 슬롯에 연결한다.
   - `Sprint Action`에는 `IA_Sprint`를 연결한다.
   - `SnowRumble|Movement`의 `Walk Speed`는 `500`, `Sprint Speed`는 `750`으로 둔다.
8. 임시 플레이어가 다른 화면에서도 보이도록 `BP_SnowRumbleCharacter`에 Static Mesh Component를 추가하고 엔진 기본 도형 등 구분 가능한 임시 메시를 지정한다. 이 컴포넌트의 충돌은 `NoCollision`으로 둔다.
9. `Content/Game/BP_SnowRumbleGameMode` Blueprint를 만들고 부모 클래스를 `SnowRumbleGameMode`로 지정한다.
10. `BP_SnowRumbleGameMode`의 `Default Pawn Class`를 `BP_SnowRumbleCharacter`로 설정한다.
11. `L_Prototype`을 열고 World Settings의 `GameMode Override`를 `BP_SnowRumbleGameMode`로 설정한 뒤 저장한다.
12. Play 옆 메뉴의 Multiplayer Options에서 플레이어 수를 `2`, Net Mode를 `Play As Listen Server`로 설정한다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드 수정 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료 — 스프린트 RPC·복제와 Blueprint 함수 UHT·C++ 컴파일 성공, 실행 중인 Editor의 DLL 점유로 최종 링크는 수동 빌드 필요
- [x] Unreal C++/Blueprint/에셋 규칙 위반 없음
- [x] 현재 task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인

- [x] Unreal Editor에서 `BP_SnowRumbleCharacter`를 열었을 때 `On Interact Input`, `On Aim Input`, `On Action Input`, `On Drop Equipment Input`, `On Emote Input` 이벤트를 추가할 수 있는지 확인한다.
- [x] 2인 PIE를 실행했을 때 호스트와 클라이언트가 각각 한 캐릭터를 소유하는지 확인한다.
- [x] 호스트에서 `WASD`, 마우스와 `Space` 입력으로 이동, 카메라 조작과 점프가 동작하는지 확인한다.
- [x] 클라이언트에서 `WASD`, 마우스와 `Space` 입력으로 이동, 카메라 조작과 점프가 동작하는지 확인한다.
- [x] 호스트 캐릭터의 이동과 점프가 클라이언트 화면에 보이는지 확인한다.
- [x] 클라이언트 캐릭터의 이동과 점프가 호스트 화면에 보이는지 확인한다.
- [x] 한 플레이어의 마우스 카메라 조작이 다른 플레이어의 카메라를 움직이지 않는지 확인한다.
- [x] 호스트와 클라이언트 각각 `Left Shift`를 누른 채 이동했을 때 걷기보다 빠르게 이동하는지 확인한다.
- [x] 호스트의 스프린트 시작과 종료가 클라이언트 화면의 `Is Sprinting` 결과에 동일하게 반영되는지 확인한다.
- [x] 클라이언트의 스프린트 시작과 종료가 호스트 화면의 `Is Sprinting` 결과에 동일하게 반영되는지 확인한다.
- [x] `Is Moving`, `Is In Air`, `Is Sprinting`, `Is Frozen` Blueprint Pure 함수를 `BP_SnowRumbleCharacter` 또는 Animation Blueprint에서 호출할 수 있는지 확인한다.
- [x] 정지 중에는 Shift를 누르고 있어도 `Is Sprinting`이 false이고, 이동을 시작하면 true가 되는지 확인한다.
- [x] 얼어있는 플레이어는 스프린트를 시작할 수 없고 `Is Sprinting`이 false인지 확인한다.

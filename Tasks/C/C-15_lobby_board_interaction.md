# Task C-15 - 로비 게시판 상호작용

## 설명

로비 게시판 Blueprint가 기존 눈덩이처럼 가까이 가면 outline이 표시되고, `E` 입력으로 서버 검증된 상호작용 이벤트를 받을 수 있게 한다.

## 상태 전이 기준

- 시작 가능: 기본 캐릭터 E 상호작용과 outline 컴포넌트 재사용 가능
- 완료 가능: 게시판 C++ 부모, 캐릭터 후보 탐색, 서버 상호작용 요청, Blueprint 인계와 수동 확인 절차 기록 완료

## 구현 항목

- [x] 게시판 Blueprint가 상속할 C++ 부모 액터를 추가한다.
- [x] 로컬 플레이어가 게시판 근처에 붙으면 기존 outline 경로로 표시되게 한다.
- [x] `E` 입력 시 서버가 거리와 캐릭터 상태를 검증해 게시판 상호작용을 확정한다.
- [x] 게시판 Blueprint가 상호작용 결과를 연결할 이벤트를 제공한다.
- [x] outline으로 잡힌 게시판이 있을 때만 `E` 게시판 상호작용을 요청한다.
- [x] 게시판 상호작용 성공 시 소유 클라이언트 카메라가 게시판을 바라보게 한다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: C++·문서: 최재원(C), 게시판 Blueprint·맵 배치: 사용자 또는 S 인계
- 생성·변경 후보: `Source/SnowRumble/Interaction/LobbyInteractionBoard_C.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Tasks/C/C-15_lobby_board_interaction.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`
- 공유 확인 대상: S-03 또는 사용자
- 병합 순서: C-03 대기방 상호작용 표현 전에 인계

## 공용 계약과 인계

- 제공받을 계약: 기존 `UOutlineComponent`, `ASnowRumbleCharacter`의 `InteractAction`
- 제공할 계약:
  - `ALobbyInteractionBoard`: 게시판 Blueprint가 상속할 C++ 부모 액터다.
  - `ALobbyInteractionBoard::BoardMeshComponent`: 게시판 모델과 Custom Depth outline이 적용될 기본 Static Mesh 컴포넌트다.
  - `ALobbyInteractionBoard::InteractionRadius`: 로컬 outline 후보 탐색과 서버 상호작용 검증에 공통으로 쓰는 거리다. 기본값은 320cm다.
  - `ALobbyInteractionBoard::CanInteractWith(const ASnowRumbleCharacter* Character)`: 캐릭터와 게시판 mesh bounds의 가장 가까운 점 사이 거리로 조건을 확인한다.
  - `ALobbyInteractionBoard::GetFocusLocation()`: 카메라가 바라볼 게시판 mesh bounds 중심을 반환한다.
  - `ALobbyInteractionBoard::OnBoardInteracted(ASnowRumbleCharacter* Character)`: 서버가 상호작용을 확정했을 때 Blueprint에서 UI나 연출을 연결할 이벤트다.
  - `ASnowRumbleCharacter::FindClosestLobbyBoardCandidate()`: 로컬 outline과 `E` 입력 대상 게시판을 찾는다.
  - `ASnowRumbleCharacter::ServerTryInteractWithLobbyBoard(ALobbyInteractionBoard* Board)`: 서버가 캐릭터 상태와 게시판 거리를 다시 검사해 상호작용을 확정한다.
  - `ASnowRumbleCharacter::ClientFocusLobbyBoard(ALobbyInteractionBoard* Board)`: 서버가 상호작용을 확정한 뒤 소유 클라이언트 카메라를 게시판으로 돌린다.
- 인계 대상: S-03 또는 사용자

## 범위 밖

- 게시판을 통한 직접 팀 선택과 팀 색 변경
- 게시판 UI 레이아웃, 모델, 머티리얼과 맵 배치
- 상호작용 안내 UI

## 사전 전제

- 기존 캐릭터 `InteractAction`과 `UOutlineComponent`

## 결정 필요

- 없음

## 변경 기록

- 2026-08-08: 사용자가 기존 눈덩이처럼 동작하는 게시판 outline과 `E` 상호작용을 요청해 C-15를 추가하고 구현했다.
- 2026-08-08: 게시판 크기 때문에 중심점 거리 기준으로 outline 후보가 잘 잡히지 않는 문제에 대응해 mesh bounds 기준 거리 검사로 바꾸고 기본 상호작용 반경을 320cm로 늘렸다.
- 2026-08-08: 눈덩이 상호작용 거리는 기존 `USnowballEquipmentComponent::PickupRadius` 180cm를 유지하고, 게시판은 `ALobbyInteractionBoard::InteractionRadius`에서 개별 조절하게 정리했다. `E` 입력은 현재 outline으로 잡힌 게시판에만 적용하며, 성공 시 카메라를 게시판으로 돌린다.

## 수동 작업

- Unreal Editor에서 `ALobbyInteractionBoard`를 부모로 하는 `BP_LobbyInteractionBoard_C` Blueprint를 만든다.
- `BP_LobbyInteractionBoard_C`의 `BoardMeshComponent`에 게시판 Static Mesh와 머티리얼을 연결한다.
- `BP_LobbyInteractionBoard_C`를 로비 맵에 배치한다.
- 상호작용 확인을 위해 `OnBoardInteracted` 이벤트에 임시 Print String 또는 열릴 UI 호출을 연결한다.
- 직접 팀 선택이나 팀 색 변경 UI는 현재 MVP 범위 밖이므로 아직 연결하지 않는다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드 변경 완료
- [x] `git diff --check` 공백 점검 통과
- [x] `SnowRumbleEditor Win64 Development` 빌드 성공
- [x] 게시판 bounds 거리 판정 변경 후 최종 링크 확인
- [x] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [x] 공용 계약과 캡슐화 규칙 위반 없음
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨

### 검증 메모

- 2026-08-08: 게시판 bounds 거리 판정 변경 후 `LobbyInteractionBoard_C.cpp`, `Module.SnowRumble.cpp`, `SnowRumbleCharacter.cpp` 컴파일은 통과했지만, 실행 중인 Unreal Editor가 `Binaries/Win64/UnrealEditor-SnowRumble.dll`을 잡고 있어 링크가 `LNK1104`로 실패했다. 에디터 종료 후 재빌드 확인이 필요하다.
- 2026-08-08: outline 대상 기준 `E` 상호작용과 `ClientFocusLobbyBoard` 카메라 focus RPC 추가 후 `LobbyInteractionBoard_C.cpp`, `Module.SnowRumble.cpp`, `OverheadNameplateWidget_C.cpp`, `SnowRumbleCharacter.cpp`, `SnowRumbleLobbyGameMode.cpp` 컴파일은 통과했다. 링크는 실행 중인 Unreal Editor의 DLL 잠금으로 다시 `LNK1104` 실패했다.
- 2026-08-08: Unreal Editor 종료 상태에서 `SnowRumbleEditor Win64 Development` 최종 빌드가 `Result: Succeeded`로 완료됐다.

### 결과 확인

- [ ] 로비 맵에서 플레이어가 게시판 가까이 가면 게시판 mesh에 outline이 표시된다.
- [ ] 게시판에서 멀어지면 outline이 사라진다.
- [ ] 게시판 가까이에서 `E`를 눌렀다 떼면 서버가 `OnBoardInteracted`를 실행한다.
- [ ] 게시판 거리 밖에서 `E`를 눌러도 `OnBoardInteracted`가 실행되지 않는다.
- [ ] 게시판 outline이 표시된 상태에서만 `E` 입력으로 게시판 상호작용이 실행된다.
- [ ] 게시판 상호작용 성공 후 로컬 카메라가 게시판을 바라본다.
- [ ] 근처에 게시판이 없으면 기존 눈덩이 획득 상호작용이 기존처럼 동작한다.

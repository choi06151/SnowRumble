# SUB UI Task S04-1 - 상호작용과 눈 제작 UI

## 설명

로컬 플레이어가 바라보는 상호작용 대상, 가능한 행동과 눈 제작 진행도를 이해할 수 있는 최소 UI를 구성한다.

## 구현 항목

- [ ] 상호작용 가능한 대상이 있을 때 안내를 표시한다.
- [ ] 현재 가능한 행동을 구분해 표시한다.
- [ ] 눈 제작 중 진행도를 표시한다.
- [ ] 대상이 사라지거나 행동할 수 없게 되면 관련 UI를 숨긴다.
- [ ] UI가 대상 판정, 제작 진행도와 소유 상태를 직접 변경하지 않는다.

## 범위 밖

- 상호작용 탐색, 제작과 소유권 규칙 구현
- 눈덩이 아웃라인과 손 애니메이션
- C++·Config·맵·비UI Blueprint 수정

## 사전 전제

- Main Task 04-1의 로컬 상호작용 대상, 행동 종류와 제작 진행도 데이터

## 작업 배정

- 담당자: SUB 프로그래머
- 선점 UI 자산: Main Task 인계 후 확정
- 공유 확인 대상: 대상·행동·제작 상태에 따른 표시 전환
- 반영 순서: Main Task 04-1 완료 후 시작

## Main Task 04-1 획득·장착 단위 인계

- 로컬 플레이어의 `BP_SnowRumbleCharacter`에서 `SnowballEquipmentComponent`를 읽는다.
- `Find Closest Pickup Candidate()`는 획득 범위 180cm 안의 가장 가까운 `Ground` 눈덩이를 반환하며 없으면 `None`이다.
- 후보 눈덩이의 `Can Be Picked Up()`으로 현재 획득 가능 상태를 읽는다.
- `Has Held Snowball()`과 캐릭터의 `Is Holding Snowball()`으로 현재 보유 여부를 읽는다.
- `Get Held Snowball()`로 현재 장착된 `SnowballItem`을 읽는다.
- `On Held Snowball Changed(HeldSnowball)` 이벤트로 획득 결과에 따른 UI 갱신을 받는다.
- 로컬 캐릭터의 `Is Creating Snowball()`과 `Get Snowball Creation Progress()`로 제작 여부와 `0~1` 진행도를 읽는다.
- `SnowballCreationComponent`의 `On Creating Changed(bool)`로 제작 시작·취소·완료에 따른 UI 갱신을 받는다.
- 캐릭터의 `Is Picking Up Item()`은 서버가 실제 아이템 획득을 확정한 뒤 기본 `0.6초` 동안 true이며 Animation Blueprint 표현에 사용한다.
- `Is Holding Snowball()`은 획득 애니메이션 상태가 끝난 다음 true가 되므로 AnimBP는 `Is Picking Up Item → Is Holding Snowball` 순서로 전환한다.
- `Is Picking Up Item()`이 true인 동안 캐릭터 이동은 서버 복제 상태에 따라 잠기며 UI나 AnimBP에서 이동속도를 별도로 변경하지 않는다.
- UI는 `Try Pickup Snowball`을 직접 호출하지 않고 캐릭터의 기존 `E` 입력과 서버 판정 결과만 표시한다.
- UI는 제작 시작·취소 RPC나 생성할 눈덩이 클래스를 변경하지 않는다.
- `OutlineComponent`는 메인 프로그래머가 담당하는 로컬 비UI 표현이므로 SUB UI에서 설정하거나 변경하지 않는다.
- 로컬 플레이어가 `G`로 눈덩이를 내려놓으면 서버 확정 후 `On Held Snowball Changed(None)`이 발생한다.
- 내려놓기 완료 후 `Has Held Snowball()`과 캐릭터의 `Is Holding Snowball()`은 false를 반환한다.
- UI는 `Drop Held Snowball()`을 직접 호출하지 않고 캐릭터의 기존 `G` 입력과 서버 확정 결과만 표시한다.
- 눈 제작 진행도 인계는 위의 기존 `Is Creating Snowball()`, `Get Snowball Creation Progress()`, `On Creating Changed(bool)`를 사용한다.

## 수동 작업 (구현 후 구체화)

## 완료 조건

### 에이전트 확인

- [ ] 관련 UI 자산 수정 완료
- [ ] Blueprint/에셋 규칙 위반 없음
- [ ] C++·Config·맵·비UI Blueprint를 수정하지 않음
- [ ] 현재 Task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인 (구현 후 구체화)

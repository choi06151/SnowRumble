# SUB UI Task S05-3 - 게임플레이 HUD

## 설명

앞선 UI 연결을 하나의 로컬 HUD로 통합하고 현재 HP, 얼기 상태와 핵심 행동 정보를 플레이 중 확인할 수 있게 한다.

## 구현 항목

- [ ] 로컬 플레이어의 현재 HP를 표시한다.
- [ ] 얼기와 관전 상태를 구분해 표시한다.
- [ ] 상호작용, 제작과 투척 충전 UI를 HUD에서 함께 사용할 수 있다.
- [ ] 보유·장착 상태 UI를 HUD의 합의된 위치에 표시한다.
- [ ] 다른 플레이어의 소유 데이터가 로컬 HUD에 섞이지 않는다.
- [ ] HUD는 서버 판정이나 복제 상태를 변경하지 않는다.

## 범위 밖

- 최종 UI 아트와 전체 옵션 화면
- 이모션 선택 UI
- C++·Config·맵·비UI Blueprint 수정

## 사전 전제

- Main Task 02-2와 05-4의 HP·얼기·관전 및 로컬 UI 데이터
- SUB Task S04-1, S04-2와 S05-2의 개별 UI

## 작업 배정

- 담당자: SUB 프로그래머
- 선점 UI 자산: Main Task 인계 후 확정
- 공유 확인 대상: 호스트와 클라이언트 각각의 로컬 HUD
- 반영 순서: 관련 Main·SUB Task 완료 후 통합

## Main Task 02-2 인계

- 로컬 플레이어의 `BP_SnowRumbleCharacter`에서 `HealthComponent`를 읽는다.
- 현재 값은 `Get Current Health`, `Get Max Health`, `Is Frozen` Blueprint Pure 함수로 읽는다.
- HP 변경은 `HealthComponent`의 `On Health Changed(CurrentHealth, MaxHealth)` 이벤트로 받는다.
- 얼기 변경은 `HealthComponent`의 `On Frozen Changed(bIsFrozen)` 이벤트로 받는다.
- 두 이벤트는 서버에서 상태를 확정한 뒤 복제 결과에 따라 각 화면에서 호출된다.
- HUD는 로컬 소유 캐릭터의 `HealthComponent`에만 연결하고 HP나 얼기 상태를 직접 변경하지 않는다.

## 수동 작업 (구현 후 구체화)

## 완료 조건

### 에이전트 확인

- [ ] 관련 UI 자산 수정 완료
- [ ] Blueprint/에셋 규칙 위반 없음
- [ ] C++·Config·맵·비UI Blueprint를 수정하지 않음
- [ ] 현재 Task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인 (구현 후 구체화)

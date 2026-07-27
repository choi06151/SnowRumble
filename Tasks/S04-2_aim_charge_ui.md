# SUB UI Task S04-2 - 조준과 충전 UI

## 설명

로컬 플레이어가 작은 눈과 큰 눈을 조준할 때 화면 중앙 방향과 현재 투척 충전 진행도를 알 수 있는 UI를 구성한다.

## 구현 항목

- [ ] 조준 중에만 화면 중앙 조준 표시를 보여준다.
- [ ] 투척 충전 중 정규화된 진행도를 표시한다.
- [ ] 작은 눈과 큰 눈의 충전 상태를 구분할 수 있다.
- [ ] 조준 취소나 투척 완료 시 표시를 초기화한다.
- [ ] UI에서 충전량, 투척 힘과 서버 판정값을 변경하지 않는다.

## 범위 밖

- 조준 카메라, 충전 계산과 투척 구현
- 투척 애니메이션·사운드·이펙트
- C++·Config·맵·비UI Blueprint 수정

## 사전 전제

- Main Task 04-2의 조준 여부와 작은 눈 충전 진행도
- 큰 눈 구분은 Main Task 04-3 완료 후 연결

## Main Task 04-3 성장 단위 인계

- `SnowballItem`의 `Get Growth Progress()`는 서버가 실제 굴린 거리로 확정한 `0~1` 성장률을 반환한다.
- `SnowballEquipmentComponent`의 `Is Rolling Snowball()`로 로컬 플레이어의 굴리기 상태를 읽는다.
- 캐릭터의 `Get Snowball Carry State()`는 `Normal`, `Small Snowball`, `Large Snowball` Enum으로 현재 보유 크기를 구분한다.
- 캐릭터의 `Get Snowball Action State()`는 운반 상태와 별개로 `None`, `Rolling Snowball` 행동을 구분한다.
- UI는 성장률이나 Actor Scale을 직접 변경하지 않고 복제된 값만 표시한다.

## Main Task 연결 지점

- `ASnowRumbleCharacter::IsAiming()`은 로컬·원격 캐릭터의 복제된 실제 조준 상태를 반환한다.
- `ASnowRumbleCharacter::IsChargingSnowball()`과 `GetSnowballChargeProgress()`로 충전 표시 여부와 `0~1` 진행도를 읽는다.
- `USnowballEquipmentComponent::IsAiming()`과 `OnAimingChanged(bool)`로 조준 UI 표시 여부를 조회하거나 변경 알림을 받을 수 있다.
- `USnowballEquipmentComponent::IsCharging()`, `GetChargeProgress()`, `OnChargingChanged(bool)`도 같은 충전 데이터를 제공한다.
- UI는 `SetAiming`을 호출하거나 이동속도, FOV, 투척 가능 여부를 변경하지 않는다.
- 현재 C++ 화면 문자열은 실제 게이지 UI 확인 후 제거할 임시 표시다.

## 작업 배정

- 담당자: SUB 프로그래머
- 선점 UI 자산: Main Task 인계 후 확정
- 공유 확인 대상: 조준·충전·취소·투척 상태의 UI 전환
- 반영 순서: Main Task 04-2 완료 후 작은 눈 UI, Main Task 04-3 완료 후 큰 눈 UI 연결

## 수동 작업 (구현 후 구체화)

## 완료 조건

### 에이전트 확인

- [ ] 관련 UI 자산 수정 완료
- [ ] Blueprint/에셋 규칙 위반 없음
- [ ] C++·Config·맵·비UI Blueprint를 수정하지 않음
- [ ] 현재 Task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인 (구현 후 구체화)

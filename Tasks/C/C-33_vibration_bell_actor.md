# Task C-33 - 진동벨 피격 연출

## 설명

상호작용 없이 눈덩이에 맞으면 위치 기반 효과음을 재생하고 Static Mesh가 좌우로 흔들리는 배치용 진동벨 액터를 제공한다.

## 상태 전이 기준

- 시작 가능: C-07 피해·이벤트 계약, C-09 눈덩이 투척 충돌
- 완료 가능: C++ 부모 액터 구현, Blueprint 연결, 호스트·클라이언트 피격 연출 확인

## 구현 항목

- [x] 눈덩이 피격만 서버에서 확정한다.
- [x] 피격 이벤트를 multicast해 모든 참가자 화면에서 사운드와 흔들림을 재생한다.
- [x] Static Mesh, Sound, Attenuation과 흔들림 값을 Blueprint에서 지정할 수 있게 한다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: 서유정(S) 협의
- 생성·변경 후보: `VibrationBell_C.h`, `VibrationBell_C.cpp`, `BP_VibrationBell_C`
- 공유 확인 대상: S·J
- 병합 순서: C-09 이후, 맵 배치 전

## 공용 계약과 인계

- 제공받을 계약: `ASnowballItem`의 서버 확정 피격 호출
- 제공할 계약: `AVibrationBell`, Blueprint의 `HitSound`, `HitSoundAttenuation`, 흔들림 조정값
- 인계 대상: S의 `BP_VibrationBell_C` 생성·에셋 연결, J의 맵 배치

## 범위 밖

- 플레이어 상호작용, 내구도, 점수와 게임 결과 변경
- 진동벨 Static Mesh와 최종 사운드 에셋 제작

## 사전 전제

- 배치용 Blueprint와 표현 에셋은 Unreal Editor에서 연결한다.

## 결정 필요

- 없음

## 변경 기록

- 2026-08-27: 눈덩이 피격 시 위치 기반 사운드와 좌우 흔들림을 재생하는 `AVibrationBell` 액터를 추가했다.

## 수동 작업 (구현 후 구체화)

- Unreal Editor에서 `BP_VibrationBell_C`를 만들고 `VibrationBell`을 부모 클래스로 지정한다.
- Static Mesh와 `HitSound`를 연결하고 필요하면 `HitSoundAttenuation`, `ShakeRotationAmplitudeDegrees`, `ShakeFrequency`, `ShakeDurationSeconds`를 조정한다.
- J가 해당 Blueprint를 맵에 배치하고 눈덩이 충돌이 가능한 Collision 설정을 확인한다.

## 완료 조건

### 에이전트 확인

- [x] 관련 C++ 코드와 Task 문서 변경 완료
- [ ] Blueprint와 표현 자산 연결 완료
- [x] `git diff --check` 기준 정적 점검 완료

### 결과 확인 (구현 후 구체화)

- [ ] 호스트가 눈덩이로 진동벨을 맞히면 벨이 좌우로 흔들리고 위치 기반 효과음이 난다.
- [ ] 클라이언트도 같은 피격 시점에 벨 흔들림과 효과음을 본다.
- [ ] 진동벨은 별도 상호작용 입력 없이 눈덩이 피격에만 반응한다.

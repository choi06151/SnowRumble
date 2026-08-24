# Task C-26 - 눈 밟힘 효과 계약

## 설명

캐릭터 걷기·달리기 애니메이션에서 발이 눈 표면에 닿을 때 Blueprint가 눈 튐, 발자국 Decal, 발소리를 연결할 수 있는 C++ 표현 계약을 제공한다.

## 상태 전이 기준

- 시작 가능: C-01 기존 캐릭터 기반이 유지되고, 현재 캐릭터 C++ 파일을 C가 수정할 수 있음.
- 완료 가능: C++ 호출 함수와 Blueprint 이벤트가 구현되고, 사용자가 AnimNotify와 표현 자산을 연결한 뒤 호스트·클라이언트 화면에서 눈 표면에서만 효과가 보이는지 확인함.

## 구현 항목

- [x] AnimNotify 또는 Blueprint에서 호출할 발걸음 함수 제공
- [x] 발 socket 위치에서 아래 방향 trace로 `SnowSurface` 태그 표면만 감지
- [x] 너무 잦은 효과 재생을 막는 최소 쿨다운과 trace 조정값 제공
- [x] Blueprint가 Niagara, Decal, Sound를 연결할 표현 이벤트 제공
- [x] AnimNotify가 유효한 눈 표면을 밟을 때 `FootstepSound`를 attenuation과 함께 위치 기반으로 재생
- [x] 같은 AnimNotify에서 `SnowSurface` 태그 유무로 눈길/일반길 발걸음 사운드를 분리

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: C++·문서 최재원(C), Blueprint·애니메이션·VFX·Sound 연결 사용자 또는 서유정(S)
- 생성·변경 후보: `Tasks/C/C-26_snow_footstep_effect.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/Player/SnowRumbleCharacter.h`, `Source/SnowRumble/Player/SnowRumbleCharacter.cpp`
- 공유 확인 대상: 사용자 또는 서유정(S)
- 병합 순서: C++ 계약 반영 후 Blueprint/AnimNotify 표현 연결

## 공용 계약과 인계

- 제공받을 계약: 기존 눈 표면 태그 `SnowSurface`
- 제공할 계약: `ASnowRumbleCharacter::RequestSnowFootstepEffect(FName FootSocketName)`, `ASnowRumbleCharacter::OnSnowFootstepEffect(FName FootSocketName, FVector FootstepLocation, FVector FootstepNormal)`
- 인계 대상: 사용자 또는 서유정(S)이 `BP_SnowRumbleCharacter`/AnimBP/애니메이션 자산에서 발 착지 Notify와 표현 자산을 연결

## 범위 밖

- Niagara, Decal, Sound 자산 제작
- `.uasset`, `.umap` 직접 수정
- 서버 권한 발자국 지속 상태, 눈 변형 지형, 플레이어 은신·추적 같은 게임 판정

## 사전 전제

- 눈 바닥 Actor에는 기존 눈덩이 제작과 같은 `SnowSurface` 태그가 붙는다.
- 애니메이션 자산에는 왼발·오른발 착지 프레임을 구분할 수 있는 Notify를 추가할 수 있다.

## 결정 필요

- 없음

## 변경 기록

- 2026-08-14: 최재원 요청으로 눈 밟힘 표현 계약 Task를 추가하고 진행중으로 전환함.
- 2026-08-14: `RequestSnowFootstepEffect`와 `OnSnowFootstepEffect` 계약을 구현함. `SnowRumbleEditor Win64 Development` 빌드는 UHT와 C++ 컴파일, `.lib` 생성까지 통과했으나 실행 중인 Unreal Editor가 `UnrealEditor-SnowRumble.dll`을 사용 중이라 최종 링크는 `LNK1104`로 보류됨.
- 2026-08-24: `FootstepSound`와 `FootstepSoundAttenuation`을 추가해 `SnowSurface` 유효 판정 후 발 socket의 충돌 위치에서 발소리를 공간음향으로 재생하게 했다.

## 수동 작업

- `BP_SnowRumbleCharacter` 또는 캐릭터 AnimBP에서 왼발 착지 Notify는 `RequestSnowFootstepEffect("foot_l")`, 오른발 착지 Notify는 `RequestSnowFootstepEffect("foot_r")`처럼 실제 Skeleton 발 socket 이름으로 호출한다.
- `BP_SnowRumbleCharacter`에서 `OnSnowFootstepEffect`를 구현해 `FootstepLocation`과 `FootstepNormal` 기준으로 Niagara 눈 튐, 발자국 Decal, 발소리를 재생한다.
- `BP_SnowRumbleCharacter`에서 `FootstepSound`와 `FootstepSoundAttenuation`을 지정한다. 발소리는 C++가 AnimNotify의 유효한 눈 표면 판정 후 자동 재생하므로 Blueprint에서 별도 Sound 노드를 중복 연결하지 않는다.
- `BP_SnowRumbleCharacter`에서 일반 바닥용 `NormalFootstepSound`와 `NormalFootstepSoundAttenuation`을 지정한다. `SnowSurface` 태그가 있으면 눈길 슬롯, 없으면 일반길 슬롯을 사용한다.
- 눈 효과를 낼 바닥 Actor 또는 바닥 Blueprint에 `SnowSurface` Actor Tag를 추가한다.
- 기본 socket 이름이 Skeleton과 다르면 `LeftFootSocketName`, `RightFootSocketName` 또는 Notify 호출 인자를 실제 socket 이름으로 맞춘다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드·Blueprint·자산 변경 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료
- [x] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [x] 공용 계약과 캡슐화 규칙 위반 없음
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인

- [ ] PIE 또는 Standalone에서 눈 표면 위를 걸을 때 발 착지 시점에만 `OnSnowFootstepEffect` 표현이 재생됨
- [ ] `SnowSurface` 태그가 없는 바닥에서는 같은 AnimNotify가 호출되어도 눈 밟힘 표현이 재생되지 않음
- [ ] Listen Server 호스트와 클라이언트 화면에서 각자 보이는 캐릭터의 눈 밟힘 표현이 과도하게 중복되거나 누락되지 않음
- [ ] `FootstepSound`가 발 socket 아래의 `SnowSurface` 위치에서 attenuation과 함께 재생됨
- [ ] 일반 바닥에서 `NormalFootstepSound`가 재생되고, 눈 표면 VFX/stamp는 실행되지 않음

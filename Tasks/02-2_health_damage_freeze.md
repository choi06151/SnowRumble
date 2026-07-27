# Task 02-2 - HP, 피해, 얼기

## 설명

최소 네트워크 눈덩이가 플레이어에게 피해를 주고 HP가 모두 소진되면 플레이어가 얼어 전투에서 제외되는 첫 전투 세로 단면을 만든다.

## 구현 항목

- [x] 플레이어가 서버 기준의 HP를 가진다.
- [x] 최소 네트워크 눈덩이의 유효한 충돌이 피해로 처리된다.
- [x] HP가 모두 소진되면 얼기 상태로 전환된다.
- [x] 얼어있는 동안 이동과 일반 행동을 할 수 없다.
- [x] 얼어있는 플레이어가 눈 공격을 막는 오브젝트로 동작한다.
- [x] HP와 얼기 상태가 모든 참가자에게 동일하게 보인다.

## 메인 프로그래머 책임

- 서버 권한의 피해·HP·얼기 상태, 행동 제한과 비UI 피격·얼기 표현을 완성한다.

## SUB UI 인계

- 연계 SUB Task: `Tasks/S05-3_gameplay_hud.md`
- 현재 HP와 얼기 상태를 로컬 UI가 읽거나 변경 알림을 받을 수 있게 제공한다.

## 작업 배정

- 담당자: 메인 프로그래머
- SUB UI 담당자: 없음
- 메인 프로그래머 선점 파일·자산: `Source/SnowRumble/Player/SnowRumbleHealthComponent.h`, `Source/SnowRumble/Player/SnowRumbleHealthComponent.cpp`, `Source/SnowRumble/Player/SnowRumbleCharacter.h`, `Source/SnowRumble/Player/SnowRumbleCharacter.cpp`, `Source/SnowRumble/Snowball/SnowballProjectile.h`, `Source/SnowRumble/Snowball/SnowballProjectile.cpp`, `Content/Characters/BP_SnowRumbleCharacter`, `Content/Snowball/BP_SnowballProjectile`, `Tasks/02-2_health_damage_freeze.md`, `Tasks/S05-3_gameplay_hud.md`, `docs/ARCHITECTURE.md`, `docs/PENDING_ISSUES.md`, `docs/PLANS.md`
- SUB 프로그래머 선점 UI 자산: 없음
- 공유 확인 대상: 호스트와 클라이언트의 HP·얼기 상태, 이동·행동 제한과 눈덩이 차단 결과
- 인계 조건: `SnowRumbleHealthComponent`의 실제 HP·얼기 프로퍼티와 변경 이벤트를 `Tasks/S05-3_gameplay_hud.md`에 기록한 뒤 SUB UI 작업에 인계한다.
- 반영 순서: C++ 피해·HP·얼기 복제 기본틀 구현 후 메인 프로그래머가 캐릭터 Blueprint의 비UI 얼기 표현을 연결하고 2인 PIE를 확인한다.

## 범위 밖

- 핫팩 부활
- 팀 전원 얼기 승패
- 최종 얼음 모델과 이펙트
- 맨손과 눈삽 피해
- 큰 눈덩이의 광역 피해

## 사전 전제

- Task 02-1의 최소 네트워크 눈덩이 충돌

## 수동 작업

1. 실행 중인 Unreal Editor를 종료한다.
2. Visual Studio에서 `SnowRumbleEditor`를 `Development Editor`, `Win64` 구성으로 빌드한 뒤 `SnowRumble.uproject`를 연다.
3. `Content/Characters/BP_SnowRumbleCharacter`를 열고 상속받은 `HealthComponent`가 보이는지 확인한다.
4. `HealthComponent`의 `Max Health`를 프로토타입 기본값 `100`으로 둔다.
5. 캐릭터가 얼었음을 구분할 임시 Static Mesh Component를 추가하고 `FrozenVisual`로 이름을 지정한다.
6. `FrozenVisual`에 엔진 기본 도형 등 구분 가능한 임시 메시를 지정하고, 캐릭터를 감싸거나 위에 표시되도록 위치와 크기를 조정한다.
7. `FrozenVisual`의 충돌을 `NoCollision`, 초기 `Visible`을 해제로 설정한다.
8. `HealthComponent`의 `On Frozen Changed` 이벤트를 Event Graph에 추가하고 `bIsFrozen` 값을 `FrozenVisual`의 `Set Visibility`에 연결한다. 디버그 문자열 출력은 추가하지 않는다.
9. `BP_SnowRumbleCharacter`를 컴파일하고 저장한다.
10. `Content/Snowball/BP_SnowballProjectile`을 열고 상속받은 `Damage`가 프로토타입 기본값 `25`인지 확인한 뒤 컴파일하고 저장한다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드 수정 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료 — 모든 C++ 번역 단위와 UHT·복제 코드 컴파일 성공, 실행 중인 Editor의 DLL 점유로 최종 링크는 수동 빌드 필요
- [x] Unreal C++/Blueprint/에셋 규칙 위반 없음
- [x] 현재 task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인

- [x] 2인 PIE를 `Play As Listen Server`로 실행하고 호스트와 클라이언트가 서로 눈덩이를 맞힐 수 있는 위치에 있는지 확인한다.
- [x] 호스트가 클라이언트에게 눈덩이를 세 번 맞혔을 때 클라이언트가 아직 이동·점프·투척할 수 있는지 확인한다.
- [x] 호스트가 클라이언트에게 네 번째 눈덩이를 맞혔을 때 양쪽 화면에서 클라이언트의 `FrozenVisual`이 보이는지 확인한다.
- [x] 얼어있는 클라이언트가 이동, 점프, 상호작용, 조준, 투척, 장비 내려놓기와 이모션 입력을 수행할 수 없는지 확인한다.
- [x] 클라이언트가 호스트에게 눈덩이를 네 번 맞혔을 때도 양쪽 화면에서 호스트가 얼고 같은 행동 제한이 적용되는지 확인한다.
- [x] 얼어있는 플레이어에게 눈덩이를 던졌을 때 눈덩이가 통과하지 않고 충돌해 제거되는지 확인한다.
- [x] 얼어있는 플레이어에게 추가 눈덩이를 맞혀도 이미 0인 HP와 얼기 상태가 중복 변경되지 않는지 확인한다.
- [x] 얼어있는 동안에도 해당 플레이어가 자신의 마우스로 카메라를 조작할 수 있는지 확인한다.

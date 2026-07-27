# Task 02-1 - 최소 네트워크 눈덩이

## 설명

2인 PIE 환경에서 호스트와 클라이언트 어느 쪽이든 입력하면 서버가 작은 눈덩이를 생성해 앞으로 던지고, 모든 화면에서 같은 눈덩이 이동과 충돌 결과가 보이는 최소 멀티플레이 세로 단면을 만든다.

## 구현 항목

- [x] 소유 플레이어의 입력으로 눈덩이 투척을 서버에 요청할 수 있다.
- [x] 서버가 요청 가능 여부를 확인한 뒤 작은 눈덩이를 생성한다.
- [x] 생성된 눈덩이가 플레이어의 앞 방향으로 이동한다.
- [x] 눈덩이의 생성, 이동, 충돌, 제거가 호스트와 클라이언트에 동일하게 보인다.
- [x] 서버에서 눈덩이와 다른 플레이어의 충돌을 확인할 수 있다.
- [x] C++ 눈덩이 기본틀을 부모로 Blueprint에서 임시 메시와 표현을 연결할 수 있다.

## 메인 프로그래머 책임

- 투척 요청 RPC, 서버 생성·충돌 판정, 눈덩이 복제·제거와 눈덩이 Blueprint의 임시 메시·충돌 표현·입력을 완성한다.
- 이 단계에는 SUB UI 작업이 없다.

## 작업 배정

- 담당자: 메인 프로그래머
- SUB UI 담당자: 없음
- 메인 프로그래머 선점 파일·자산: `Source/SnowRumble/Player/SnowRumbleCharacter.h`, `Source/SnowRumble/Player/SnowRumbleCharacter.cpp`, `Source/SnowRumble/Snowball/SnowballProjectile.h`, `Source/SnowRumble/Snowball/SnowballProjectile.cpp`, `Content/Characters/BP_SnowRumbleCharacter`, `Content/Snowball/BP_SnowballProjectile`, `Tasks/02-1_basic_network_snowball.md`, `docs/ARCHITECTURE.md`, `docs/PENDING_ISSUES.md`, `docs/PLANS.md`
- SUB 프로그래머 선점 UI 자산: 없음
- 공유 확인 대상: 호스트와 클라이언트가 각각 요청한 눈덩이의 서버 생성, 이동, 충돌과 제거 결과
- 인계 조건: 없음
- 반영 순서: C++ 눈덩이·투척 요청 기본틀 구현 후 메인 프로그래머가 눈덩이 Blueprint와 캐릭터 Blueprint를 연결하고 2인 PIE를 확인한다.

## 범위 밖

- 바닥에서 눈 만들기
- 눈덩이 줍기와 들기
- 조준과 충전 게이지
- 실제 HP 감소와 얼기
- 큰 눈덩이
- 최종 모델, 애니메이션, 이펙트

## 사전 전제

- Task 01-2의 2인 PIE 플레이어 이동과 입력

## 수동 작업

1. 실행 중인 Unreal Editor를 종료한다.
2. Visual Studio에서 `SnowRumbleEditor`를 `Development Editor`, `Win64` 구성으로 빌드한 뒤 `SnowRumble.uproject`를 연다.
3. Content Browser에 `Content/Snowball` 폴더를 만든다.
4. `Content/Snowball/BP_SnowballProjectile` Blueprint를 만들고 부모 클래스를 `SnowballProjectile`로 지정한다.
5. `BP_SnowballProjectile`에 Static Mesh Component를 추가하고 상속받은 `CollisionComponent`에 연결한다.
6. Static Mesh Component에 엔진 기본 구체 등 구분 가능한 임시 메시를 지정하고, 메시 컴포넌트 자체의 충돌은 `NoCollision`로 설정한다.
7. 임시 메시가 상속받은 `CollisionComponent`의 반지름 15cm 안에 들어오도록 크기를 조정한 뒤 Blueprint를 컴파일하고 저장한다.
8. `Content/Characters/BP_SnowRumbleCharacter`를 열고 `SnowRumble|Snowball`의 `Snowball Projectile Class`를 `BP_SnowballProjectile`로 설정한 뒤 컴파일하고 저장한다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드 수정 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료 — 새 C++ 번역 단위와 RPC 생성 코드 컴파일 성공, 실행 중인 Editor의 DLL 점유로 최종 링크는 수동 빌드 필요
- [x] Unreal C++/Blueprint/에셋 규칙 위반 없음
- [x] 현재 task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인

- [x] 2인 PIE를 `Play As Listen Server`로 실행했을 때 호스트와 클라이언트가 각각 한 캐릭터를 소유하는지 확인한다.
- [x] 호스트에서 좌클릭했을 때 캐릭터 앞에 눈덩이가 생성되어 앞으로 이동하는지 확인한다.
- [x] 클라이언트에서 좌클릭했을 때 캐릭터 앞에 눈덩이가 생성되어 앞으로 이동하는지 확인한다.
- [x] 호스트가 던진 눈덩이의 생성과 이동이 클라이언트 화면에도 동일하게 보이는지 확인한다.
- [x] 클라이언트가 던진 눈덩이의 생성과 이동이 호스트 화면에도 동일하게 보이는지 확인한다.
- [x] 호스트가 던진 눈덩이가 다른 플레이어나 월드에 충돌하면 양쪽 화면에서 제거되는지 확인한다.
- [x] 클라이언트가 던진 눈덩이가 다른 플레이어나 월드에 충돌하면 양쪽 화면에서 제거되는지 확인한다.
- [x] 눈덩이가 투척한 캐릭터 자신에게 즉시 충돌해 사라지지 않는지 확인한다.

# Task C-35 - 피격 데미지 텍스트 표시 계약

## 설명

서버가 실제로 적용한 피해량을 기준으로 모든 클라이언트에 데미지 숫자 표시 이벤트를 전달한다. 일반/헤드샷 WBP 슬롯을 분리해 다음 헤드샷 판정에서도 표시 위젯을 다르게 사용할 수 있게 한다.

## 상태 전이 기준

- 시작 가능: C-09 눈 전투 피해 처리 경로 존재
- 완료 가능: 실제 적용 피해량 기준 multicast와 Blueprint 이벤트 계약 추가, 정적 점검 완료

## 구현 항목

- [x] `ASnowRumbleCharacter::TakeDamage()`의 `AppliedDamage > 0` 지점에서 데미지 텍스트 표시 요청 추가
- [x] 모든 클라이언트용 `MulticastRequestDamageText()` 추가
- [x] 캐릭터 Blueprint에서 받을 `OnDamageTextRequested()` 이벤트와 위치 오프셋 추가
- [x] `UDamageTextWidget` 부모 WBP 계약과 일반/헤드샷 WBP 클래스 슬롯 추가
- [x] 직접 투척 눈덩이의 머리 bone 충돌 시 헤드샷 피해 배율과 헤드샷 데미지 텍스트 타입 적용

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: 사용자/S 후속
- 생성·변경 후보: `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Source/SnowRumble/UI/DamageTextWidget_C.*`, `Source/SnowRumble/Snowball/SnowballItem.*`, `Source/SnowRumble/Snowball/SnowballDamageTypes.h`
- 공유 확인 대상: C-09 눈 전투, HUD/피격 표현 Blueprint
- 병합 순서: C-35 C++ 계약 후 Blueprint 데미지 텍스트 표시 연결

## 공용 계약과 인계

- `DamageTextWidgetClass`와 `HeadshotDamageTextWidgetClass`에 `UDamageTextWidget` 기반 WBP를 지정한다.
- `UDamageTextWidget` 기반 WBP에 `DamageText` TextBlock을 만들면 C++가 피해량 숫자를 자동 입력한다.
- `OnDamageTextRequested(float AppliedDamage, FVector DamageTextWorldLocation, ESnowRumbleDamageTextType DamageTextType)`를 캐릭터 Blueprint에서 추가 연출용으로 구현할 수 있다.
- `DamageTextWorldOffset`으로 피격자 기준 표시 위치를 조정한다.
- `ASnowballItem::HeadshotBoneNames`와 `HeadshotDamageMultiplier`로 머리 판정 bone 이름과 피해 배율을 조정한다.

## 범위 밖

- 데미지 텍스트 WBP 디자인과 애니메이션 제작
- 데미지 공식 변경

## 변경 기록

- 2026-08-27: 사용자 요청으로 피격 데미지 텍스트 표시 계약을 추가했다.
- 2026-08-27: 일반/헤드샷 데미지 텍스트 WBP 슬롯을 분리하고, `UDamageTextWidget` 부모가 `DamageText` TextBlock에 적용 피해량을 자동 입력하도록 추가했다.
- 2026-08-27: 직접 투척 눈덩이가 `HeadshotBoneNames`에 포함된 bone을 맞히면 `HeadshotDamageMultiplier`를 적용하고 `HeadshotDamageTextWidgetClass`가 사용되도록 연결했다.

## 수동 작업

- 일반 데미지 WBP와 헤드샷 데미지 WBP를 `UDamageTextWidget` 부모로 만든다.
- WBP 안에 `DamageText` 이름의 TextBlock을 두거나, `OnDamageTextInitialized`에서 직접 표시를 꾸민다.
- 캐릭터 Blueprint의 `DamageTextWidgetClass`, `HeadshotDamageTextWidgetClass`에 각각 WBP를 지정한다.
- 필요 시 `DamageTextWorldOffset`을 캐릭터 머리 위 위치에 맞게 조정한다.
- 캐릭터 Physics Asset 또는 hit 결과 bone 이름을 확인해 `HeadshotBoneNames` 기본값(`head`, `Head`, `neck_01`, `Neck`)과 다르면 Blueprint에서 맞춘다.

## 완료 조건

### 에이전트 확인

- [x] 관련 C++ 코드와 Task 문서 갱신 완료
- [x] 실제 적용 피해량 기준으로만 표시 요청됨

### 결과 확인

- [ ] 플레이어가 실제 피해를 받으면 모든 클라이언트에서 데미지 숫자 표시 이벤트가 호출된다.
- [ ] `DamageTextWidgetClass`를 지정하면 별도 Blueprint 이벤트 구현 없이 일반 데미지 숫자가 표시된다.
- [ ] 직접 투척 눈덩이가 머리 bone에 맞으면 헤드샷 피해량과 `HeadshotDamageTextWidgetClass`가 우선 사용된다.
- [ ] 무적·로비·팀킬 차단 등으로 피해가 0이면 데미지 숫자가 뜨지 않는다.

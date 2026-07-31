# Pending Issues

## 목적

이 문서는 아직 해소되지 않은 구조 의심과 보류 이슈를 기록하고 추적하기 위한 문서입니다.
<!-- 새 프로젝트 시작 시 보류된 의심 기록을 초기화한다. -->

## 보류된 의심

- Task 04-3: 굴리기 중 실제 눈덩이 충돌을 끄고 서버가 Sweep하는 `RollingSnowballCollision`을 `ASnowRumbleCharacter`에 직접 두었다. 현재는 전용 Sphere Component 하나와 최소 제어 함수만 사용하지만, 굴리기 충돌 규칙이 더 늘어나면 별도 `URollingSnowballCollisionComponent`로 분리하는 것을 검토한다.
- Task 04-2: `ASnowRumbleCharacter`의 Task 02-1 즉시 생성 투척 RPC·프로퍼티를 제거하고, 실제 보유 `ASnowballItem`에 이동·충돌·피해 책임을 통합했다. 기존 `ASnowballProjectile` 클래스는 Task 02-1 기록 호환을 위해 남아 있지만 현재 플레이 파이프라인에서는 사용하지 않으므로 참조 확인 후 별도 정리할 수 있다.
- Task 03-1: `USnowRumbleSessionSubsystem`은 현재 NULL LAN의 Host·검색·Join 생명주기를 한 곳에서 관리한다. Steam, 세션 종료·재생성, 재접속처럼 공급자나 생명주기 분기가 늘어나면 요청별 책임 또는 공급자 어댑터 분리를 검토한다.

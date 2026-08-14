# Pending Issues

## 목적

이 문서는 아직 해소되지 않은 구조 의심과 보류 이슈를 기록하고 추적하기 위한 문서입니다.
<!-- 새 프로젝트 시작 시 보류된 의심 기록을 초기화한다. -->

## 보류된 의심

- S04-3: `UOverheadTimedActionWidget`과 `ESnowRumbleTimedActionState`는 처음 시간제 행동만 표시하도록 이름을 정했지만, 사용자 요청으로 거리 기반 눈 굴리기 성장도 함께 표시한다. 현재 WBP 부모 호환성을 위해 이름을 유지하며, 비시간제 진행 행동이 더 늘어나면 `ProgressAction` 기준 이름으로 변경하고 기존 WBP 부모 리디렉션을 함께 검토한다.
- Task 04-3: 굴리기 중 실제 눈덩이 충돌을 끄고 서버가 Sweep하는 `RollingSnowballCollision`을 `ASnowRumbleCharacter`에 직접 두었다. 현재는 전용 Sphere Component 하나와 최소 제어 함수만 사용하지만, 굴리기 충돌 규칙이 더 늘어나면 별도 `URollingSnowballCollisionComponent`로 분리하는 것을 검토한다.
- Task 04-2: `ASnowRumbleCharacter`의 Task 02-1 즉시 생성 투척 RPC·프로퍼티를 제거하고, 실제 보유 `ASnowballItem`에 이동·충돌·피해 책임을 통합했다. 기존 `ASnowballProjectile` 클래스는 Task 02-1 기록 호환을 위해 남아 있지만 현재 플레이 파이프라인에서는 사용하지 않으므로 참조 확인 후 별도 정리할 수 있다.
- Task 03-1: `USnowRumbleSessionSubsystem`은 현재 NULL LAN의 Host·검색·Join 생명주기를 한 곳에서 관리한다. Steam, 세션 종료·재생성, 재접속처럼 공급자나 생명주기 분기가 늘어나면 요청별 책임 또는 공급자 어댑터 분리를 검토한다.
- C-25: `ASnowRumbleCharacter`가 게시판, 선물상자, 선물상자 아이템 Pickup, 눈덩이 상호작용 후보 우선순위를 직접 관리한다. 현재는 대상이 네 종류라 유지하지만, 상호작용 대상이 더 늘어나면 공용 상호작용 인터페이스 또는 전용 탐색 컴포넌트로 분리하는 것을 검토한다.
- C-26: `ASnowRumbleCharacter`가 발 socket 기반 눈 밟힘 표현 trace와 Blueprint 이벤트까지 직접 제공한다. 현재는 상태·복제 수명주기 없는 작은 애니메이션 표현 계약이라 유지하지만, 발소리·표면별 효과·지속 발자국·지형 변형처럼 표면 표현 규칙이 늘어나면 전용 발걸음 표현 컴포넌트로 분리하는 것을 검토한다.
- C-27: `ASnowTrailRenderTargetManager`는 현재 맵당 단일 RenderTarget으로 눈길 마스크를 관리한다. 대형 맵, 여러 높이층, 월드 파티션, 오래 지속되는 많은 stamp가 필요해지면 영역별 타일 RenderTarget 또는 맵 전용 환경 시스템으로 분리하는 것을 검토한다.

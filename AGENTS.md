# AGENTS.md

이 문서는 `SnowRumble` 저장소에서 작업하는 AI 에이전트의 시작 문서입니다.

## 프로젝트 개요

- 프로젝트명: `SnowRumble` (`눈싸움 대소동`)
- 장르: 3인칭 멀티플레이 파티 PvP
- 주요 플랫폼: PC 우선, 콘솔·모바일 추후 검토
- 주요 환경: Unreal Engine 5.8, C++, Blueprint
- 네트워크: Listen Server, OnlineSubsystem NULL, LAN Session
- 지원 인원: 2~16명, 권장 4명 이상
- 핵심 목표: 눈을 만들고 던져 상대 팀 전원을 얼리면 승리하는 캐주얼 눈싸움 게임
- 개발 방식: 작은 단위의 프로토타입을 검증한 뒤 단계적으로 확장합니다.

## 개발자와 협업 목표

- 개발자는 기존 싱글플레이 Unreal 개발에는 능숙하지만 멀티플레이 개발은 처음입니다.
- 멀티플레이 기능은 Codex가 서버 권한·RPC·복제의 C++ 기본틀을 만들고, 개발자가 Blueprint에서 게임플레이 디테일과 표현을 완성하는 방식을 목표로 합니다.
- 구체적인 진행 절차는 `docs/WORKFLOW.md`의 `멀티플레이 기능 추가 절차`, 책임 경계는 `docs/STYLEGUIDE.md`의 `C++과 Blueprint 책임 경계`를 따릅니다.

## 기획 기준

- `docs/GDD/Game_GDD.md`는 전체 게임 기획의 기준 문서입니다.
- `docs/GDD/GDD_PROGRESS.md`는 현재 MVP 범위, 기획 진행 상태, 미결정 사항을 관리합니다.
- 전체 GDD에 있는 기능이라도 현재 MVP나 진행 중 Task에 포함되지 않았다면 임의로 구현하지 않습니다.
- 러프 기획의 표현이 모호하거나 문서끼리 충돌하면 구현 전에 사용자에게 확인합니다.

## 문서 맵

- `AGENTS.md`: AI 에이전트용 시작 문서와 프로젝트 개요
- `docs/GDD/Game_GDD.md`: 전체 게임 규칙과 기능 기획
- `docs/GDD/GDD_PROGRESS.md`: 현재 MVP 범위와 기획 진행 상태
- `docs/GDD/SnowRumble_Game_Overview.md`: 외부 공유와 프로젝트 소개를 위한 전체 기획 요약
- `docs/HARNESS.md`: 하네스 개념, 이탈 대응, 규칙 변경 절차
- `docs/WORKFLOW.md`: Task 진행 순서, 구현 전 승인, 상태 관리 규칙
- `docs/COLLABORATION.md`: 메인·SUB 프로그래머 역할, 파일 소유권, 충돌 방지와 인계 절차
- `docs/PLANS.md`: 단계별 개발 순서와 최근 작업 로그
- `docs/ARCHITECTURE.md`: 폴더 책임과 파일·자산 배치 원칙
- `docs/STYLEGUIDE.md`: Unreal C++·Blueprint·멀티플레이 작성 규칙과 이름 규칙
- `docs/TESTING.md`: 정적 점검과 멀티플레이 실행 확인 기준
- `docs/TASK_WRITING.md`: Task 문서 작성과 구현 후 갱신 규칙
- `docs/LOGS.md`: `docs/PLANS.md`에서 이동한 이전 작업 로그
- `docs/PENDING_ISSUES.md`: 해소되지 않은 구조 의심과 보류 이슈
- `docs/RULE_ORIGINS.md`: 규칙별 출처와 설계 의도
- `docs/AUDIT_LOG.md`: 사용자가 요청한 하네스 감사 이력
- `Tasks/_template.md`: 새 Task 문서 작성용 템플릿
- `Tasks/_sub_ui_template.md`: SUB 프로그래머 UI Task 작성용 템플릿
- `Tasks/MAKE_REQUEST_TEMPLATE.md`: 사용자가 새 Task를 요청할 때 쓰는 양식
- `Tasks/`: 단계별 구현 범위와 완료 조건

## 먼저 읽을 문서

세션 첫 작업 시작 전 아래 문서를 순서대로 읽습니다.

1. `docs/HARNESS.md`
2. `docs/WORKFLOW.md`
3. `docs/PLANS.md`

세션 시작 시에는 위 세 문서만 읽습니다. 다른 문서는 아래 트리거나 현재 Task의 작업 흐름을 실제로 진행할 때만 확인합니다.

같은 세션에서 이미 읽은 문서는 다시 읽지 않습니다. 재확인이 필요하면 관련 구간만 조회합니다. 다만 Task 전환, 문서 충돌, 상태 불일치처럼 흐름 판단에 영향이 있으면 관련 문서를 다시 넓게 확인할 수 있습니다.

## 문서 확인 트리거

- Task 시작·전환·상태 갱신 전 → `docs/PLANS.md`, `docs/WORKFLOW.md`
- 메인·SUB 역할 배정, 동시 작업, 파일·자산 소유권 판단, 충돌 발견 전 → `docs/COLLABORATION.md`
- 게임 규칙, 조작, 플레이어 기능, 아이템, 맵, 승리 조건 변경 전 → `docs/GDD/Game_GDD.md`
- MVP 포함 여부나 기획 미결정 사항 확인 전 → `docs/GDD/GDD_PROGRESS.md`
- 구조 변경, 새 폴더·맵·에셋 타입 추가, 파일·자산 배치 결정 전 → `docs/ARCHITECTURE.md`
- 코드 수정, 새 시스템 구현, 파일·자산 이름 결정 전 → `docs/STYLEGUIDE.md`
- 검증 단계 전 → `docs/TESTING.md`
- 메인 Task 문서 작성 또는 갱신 전 → `docs/TASK_WRITING.md`, `Tasks/_template.md`
- SUB UI Task 문서 작성 또는 갱신 전 → `docs/TASK_WRITING.md`, `Tasks/_sub_ui_template.md`
- 사용자가 하네스 감사를 요청할 때 → `docs/AUDIT_LOG.md`
- 하네스 규칙 추가, 문서 절차 조정, 에이전트 흐름 이탈 대응, Markdown 규칙 위반 발견 시 → 답변이나 수정 제안 전에 `docs/HARNESS.md`

## 문서 처리 규칙

- Markdown 문서는 UTF-8로 저장합니다.
- PowerShell에서 Markdown을 읽을 때는 `Get-Content -Encoding UTF8`을 사용합니다.

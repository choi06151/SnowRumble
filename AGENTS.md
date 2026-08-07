# AGENTS.md

이 문서는 `SnowRumble` 저장소에서 작업하는 AI 에이전트의 시작 문서입니다.

## 프로젝트 개요

- 프로젝트명: `SnowRumble` (`눈싸움 대소동`)
- 장르: 3인칭 멀티플레이 파티 PvP
- 주요 플랫폼: PC 우선, 콘솔·모바일 추후 검토
- 주요 환경: Unreal Engine 5.8, C++, Blueprint
- 네트워크: Listen Server, OnlineSubsystem NULL, LAN Session
- 지원 인원: 1vs1~4vs4, 양 팀 인원이 같을 때 시작
- 핵심 목표: 눈을 만들고 던져 상대 팀 전원을 얼리면 승리하는 캐주얼 눈싸움 게임
- 개발 방식: 작은 단위의 프로토타입을 검증한 뒤 단계적으로 확장합니다.

## 세션 담당자 확인

- Codex는 세션의 첫 작업 전에 현재 대상 담당자가 아래 네 명 중 누구인지 확인합니다.
- 사용자가 첫 요청에 담당자를 이미 명시했다면 다시 묻지 않습니다.
- 담당자가 확인되기 전에는 프로젝트 파일과 기준 문서를 수정하지 않습니다.
- 담당자가 확정되면 해당 담당자의 역할·소유권·금지 범위를 `docs/COLLABORATION.md`에서 확인합니다.
- 담당자 확인 직후 `$snowrumble-run-task`로 원격 변경을 확인하고 담당 브랜치 전환과 안전한 fast-forward pull을 완료한 뒤 작업 문서를 수정합니다.

| 코드·브랜치 | 담당자 | 기본 역할 |
| --- | --- | --- |
| `S` | 서유정 | UI/UX, 그래픽 디자인, 레벨 디자인, 캐릭터·아이템 모델링 |
| `J` | 정다영 | 레벨 디자인 |
| `K` | 강혜원 | 서브 프로그래머, 멀티플레이와 아이템 기본 로직 |
| `C` | 최재원 | 메인 프로그래머, 멀티플레이 통합과 PvP·플레이어 |

## 개발자와 협업 목표

- 네 담당자는 각자 별도 Codex와 작업 환경에서 자기 소유 범위를 개발합니다.
- 멀티플레이 기능은 서버 권한·RPC·복제의 C++ 기본틀과 Blueprint 표현·자산 연결을 분리합니다.
- 공용 네트워크 계약과 파트 간 연결은 메인 프로그래머가 검토하고 통합합니다.
- 같은 파일이나 Unreal 자산에는 동시에 한 명의 수정자만 둡니다.
- 각 담당자는 자기 코드와 같은 `C`, `K`, `S`, `J` 브랜치와 별도 worktree 또는 clone을 사용하며, `master`는 검증된 통합 기준으로 유지합니다.
- 구체적인 진행 절차는 `docs/WORKFLOW.md`, 역할과 충돌 대응은 `docs/COLLABORATION.md`, 책임 경계는 `docs/STYLEGUIDE.md`를 따릅니다.

## 공용 Task 실행 Skill

- Task 시작·계속·순서 변경·추가·분할·오류 대응·대기·대체·인계·충돌 대응에는 프로젝트 공용 Skill `$snowrumble-run-task`를 사용합니다.
- 사용자가 작업 완료를 확인하고 push를 요청하면 같은 Skill로 역할 브랜치 커밋·push와 조건을 통과한 `master` 병합·push까지 수행합니다.
- 이 Skill은 확정된 담당자의 `ROLE_*.md`와 `PLAN_*.md`를 기준으로 작업을 분류하고, 개발자와의 대화 내용을 Codex가 계획과 Task 문서에 반영하게 합니다.
- 개발자는 `PLAN_*.md`를 직접 작성할 필요가 없습니다. 원하는 결과, 문제, 우선순위만 대화로 전달합니다.
- 역할 내부 변경은 해당 역할 Codex가 관리하며, 공용 계약·다른 역할 소유권·통합 순서에 영향이 있으면 C의 통합 경로를 사용합니다.

## 기획 기준

- `docs/GDD/Game_GDD.md`는 전체 게임 기획의 기준 문서입니다.
- `docs/GDD/GDD_PROGRESS.md`는 현재 MVP 범위, 기획 진행 상태, 미결정 사항을 관리합니다.
- 전체 GDD에 있는 기능이라도 현재 MVP나 진행 중 Task에 포함되지 않았다면 임의로 구현하지 않습니다.
- 러프 기획의 표현이 모호하거나 문서끼리 충돌하면 구현 전에 사용자에게 확인합니다.

## 문서 맵

- `AGENTS.md`: AI 에이전트용 시작 문서와 프로젝트 개요
- `.agents/skills/snowrumble-run-task/SKILL.md`: 네 역할이 공통으로 사용하는 Task 실행·변경·인계 절차
- `docs/GDD/Game_GDD.md`: 전체 게임 규칙과 기능 기획
- `docs/GDD/GDD_PROGRESS.md`: 현재 MVP 범위와 기획 진행 상태
- `docs/GDD/SnowRumble_Game_Overview.md`: 외부 공유와 프로젝트 소개를 위한 전체 기획 요약
- `docs/HARNESS.md`: 하네스 개념, 이탈 대응, 규칙 변경 절차
- `docs/WORKFLOW.md`: Task 진행 순서, 구현 전 승인, 상태 관리 규칙
- `docs/COLLABORATION.md`: 4인 역할, 파일 소유권, 충돌 방지·대응과 인계 절차
- `docs/PLANS.md`: 파트 간 필수 의존성, 공용 계약, 선점과 통합 상태
- `docs/PROGRESS_BASELINE_2026-08-07.md`: 기획·협업 구조 개편 전 구현·검증 상태 스냅샷
- `docs/ARCHITECTURE.md`: 폴더 책임과 파일·자산 배치 원칙
- `docs/STYLEGUIDE.md`: Unreal C++·Blueprint·멀티플레이 작성 규칙과 이름 규칙
- `docs/TESTING.md`: 정적 점검과 멀티플레이 실행 확인 기준
- `docs/TASK_WRITING.md`: Task 문서 작성과 구현 후 갱신 규칙
- `docs/LOGS.md`: `docs/PLANS.md`에서 이동한 이전 작업 로그
- `docs/PENDING_ISSUES.md`: 해소되지 않은 구조 의심과 보류 이슈
- `docs/RULE_ORIGINS.md`: 규칙별 출처와 설계 의도
- `docs/AUDIT_LOG.md`: 사용자가 요청한 하네스 감사 이력
- `Tasks/_template.md`: 4인 역할별 새 Task 문서 작성용 공통 템플릿
- `Tasks/C/ROLE_C.md`: 최재원(C) 역할과 Task 진입 문서
- `Tasks/C/PLAN_C.md`: 최재원(C) Codex가 대화로 관리하는 작업 대기열
- `Tasks/K/ROLE_K.md`: 강혜원(K) 역할과 Task 진입 문서
- `Tasks/K/PLAN_K.md`: 강혜원(K) Codex가 대화로 관리하는 작업 대기열
- `Tasks/S/ROLE_S.md`: 서유정(S) 역할과 Task 진입 문서
- `Tasks/S/PLAN_S.md`: 서유정(S) Codex가 대화로 관리하는 작업 대기열
- `Tasks/J/ROLE_J.md`: 정다영(J) 역할과 Task 진입 문서
- `Tasks/J/PLAN_J.md`: 정다영(J) Codex가 대화로 관리하는 작업 대기열
- `Tasks/LEGACY_MIGRATION.md`: 구조 개편 전 Task와 새 Task의 마이그레이션 장부
- `Tasks/_sub_ui_template.md`: 구조 개편 전 SUB UI Task 작성용 기존 템플릿
- `Tasks/MAKE_REQUEST_TEMPLATE.md`: 사용자가 새 Task를 요청할 때 쓰는 양식
- `Tasks/`: 단계별 구현 범위와 완료 조건

## 먼저 읽을 문서

세션 담당자를 확인한 뒤 첫 작업 시작 전 아래 문서를 순서대로 읽습니다.

1. `docs/HARNESS.md`
2. `docs/WORKFLOW.md`
3. `docs/PLANS.md`
4. 현재 담당자의 `Tasks/{코드}/ROLE_{코드}.md`
5. 현재 담당자의 `Tasks/{코드}/PLAN_{코드}.md`

세션 시작 시에는 위 다섯 문서만 읽습니다. 다른 문서는 아래 트리거나 현재 Task의 작업 흐름을 실제로 진행할 때만 확인합니다.

같은 세션에서 이미 읽은 문서는 다시 읽지 않습니다. 재확인이 필요하면 관련 구간만 조회합니다. 다만 Task 전환, 문서 충돌, 상태 불일치처럼 흐름 판단에 영향이 있으면 관련 문서를 다시 넓게 확인할 수 있습니다.

## 문서 확인 트리거

- 세션 첫 작업·담당자 변경·Task 재개 전 → `$snowrumble-run-task`의 브랜치 확인·원격 동기화 절차
- 사용자가 작업 완료 후 push를 요청할 때 → `$snowrumble-run-task`의 완료 작업 게시 절차
- Task 실행·계획 변경·오류 대응·인계·충돌 처리 전 → `$snowrumble-run-task`
- Task 시작·전환·상태 갱신 전 → 현재 담당자의 `PLAN_*.md`, `docs/PLANS.md`, `docs/WORKFLOW.md`
- 기획 또는 협업 구조 개편 전 → `docs/PROGRESS_BASELINE_2026-08-07.md`
- 담당자·역할 배정, 동시 작업, 파일·자산 소유권 판단, 공용 계약 변경, 충돌 발견 전 → `docs/COLLABORATION.md`
- 게임 규칙, 조작, 플레이어 기능, 아이템, 맵, 승리 조건 변경 전 → `docs/GDD/Game_GDD.md`
- MVP 포함 여부나 기획 미결정 사항 확인 전 → `docs/GDD/GDD_PROGRESS.md`
- 구조 변경, 새 폴더·맵·에셋 타입 추가, 파일·자산 배치 결정 전 → `docs/ARCHITECTURE.md`
- 코드 수정, 새 시스템 구현, 파일·자산 이름 결정 전 → `docs/STYLEGUIDE.md`
- 검증 단계 전 → `docs/TESTING.md`
- 새 역할별 Task 문서 작성 또는 갱신 전 → `docs/TASK_WRITING.md`, `Tasks/_template.md`, 현재 담당자의 `ROLE_*.md`
- Task 우선순위 변경·추가·분할·대기·대체 전 → 현재 담당자의 `PLAN_*.md`, 파트 간 영향이 있으면 `docs/PLANS.md`, `docs/COLLABORATION.md`
- 기존 Task 마이그레이션 판단 전 → `Tasks/LEGACY_MIGRATION.md`, `docs/PROGRESS_BASELINE_2026-08-07.md`
- 사용자가 하네스 감사를 요청할 때 → `docs/AUDIT_LOG.md`
- 하네스 규칙 추가, 문서 절차 조정, 에이전트 흐름 이탈 대응, Markdown 규칙 위반 발견 시 → 답변이나 수정 제안 전에 `docs/HARNESS.md`

## 문서 처리 규칙

- Markdown 문서는 UTF-8로 저장합니다.
- PowerShell에서 Markdown을 읽을 때는 `Get-Content -Encoding UTF8`을 사용합니다.

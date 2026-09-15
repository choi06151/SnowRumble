# Task C-41 - EditorConfig 공통 파일 컨벤션

## 설명

EditorConfig를 저장소 루트에 추가하고 Husky pre-commit 검사에 연결해 EditorConfig를 지원하는 에디터와 커밋 단계에서 텍스트 파일의 문자 인코딩, 개행, 최종 개행, 공백과 들여쓰기 규칙을 일관되게 적용한다.

## 상태 전이 기준

- 시작 가능: 사용자 승인, C 브랜치 동기화 완료
- 완료 가능: `.editorconfig`와 staged 파일 검사 연결, 기존 `.clang-format`과의 충돌 방지 확인, 정적 점검 및 에디터 적용 확인 완료

## 구현 항목

- [x] 저장소 루트에 UTF-8·LF·최종 개행·trailing whitespace 규칙을 추가한다.
- [x] C++ 파일은 기존 `.clang-format`의 탭·4칸 정책을 유지하도록 별도 규칙을 추가한다.
- [x] Markdown의 의미 있는 trailing whitespace를 보존하도록 예외를 추가한다.
- [x] Husky pre-commit에서 staged 텍스트 파일을 `.editorconfig` 기준으로 검사한다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: 없음
- 생성·변경 파일: `.editorconfig`, `Scripts/check-editorconfig.mjs`, `.husky/pre-commit`, `package.json`, `package-lock.json`, 본 Task, `Tasks/C/PLAN_C.md`
- 공유 확인 대상: 모든 담당자
- 병합 순서: C 브랜치 검증 후 역할 브랜치 게시

## 공용 계약과 인계

- 제공받을 계약: 기존 `.clang-format`의 C++ 포맷 규칙
- 제공할 계약: EditorConfig 지원 에디터와 Husky pre-commit의 저장·들여쓰기 기본 규칙
- 인계 대상: 모든 담당자

## 범위 밖

- 기존 파일의 일괄 개행·공백 변환
- `.clang-format` 규칙 변경
- GitHub Actions에 EditorConfig 검사를 추가하는 것

## 사전 전제

- EditorConfig를 지원하는 에디터 또는 플러그인

## 결정 필요

- 없음

## 변경 기록

- 2026-09-15: 사용자 요청과 승인으로 저장소 공통 `.editorconfig` Task를 추가했다.

## 수동 작업

- 각 개발자는 저장소 루트에서 `npm install`을 한 번 실행한다.
- 각 개발자는 사용하는 에디터에서 EditorConfig 지원을 활성화하거나 플러그인을 설치한다.

## 완료 조건

### 에이전트 확인

- [x] `.editorconfig`와 Husky 검사 연결 완료
- [x] 기존 `.clang-format`과 C++ 들여쓰기 정책 충돌 없음 확인
- [x] 로컬 EditorConfig 검사와 Git diff 검사 완료
- [x] Task 문서와 C 계획 갱신 완료

### 결과 확인

- [ ] EditorConfig 지원 에디터에서 C++ 파일 저장 시 탭 4칸, 일반 텍스트 파일 저장 시 공통 개행·공백 규칙이 적용되는지 확인

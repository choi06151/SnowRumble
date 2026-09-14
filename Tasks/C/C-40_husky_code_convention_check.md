# Task C-40 - Husky·GitHub Actions 코드 컨벤션 검사

## 설명

로컬 커밋과 GitHub Actions에서 변경된 Unreal C++ 파일에 동일한 `.clang-format` 검사를 적용해 컨벤션 위반을 조기에 차단한다.

## 상태 전이 기준

- 시작 가능: 사용자 승인, C 브랜치 동기화 완료
- 완료 가능: Husky 훅·공통 검사 스크립트·GitHub Actions 추가와 로컬 검사 성공

## 구현 항목

- [x] staged C++ 파일을 검사하는 Husky pre-commit 훅을 추가한다.
- [x] 로컬·CI에서 재사용하는 clang-format 검사 스크립트를 추가한다.
- [x] Push와 Pull Request에서 변경 C++ 파일을 검사하는 GitHub Actions를 추가한다.
- [x] 설치·사용 방법과 다른 담당자 파일에 대한 점진적 적용 범위를 기록한다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: 없음
- 생성·변경 파일: `package.json`, `package-lock.json`, `.husky/pre-commit`, `Scripts/check-clang-format.mjs`, `.github/workflows/code-convention.yml`, `.gitignore`, 본 Task, `Tasks/C/PLAN_C.md`
- 공유 확인 대상: 모든 담당자
- 병합 순서: C 브랜치 검증 후 역할 브랜치 게시

## 공용 계약과 인계

- 제공받을 계약: `.clang-format`
- 제공할 계약: 변경된 `.h`·`.cpp` 파일은 커밋과 CI에서 포맷 검사를 통과해야 한다.
- 인계 대상: 모든 담당자 — 최초 설치 시 `npm install`, clang-format 설치 필요

## 범위 밖

- 기존 미변경 파일의 자동 재포맷
- Unreal 빌드·패키징을 GitHub Actions에서 수행하는 것
- 다른 담당자 소유 코드 수정

## 사전 전제

- Node.js와 npm
- 로컬 clang-format 또는 Visual Studio LLVM 도구

## 결정 필요

- 없음

## 변경 기록

- 2026-09-14: 사용자 승인으로 Husky와 GitHub Actions 기반의 변경 파일 컨벤션 검사를 추가했다.

## 수동 작업

- 각 개발자는 저장소 루트에서 `npm install`을 한 번 실행한다.
- 로컬에서 clang-format을 PATH에 등록하거나 Visual Studio LLVM 도구를 설치한다.

## 완료 조건

### 에이전트 확인

- [x] Husky·검사 스크립트·GitHub Actions 추가 완료
- [x] staged 검사와 동일한 파일 인자 기반 C++ 검사 성공
- [x] 로컬 정적 점검 완료 — `node --check`, `npm run format:check`, `git diff --check`
- [x] Task 문서 갱신 완료

### 결과 확인

- [x] 테스트 커밋에서 포맷 위반 시 커밋이 차단되는지 확인 — pre-commit에서 변경 C++ 파일을 검사하는 경로를 검증했다.
- [x] 포맷 수정 후 커밋 검사가 통과하는지 확인 — 게시 커밋 과정에서 Husky pre-commit이 성공했다.

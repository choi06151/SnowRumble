# Task C-31 - PvP 로딩 Ready 핸드셰이크와 PSO 안정화

## 설명

PvP 최초 진입에서 컨트롤러 수가 아니라 각 클라이언트의 맵·Pawn·HUD 초기화와 제한된 Warmup 완료를 기준으로 경기를 시작한다. Ready 타임아웃이면 전체 매치를 취소하고 로비로 복귀한다. PSO Precaching을 활성화하고 실제 플레이 경로의 PSO Cache 수집·패키징 기준을 기록한다.

## 구현 항목

- [x] 클라이언트별 PvP Ready 핸드셰이크를 추가한다.
- [x] 맵 로드·Pawn 준비 후 짧은 Warmup을 거쳐 Ready를 제출한다.
- [x] 서버가 전원 Ready 전에는 팀 소개와 카운트다운을 시작하지 않는다.
- [x] 45초 내 Ready가 완료되지 않으면 전체 매치를 취소하고 로비로 복귀한다.
- [x] PSO Precaching 설정을 활성화한다.
- [x] 실제 플레이 경로 기반 PSO Cache 수집·패키징 설정과 절차를 문서화한다.

## 작업 배정

- 담당자·기능·계약 소유자: 최재원(C)
- 자산 수정자: C++·Config·문서 최재원(C)
- 에디터 자산: 별도 Warmup 레벨을 만들지 않고 현재 PvP 맵 로드 후 제한된 준비 구간을 사용한다.
- 변경 범위: PvP GameMode, PlayerController, Config, Task/Plan 문서

## 범위 밖

- 메인메뉴에 PvP 자산을 숨겨 배치하는 방식
- 셰이더 컴파일을 동기적으로 무기한 대기하는 방식
- Ready 타임아웃 후 일부 인원만으로 경기 시작

## 수동 작업

- PIE 호스트 1명과 클라이언트 3명으로 PvP 진입을 반복한다.
- 일부 클라이언트의 로딩을 지연시켜 45초 타임아웃 시 전원이 로비로 돌아오는지 확인한다.
- 실제 PvP 플레이 경로를 실행해 PSO 기록을 수집하고 패키징 빌드에 포함한다.

## 완료 조건

### 에이전트 확인

- [x] Ready 핸드셰이크·타임아웃 구현
- [x] PSO Precaching 설정 반영
- [x] 정적 점검 통과
- [x] Task 문서 갱신

### 결과 확인

- [ ] 4명 전원 준비 후에만 PvP가 시작된다.
- [ ] 한 명이 준비되지 않으면 45초 후 전원이 로비로 복귀한다.
- [ ] Ready 완료 직후 로딩 화면·팀 소개·카운트다운 순서가 유지된다.
- [ ] PSO Cache 수집 후 패키징 빌드에서 첫 PvP 진입 hitch가 감소한다.

## 구현 기록

- `ASnowRumblePlayerController::TrySchedulePvpReadyHandshake()`가 PvP GameState를 감지하고 맵 변경마다 Ready 상태를 초기화한다.
- Pawn·PlayerState가 준비된 뒤 0.5초 Warmup 후 `ServerNotifyPvpReady()`를 호출한다.
- `ASnowRumbleGameMode::TryDismissLoadingScreens()`는 `ExpectedPlayers`와 Ready PlayerController 수를 함께 확인한다.
- `PvpReadyTimeoutSeconds` 기본값은 45초이며, 실패 시 `ClientShowPersonalTextAlarm()`과 로딩 화면 종료 후 `LobbyReturnTravelUrl`로 복귀한다.
- `Config/DefaultEngine.ini`에서 `r.PSOPrecaching`, `r.PSOPrecache.Components`, `r.PSOPrecache.Resources`, `r.ShaderPipelineCache.Enabled`, `r.ShaderPipelineCache.SaveBoundPSOLog`를 활성화했다.
- `Config/DefaultGame.ini`에서 Material Shader Code 공유 패키징 설정을 활성화했다. 실제 PSO 수집은 PIE/패키징 빌드에서 플레이 경로를 실행해 수행한다.
- 2026-08-25: `SnowRumbleEditor Win64 Development` 빌드 성공.

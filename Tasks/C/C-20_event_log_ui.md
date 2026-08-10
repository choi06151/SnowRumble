# Task C-20 - 이벤트 로그 UI

## 설명

로비와 PvP HUD에서 채팅과 분리된 이벤트 로그를 표시한다. 로그는 TextBlock에 여러 줄로 누적되고 일정 시간이 지나면 오래된 줄부터 사라진다.

## 상태 전이 기준

- 시작 가능: 로비 PlayerState 이름/준비 상태 계약, PvP 얼음/사망 상태 변경 감지, 로컬 UI 위젯 부모 사용 가능
- 완료 가능: 서버 이벤트 전달, 로비/HUD 로그 표시 API, WBP 인계와 결과 확인 절차 기록 완료

## 구현 항목

- [x] 로비 WBP 부모에 이벤트 로그 전용 TextBlock 선택 바인딩을 추가한다.
- [x] Main HUD WBP 부모에 이벤트 로그 전용 TextBlock 선택 바인딩을 추가한다.
- [x] 로그 메시지를 여러 줄로 누적하고 일정 시간이 지나면 제거한다.
- [x] 서버가 모든 클라이언트에 이벤트 로그 메시지를 전달하는 PlayerController RPC를 제공한다.
- [x] 로비에서 닉네임 확정 후 최초 1회 `~~님이 접속했습니다`를 표시한다.
- [x] 로비에서 플레이어가 준비 완료하면 `~~님이 준비 완료했습니다`를 표시한다.
- [x] PvP에서 플레이어가 얼면 `~~님이 얼었습니다`를 표시한다.
- [x] PvP에서 플레이어가 사망하면 `~~님이 죽었습니다`를 표시한다.
- [ ] 아이템 획득 로그는 K 아이템 Task 또는 후속 통합에서 공용 로그 RPC를 호출해 연결한다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: C++·문서 최재원(C), 로비/HUD WBP 배치와 스타일은 사용자 또는 S 인계
- 생성 파일: `Tasks/C/C-20_event_log_ui.md`
- 변경 파일: `Tasks/C/ROLE_C.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `Source/SnowRumble/UI/SnowRumblePlayerController.*`, `Source/SnowRumble/UI/LobbyWidget.*`, `Source/SnowRumble/UI/MainHUDWidget.*`, `Source/SnowRumble/Game/SnowRumblePlayerState.*`, `Source/SnowRumble/Game/SnowRumbleGameMode.*`
- 공유 확인 대상: 사용자, S UI, 추후 K 아이템 로그 연동
- 병합 순서: 공용 로그 RPC와 UI 부모 계약 선행, WBP 배치 후 결과 확인

## 공용 계약과 인계

- 제공할 계약:
  - `ASnowRumblePlayerController::ClientReceiveEventLogMessage(const FText& Message)`: 서버가 로컬 클라이언트 UI에 이벤트 로그를 전달하는 클라이언트 RPC다.
  - `ULobbyWidget::AddEventLogMessage(const FText& Message)`: 로비 로그 TextBlock에 메시지를 추가한다.
  - `UMainHUDWidget::AddEventLogMessage(const FText& Message)`: PvP HUD 로그 TextBlock에 메시지를 추가한다.
  - `ULobbyWidget::EventLogText`: WBP에서 같은 이름으로 만든 TextBlock이 있으면 로비 이벤트 로그를 자동 표시한다.
  - `UMainHUDWidget::EventLogText`: WBP에서 같은 이름으로 만든 TextBlock이 있으면 PvP HUD 이벤트 로그를 자동 표시한다.
  - `EventLogEntryVisibleSeconds`: 각 로그 줄이 유지되는 시간이다. 기본값은 5초다.
- 인계 대상: 사용자, S UI, K 아이템 담당

## 범위 밖

- 로그 전용 행 위젯, 아이콘, 색상 구분, 애니메이션
- 시스템 알람 색상 정책
- K 소유 아이템의 최종 획득 로그 연결
- 로그 저장, 리플레이, 필터링

## 사전 전제

- C-03 로비 이름/준비 상태 계약
- C-06 얼음/사망 상태 변경 이벤트
- C-19 채팅과 별도 UI로 유지

## 결정 필요

- 없음

## 변경 기록

- 2026-08-10: 사용자가 배틀그라운드식 누적 이벤트 로그를 요청해 C-20을 추가하고 구현했다.

## 수동 작업

- `WBP_Lobby` 또는 `ULobbyWidget` 부모 WBP에 TextBlock을 만들고 이름을 `EventLogText`로 맞춘다.
- `WBP_MainHUDWidget` 또는 `UMainHUDWidget` 부모 WBP에 TextBlock을 만들고 이름을 `EventLogText`로 맞춘다.
- 로그 줄 유지 시간을 바꾸려면 각 WBP 기본값에서 `EventLogEntryVisibleSeconds`를 조정한다.
- 로그 TextBlock 위치, 폰트, 색상, 정렬, 애니메이션은 WBP에서 조정한다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드 변경 완료
- [x] `git diff --check` 공백 점검 통과
- [x] `SnowRumbleEditor Win64 Development` 빌드 통과
- [x] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [x] 공용 계약과 캡슐화 규칙 위반 없음
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨

### 검증 메모

- 2026-08-10: 공용 이벤트 로그 RPC, 로비/HUD 로그 TextBlock 누적 표시, 로비 접속/준비 완료, PvP 얼음/사망 로그 연결을 구현했다. `git diff --check`와 `SnowRumbleEditor Win64 Development` 빌드가 통과했다.

### 결과 확인

- [ ] 로비에서 플레이어 닉네임이 확정되면 모든 참가자에게 `~~님이 접속했습니다`가 표시된다.
- [ ] 로비에서 플레이어가 준비 완료하면 모든 참가자에게 `~~님이 준비 완료했습니다`가 표시된다.
- [ ] 로비 로그 줄은 누적 표시되고 약 5초 뒤 오래된 줄부터 사라진다.
- [ ] PvP에서 플레이어가 얼면 모든 참가자 HUD에 `~~님이 얼었습니다`가 표시된다.
- [ ] PvP에서 플레이어가 사망하면 모든 참가자 HUD에 `~~님이 죽었습니다`가 표시된다.
- [ ] PvP HUD 로그 줄은 누적 표시되고 약 5초 뒤 오래된 줄부터 사라진다.

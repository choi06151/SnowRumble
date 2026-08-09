# Task C-05 - 3판 2선승 경기 흐름

## 설명

한 라운드 약 6분의 소프트 목표와 환경 압박을 사용하고 먼저 2승한 팀이 매치에서 승리하는 흐름을 만든다.

## 상태 전이 기준
- 시작 가능: C-04, C-06 완료
- 완료 가능: 라운드·매치 상태 복제와 S-11·K-02 인계 확인

## 구현 항목
- [ ] 라운드 번호, 팀별 승수, 현재 경기 단계와 결과를 서버가 확정한다.
- [ ] 6분이 지나도 강제 종료하지 않고 맵 담당 환경 압박 로직이 사용할 경기 시간·단계 상태를 제공한다.
- [x] 사망 또는 얼음 상태가 아닌 생존자가 한 팀 색에만 남으면 라운드를 종료하고 승리 팀을 확정한다.
- [x] 라운드 종료 후 전체 플레이어 입력을 잠근다.
- [x] HUD WBP의 라운드 종료 패널 표시 바인딩을 제공한다.
- [ ] 5분 30초 마지막 금색 상자 발생 시점을 제공한다.
- [ ] 매치 종료 뒤 대기방 복귀와 방 나가기 결과를 제공한다.

## 작업 배정
- 담당자·기능·계약 소유자: 최재원(C)
- 자산 수정자: C++·문서 최재원(C), 결과 UI/연출은 사용자 또는 S 인계
- 생성 파일: 없음
- 변경 파일: `Source/SnowRumble/Game/SnowRumbleGameMode.*`, `Source/SnowRumble/Game/SnowRumbleGameState_C.*`, `Source/SnowRumble/Player/SnowRumbleHealthComponent.cpp`, `Tasks/C/C-05_round_match_flow.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`
- 공유 확인 대상: K-11, S-11, S-10, J-02, J-04
- 병합 순서: C-04 후, K-02·S-11 전

## 공용 계약과 인계
- 제공받을 계약: C-04 맵 이동, C-06 `IsFrozen()`/`IsDead()` 생명 상태
- 제공할 계약:
  - `ASnowRumbleGameState::IsRoundEnded()`: 현재 PvP 라운드 종료 여부를 반환한다.
  - `ASnowRumbleGameState::GetRoundWinningTeam()`: 현재 라운드 승리 팀 색을 반환한다.
  - `ASnowRumbleGameState::OnRoundResultChanged`: 라운드 결과 복제 갱신을 UI/연출에 알린다.
  - `UMainHUDWidget::EndRoundPanel`: HUD WBP에 같은 이름의 Panel이 있으면 라운드 종료 시 자동 표시한다.
  - `UMainHUDWidget::EndRoundResultText`: HUD WBP에 같은 이름의 TextBlock이 있으면 `{승리팀} 승리` 문구를 자동 표시한다.
  - `ASnowRumbleGameMode::EvaluateRoundEndCondition()`: 서버가 팀별 생존자 상태를 다시 검사한다.
  - 라운드 시간·단계·승수·금색 상자 시점은 C-05 후속 범위에서 제공한다.
- 인계 대상: K-11, S-11, S-10, J-02, J-04, C-12

## 범위 밖
- 결과 화면 레이아웃
- 개별 맵 환경 압박 로직 구현

## 사전 전제
- C-04
- C-06

## 결정 필요
- 라운드 사이 HP·장비·아이템 초기화 범위
- 6분 이후 최대 연장시간 사용 여부

## 수동 작업

- HUD WBP에 라운드 종료 패널을 추가하고 이름을 `EndRoundPanel`로 맞춘다.
- 승리 팀 문구 자동 표시가 필요하면 `EndRoundPanel` 안에 TextBlock을 추가하고 이름을 `EndRoundResultText`로 맞춘다.
- `EndRoundPanel`의 기본 Visibility는 숨김 또는 Collapsed로 둔다.

## 완료 조건
### 에이전트 확인
- [x] 서버 단일 라운드 종료 조건 완료
- [x] 라운드 승리 팀 복제 계약 제공
- [x] HUD 라운드 종료 패널 바인딩 제공
- [x] `git diff --check` 공백 점검 통과
- [x] 관련 C++ 컴파일 통과
- [ ] Unreal Editor 종료 후 `SnowRumbleEditor Win64 Development` 최종 링크 확인
- [ ] 서버 라운드·매치 전체 상태 완료
- [x] 소비 파트 라운드 결과 계약 인계 완료
- [ ] 3판 2선승 상태 전이 정적 점검 완료

### 검증 메모

- 2026-08-10: 서버 `ASnowRumbleGameMode`가 플레이어 얼음/사망 상태 변경 시 라운드 종료 조건을 재검사한다. 사망 또는 얼음 상태가 아닌 생존 플레이어가 한 팀 색에만 남으면 `ASnowRumbleGameState::EndRoundFromServer`로 라운드 승리 팀을 복제하고, `IsMatchInputLocked()`가 true가 되어 전체 입력을 잠근다. `UMainHUDWidget`은 `EndRoundPanel`과 `EndRoundResultText` 선택 바인딩을 제공한다. `git diff --check`는 통과했고 `MainHUDWidget.cpp`를 포함한 관련 C++ 컴파일도 통과했지만, 실행 중인 Unreal Editor가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크는 `LNK1104`로 실패했다. 에디터 종료 후 재빌드 확인이 필요하다.

### 결과 확인

- [ ] 두 팀 이상이 PvP에 있을 때 한 팀을 제외한 모든 팀원이 얼음 또는 사망 상태가 되면 라운드가 종료된다.
- [ ] 얼음 상태도 생존자로 계산되지 않는다.
- [ ] 사망 상태도 생존자로 계산되지 않는다.
- [ ] 라운드 종료 후 승리 팀 색이 `GetRoundWinningTeam()`으로 읽힌다.
- [ ] 라운드 종료 후 모든 플레이어 입력이 잠긴다.
- [ ] HUD WBP에 `EndRoundPanel`이 있으면 라운드 종료 후 패널이 표시된다.
- [ ] HUD WBP에 `EndRoundResultText`가 있으면 승리 팀 문구가 표시된다.

# Task C-29 - 전환 URL과 PvP 로딩 안정화

## 설명

메인메뉴, 로비, PvP, 포디움, 로비 복귀 흐름에서 맵별 WorldSettings 의존을 줄이고, PvP 시작 전 로딩창이 travel 중 검은 화면을 덮은 뒤 팀 소개 시퀀서 시작 직전까지 유지되게 한다.

## 상태 전이 기준

- 시작 가능: C-04 랜덤 PvP 맵, C-05 라운드/매치 흐름, C-17 PvP 시작 카운트다운의 기존 C++ 계약이 존재한다.
- 완료 가능: 메인메뉴->로비, 로비->PvP, PvP->포디움, 포디움->로비 travel URL이 대상 GameMode를 강제하고, PvP 로딩창은 레벨 로딩 중 MoviePlayer 화면으로 검은 화면을 덮은 뒤 모든 예상 플레이어 접속 후 팀 소개 시퀀서 시작 직전에 닫힌다.

## 구현 항목

- [x] 로비에서 랜덤 PvP 맵을 선택해도 `BP_SnowRumblePVPGameMode`를 travel URL에 강제로 추가한다.
- [x] 메인메뉴에서 호스트 로비를 열 때 `BP_LobbyGameMode`를 travel URL에 강제로 추가한다.
- [x] PvP 다음 라운드, 타이브레이커, 포디움 이동 URL에 대상 GameMode와 `ExpectedPlayers`를 중복 없이 붙인다.
- [x] 포디움에서 로비 복귀 URL에 `BP_LobbyGameMode`를 강제로 추가한다.
- [x] PvP 로딩창은 접속 완료 즉시 닫지 않고, 팀 소개 시퀀서 시작 직전에 닫는다.
- [x] LAN 세션 광고 맵을 호스트의 현재 맵으로 갱신하고, 메인메뉴 참가 요청은 `L_Lobby` 대기방 세션만 허용한다.
- [x] PvP travel 중 렌더가 멈추는 구간은 `MoviePlayer` Slate 로딩 화면으로 덮고, 맵 로드 후에는 기존 WBP 로딩창을 다시 viewport에 붙인다.
- [x] 로딩 UI에 선택된 랜덤 맵 표시명·이미지와 로컬 플레이어 기준 같은 팀 이름 목록을 제공한다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: 없음
- 생성·변경 후보: `Source/SnowRumble/Game/*GameMode*`, `Source/SnowRumble/Online/SnowRumbleSessionSubsystem.cpp`, `Source/SnowRumble/UI/LoadingScreenSubsystem.*`, `Source/SnowRumble/SnowRumble.Build.cs`
- 공유 확인 대상: S, J는 맵 WorldSettings가 누락돼도 C++ travel URL이 GameMode를 강제한다는 점을 확인한다.
- 병합 순서: C 전환 계약 선반영 후 맵별 WorldSettings 정리는 각 맵 담당자가 후속으로 맞춘다.

## 공용 계약과 인계

- 제공받을 계약: C-04 PvP 후보 맵 선택, C-05 매치 종료/포디움 이동, C-17 카운트다운 시작 계약.
- 제공할 계약: 모든 서버 travel URL은 코드에서 목적 GameMode를 강제하고, 로딩창 종료는 팀 소개 시퀀서 시작 직전 서버가 제어한다. 클라이언트 로딩 UI는 travel 중 `MoviePlayer` Slate 화면을 사용하고, 맵 로드 후 기존 WBP를 복구한다.
- 인계 대상: 맵 담당자는 PlayerStart와 WorldSettings를 계속 정리하되, GameMode 누락이 전환 실패의 단일 원인이 되지 않는다.

## 범위 밖

- 메인메뉴 세션 생성/참가 UI 재작성.
- PvP 후보 맵의 PlayerStart 재배치나 배치 캐릭터 제거.
- 포디움 UI/카메라 연출 변경.
- 로딩 화면의 최종 커스터마이징 UI. 후속으로 오른쪽 하단에 같은 팀 플레이어 이름 목록을 표시할 수 있게 한다.
- 로딩 화면의 최종 아트 제작과 세부 스타일링.

## 사전 전제

- 로비 GameMode, PvP GameMode, Podium GameMode Blueprint 경로가 현재 프로젝트 경로를 유지한다.

## 결정 필요

- 없음. 사용자가 랜덤 맵 유지와 강제 URL 방식을 승인했다.
- 없음. 맵별 이미지는 로비 GameMode Blueprint의 `PvPLevelLoadingPresentations`에서 레벨 기준으로 지정하고, 같은 팀 이름 목록은 로비 PlayerState의 팀 배정과 이름을 기준으로 서버가 클라이언트별로 내려준다.

## 변경 기록

- 2026-08-21: 사용자가 전환 불안정 원인 확인 후 랜덤 PvP 맵과 포디움/로비 복귀 URL의 GameMode 강제를 승인해 Task를 추가하고 구현했다.
- 2026-08-21: 메인메뉴->로비 호스트 이동 경로도 `/Game/Maps/L_Lobby?listen`만 쓰고 있음을 확인해 `BP_LobbyGameMode` 강제 옵션을 추가했다.
- 2026-08-21: PvP 로딩창 종료 타이밍을 서버 확정 3-2-1 카운트다운 직전에서 팀 소개 시퀀서 시작 직전으로 옮겼다.
- 2026-08-21: 빠른참가가 인원만 보고 첫 LAN 세션에 붙어 이미 다른 맵에 있는 에디터 세션으로 이동할 수 있어, 검색 결과에 `SETTING_MAPNAME`을 포함하고 호스트 맵 전환마다 세션 광고 맵과 join-in-progress 허용 여부를 갱신하게 했다.
- 2026-08-21: 로비->PvP hard travel 중 검은 화면이 보이지 않도록 `ULoadingScreenSubsystem`에 `MoviePlayer` Slate 로딩 화면을 추가했다. travel 중 progress는 시간 기반으로 자연스럽게 90%까지 차고, 맵 로드 후 기존 WBP가 실제 접속 progress를 이어받는다.
- 2026-08-21: 사용자가 후속 로딩바 커스터마이징 방향으로 오른쪽 하단 같은 팀 플레이어 이름 목록 표시를 요청했다.
- 2026-08-21: 사용자가 후속 로딩 화면 커스터마이징 방향으로 랜덤 선택된 맵에 따른 이미지 설정을 요청했다.
- 2026-08-21: `PvPLevelLoadingPresentations`를 추가해 선택 맵별 표시명과 Texture2D 이미지를 설정할 수 있게 했다. 로비 GameMode는 매치 시작 직전 각 클라이언트에 선택 맵 정보와 같은 팀 플레이어 이름 목록을 전달하고, MoviePlayer Slate 화면은 travel 중 이를 표시한다.

## 수동 작업

- Unreal Editor에서 로비 호스트와 클라이언트가 게임 시작을 눌렀을 때 PvP 로딩창이 모든 플레이어 접속 후에도 유지되고, 팀 소개 시퀀서 시작 직전에 닫히는지 확인한다.
- 로비->PvP 이동 중 검은 화면 대신 MoviePlayer 로딩 화면이 전체 화면을 덮고, progress가 멈춘 것처럼 보이지 않고 자연스럽게 차는지 확인한다.
- 로비 GameMode Blueprint의 `PvPLevelLoadingPresentations`에 후보 맵과 이미지를 지정한 뒤, 랜덤 선택된 맵의 이미지와 같은 팀 이름 목록이 로딩 화면에 표시되는지 확인한다.
- Unreal Editor 두 개를 켠 상태에서 한쪽이 로비가 아닌 맵에 있을 때 빠른참가가 해당 세션을 건너뛰고, `L_Lobby` 대기방 세션만 참가하는지 확인한다.
- PvP 매치 종료 후 포디움, 포디움 10초 후 로비 복귀가 각각 올바른 GameMode로 실행되는지 확인한다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드 변경 완료
- [x] `git diff --check` 기준 정적 점검 완료
- [x] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [x] 공용 계약과 캡슐화 규칙 위반 없음
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인

- [ ] 사용자 또는 담당자가 멀티플레이 수동 실행으로 메인메뉴->로비->PvP->포디움->로비 흐름을 확인한다.

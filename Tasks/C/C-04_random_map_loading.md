# Task C-04 - 랜덤 맵과 로딩 동기화

## 설명

대기방에서 맵을 고르지 않고 서버가 라운드 맵을 무작위로 결정하며 참가자 로딩 완료 또는 30초 제한 뒤 경기를 시작한다.

## 상태 전이 기준
- 시작 가능: C-03 완료, 플레이할 맵 자산 식별
- 완료 가능: 랜덤 선택·로딩 상태·실패 처리와 S-04 인계 확인

## 구현 항목
- [x] 서버가 등록된 PvP 후보 레벨 중 진입 맵을 무작위로 선택한다.
- [x] 후보가 2개 이상이면 직전 선택 레벨을 다음 선택 후보에서 제외한다.
- [x] 다음 라운드가 남아 있으면 같은 후보 목록에서 다음 PvP 레벨을 다시 선택해 이동한다.
- [ ] 맵 정보와 참가자별 로딩 완료 상태를 제공한다.
- [ ] 전원 완료 또는 30초 경과 후 라운드를 시작하고 미완료 참가자를 연결 해제로 처리한다.

## 작업 배정
- 담당자·기능·계약 소유자: 최재원(C)
- 자산 수정자: C++·문서 최재원(C), PvP 후보 레벨 자산 지정은 사용자/S/J
- 변경 파일: `Source/SnowRumble/Game/SnowRumbleLobbyGameMode.h`, `Source/SnowRumble/Game/SnowRumbleLobbyGameMode.cpp`, `Source/SnowRumble/Game/SnowRumbleMatchSubsystem_C.h`, `Source/SnowRumble/Game/SnowRumbleMatchSubsystem_C.cpp`, `Tasks/C/C-04_random_map_loading.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`
- 공유 확인 대상: S-04, S-10, J-01, J-03
- 병합 순서: C-03 후, C-05·S-04 전

## 공용 계약과 인계
- 제공받을 계약: C-03 시작 요청, S·J 맵 자산
- 제공할 계약: `ASnowRumbleLobbyGameMode::PvPLevelCandidates`에 등록된 레벨 중 서버가 선택한 PvP travel URL, `USnowRumbleMatchSubsystem::SelectNextPvPLevelPath()` 다음 라운드 레벨 선택, 로딩 인원 진행률
- 인계 대상: S-04, S-10, J-01, J-03, C-05

## 범위 밖
- 맵 레벨 디자인과 로딩 화면 그래픽

## 사전 전제
- C-03

## 결정 필요
- 세 맵에서 직전 맵 제외만 적용할지 한 매치 내 완전 중복 금지할지

## 수동 작업
- 로비 GameMode Blueprint 또는 해당 기본 클래스 설정에서 `PvPLevelCandidates` 배열에 PvP 후보 레벨을 추가한다.
- 후보 레벨은 실제 travel 가능한 레벨 자산으로 지정한다. 비어 있으면 기존 `MatchTravelUrl` 값인 `/Game/LowpolyStyle/WinterEnvironment/Maps/DemoMap?listen`로 이동한다.

## 완료 조건
### 에이전트 확인
- [x] 서버 랜덤 PvP 레벨 선택 계약 첫 범위 완료
- [x] 다음 라운드 랜덤 PvP 레벨 재선택 계약 제공
- [ ] 실패·시간초과 경로 점검 완료
- [x] S/J 후보 레벨 지정 인계 기록
### 결과 확인
- [ ] Listen Server 호스트와 클라이언트 1명으로 로비에 입장한다.
- [ ] 로비 GameMode Blueprint의 `PvPLevelCandidates`에 후보 레벨을 2개 이상 등록한 뒤 호스트가 `게임 시작`을 누른다.
- [ ] 로딩창 이후 등록된 후보 중 하나의 PvP 레벨로 이동하는지 확인한다.
- [ ] 여러 번 시작해 후보가 랜덤하게 선택되고, 후보가 2개 이상일 때 직전 선택 레벨이 가능한 한 연속 선택되지 않는지 확인한다.

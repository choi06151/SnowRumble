# Task C-05 - 1/3/5 라운드 경기 흐름

## 설명

로비에서 설정한 1/3/5 총 라운드 수를 기준으로 PvP 라운드를 반복하고, 모든 라운드가 끝났을 때 누적 승수가 가장 높은 팀을 매치 1등으로 확정한다.

## 상태 전이 기준
- 시작 가능: C-04, C-06 완료
- 완료 가능: 라운드·매치 상태 복제와 S-11·K-02 인계 확인

## 구현 항목
- [x] 라운드 번호, 총 라운드 수, 팀별 승수, 라운드 결과와 매치 결과를 서버가 확정한다.
- [x] 라운드 승리 팀의 매치 점수를 1점 올리고 팀별 점수를 복제한다.
- [x] HUD WBP의 팀별 ScoreText 선택 바인딩을 제공한다.
- [x] 현재 PvP에 참가 중인 팀의 점수만 표시한다.
- [x] 6분이 지나도 강제 종료하지 않고 맵 담당 환경 압박 로직이 사용할 경기 시간·단계 상태를 제공한다.
- [x] 로비 게시판에서 선택한 게임 속도에 따라 맵 축소 주기를 느리게 90초, 보통 60초, 빠르게 30초로 확정한다.
- [x] PvP HUD가 현재 경기 시간과 다음 맵 축소까지 남은 시간을 표시할 TextBlock 바인딩을 제공한다.
- [x] 맵 축소 시점에 PvP GameMode Blueprint가 받을 이벤트를 호출하고, 임시로 5초 후 축소 완료를 가정해 다음 주기를 시작한다.
- [x] 사망 또는 얼음 상태가 아닌 생존자가 한 팀 색에만 남으면 라운드를 종료하고 승리 팀을 확정한다.
- [x] PvP 시작 전 로딩·카운트다운 입력 잠금 중에는 라운드 종료 판정을 실행하지 않는다.
- [x] 라운드 종료 후 전체 플레이어 입력을 잠근다.
- [x] HUD WBP의 라운드 종료 패널 표시 바인딩을 제공한다.
- [ ] 5분 30초 마지막 금색 상자 발생 시점을 제공한다.
- [x] 라운드가 남아 있으면 결과 표시 후 다음 랜덤 PvP 맵으로 이동한다.
- [x] 모든 정규 라운드 종료 시 공동 1등이 있으면 단판 승부 전용 PvP 맵으로 이동한다.
- [x] 단판 승부 중에는 공동 1등 대상 팀끼리의 피해만 유효하게 처리한다.
- [x] 단판 승부 대상이 아닌 팀은 관전자 상태로 전환해 이동·상호작용·충돌 간섭을 막고 경기 참가자 시점을 보게 한다.
- [x] 단판 승부 라운드에서는 맵 축소 기믹을 사용하지 않고 경기 시간과 아이템 스폰은 유지한다.
- [x] 매치 종료 뒤 포디움 레벨로 이동해 현재 매치 참가 팀만 기준으로 1~3등을 표시하고 10초 뒤 로비로 복귀한다.
- [x] 매치 종료 뒤 대기방 복귀를 제공한다.
- [ ] 방 나가기 결과를 제공한다.

## 작업 배정
- 담당자·기능·계약 소유자: 최재원(C)
- 자산 수정자: C++·문서 최재원(C), 결과 UI/연출은 사용자 또는 S 인계
- 생성 파일: `Source/SnowRumble/Game/SnowRumbleMatchSubsystem_C.h`, `Source/SnowRumble/Game/SnowRumbleMatchSubsystem_C.cpp`, `Source/SnowRumble/Game/PodiumGameMode.*`, `Source/SnowRumble/Game/PodiumPlayerController.*`, `Source/SnowRumble/UI/PodiumWidget.*`
- 변경 파일: `Source/SnowRumble/Game/SnowRumbleGameMode.*`, `Source/SnowRumble/Game/SnowRumbleGameState_C.*`, `Source/SnowRumble/Game/SnowRumbleLobbyGameMode.*`, `Source/SnowRumble/Game/SnowRumbleLobbyGameState.*`, `Source/SnowRumble/UI/LobbyBoardWidget_C.*`, `Source/SnowRumble/UI/MainHUDWidget.cpp`, `Source/SnowRumble/Player/SnowRumbleHealthComponent.cpp`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Tasks/C/C-05_round_match_flow.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`
- 공유 확인 대상: K-11, S-11, S-10, J-02, J-04
- 병합 순서: C-04 후, K-02·S-11 전

## 공용 계약과 인계
- 제공받을 계약: C-04 맵 이동, C-06 `IsFrozen()`/`IsDead()` 생명 상태
- 제공할 계약:
  - `ASnowRumbleGameState::IsRoundEnded()`: 현재 PvP 라운드 종료 여부를 반환한다.
  - `ASnowRumbleGameState::GetRoundWinningTeam()`: 현재 라운드 승리 팀 색을 반환한다.
  - `ASnowRumbleGameState::GetTeamRoundWinCount(ESnowRumbleTeam Team)`: 현재 매치에서 해당 팀이 획득한 라운드 승수를 반환한다.
  - `ASnowRumbleGameState::GetCurrentRoundNumber()`: 현재 라운드 번호를 반환한다.
  - `ASnowRumbleGameState::GetRoundLimit()`: 로비에서 설정한 총 라운드 수를 반환한다.
  - `ASnowRumbleGameState::IsMatchEnded()`: 모든 라운드가 끝나 매치 결과가 확정됐는지 반환한다.
  - `ASnowRumbleGameState::IsTiebreakerRound()`: 현재 PvP 라운드가 공동 1등 단판 승부인지 반환한다.
  - `ASnowRumbleGameState::GetMatchWinningTeam()`: 매치 1등 팀 색을 반환한다.
  - `ASnowRumbleGameState::OnRoundResultChanged`: 라운드 결과 복제 갱신을 UI/연출에 알린다.
  - `ASnowRumbleLobbyGameState::GetMatchRoundLimit()`: 로비에서 선택한 총 라운드 수를 반환한다.
  - `ASnowRumbleLobbyGameState::GetGameSpeed()`: 로비에서 선택한 게임 속도 `Slow`, `Normal`, `Fast`를 반환한다.
  - `ASnowRumbleLobbyGameState::GetAssignedLobbyTeamCount()`: 현재 유효하게 선택된 팀 색 수를 반환한다.
  - `ASnowRumbleLobbyGameState::GetStartMatchInvalidReasonText()`: 시작 불가 사유를 UI 표시용 텍스트로 반환한다.
  - `ASnowRumbleLobbyGameState::SetMatchRoundLimitFromServer(int32 NewRoundLimit)`: 서버가 총 라운드 수를 1, 3, 5 중 하나로 변경한다.
  - `ASnowRumbleLobbyGameState::SetGameSpeedFromServer(ESnowRumbleGameSpeed NewGameSpeed)`: 서버가 게임 속도를 변경한다.
  - `USnowRumbleMatchSubsystem::GetGameSpeed()`: travel 사이 유지되는 현재 매치 게임 속도를 반환한다.
  - `USnowRumbleMatchSubsystem::StartTiebreakerForLeadingTie()`: 정규 라운드가 모두 끝난 뒤 공동 1등 팀이 2팀 이상이면 단판 승부 상태로 전환한다.
  - `USnowRumbleMatchSubsystem::IsTiebreakerActive()`: travel 사이 유지되는 단판 승부 진행 여부를 반환한다.
  - `USnowRumbleMatchSubsystem::IsTiebreakerTeam(ESnowRumbleTeam Team)`: 해당 팀이 현재 단판 승부 대상 팀인지 반환한다.
  - `USnowRumbleMatchSubsystem::GetMapShrinkIntervalSeconds(ESnowRumbleGameSpeed GameSpeed)`: 속도별 맵 축소 주기를 반환한다. `Slow=90`, `Normal=60`, `Fast=30`초다.
  - `ULobbyWidget::MatchRoundLimitText`: `WBP_Lobby`에 같은 이름의 TextBlock이 있으면 로비에서 선택한 총 라운드 수를 표시한다.
  - `ULobbyWidget::GameSpeedText`: `WBP_Lobby`에 같은 이름의 TextBlock이 있으면 현재 게임 속도를 `느리게`, `보통`, `빠르게` 중 하나로 표시한다.
  - `ULobbyWidget::InvalidActionAnimation`: `WBP_Lobby`에 같은 이름의 Widget Animation이 있으면 시작 불가 상태에서 시작 요청 시 재생한다.
  - `ULobbyWidget::InvalidActionReasonText`: `WBP_Lobby`에 같은 이름의 TextBlock이 있으면 시작 불가 사유를 C++가 표시한다.
  - `ULobbyWidget::OnInvalidActionFeedback(const FText& ReasonText)`: `WBP_Lobby`가 사유 텍스트를 받아 직접 패널 표시나 애니메이션 재생을 연결할 수 있다.
  - `ALobbyPlayerController::ShowLobbyInvalidActionFeedback(const FText& ReasonText)`: 게시판 월드 위젯에서 발생한 예외행동 사유를 로컬 `WBP_Lobby`에 전달한다.
  - `ALobbyPlayerController::LobbyEscapeMenuWidgetClass`: 로비에서 ESC를 눌렀을 때 생성할 전용 메뉴 WBP 클래스다.
  - `ALobbyPlayerController::MainMenuTravelUrl`: ESC 메뉴에서 메인메뉴로 이동하기를 눌렀을 때 사용할 travel URL이다. 기본값은 `/Game/Maps/L_MainMenu`이다.
  - `ULobbyEscapeMenuWidget::ReturnToMainMenuButton`, `SettingsButton`, `InviteFriendsButton`, `BackButton`: 로비 ESC 메뉴 WBP에 같은 이름의 Button이 있으면 자동 연결된다.
  - `ULobbyEscapeMenuWidget::ReturnToLobbyButton`: 기존 WBP 이름 호환용 메인메뉴 이동 Button이다.
  - `ULobbyEscapeMenuWidget::OnReturnToMainMenuRequested()`, `OnSettingsRequested()`, `OnInviteFriendsRequested()`: 로비 ESC 메뉴 버튼 클릭을 Blueprint가 필요하면 처리한다.
  - `ULobbyBoardWidget::Round1Button`, `Round3Button`, `Round5Button`: WBP에 같은 이름의 Button이 있으면 호스트가 총 라운드 수를 설정한다.
  - `ULobbyBoardWidget::SlowGameSpeedButton`, `NormalGameSpeedButton`, `FastGameSpeedButton`: WBP에 같은 이름의 Button이 있으면 호스트가 게임 속도를 느리게, 보통, 빠르게로 설정한다.
  - `ULobbyBoardWidget::Shuffle2TeamsButton`, `Shuffle3TeamsButton`, `Shuffle4TeamsButton`: WBP에 같은 이름의 Button이 있으면 호스트가 현재 로비 인원을 2/3/4팀으로 무작위 균등 배정한다.
  - `ULobbyBoardWidget::ShuffleSoloButton`: WBP에 같은 이름의 Button이 있으면 호스트가 현재 로비 인원을 모두 서로 다른 팀 색으로 무작위 배정한다.
  - `ULobbyBoardWidget::MatchRoundLimitText`: WBP에 같은 이름의 TextBlock이 있으면 현재 총 라운드 수를 표시한다.
  - `ULobbyBoardWidget::InvalidActionAnimation`: 게시판 WBP에 같은 이름의 Widget Animation이 있으면 클라이언트가 방 설정을 변경하려 하거나 호스트가 시작 불가 상태에서 시작을 누를 때 재생한다.
  - `ULobbyBoardWidget::InvalidActionReasonText`: 게시판 WBP에 같은 이름의 TextBlock이 있으면 예외행동 사유를 C++가 표시한다.
  - `ULobbyBoardWidget::OnInvalidActionFeedback(const FText& ReasonText)`: 게시판 WBP가 사유 텍스트를 받아 직접 패널 표시나 애니메이션 재생을 연결할 수 있다.
  - `UMainHUDWidget::CurrentRoundText`: `WBP_MainHUDWidget`에 같은 이름의 TextBlock이 있으면 현재 라운드를 `라운드 {현재} / {전체}` 형식으로 표시하고, 단판 승부 라운드에서는 `단판승부`로 표시한다.
  - `UMainHUDWidget::MatchElapsedTimeText`: `WBP_MainHUDWidget`에 같은 이름의 TextBlock이 있으면 PvP 시작 후 현재 경기 시간을 `경기 시간 0:00` 형식으로 표시한다.
  - `UMainHUDWidget::MapShrinkCountdownText`: `WBP_MainHUDWidget`에 같은 이름의 TextBlock이 있으면 `{초}초 후 맵이 축소됩니다` 또는 `맵이 축소됩니다!`를 표시한다. 단판 승부 라운드에서는 숨긴다.
  - `UMainHUDWidget::EndRoundPanel`: HUD WBP에 같은 이름의 Panel이 있으면 라운드 종료 시 자동 표시한다.
  - `UMainHUDWidget::EndRoundResultText`: HUD WBP에 같은 이름의 TextBlock이 있으면 `{승리팀} 승리` 문구를 자동 표시한다.
  - `UMainHUDWidget::RedTeamScoreText`, `SkyTeamScoreText`, `GreenTeamScoreText`, `YellowTeamScoreText`, `PurpleTeamScoreText`, `PinkTeamScoreText`, `BlueTeamScoreText`, `WhiteTeamScoreText`: HUD WBP에 같은 이름의 TextBlock이 있으면 해당 팀의 라운드 승수를 자동 표시한다.
  - `UMainHUDWidget::RedTeamScoreRow`, `SkyTeamScoreRow`, `GreenTeamScoreRow`, `YellowTeamScoreRow`, `PurpleTeamScoreRow`, `PinkTeamScoreRow`, `BlueTeamScoreRow`, `WhiteTeamScoreRow`: HUD WBP에 같은 이름의 Row 위젯이 있으면 참가하지 않은 팀 Row를 숨긴다. Row가 없으면 팀별 ScoreText 자체를 숨김 대상으로 사용한다.
  - `ASnowRumbleGameMode::EvaluateRoundEndCondition()`: 서버가 팀별 생존자 상태를 다시 검사한다.
  - `ASnowRumbleGameMode::OnMapShrinkRequested(int32 ShrinkStage, float RoundElapsedSeconds, float ShrinkDurationSeconds)`: 서버가 맵 축소 시점에 PvP GameMode Blueprint로 호출하는 이벤트다. 실제 맵 축소 로직은 J 또는 맵 담당 Blueprint가 구현한다.
  - `ASnowRumbleGameMode::CompleteMapShrinkFromBlueprint()`: 맵 Blueprint가 실제 맵 축소 완료 시 호출할 수 있는 완료 신호다. 현재는 완료 신호가 없으므로 C++가 `TemporaryMapShrinkDurationSeconds` 기본 5초 후 자동 완료 처리한다.
  - `ASnowRumbleGameState::GetRoundElapsedSeconds()`, `GetRoundElapsedTimeText()`: PvP 시작 카운트다운 종료 후 경과 시간을 반환한다.
  - `ASnowRumbleGameState::GetSecondsUntilNextMapShrink()`, `GetMapShrinkCountdownText()`, `IsMapShrinkInProgress()`: HUD와 맵 표현이 다음 축소 상태를 읽는 계약이다.
  - `ASnowRumbleGameState::GetGameSpeed()`, `GetMapShrinkIntervalSeconds()`: 현재 속도와 축소 주기를 읽는다.
  - `ASnowRumbleGameMode::LobbyReturnTravelUrl`: 모든 라운드가 끝난 뒤 복귀할 로비 맵 travel URL이다. 기본값은 `/Game/Maps/L_Lobby?listen`이다.
  - `ASnowRumbleGameMode::TiebreakerTravelUrl`: 공동 1등 발생 시 이동할 단판 승부 전용 PvP 맵 travel URL이다. 기본값은 `/Game/Maps/L_Tiebreaker?listen`이다.
  - `ASnowRumbleGameMode::PodiumTravelUrl`: 매치 종료 뒤 이동할 포디움 맵 travel URL이다. 기본값은 `/Game/Maps/L_Podium?listen`이며 `ExpectedPlayers` 옵션을 함께 넘긴다.
  - `APodiumGameMode`: 포디움 맵 전용 GameMode다. 서버가 매치에 실제 참가한 팀 색만 수집해 누적 라운드 승수 내림차순으로 1~3등을 정하고, 포디움 배치와 UI 결과 문구를 확정한다.
  - `APodiumGameMode::PodiumLobbyReturnTravelUrl`: 포디움 결과 표시 후 복귀할 로비 travel URL이다. 기본값은 `/Game/Maps/L_Lobby?listen`이다.
  - `APodiumGameMode::PodiumReturnDelaySeconds`: 포디움 결과를 보여준 뒤 로비로 돌아가기까지의 시간이다. 기본값은 10초다.
  - `APodiumPlayerController::PodiumWidgetClass`: 포디움에서 로컬 플레이어에게 표시할 `UPodiumWidget` 기반 WBP 클래스다.
  - `UPodiumWidget::FirstPlaceText`, `SecondPlaceText`, `ThirdPlaceText`, `SubtitleText`: 같은 이름의 TextBlock이 WBP에 있으면 C++가 1~3등 팀과 10초 후 복귀 안내를 자동 표시한다.
  - `ASnowRumbleCharacter::PlayServerDirectedEmote(int32 EmoteIndex)`: 서버 전용 연출 흐름에서 확정한 이모션을 모든 화면에 재생한다.
  - 라운드 시간·단계·금색 상자 시점은 C-05 후속 범위에서 제공한다.
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
- 팀별 승리 점수 표시가 필요하면 `WBP_MainHUDWidget`에 VerticalBox를 배치하고, 각 팀 TextBlock 이름을 `RedTeamScoreText`, `SkyTeamScoreText`, `GreenTeamScoreText`, `YellowTeamScoreText`, `PurpleTeamScoreText`, `PinkTeamScoreText`, `BlueTeamScoreText`, `WhiteTeamScoreText` 중 필요한 팀 이름으로 맞춘다.
- 팀 이름과 점수를 한 줄로 묶어 표시하려면 각 줄의 루트 위젯 이름을 `RedTeamScoreRow`, `SkyTeamScoreRow`, `GreenTeamScoreRow`, `YellowTeamScoreRow`, `PurpleTeamScoreRow`, `PinkTeamScoreRow`, `BlueTeamScoreRow`, `WhiteTeamScoreRow`로 맞춘다. C++는 참가하지 않은 팀 Row만 숨기고 Row 순서는 바꾸지 않는다.
- 로비 게시판 WBP에서 라운드 수 설정이 필요하면 Button 이름을 `Round1Button`, `Round3Button`, `Round5Button`으로 맞춘다.
- 로비 게시판 WBP에서 게임 속도 설정이 필요하면 Button 이름을 `SlowGameSpeedButton`, `NormalGameSpeedButton`, `FastGameSpeedButton`으로 맞춘다.
- 로비 게시판 WBP에서 팀 섞기 버튼이 필요하면 Button 이름을 `Shuffle2TeamsButton`, `Shuffle3TeamsButton`, `Shuffle4TeamsButton`으로 맞춘다.
- 로비 게시판 WBP에서 개인전 섞기 버튼이 필요하면 Button 이름을 `ShuffleSoloButton`으로 맞춘다.
- 로비 게시판 WBP에서 현재 라운드 수 표시가 필요하면 TextBlock 이름을 `MatchRoundLimitText`로 맞춘다.
- 기존 `WBP_Lobby`에서 현재 라운드 수 표시가 필요하면 TextBlock 이름을 `MatchRoundLimitText`로 맞춘다.
- 기존 `WBP_Lobby`에서 현재 게임 속도 표시가 필요하면 TextBlock 이름을 `GameSpeedText`로 맞춘다.
- 기존 `WBP_Lobby`에서 시작 불가 피드백이 필요하면 Widget Animation 이름을 `InvalidActionAnimation`으로 맞춘다.
- 기존 `WBP_Lobby`에서 시작 불가 사유 텍스트가 필요하면 TextBlock 이름을 `InvalidActionReasonText`로 맞춘다.
- 이름 바인딩 대신 Blueprint에서 직접 처리하려면 `WBP_Lobby`의 `OnInvalidActionFeedback` 이벤트에서 사유 텍스트 표시와 애니메이션 재생을 연결한다.
- 로비 ESC 메뉴 WBP를 만들고 `LobbyPlayerController`의 `LobbyEscapeMenuWidgetClass`에 연결한다.
- 로비 ESC 메뉴 WBP에는 Button 이름을 `ReturnToMainMenuButton`, `SettingsButton`, `InviteFriendsButton`, `BackButton`으로 맞춘다. 기존 `ReturnToLobbyButton` 이름도 메인메뉴 이동 버튼으로 호환된다.
- 설정, 친구 부르기, 메인메뉴로 이동하기의 추가 세부 동작이 필요하면 로비 ESC 메뉴 WBP에서 `OnSettingsRequested`, `OnInviteFriendsRequested`, `OnReturnToMainMenuRequested` 이벤트를 구현한다.
- 친구 부르기 버튼은 현재 LAN 개발 중에는 `OnInviteFriendsRequested` 이벤트 계약만 유지한다. Steam Overlay 친구 초대와 초대 수락 연결은 C-18 최종 Steam 세션 통합 Task에서 처리한다.
- 로비 게시판에서 발생한 예외행동도 로컬 `WBP_Lobby`의 `InvalidActionAnimation`과 `InvalidActionReasonText`로 전달된다.
- 로비 게시판 WBP 자체에도 별도 피드백을 띄우고 싶으면 같은 이름의 `InvalidActionAnimation`, `InvalidActionReasonText`를 추가할 수 있다.
- PvP HUD 중앙 상단에 현재 라운드를 표시하려면 `WBP_MainHUDWidget`에 TextBlock을 배치하고 이름을 `CurrentRoundText`로 맞춘다.
- PvP HUD에 현재 경기 시간을 표시하려면 `WBP_MainHUDWidget`에 TextBlock을 배치하고 이름을 `MatchElapsedTimeText`로 맞춘다.
- PvP HUD에 다음 맵 축소 안내를 표시하려면 `WBP_MainHUDWidget`에 TextBlock을 배치하고 이름을 `MapShrinkCountdownText`로 맞춘다.
- PvP GameMode Blueprint 또는 맵별 GameMode Blueprint에서 `OnMapShrinkRequested` 이벤트를 구현하면 실제 맵 축소를 시작할 수 있다.
- 실제 맵 축소가 완료되는 시점에 `CompleteMapShrinkFromBlueprint()`를 호출하면 5초 임시 완료 타이머 대신 완료 신호 기준으로 다음 축소 주기가 시작된다.
- 단판 승부 전용 작은 PvP 맵을 만들고, PvP GameMode Blueprint의 `TiebreakerTravelUrl`에 해당 맵 travel URL을 넣는다.
- 단판 승부 맵은 같은 GameMode, Controller, HUD를 사용한다.
- 단판 승부 맵에도 기존 선물상자 TargetPoint를 배치하면 아이템 스폰은 정규 PvP와 동일하게 동작한다.
- 포디움 맵 `L_Podium`에는 `BP_SnowRumblePodiumGameMode` 또는 `APodiumGameMode` 기반 GameMode를 지정한다.
- 포디움 맵의 PlayerStart에는 1등 자리 `Podium_Team1`, 2등 자리 `Podium_Team2`, 3등 자리 `Podium_Team3` 태그를 붙인다. 같은 등수에 팀원이 여러 명이면 같은 태그 PlayerStart를 여러 개 배치한다.
- 포디움 카메라는 CameraActor에 `Podium_Camera` 태그를 붙인다.
- 포디움 UI WBP는 `UPodiumWidget`을 부모로 만들고, 자동 텍스트 표시가 필요하면 `FirstPlaceText`, `SecondPlaceText`, `ThirdPlaceText`, `SubtitleText` TextBlock을 배치한다.
- `BP_SnowRumblePodiumController` 또는 `APodiumPlayerController` 기본값에서 `PodiumWidgetClass`에 포디움 UI WBP를 지정한다.

## 완료 조건
### 에이전트 확인
- [x] 서버 단일 라운드 종료 조건 완료
- [x] 라운드 승리 팀 복제 계약 제공
- [x] 팀별 라운드 승수 복제 계약 제공
- [x] HUD 라운드 종료 패널 바인딩 제공
- [x] HUD 팀별 점수 TextBlock 바인딩 제공
- [x] HUD 참가 팀만 표시 제공
- [x] `git diff --check` 공백 점검 통과
- [x] 관련 C++ 컴파일 통과
- [x] `SnowRumbleEditor Win64 Development` 최종 빌드 성공
- [ ] 서버 라운드·매치 전체 상태 완료
- [x] 로비 라운드 수 설정 계약 제공
- [x] travel 사이 유지되는 매치 누적 상태 제공
- [x] 남은 라운드가 있을 때 다음 랜덤 PvP 맵 이동 제공
- [x] 모든 라운드 종료 후 로비 복귀 제공
- [x] 소비 파트 라운드 결과 계약 인계 완료
- [x] 1/3/5 라운드 상태 전이 정적 점검 완료
- [x] 게임 속도별 맵 축소 주기 계약 제공
- [x] HUD 경기 시간·맵 축소 안내 TextBlock 바인딩 제공
- [x] 맵 담당 Blueprint 축소 시작 이벤트와 완료 신호 계약 제공
- [x] 공동 1등 단판 승부 전용 맵 이동 계약 제공
- [x] 단판 승부 HUD 라운드 문구와 맵 축소 숨김 처리 제공
- [x] 매치 종료 후 포디움 레벨 이동 계약 제공
- [x] 포디움 참가 팀 기준 순위 산정과 10초 후 로비 복귀 계약 제공

### 검증 메모

- 2026-08-13: 매치 종료 후 로비로 바로 복귀하지 않고 `ASnowRumbleGameMode::PodiumTravelUrl`로 포디움 맵에 이동하게 정리했다. 포디움 전용 `APodiumGameMode`가 현재 접속한 PlayerState의 팀 색만 참가 팀으로 수집하고, `USnowRumbleMatchSubsystem::GetTeamRoundWinCount()` 기준으로 1~3등을 산정한다. 포디움 맵의 `Podium_Team1`/`Podium_Team2`/`Podium_Team3` PlayerStart에 팀원을 배치하고, `Podium_Camera` 카메라로 시점을 고정하며, `UPodiumWidget` 기반 WBP에 결과 문구를 전달한다. 결과 표시 10초 뒤 `ResetPvPMatch()` 후 로비로 복귀한다. `git diff --check`와 `SnowRumbleEditor Win64 Development` 빌드를 통과했다.
- 2026-08-13: 포디움은 경기 규칙을 실행하지 않는 결과 표시 레벨이므로 `APodiumGameMode`를 `ASnowRumbleGameMode` 상속에서 `AGameModeBase` 상속으로 분리했다. `APodiumPlayerController`도 `ASnowRumblePlayerController` 상속을 제거해 PvP HUD, ESC, 채팅, 입력 복구 흐름이 포디움에 섞이지 않게 했다. C++ 컴파일은 통과했고, 최종 링크는 실행 중인 Unreal Editor DLL 잠금 `LNK1104`로 보류됐다.
- 2026-08-13: 정규 라운드가 모두 끝난 뒤 공동 1등 팀이 있으면 `USnowRumbleMatchSubsystem::StartTiebreakerForLeadingTie()`가 단판 승부 상태를 travel 사이 유지하고, `ASnowRumbleGameMode::TiebreakerTravelUrl`로 전용 PvP 맵에 이동하게 했다. 단판 승부에서는 라운드 문구가 `단판승부`로 표시되고 `MapShrinkCountdownText`는 숨겨지며, 맵 축소 타이머와 `OnMapShrinkRequested` 호출은 중지된다. 선물상자 스폰 타이머는 그대로 예약된다. `git diff --check`는 통과했고 UHT/C++ 컴파일도 통과했지만, 실행 중인 Unreal Editor가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크는 `LNK1104`로 실패했다.
- 2026-08-13: 단판 승부 맵에 모든 플레이어가 같이 travel되더라도 비동점 팀이 판정에 간섭하지 못하도록, 캐릭터 피해 적용 시 공격자와 대상이 모두 단판 승부 대상 팀일 때만 피해를 허용하게 했다. `git diff --check`와 UHT/C++ 컴파일은 통과했고, 최종 링크는 실행 중인 Unreal Editor DLL 잠금 `LNK1104`로 보류됐다.
- 2026-08-13: 단판 승부 대상이 아닌 팀 플레이어는 `ASnowRumbleCharacter::SetTiebreakerSpectatorFromServer()`로 관전자 상태가 복제된다. 관전자 상태에서는 이동·상호작용·아이템 사용이 `CanPerformGameplayAction()`에서 막히고, 로컬 이동 입력과 Pawn/WorldDynamic/PhysicsBody 충돌 응답도 비활성화해 경기 간섭을 줄인다. 로컬 카메라는 `RefreshTiebreakerSpectatorViewTarget()`이 단판 승부 참가 캐릭터를 찾아 `SetViewTargetWithBlend()`로 붙이며, 대상이 아직 복제되지 않았거나 사망하면 Tick에서 다시 찾는다. `git diff --check`와 `SnowRumbleEditor Win64 Development` 빌드를 통과했다.

- 2026-08-10: 서버 `ASnowRumbleGameMode`가 플레이어 얼음/사망 상태 변경 시 라운드 종료 조건을 재검사한다. 사망 또는 얼음 상태가 아닌 생존 플레이어가 한 팀 색에만 남으면 `ASnowRumbleGameState::EndRoundFromServer`로 라운드 승리 팀을 복제하고, `IsMatchInputLocked()`가 true가 되어 전체 입력을 잠근다. `UMainHUDWidget`은 `EndRoundPanel`과 `EndRoundResultText` 선택 바인딩을 제공한다. `git diff --check`는 통과했고 `MainHUDWidget.cpp`를 포함한 관련 C++ 컴파일도 통과했지만, 실행 중인 Unreal Editor가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크는 `LNK1104`로 실패했다. 에디터 종료 후 재빌드 확인이 필요하다.
- 2026-08-10: PvP 스폰 중 첫 팀 Pawn만 생존한 순간 라운드 종료가 조기 확정되지 않도록, `ASnowRumbleGameMode::EvaluateRoundEndCondition()`은 `ASnowRumbleGameState::IsMatchInputLocked()`가 true인 로딩·카운트다운·시작 전 구간에서는 판정을 건너뛴다. `git diff --check`는 통과했고 C++ 컴파일도 통과했지만, 실행 중인 Unreal Editor가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크는 `LNK1104`로 실패했다.
- 2026-08-10: `ASnowRumbleGameState`가 라운드 승리 팀의 매치 점수를 1점 올리고 팀별 점수를 복제하게 했다. `UMainHUDWidget`은 `RedTeamScoreText`, `SkyTeamScoreText`, `GreenTeamScoreText`, `YellowTeamScoreText`, `PurpleTeamScoreText`, `PinkTeamScoreText`, `BlueTeamScoreText`, `WhiteTeamScoreText` 선택 바인딩으로 팀별 점수를 표시한다. `git diff --check`는 통과했고 C++ 컴파일도 통과했지만, 실행 중인 Unreal Editor가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크는 `LNK1104`로 실패했다.
- 2026-08-10: 로비에서 `ASnowRumbleLobbyGameState::MatchRoundLimit`로 1/3/5 총 라운드 수를 설정하고, `USnowRumbleMatchSubsystem`이 seamless travel 사이 현재 라운드·총 라운드·팀별 승수를 유지하게 했다. 라운드 종료 시 `ASnowRumbleGameMode`가 누적 점수를 기록하고 남은 라운드가 있으면 `NextRoundTravelDelaySeconds` 후 `USnowRumbleMatchSubsystem::SelectNextPvPLevelPath()`로 다음 랜덤 PvP 맵에 이동한다. 모든 라운드가 끝나면 `ASnowRumbleGameState::IsMatchEnded()`와 `GetMatchWinningTeam()`으로 매치 1등 팀을 복제한다. `git diff --check`와 `SnowRumbleEditor Win64 Development` 빌드를 통과했다.
- 2026-08-10: 로비 일반 WBP와 PvP HUD 표시 계약을 보강했다. `ULobbyWidget::MatchRoundLimitText`는 `WBP_Lobby`에서 현재 총 라운드 수를 표시하고, `UMainHUDWidget::CurrentRoundText`는 PvP HUD에서 현재 라운드를 `라운드 {현재} / {전체}` 형식으로 표시한다. `git diff --check`와 C++ 컴파일은 통과했고, 실행 중인 Unreal Editor가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크는 `LNK1104`로 실패했다.
- 2026-08-10: 라운드 사이 PvP 맵 이동 후 이름과 팀 색이 `DESKTOP...`/기본색으로 초기화되는 문제를 수정했다. 로비에서 PvP로 갈 때만 seamless travel이 켜져 있고 PvP GameMode에는 꺼져 있어 다음 라운드 PlayerState 복사가 되지 않는 경로였으므로, `ASnowRumbleGameMode`도 `bUseSeamlessTravel = true`와 PIE seamless travel 허용을 설정하게 했다. `git diff --check`와 C++ 컴파일은 통과했고, 실행 중인 Unreal Editor가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크는 `LNK1104`로 실패했다.
- 2026-08-10: 팀 점수 HUD의 내림차순 정렬과 순위 전용 TextBlock 계약을 제거했다. 현재는 참가 중인 팀 색의 Row 또는 ScoreText만 표시하고, WBP에 배치된 순서는 유지한다.
- 2026-08-10: 모든 라운드를 완료한 뒤 임시로 로비에 복귀하는 흐름을 추가했다. `ASnowRumbleGameMode`는 매치 종료 결과를 `MatchEndLobbyReturnDelaySeconds` 동안 보여준 뒤 `LobbyReturnTravelUrl` 기본값 `/Game/Maps/L_Lobby?listen`으로 `ServerTravel`하고, 복귀 직전에 `USnowRumbleMatchSubsystem::ResetPvPMatch()`로 매치 누적 상태를 초기화한다. `git diff --check`와 C++ 컴파일은 통과했고, 실행 중인 Unreal Editor가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크는 `LNK1104`로 실패했다.
- 2026-08-10: 로비 예외행동 피드백 애니메이션과 사유 텍스트 계약을 추가했다. `InvalidActionAnimation`이 있으면 클라이언트가 게시판 방 설정을 건드리려 할 때, 호스트가 시작 조건 미충족 상태에서 게임 시작을 누를 때 로컬에서 재생한다. `InvalidActionReasonText`가 있으면 C++가 `방 설정은 호스트만 변경할 수 있습니다.`, `라운드 수는 호스트만 변경할 수 있습니다.`, `모든 플레이어가 준비 완료해야 시작할 수 있습니다.` 중 해당 사유를 표시한다. 게시판 월드 위젯에서 발생한 사유도 `ALobbyPlayerController::ShowLobbyInvalidActionFeedback()`을 통해 로컬 `WBP_Lobby`로 전달한다. 이후 이름 바인딩이 맞지 않거나 별도 패널 구조를 쓰는 WBP도 처리할 수 있도록 `OnInvalidActionFeedback` Blueprint 이벤트를 추가했다. `git diff --check`와 `SnowRumbleEditor Win64 Development` 빌드를 통과했다.
- 2026-08-10: 로비 예외행동 피드백을 보강했다. 호스트가 게임 시작을 눌렀을 때 유효 팀 색이 2개 미만이면 `두 개 이상의 팀이 있어야 게임을 시작할 수 있습니다.` 사유를 표시한다. 준비 완료 상태에서 팀 색을 바꾸려는 요청은 일반 `WBP_Lobby`, 게시판 WBP, PlayerController 요청 경로에서 로컬 피드백을 표시하고, 서버 `ASnowRumblePlayerState::ServerSetLobbyTeam()`도 최종 거부한다. `git diff --check`는 통과했고 C++ 컴파일도 통과했지만, 실행 중인 Unreal Editor가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크는 `LNK1104`로 실패했다.
- 2026-08-10: 로비 게시판 팀 섞기 버튼 계약을 추가했다. `Shuffle2TeamsButton`, `Shuffle3TeamsButton`, `Shuffle4TeamsButton` 중 같은 이름의 Button이 있으면 호스트가 현재 로비 인원을 포함해 자기 자신까지 무작위로 섞고, 선택한 팀 수에 맞춰 각 팀 인원 차이가 최대 1명이 되도록 서버에서 균등 배정한다. 사용할 팀 색도 무작위로 고른다. 클라이언트가 누르거나 플레이어 수가 팀 수보다 적으면 기존 예외행동 피드백으로 사유를 표시한다. `git diff --check`는 통과했고 C++ 컴파일도 통과했지만, 실행 중인 Unreal Editor가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크는 `LNK1104`로 실패했다.
- 2026-08-10: 로비 게시판 개인전 섞기 버튼 계약을 추가했다. `ShuffleSoloButton`과 같은 이름의 Button이 있으면 호스트가 현재 로비 인원을 모두 서로 다른 팀 색으로 무작위 배정한다. 클라이언트가 누르면 기존 예외행동 피드백으로 호스트 전용 사유를 표시한다. `git diff --check`는 통과했고 C++ 컴파일도 통과했지만, 실행 중인 Unreal Editor가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크는 `LNK1104`로 실패했다.
- 2026-08-10: 로비 전용 ESC 메뉴 계약을 추가했다. `ALobbyPlayerController`가 로비에서 ESC 입력을 받으면 `LobbyEscapeMenuWidgetClass` 위젯을 뷰포트 최상단에 띄우고, 메뉴가 열려 있는 동안 마우스 커서와 UI 전용 입력만 사용한다. PIE 디버깅용으로 F10도 같은 메뉴 토글에 연결했다. 메뉴가 열린 상태에서는 `ULobbyEscapeMenuWidget`이 ESC와 F10 키 입력을 직접 받아 다시 닫는다. `ReturnToMainMenuButton`은 `MainMenuTravelUrl` 기본값 `/Game/Maps/L_MainMenu`로 메인메뉴 이동을 실행한다. 기존 `ReturnToLobbyButton` 이름도 메인메뉴 이동 버튼으로 호환된다. `SettingsButton`, `InviteFriendsButton`, `BackButton`은 같은 이름의 WBP Button에 자동 연결된다. `git diff --check`와 `SnowRumbleEditor Win64 Development` 빌드를 통과했다.
- 2026-08-10: Steam 출시는 최종 목표로 유지하되 현재 개발과 테스트는 LAN/NULL 세션으로 계속 진행하기로 결정했다. ESC 메뉴의 친구 부르기 버튼은 현재 `OnInviteFriendsRequested` 이벤트만 제공하고, 실제 Steam Overlay 초대와 초대 수락 처리는 C-18에서 통합한다.
- 2026-08-10: 게임 속도별 맵 축소 호출 준비를 추가했다. 로비 게시판 WBP는 `SlowGameSpeedButton`, `NormalGameSpeedButton`, `FastGameSpeedButton`으로 느리게 90초, 보통 60초, 빠르게 30초 축소 주기를 설정하고, 기존 `WBP_Lobby`는 `GameSpeedText`로 현재 속도를 표시한다. PvP 시작 카운트다운 종료 후 `ASnowRumbleGameState`가 경기 경과 시간과 다음 축소까지 남은 시간을 서버 시간 기준으로 제공하고, `UMainHUDWidget`은 `MatchElapsedTimeText`, `MapShrinkCountdownText`로 표시한다. 0초가 되면 `ASnowRumbleGameMode::OnMapShrinkRequested` Blueprint 이벤트를 호출하고 `맵이 축소됩니다!` 상태를 복제한다. 현재는 실제 완료 신호가 없으므로 5초 후 자동 완료되며, J 또는 맵 담당 Blueprint가 실제 축소 완료 시 `CompleteMapShrinkFromBlueprint()`를 호출하면 그 시점부터 다음 주기를 시작할 수 있다. `git diff --check`와 `SnowRumbleEditor Win64 Development` 빌드를 통과했다.
- 2026-08-10: `GameSpeedText` 표시를 축소 주기 포함 문구에서 `느리게`, `보통`, `빠르게` 속도명만 표시하도록 조정했다. `git diff --check`는 통과했고 `LobbyWidget.cpp` 컴파일도 통과했지만, 실행 중인 Unreal Editor가 `Binaries/Win64/UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크는 `LNK1104`로 실패했다.

### 결과 확인

- [ ] 두 팀 이상이 PvP에 있을 때 한 팀을 제외한 모든 팀원이 얼음 또는 사망 상태가 되면 라운드가 종료된다.
- [ ] 얼음 상태도 생존자로 계산되지 않는다.
- [ ] 사망 상태도 생존자로 계산되지 않는다.
- [ ] 라운드 종료 후 승리 팀 색이 `GetRoundWinningTeam()`으로 읽힌다.
- [ ] 라운드 종료 후 승리 팀의 `GetTeamRoundWinCount()` 값이 1 증가한다.
- [ ] HUD 또는 Blueprint에서 `GetCurrentRoundNumber()`와 `GetRoundLimit()`으로 현재 라운드 진행을 읽을 수 있다.
- [ ] 라운드 종료 후 모든 플레이어 입력이 잠긴다.
- [ ] HUD WBP에 팀별 ScoreText가 있으면 승리 팀 점수가 1 증가해 표시된다.
- [ ] HUD WBP에 참가하지 않은 팀 ScoreText 또는 ScoreRow가 있으면 숨겨진다.
- [ ] HUD WBP의 참가 팀 ScoreText 또는 ScoreRow만 표시되고 참가하지 않은 팀은 숨겨진다.
- [ ] HUD WBP에 `EndRoundPanel`이 있으면 라운드 종료 후 패널이 표시된다.
- [ ] HUD WBP에 `EndRoundResultText`가 있으면 승리 팀 문구가 표시된다.
- [ ] 로비 게시판 WBP에 `Round1Button`, `Round3Button`, `Round5Button`을 연결하면 호스트가 1/3/5 총 라운드 수를 선택할 수 있다.
- [ ] 로비 게시판 WBP에 `Shuffle2TeamsButton`, `Shuffle3TeamsButton`, `Shuffle4TeamsButton`을 연결하면 호스트가 현재 인원을 2/3/4팀으로 무작위 균등 배정할 수 있다.
- [ ] 로비 게시판 WBP에 `ShuffleSoloButton`을 연결하면 호스트가 현재 인원을 모두 서로 다른 색으로 무작위 배정할 수 있다.
- [ ] 클라이언트가 팀 섞기 버튼을 누르면 팀 배정이 바뀌지 않고 호스트 전용 사유가 표시된다.
- [ ] 플레이어 수보다 많은 팀 수로 섞기를 누르면 팀 배정이 바뀌지 않고 인원 부족 사유가 표시된다.
- [ ] 로비에서 ESC 또는 F10을 누르면 로비 ESC 메뉴가 표시되고 마우스 커서로 버튼을 클릭할 수 있다.
- [ ] 로비 ESC 메뉴가 열린 상태에서 ESC 또는 F10을 다시 누르면 메뉴가 닫히고 로비 게임 입력으로 돌아간다.
- [ ] 로비 ESC 메뉴의 `BackButton`을 누르면 메뉴가 닫히고 로비 게임 입력으로 돌아간다.
- [ ] 로비 ESC 메뉴의 `ReturnToMainMenuButton` 또는 기존 `ReturnToLobbyButton`을 누르면 `MainMenuTravelUrl`로 메인메뉴 이동을 실행한다.
- [ ] 로비 ESC 메뉴의 `SettingsButton`, `InviteFriendsButton`을 누르면 대응 Blueprint 이벤트가 호출된다.
- [ ] 클라이언트가 게시판의 방 설정 버튼을 누르면 `InvalidActionAnimation`이 재생된다.
- [ ] 호스트가 준비 조건 미충족 상태에서 시작 버튼을 누르면 `InvalidActionAnimation`이 재생된다.
- [ ] 호스트가 모든 플레이어가 같은 팀 색인 상태에서 시작 버튼을 누르면 팀 2개 이상 필요 사유가 표시된다.
- [ ] 준비 완료 상태에서 팀 색 변경 버튼을 누르면 팀 변경이 되지 않고 예외행동 사유가 표시된다.
- [ ] `InvalidActionReasonText`가 있으면 예외행동 사유 문구가 표시된다.
- [ ] `WBP_Lobby`에 `MatchRoundLimitText`를 배치하면 로비에서 선택한 총 라운드 수가 표시된다.
- [ ] `WBP_MainHUDWidget` 중앙 상단에 `CurrentRoundText`를 배치하면 PvP 중 현재 라운드가 표시된다.
- [ ] 로비 게시판 WBP에 `SlowGameSpeedButton`, `NormalGameSpeedButton`, `FastGameSpeedButton`을 연결하면 호스트가 게임 속도를 바꿀 수 있다.
- [ ] 클라이언트가 게임 속도 버튼을 누르면 속도가 바뀌지 않고 호스트 전용 사유가 표시된다.
- [ ] 기존 `WBP_Lobby`에 `GameSpeedText`를 배치하면 현재 속도가 `느리게`, `보통`, `빠르게` 중 하나로 표시된다.
- [ ] `WBP_MainHUDWidget`에 `MatchElapsedTimeText`를 배치하면 `시작!` 이후 현재 경기 시간이 증가한다.
- [ ] `WBP_MainHUDWidget`에 `MapShrinkCountdownText`를 배치하면 설정된 속도에 따라 다음 맵 축소까지 남은 시간이 감소한다.
- [ ] `MapShrinkCountdownText`가 0초에 도달하면 `맵이 축소됩니다!`로 바뀐다.
- [ ] PvP GameMode Blueprint의 `OnMapShrinkRequested` 이벤트가 축소 시점마다 호출된다.
- [ ] 현재 임시 완료 기준으로 `맵이 축소됩니다!` 표시 약 5초 후 다음 축소 카운트다운이 다시 시작된다.
- [ ] 3라운드 또는 5라운드 설정에서 라운드 종료 후 남은 라운드가 있으면 로딩창이 다시 표시되고 다른 PvP 후보 맵으로 이동한다.
- [ ] 마지막 라운드 종료 후 `IsMatchEnded()`가 true가 되고 `GetMatchWinningTeam()`이 최종 1등 팀을 반환한다.
- [ ] 마지막 정규 라운드 종료 후 공동 1등이 있으면 로비로 복귀하지 않고 `TiebreakerTravelUrl` 맵으로 이동한다.
- [ ] 단판 승부 HUD의 `CurrentRoundText`가 `단판승부`로 표시된다.
- [ ] 단판 승부 HUD에서는 `MatchElapsedTimeText`만 보이고 `MapShrinkCountdownText`는 숨겨진다.
- [ ] 단판 승부 맵에서는 맵 축소 이벤트가 호출되지 않는다.
- [ ] 단판 승부 맵에서도 선물상자가 기존 TargetPoint 기준으로 스폰된다.
- [ ] 단판 승부 대상이 아닌 팀 플레이어의 눈덩이는 피해를 주지 못한다.
- [ ] 단판 승부 대상이 아닌 팀 플레이어는 이동·상호작용을 할 수 없고 경기 참가자 시점으로 관전한다.
- [ ] 단판 승부 대상이 아닌 팀 플레이어는 Pawn/눈덩이 충돌로 경기에 간섭하지 않는다.
- [ ] 단판 승부 승리 팀이 최종 1등 팀으로 확정되고 이후 로비로 복귀한다.
- [ ] 마지막 라운드 종료 후 약 `MatchEndLobbyReturnDelaySeconds` 뒤 포디움 맵으로 이동한다.
- [ ] 포디움 맵에서 실제 참가 팀 색만 1~3등 후보로 표시되고, 매치에 없던 팀 색은 표시되지 않는다.
- [ ] 포디움 맵에서 `Podium_Team1`, `Podium_Team2`, `Podium_Team3` 태그 PlayerStart에 순위별 팀원이 배치된다.
- [ ] 포디움 화면에서 `FirstPlaceText`, `SecondPlaceText`, `ThirdPlaceText`, `SubtitleText`가 있으면 결과와 10초 후 복귀 안내가 표시된다.
- [ ] 포디움 결과 표시 약 10초 후 매치 상태가 초기화되고 로비로 복귀한다.

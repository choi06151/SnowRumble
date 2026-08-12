# Task C-08 - 팀 식별 데이터

## 설명

팀 색과 닉네임 데이터를 표현 파트에 제공해 머리 위 이름표와 팀 구분 UI가 서버 상태를 읽게 한다.

## 상태 전이 기준
- 시작 가능: C-03 완료
- 완료 가능: 닉네임·팀 색·표시 상태와 S-05 인계 확인

## 구현 항목
- [x] 닉네임을 머리 위 표시가 읽을 수 있게 제공한다.
- [x] 머리 위 닉네임 표시용 WidgetComponent와 WBP 부모 클래스를 추가한다.
- [x] 이름표가 거리에 따라 자연스럽게 보이도록 월드 공간 WidgetComponent로 표시한다.
- [ ] 팀 색을 머리 위 UI가 읽을 수 있게 제공한다.
- [ ] 이름표가 읽을 생존·관전 표시 보조 상태를 제공한다.

## 작업 배정
- 담당자·기능·계약 소유자: 최재원(C)
- 자산 수정자·생성·변경 후보: 구현 승인 전 확정
- 공유 확인 대상: S-05
- 병합 순서: C-03 후, S-05 전

## 공용 계약과 인계
- 제공받을 계약: C-02 닉네임 입력, `ALobbyPlayerController::RequestApplyLobbyPlayerName`, `ASnowRumblePlayerState::LobbyPlayerName`, C-03 팀 상태
- 제공할 계약:
  - `ASnowRumblePlayerState::GetLobbyPlayerName()`: 서버에 저장되고 복제된 표시 닉네임을 반환한다.
  - `ASnowRumbleCharacter::OverheadNameplateComponent`: 캐릭터 머리 위에 붙는 `UWidgetComponent`다.
  - `ASnowRumbleCharacter::OverheadNameplateWidgetClass`: S-05가 만든 이름표 WBP 클래스를 지정할 수 있는 확장 지점이다.
  - `ASnowRumbleCharacter::OverheadNameplateDrawSize`: 월드 공간 이름표의 위젯 draw size다.
  - `ASnowRumbleCharacter::OverheadNameplateWorldScale`: 월드 공간 이름표의 기본 월드 스케일이다.
  - `UOverheadNameplateWidget`: 이름표 WBP 부모 클래스다. WBP에서 `PlayerNameTextBlock` 이름의 TextBlock을 만들면 C++ 부모가 `ASnowRumbleCharacter::GetOverheadPlayerName()` 값을 자동 표시한다.
- 인계 대상: S-05

## 범위 밖
- 이름표 레이아웃과 카메라 연출 디테일
- 팀 스폰, 시작 제한과 팀 소개 연출

## 사전 전제
- C-03

## 기존 구현 인수
- `ASnowRumblePlayerState`의 `LobbyPlayerName` 복제와 `OnLobbyPlayerChanged` 델리게이트를 닉네임 표시 원본으로 사용한다.
- `ASnowRumbleCharacter`는 `PlayerState` 복제 시점과 닉네임 변경 이벤트에 맞춰 머리 위 이름표 위젯을 갱신한다.
- 서버 `PostLogin` 후 `ALobbyPlayerController::ClientRequestApplySavedLobbyPlayerName()`이 소유 클라이언트의 저장 닉네임 제출을 요청하고, 서버 `PlayerState`에 반영한다.
- 서버 닉네임 적용 시 기본 `APlayerState::PlayerName`도 같은 값으로 갱신해 WBP가 기본 이름을 읽어도 `DESKTOP-...` 값이 남지 않게 한다.
- `UOverheadNameplateWidget`은 표시 이름이 바뀌면 tick에서 다시 읽어 `PlayerNameTextBlock`을 갱신한다. 서버 화면에서 클라이언트 캐릭터 이름표가 초기 기본 PC 이름으로 만들어진 뒤에도 복제 닉네임으로 교체되게 하기 위한 보강이다.
- `ASnowRumbleCharacter::RefreshOverheadPlayerName()`은 `UWidgetComponent::InitWidget()`으로 이름표 위젯 인스턴스를 보장한 뒤 관찰 캐릭터를 연결한다.

## 결정 필요
- 없음

## 수동 작업
- S-05에서 이름표 WBP를 만들 때 부모 클래스를 `UOverheadNameplateWidget`으로 지정한다.
- WBP 안에 TextBlock을 만들고 `PlayerNameTextBlock` 이름으로 바인딩하면 C++ 부모가 닉네임을 자동 표시한다.
- 캐릭터 Blueprint에서 `OverheadNameplateWidgetClass`를 해당 WBP로 지정하면 최종 이름표 표현을 사용할 수 있다.
- 이름표 크기가 너무 크거나 작으면 캐릭터 Blueprint에서 `OverheadNameplateDrawSize`와 `OverheadNameplateWorldScale`을 조정한다.

## 완료 조건
### 에이전트 확인
- [x] 닉네임 표시 계약 완료
- [x] WidgetComponent 기반 머리 위 닉네임 표시 완료
- [x] S-05 닉네임 표시 인계 기록 완료
- [x] `SnowRumbleEditor Win64 Development` 빌드 성공
- [x] 이름표 WBP 부모 `UOverheadNameplateWidget` 빌드 성공
- [x] 서버 화면에서 클라이언트 이름표가 늦게 적용된 복제 닉네임으로 갱신되도록 위젯 갱신 타이밍 보강 완료
- [x] 이름표 월드 공간 거리감 적용 완료
- [ ] 팀 색 계약 완료
- [ ] 표시 상태가 서버 원본을 중복하지 않음

### 검증 메모

- 2026-08-10: `ASnowRumbleCharacter::OverheadNameplateComponent`를 `World` space로 전환하고, `OverheadNameplateDrawSize`, `OverheadNameplateWorldScale` 조정값을 추가했다. Tick에서 `RefreshOverheadNameplateFacing()`이 로컬 PlayerCameraManager를 기준으로 이름표를 카메라 쪽으로 회전시킨다. `git diff --check`는 통과했고 C++ 컴파일도 통과했지만, 실행 중인 Unreal Editor가 `UnrealEditor-SnowRumble.dll`을 잡고 있어 최종 링크는 `LNK1104`로 실패했다.

### 결과 확인
- [ ] 호스트가 로비에 들어오면 자기 캐릭터 머리 위에 메인메뉴 닉네임이 표시된다.
- [ ] 클라이언트가 로비에 들어오면 각 캐릭터 머리 위에 각자의 메인메뉴 닉네임이 표시된다.
- [ ] 한 화면에서 다른 플레이어의 닉네임도 복제된 값으로 보인다.
- [ ] 서버 화면에서 클라이언트 캐릭터 머리 위 이름표가 `DESKTOP-...` 같은 기본 PC 이름이 아니라 클라이언트 메인메뉴 닉네임으로 바뀐다.
- [ ] 이름표가 가까운 플레이어보다 먼 플레이어에서 더 작게 보인다.
- [ ] 이름표가 카메라를 향해 읽을 수 있게 보인다.

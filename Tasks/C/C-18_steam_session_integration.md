# Task C-18 - Steam 세션 최종 통합

## 설명

패키지/Standalone 기본 세션을 `OnlineSubsystemSteam` 기반 Steam Session으로 전환한다. 기존 메인메뉴·로비 Blueprint API 이름은 호환을 위해 유지하되, 내부 세션 생성·검색·참가·정리 경로는 현재 활성 OnlineSubsystem을 기준으로 동작하게 한다. Steam 초기화가 불가능한 로컬 환경은 fallback 검증용으로만 유지한다.

## 상태 전이 기준
- 시작 가능: 핵심 로비·PvP·라운드·메인메뉴 복귀 흐름이 기본 검증 가능한 상태가 된 뒤
- 완료 가능: Steam 빌드에서 방 생성·검색·참가·초대·초대 수락·세션 정리가 검증된 뒤

## 구현 항목
- [x] `OnlineSubsystemSteam` 플러그인·Steam NetDriver 설정과 Steam App ID 적용 경로를 정리한다. 실제 App ID는 사용자 확인 전까지 placeholder로 유지한다.
- [x] 패키지/Standalone 기본 OnlineSubsystem을 Steam으로 전환한다.
- [x] `USnowRumbleSessionSubsystem`의 공개 API는 유지하고 내부 구현만 LAN/Steam 경로로 분리한다.
- [x] Steam 사용 가능 상태에서는 Steam Presence/Lobby 세션 생성·검색·참가를 사용한다.
- [x] Steam을 사용할 수 없는 에디터·로컬 개발 환경에서는 기존 NULL LAN fallback 경로가 깨지지 않게 한다.
- [ ] 로비 ESC 메뉴의 친구 부르기 버튼을 Steam Overlay 친구 초대 또는 초대창으로 연결한다.
- [ ] Steam 초대 수락 후 메인메뉴·로비·PvP 상태에서 안전하게 세션 참가하는 흐름을 제공한다.
- [ ] 메인메뉴 복귀, 로비 복귀, 게임 종료 시 세션 정리 정책을 Steam 기준으로 재검증한다.
- [x] Steam 호스트 종료·메인메뉴 복귀 시 클라이언트가 기존 호스트 이탈 알람과 함께 메인메뉴로 복귀한다.
- [x] 반복 테스트 빌드를 Development 구성으로 압축하고 SHA-256을 생성하며, 2 GiB 이상 ZIP은 Release 업로드용 조각 파일로 분할한다.
- [x] GitHub 사전 공개 Release에서 빌드 파일을 버전별로 공유하는 절차를 기록한다.

## 작업 배정
- 담당자·기능·계약 소유자: 최재원(C)
- 자산 수정자: C++·문서·배포 스크립트 최재원(C), Steam App ID·테스트 계정·빌드 환경은 사용자 확인 필요
- 생성 파일: `Scripts/prepare-steam-test-release.ps1`
- 변경 파일 후보: `Config/DefaultEngine.ini`, `Source/SnowRumble/Online/SnowRumbleSessionSubsystem.*`, `Source/SnowRumble/UI/LobbyEscapeMenuWidget.*`, `Source/SnowRumble/UI/MainMenuWidget.*`, `Source/SnowRumble/UI/LobbyPlayerController.*`, `Tasks/C/C-18_steam_session_integration.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`
- 공유 확인 대상: C-02, S-02, 사용자
- 병합 순서: LAN 기반 MVP 흐름 안정화 후, 출시 준비 통합 전

## 공용 계약과 인계
- 제공받을 계약:
  - C-02 기존 세션 공개 API와 메인메뉴 방 만들기·검색·참가 UI 계약
  - C-05 로비 ESC 메뉴 `OnInviteFriendsRequested`와 메인메뉴 이동 흐름
- 제공할 계약:
  - Steam 사용 가능 여부를 UI가 표시하거나 기능 활성화에 사용할 수 있는 상태
  - Steam 친구 초대 버튼 실행 결과와 실패 사유
  - 초대 수락 후 세션 참가와 travel 결과
- Steam 기본 사용과 LAN fallback 유지 정책

## 범위 밖
- Steam 상점 페이지, 업적, 리더보드
- 전용 서버 전환
- 플랫폼별 비Steam 친구 초대

## 사전 전제
- 개발 테스트는 SpaceWar App ID 480, 출시 전 SnowRumble 전용 App ID로 교체
- Steam 클라이언트 로그인과 Overlay 사용 가능 환경
- Standalone 또는 패키지 빌드 기준 테스트 환경

## 결정 필요
- 출시 App ID 사용 시점
- Steam 초대 UI를 단순 친구창으로 먼저 열지, 세션 초대창까지 바로 연결할지
- 방 코드 기능을 Steam 검색 필터와 병행할 방식

## 수동 작업

- Steamworks/Steam 계정 테스트 환경을 준비한다.
- Unreal Editor에서 Windows용 `Development` 패키지를 만든다. GitHub Actions hosted runner는 이 프로젝트의 Unreal Engine 설치 환경으로 사용하지 않는다.
- 저장소 PowerShell에서 `.\Scripts\prepare-steam-test-release.ps1 -BuildDirectory "C:\Builds\SnowRumble\Windows" -Version "dev-2026.10.26.1"` 형식으로 실행한다. `BuildDirectory`를 실제 패키지 폴더로 바꾸고 매 빌드마다 새 버전명을 사용한다. 기본 출력은 Git에 포함되지 않는 `Saved/ReleasePackages`다.
- GitHub 저장소의 **Releases → Draft a new release**에서 위와 같은 `dev-...` 태그와 제목을 만들고, 생성된 ZIP과 `.sha256` 파일을 함께 올린다. 분할된 빌드라면 모든 `.partNNN` 파일과 `.parts.sha256`, 원본 ZIP의 `.sha256`도 함께 올린 뒤 **Set as a pre-release**를 선택해 게시한다. 저장소가 비공개라면 내려받는 테스트 계정에도 저장소 접근 권한이 필요하다.
- 분할된 빌드를 받은 사람은 모든 `.partNNN` 파일과 `.sha256`을 한 폴더에 다운로드한다. PowerShell에서 아래처럼 순서대로 이어 붙이고 원본 ZIP 해시를 확인한 다음 압축을 풀어 실행한다.

  ```powershell
  $zipName = 'SnowRumble-Windows-Development-dev-2026.10.26.1.zip'
  $parts = Get-ChildItem "$zipName.part*" | Sort-Object Name
  $output = [System.IO.File]::Create($zipName)
  try {
      foreach ($part in $parts) {
          $input = [System.IO.File]::OpenRead($part.FullName)
          try { $input.CopyTo($output) } finally { $input.Dispose() }
      }
  } finally { $output.Dispose() }
  $expectedHash = (Get-Content "$zipName.sha256").Split(' ')[0]
  $actualHash = (Get-FileHash $zipName -Algorithm SHA256).Hash.ToLowerInvariant()
  if ($actualHash -ne $expectedHash) { throw 'ZIP SHA-256이 일치하지 않습니다.' }
  ```

- 빌드 노트에는 기준 브랜치, 커밋 SHA, Development 구성, 변경 내용과 알려진 문제를 적는다.
- GitHub Release 파일 하나당 2 GiB 미만이어야 한다. 스크립트는 2 GiB 이상 ZIP을 1900 MiB 조각으로 분할해 각 파일을 제한보다 작게 유지한다.
- Steam Overlay가 동작하는 패키지 테스트 환경에서 호스트와 참가 절차를 확인한다.
- 필요하면 테스트용 Steam 계정 2개 이상 또는 2PC 환경을 준비한다.
- 호스트가 로비 또는 PvP에서 메인메뉴로 이동하거나 게임을 종료할 때 클라이언트가 `L_MainMenu`로 이동하고 `호스트의 연결이 해제되었습니다.` 알람을 한 번 표시하는지 확인한다.
- 위 복귀 때 이전 네트워크 플레이어 Pawn이 메뉴에 남지 않고 메뉴 전용 프리뷰만 표시되는지 확인한다.

## 완료 조건
### 에이전트 확인
- [x] Steam 설정과 빌드 구성이 문서화되어 있다.
- [ ] LAN fallback이 기존 메인메뉴·로비 테스트 흐름을 깨지 않는다.
- [x] Steam 세션 생성·검색·참가 코드 경로가 분리되어 있다.
- [ ] 친구 부르기 버튼이 Steam 가능 상태에서 Steam 초대 UI를 연다.
- [ ] 초대 수락 후 참가 흐름이 처리된다.
- [x] `git diff --check` 공백 점검 통과
- [x] `SnowRumbleEditor Win64 Development` 빌드 성공
- [ ] 패키징 폴더가 유효할 때 스크립트가 Development ZIP과 SHA-256을 만든다.
- [ ] GitHub 사전 공개 Release에서 호스트·클라이언트 테스트용 빌드를 내려받을 수 있다.

### 검증 메모

- 2026-10-26: `Build/Windows` Development 패키지(폴더 크기 약 4.39 GiB)를 Release 준비 스크립트로 압축했을 때 ZIP이 GitHub 2 GiB 단일 파일 제한을 초과해 게시 전 중단됐다. 이 결과에 맞춰 1900 MiB 조각 분할·복원 절차를 추가했으며, 수정한 분할 경로 실행과 GitHub 업로드는 대기다.
- 2026-08-27: 로비에서 선택한 `Snowman`/`TeamPvP` 모드를 온라인 세션 광고값에 갱신하고, 로비 밖에서 수락된 Steam 초대는 기존 세션을 정리하지 않고 거절하도록 보강했다.
- 2026-08-27: 사용자 요청으로 기본 세션 기준을 LAN에서 Steam으로 전환했다. `DefaultPlatformService=Steam`, Steam App ID 480, Steam listen host용 `bInitServerOnClient=True`를 설정하고, 기존 Blueprint API 이름은 유지한 채 세션 상태 문구와 로그를 현재 백엔드 기준으로 표시하게 했다.
- 2026-08-10: 사용자 결정에 따라 현재 개발과 테스트는 LAN/NULL 세션으로 계속 진행하고, Steam 세션 전환은 최종 통합 Task로 분리했다. 앞으로 새 로비·메인메뉴·PvP 기능은 `USnowRumbleSessionSubsystem` 등 공개 세션 계약을 통해서만 세션 기능을 사용하고, UI나 게임 규칙 코드가 LAN/Steam 구현 세부사항에 직접 의존하지 않게 유지한다.
- 2026-08-25: `LanToSteam` 브랜치에서 공개 LAN API를 유지한 Steam/NULL 세션 분기를 추가했다. Steam 사용 시 `bIsLANMatch=false`, Presence/Lobby 검색, Steam 초대 허용을 사용한다. 개발 테스트를 위해 기본 서비스를 Steam으로 전환하고 SpaceWar App ID 480을 적용했으며, 출시 전 전용 App ID로 교체한다.

### 결과 확인

- [ ] Steam을 사용할 수 없는 fallback 환경에서 기존 방 만들기·빠른 참가·방 코드 참가가 유지된다.
- [ ] Steam 모드에서 호스트가 세션을 만들 수 있다.
- [ ] Steam 모드에서 다른 클라이언트가 세션을 검색하고 참가할 수 있다.
- [ ] 친구 부르기 버튼이 Steam Overlay 초대 흐름을 연다.
- [ ] 초대받은 클라이언트가 초대를 수락해 세션에 참가할 수 있다.
- [ ] 메인메뉴 복귀 또는 게임 종료 시 세션이 올바르게 정리된다.
- [ ] Steam 호스트 이탈 시 클라이언트가 엉뚱한 맵에 남지 않고 메인메뉴로 복귀하며 호스트 이탈 알람을 표시한다.

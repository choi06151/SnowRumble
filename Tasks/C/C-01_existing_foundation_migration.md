# Task C-01 - 기존 기반 마이그레이션

## 설명

기존 구현과 자산을 새 GDD와 4인 소유권 구조에서 안전하게 재사용할 기준으로 고정한다.

## 상태 전이 기준

- 시작 가능: 즉시
- 완료 가능: 기존 구현 분류, 공용 파일 소유권과 후속 인계가 실제 저장소 기준으로 기록됨

## 구현 항목

- [x] 새 GDD와 4인 협업 구조에 맞춰 역할별 Task·PLAN·공용 Skill을 정리하고 모든 역할 브랜치에 배포한다.
- [x] Task 분할 재검토 결과를 반영해 병목 Task를 분리하고, 환경 압박 로직 소유권을 레벨 담당자에게 배정한다.
- [x] 각 담당자의 최초 Task 시작 전 개발 스타일 확인 절차를 추가한다.
- [x] 기존 플레이어·세션·눈덩이·UI 연결의 실제 파일과 참조를 확인한다.
- [x] 유지·수정·대체·폐기·재검증 분류를 실제 코드와 자산 기준으로 확정한다.
- [x] 공용 파일과 기존 진행중 변경의 단독 수정자를 지정한다.
- [x] 레거시 눈덩이 투사체와 이름 불일치 자산의 참조 상태를 확인한다.

## 작업 배정

- 담당자·기능·계약 소유자: 최재원(C)
- 자산 수정자: 구현 승인 전 확정
- 생성·변경 후보: 구현 승인 전 확정
- 공유 확인 대상: C-02~C-14, K-01~K-11, S-01~S-12, J-01~J-05에 필요한 기존 연결
- 병합 순서: 모든 새 구현 Task보다 먼저

## 공용 계약과 인계

- 제공받을 계약: 없음
- 제공할 계약: 기존 공용 클래스와 재사용 가능한 연결 지점 목록
- 인계 대상: C-02~C-14, K-01~K-11, S-01~S-12, J-01~J-05

## 범위 밖

- 새 게임플레이 기능 구현
- 참조 확인 전 기존 파일·자산 삭제 또는 이름 변경

## 사전 전제

- `docs/PROGRESS_BASELINE_2026-08-07.md`
- `Tasks/LEGACY_MIGRATION.md`

## 결정 필요

- 없음

## 현재까지 반영된 구조 개편

- 새 기획 기준 문서와 역할별 Task 구조를 `C`, `K`, `S`, `J`로 분리했다.
- 역할 내부 계획은 각 `PLAN_*.md`가 관리하고, 중앙 `docs/PLANS.md`는 공용 계약·선점·통합 상태만 관리하도록 정리했다.
- `$snowrumble-run-task`로 세션 시작, 브랜치 동기화, Task 실행·변경·오류 대응·게시 절차를 공통화했다.
- 구조 재검토 결과 `C-13` 핫팩 부활 계약, `C-14` 팀 스폰과 시작 연출, `K-11` 선물상자 라운드 스케줄 연결 Task를 추가했다.
- `C-07`은 플레이어 효과 공용 계약으로 축소하고, `C-08`은 팀 식별 데이터로 축소했다.
- 환경 압박 로직은 레벨 담당자가 자기 맵 전용 Actor·Component·Blueprint로 개발하되, C가 제공한 공용 피해·경기 계약을 사용하도록 역할 경계를 확정했다.
- 각 담당자의 최초 Task 시작 전 개발 스타일(Blueprint 위주, C++ 위주, 혼합)을 확인하고 Task 대기열을 재검토하는 절차를 추가했다.
- 위 구조 개편 커밋 `a1d4bfb`를 `master`, `C`, `K`, `S`, `J` 원격 브랜치에 배포했다.
- 최재원(C)의 개발 스타일은 C++ 위주로 확정했다. C-01의 남은 실제 확인은 기존 C++ 공용 클래스, 서버 권한 상태, RPC·복제 원본, Blueprint 연결 지점을 우선 분류한다.

## C++ 위주 확인 기준

- 서버 권한 상태와 복제 원본을 가진 C++ 클래스는 C 소유 공용 계약 후보로 우선 분류한다.
- Blueprint는 서버 상태를 직접 확정하지 않고 C++에서 제공한 읽기 상태·요청 함수·이벤트를 연결하는 대상으로 분류한다.
- 기존 Blueprint 자산은 부모 C++ 클래스, 노출 프로퍼티, 이벤트 연결 지점과 실제 자산 수정자를 분리해 기록한다.
- K/S/J가 직접 구현할 수 있는 맵·아이템·UI 내부 로직과 C가 제공해야 하는 공용 API를 구분해 후속 Task 인계에 반영한다.

## 개편 전 구현 인수 결과

아래 분류는 현재 `master`의 실제 `Source/`, `Content/`, 기준선 문서와 레거시 Task 기록을 대조한 결과다. 기존 루트 `Tasks/*.md`의 완료 표시는 당시 검증 이력으로 보존하고, 새 역할별 Task 완료로 직접 승격하지 않는다.

| 영역 | 실제 파일·자산 | 분류 | 새 Task 적용 |
| --- | --- | --- | --- |
| 프로젝트·입력·기본 캐릭터 | `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Content/Characters/BP_SnowRumbleCharacter.uasset`, `Content/Input/IMC_Player.uasset`, `Content/Input/IA_*.uasset` | 유지·재검증 | C-01 기준선으로 보존, C-12에서 4vs4 통합 입력 충돌 재확인 |
| LAN 세션 | `USnowRumbleSessionSubsystem`, `FSnowRumbleSessionInfo`, `FOnSnowRumbleSessionStateChanged`, `FOnSnowRumbleSessionSearchCompleted`, `Content/WBP/WBP_MainMenu.uasset` | 수정·재검증 | C-02에서 최대 8명, 빠른 게임, 선택 비밀번호와 S-02 인계로 확장 |
| 로비 팀·준비 | `ASnowRumblePlayerState`, `ASnowRumbleLobbyGameState`, `ASnowRumbleLobbyGameMode`, `ULobbyWidget`, `Content/WBP/WBP_Lobby.uasset` | 대체·재사용 | C-03에서 수동 팀 선택을 서버 랜덤 배정으로 대체하고 준비·시작 조건은 재사용 |
| HP·얼기 | `USnowRumbleHealthComponent`, `ASnowRumbleCharacter::TakeDamage`, `IsFrozen`, `OnFrozenChanged` | 수정·재검증 | C-06에서 60초 얼음, 사망, 관전, 팀 전멸 판정으로 확장 |
| 눈 제작 | `USnowballCreationComponent`, `SnowSurface` 태그, `SnowballItemClass`, `Content/Snowball/BP_SnowballItem.uasset` | 유지·재검증 | C-09에서 제작 가능 지면과 제작 진행도 연결을 보존 |
| 눈 획득·보유·내려놓기·굴리기 | `USnowballEquipmentComponent`, `ASnowballItem`, `SnowballHoldPoint`, `SnowballSocket`, `RollingSnowballCollision` | 유지·수정·재검증 | C-09에서 입력 우선순위, 큰 눈 광역 피해, 새 피해량 기준을 반영 |
| 레거시 눈 투사체 | `ASnowballProjectile`, 이전 문서의 `BP_SnowballProjectile` 기록 | 폐기 보류 | 현재 `Source` 사용처 없음. 참조 확인 전 삭제 금지, C-09 또는 별도 정리에서 제거 판단 |
| UI C++ 부모 | `UMainMenuWidget`, `ULobbyWidget`, `UMainHUDWidget`, `UHealthBarWidget`, `UOverheadTimedActionWidget`, `UEmoteRadialMenuWidget` | 분리·재사용 | C는 데이터·요청 계약만 유지하고 WBP 구성·그래픽은 S-02/S-03/S-06/S-07/S-09/S-11에 인계 |
| 이모션 | `ASnowRumbleCharacter::RequestPlayEmote`, `ServerRequestPlayEmote`, `MulticastPlayEmote`, `UEmoteRadialMenuWidget`, `Content/WBP/WBP_EmoteRadialMenuWidget.uasset` | 유지·재검증 | C-10에서 입력·행동 제한·네트워크 결과를 재검증하고 S-06에 자산 소유 인계 |
| 눈 VFX와 환경 표현 | `Content/VFX/NS_Snow.uasset`, `LocalSnowEffect`, 겨울 환경 에셋 | 일부 재사용 | 맵 전용 환경 압박은 S/J가 담당하고 C는 공용 피해·경기 계약만 제공 |

## 공용 파일 소유권과 후속 인계

| 대상 | 단독 수정자 | 이유 | 후속 인계 |
| --- | --- | --- | --- |
| `Source/SnowRumble/Online/*` | C | 세션 생성·검색·참가와 맵 이동 계약 원본 | C-02 -> S-02 |
| `Source/SnowRumble/Game/*GameMode*`, `*GameState*`, `*PlayerState*` | C | 팀, 준비, 시작 가능, 경기 흐름의 서버 권한 원본 | C-03/C-04/C-05/C-08/C-14 -> S/J/K 소비 Task |
| `Source/SnowRumble/Player/SnowRumbleCharacter.*` | C | 공용 입력, 이동, 플레이어 상태, 여러 컴포넌트 연결점 | C-06/C-09/C-10/C-12, S/K/J는 공개 API로만 사용 |
| `Source/SnowRumble/Player/SnowRumbleHealthComponent.*` | C | HP·얼기 복제 상태 원본 | C-06/C-07/C-13, K-07/S-07 |
| `Source/SnowRumble/Snowball/*` | C | 기존 눈 제작·보유·투척·큰 눈 전투 원본 | C-09, K-05/K-06/K-08/S-06 |
| `Source/SnowRumble/UI/*` | 계약은 C, WBP 자산은 S | 현재 C++ 부모는 게임플레이 데이터 노출점이고 실제 UI 구성은 S 소유 | C-02/C-03/C-06/C-07/C-09/C-10에서 S Task로 실제 이름 인계 |
| `Content/Characters/BP_SnowRumbleCharacter.uasset` | 구현 승인 전 C 또는 S 중 확정 | C++ 부모 연결과 캐릭터 표현 자산이 함께 걸린 공용 Blueprint | C 작업에서 서버 상태 연결이 필요하면 단독 선점 후 수정, 표현 변경은 S |
| `Content/Snowball/BP_SnowballItem.uasset` | 구현 승인 전 C 또는 S/K 중 확정 | 눈 전투 부모 Blueprint와 표현 자산 연결 대상 | C-09 계약 확정 뒤 표현·모델 연결은 S/K 범위로 분리 |
| `Content/WBP/*.uasset` | S | UI 자산과 레이아웃 소유 | C는 부모 클래스·함수·이벤트 이름만 인계 |
| `Content/Maps/*.umap` | 배정 전 수정 금지 | 맵은 단일 자산 수정자 원칙 대상 | C-04/C-14 계약 후 S/J가 배정 맵에서 수정 |

## 이름·참조 불일치 적용

- 기존 문서의 `WBP_MainHUD`는 실제 자산 `Content/WBP/WBP_MainHUDWidget.uasset`로 인계한다.
- 기존 문서의 `WBP_OverheadActionProgress`는 실제 자산 `Content/WBP/WBP_OverheadTimedActionWidget.uasset`로 인계한다.
- 기존 문서의 `WBP_EmoteRadialMenu`는 실제 자산 `Content/WBP/WBP_EmoteRadialMenuWidget.uasset`로 인계한다.
- 레거시 `BP_SnowballProjectile`은 현재 `Content/Snowball/`에 존재하지 않고, 현재 C++ 플레이 파이프라인은 `ASnowballItem`과 `BP_SnowballItem` 기준이다.

## 새 C Task에 반영할 기존 구현

- C-02: `USnowRumbleSessionSubsystem`의 Host/Search/Join 상태·검색 결과 이벤트를 기반으로 새 방 설정 계약을 확장한다.
- C-03: 기존 `ASnowRumblePlayerState`의 이름·팀·준비 복제와 `ASnowRumbleLobbyGameState::CanStartLobbyMatch`를 재사용하되, 팀 선택은 서버 랜덤 배정으로 대체한다.
- C-06: `USnowRumbleHealthComponent`의 HP·얼기 복제와 `ASnowRumbleCharacter::IsFrozen`을 출발점으로 삼고, 사망·관전·팀 전멸은 새 서버 상태로 추가한다.
- C-09: `USnowballCreationComponent`, `USnowballEquipmentComponent`, `ASnowballItem`의 현재 루프를 보존하고 큰 눈 광역 피해와 새 피해량 기준만 계약에 맞춰 수정한다.
- C-10: 기존 이모션 요청·검증·멀티캐스트 몽타주 흐름과 8칸 위젯 부모를 보존하고, 얼음·사망·시작 제한 조건만 추가한다.
- C-12: 위 재검증 결과와 S/K/J 결과를 마지막에 통합한다.

## 수동 작업 (구현 후 구체화)

- Unreal Editor에서 `BP_SnowRumbleCharacter`, `BP_SnowballItem`, `WBP_MainHUDWidget`, `WBP_OverheadTimedActionWidget`, `WBP_EmoteRadialMenuWidget`의 부모 클래스와 주요 프로퍼티 연결을 열어 확인한다.
- `ASnowballProjectile` 또는 `BP_SnowballProjectile` 참조가 남아 있는지 에디터 Reference Viewer로 확인한다.

## 완료 조건

### 에이전트 확인
- [x] 구조 개편 문서와 역할별 Task 배포 완료
- [x] 실제 코드·자산 기준 분류 완료
- [x] 소유권과 후속 인계 기록 완료
- [x] 기존 변경 보존 확인

### 결과 확인

- [x] 최재원(C)이 위 인수 결과가 개편 전 작업을 새 구조에 맞게 반영한 기준으로 충분한지 확인한다.
- [x] Unreal Editor에서 위 수동 작업의 자산 참조 확인을 완료한다.
- [x] 2026-08-08: 최재원(C)이 프로젝트 실행 결과 정상 동작을 확인했다.

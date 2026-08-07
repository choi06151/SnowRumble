# SnowRumble 개발 진행 기준선 - 2026-08-07

## 목적

이 문서는 기획과 협업 구조를 개편하기 전에 현재까지 구현된 기능, 검증 상태, 남은 범위와 보존할 연결 지점을 고정한 스냅샷이다.

- 기준 일자: `2026-08-07`
- 기준 브랜치와 커밋: `master`, `a58825e`
- 기준 시점 작업 트리: 변경 없음
- 확인 근거: `docs/PLANS.md`, 개별 Task 체크리스트, `Source/`, `Content/`, 기존 사용자 결과 확인 기록
- 이 문서는 당시 상태를 보존하는 기준선이며 이후 진행 상태를 계속 갱신하는 문서가 아니다.
- 새 기획에 따른 유지·변경·폐기 여부는 이 문서만으로 확정하지 않고 새 GDD와 Task에서 사용자가 결정한다.

## 다음 개편 작업에서 사용하는 방법

1. 기획 또는 협업 구조를 개편하기 전에 이 문서와 실제 코드·자산을 함께 확인한다.
2. 아래 기능을 `유지`, `수정`, `대체`, `폐기`, `재검증`으로 분류한다.
3. 기존 구현을 수정하거나 제거하려면 영향을 받는 C++ 클래스, Blueprint 연결 지점, 맵과 UI 자산을 먼저 기록한다.
4. 완료된 Task의 기록은 당시 검증 이력으로 보존한다. 새 기획으로 동작을 바꾸는 경우 기존 기록을 지우지 않고 후속 Task에서 변경한다.
5. `계획 상태`와 `실제 잔여 항목`이 다른 행은 계획 상태만 믿지 않고 해당 Task 체크리스트와 코드를 다시 확인한다.

## 기술 기준선

| 구분 | 현재 기준 |
| --- | --- |
| 엔진 | Unreal Engine `5.8` |
| 런타임 | `SnowRumble` C++ 모듈 |
| 네트워크 | Listen Server, OnlineSubsystem NULL, LAN Session |
| 기본 검증 인원 | 호스트 1명 + 클라이언트 1명 |
| 플레이어 수 목표 | 2~16명 |
| 입력 | Enhanced Input 기반 이동, 시점, 점프, 스프린트, 상호작용, 조준, 행동, 내려놓기, 이모션 |
| 주요 맵 | `L_MainMenu`, `L_Lobby`, `L_Prototype` |
| 주요 플레이어 자산 | `BP_SnowRumbleCharacter`, `ABP_SnowRumbleCharacter` |
| 주요 게임 자산 | `BP_MainMenuGameMode`, `BP_LobbyGameMode`, `BP_SnowRumbleGameMode`, `BP_SnowballItem` |

## Main Task 진행 시트

| Task | 계획 상태 | 구현·검증된 기준선 | 실제 잔여 항목과 주의점 |
| --- | --- | --- | --- |
| 01-1 프로젝트 기반 | 완료 | UE 5.8 프로젝트, 런타임 모듈, `L_Prototype` 기본 실행 환경을 확인했다. | 새 기획에서도 프로젝트 기준선으로 보존한다. |
| 01-2 이동·카메라·입력 | 완료 | 2인 Listen Server PIE에서 이동, 점프, 카메라, 서버 기준 스프린트와 상태 조회를 확인했다. | 입력 우선순위가 새 기획에서 바뀌면 기존 Enhanced Input 자산과 캐릭터 입력 분기를 함께 검토한다. |
| 02-1 최소 네트워크 눈덩이 | 완료 | 서버 생성 눈덩이의 생성·이동·충돌·제거 복제를 확인했다. | 현재 실제 눈싸움 파이프라인은 `ASnowballItem`을 사용한다. `ASnowballProjectile`은 기록 호환용 레거시이며 현재 플레이에서는 사용하지 않는다. |
| 02-2 HP·피해·얼기 | 완료 | 서버 권한 HP, 눈덩이 피해, 얼기, 행동 제한, 얼어 있는 플레이어의 충돌과 상태 복제를 확인했다. | 관전, 부활, 팀 전멸 판정은 아직 연결되지 않았다. |
| 03-1 LAN Host/Join | 완료 | NULL LAN 세션 생성·검색·참가와 작업 상태 이벤트를 별도 Editor 프로세스에서 확인했다. | 03-2 이후 Host 목적지는 `L_Prototype`이 아니라 `L_Lobby`다. 종료·재생성·재접속은 구현 범위가 아니다. |
| 03-2 로비 팀·준비 | 진행중 | 이름·Red/Blue 팀·준비 상태 복제, 시작 조건, Host 전용 시작 요청, 메뉴·로비 GameMode/PlayerController/Widget C++ 기본틀과 WBP 자산이 존재한다. | Host→Lobby, Join→Lobby, 양쪽 UI 동기화, 시작 조건과 ServerTravel에 대한 모든 결과 확인이 남아 있다. |
| 04-1 눈 제작·획득·내려놓기 | 완료 | 눈 바닥 제작, 서버 생성, 획득·손 장착, 획득 연출 상태, 로컬 아웃라인, `G` 내려놓기, 지면 고정과 재획득을 확인했다. | `SnowSurface` 태그, `SnowballSocket`, `BP_SnowballItem` 연결을 보존하거나 명시적으로 마이그레이션해야 한다. |
| 04-2 작은 눈 조준·충전·투척 | 완료 | 조준 이동, FOV·어깨 카메라, 서버 충전량, 카메라 Line Trace 조준점, 실제 보유 눈덩이 투척, 충전량 기반 피해·넉백을 확인했다. | 카메라와 투척 조정값을 새 기획에서 변경하더라도 서버의 조준 정보 검증은 유지해야 한다. |
| 04-3 굴리기·큰 눈 | `PLANS.md` 완료 | 굴리기, 거리 기반 성장, 성장 크기·속도, 전용 충돌 프록시, 장애물 밀림, 큰 눈 운반, 느린 포물선 투척과 직접 피해·넉백 코드가 존재한다. | 큰 눈 광역 영향은 미구현이다. 굴리기 프록시·지면 접촉과 큰 눈 직접 피격의 일부 호스트·클라이언트 결과 확인이 남아 있어 실제 범위는 완전 완료가 아니다. |
| 05-1 관전·핫팩 부활 | 예정 | 구현 없음. | 관전 대상 선택, 핫팩 소유·소모, 아군 부활, 부분 HP 회복이 모두 남아 있다. |
| 05-2 선물상자 | 예정 | 구현 없음. | 생성, 등급, 공격 개봉, 중복 방지, 보상 결정과 복제가 모두 남아 있다. |
| 05-3 MVP 아이템 | 예정 | 구현 없음. | 아이템 목록 확정부터 필요하다. |
| 05-4 게임플레이 UI 데이터 | 진행중 | `MainHUDWidget`, `HealthBarWidget`과 WBP 자산이 있으며 내·다른 플레이어 HP, 조준점, 투척 충전 바, 머리 위 진행 바 연결 기본틀이 존재한다. | Main HUD 전체 결과 확인이 남아 있다. 독립 제작 UI, 상호작용 행동, 보유·착용 대상, 얼기·관전 UI 데이터는 미구현이다. |
| 06-1 눈 쌓인 섬·경기 흐름 | 진행중 | 로컬 소유 카메라 전용 눈 Niagara VFX만 구현·확인했다. | 팀별 시작 위치·배치, 얼음물 피해, 영역 축소, 팀 전멸 판정, 승패 복제가 모두 남아 있다. |
| 06-2 결과·MVP 통합 검증 | 예정 | 구현 없음. | 결과 데이터·화면 인계, 다음 흐름, 사운드와 전체 MVP 검증이 남아 있다. |
| 07-1 이모션 원형 메뉴 | 진행중 | 8칸 C++ 위젯 연결, 로컬 메뉴 생성·입력 모드, 서버 인덱스 검증, 전체 화면 몽타주 재생 기본틀과 이모션 자산이 존재한다. | 호스트·클라이언트에서 메뉴 열기·닫기, 호버 선택, 행동 차단과 몽타주 재생 결과 확인이 전부 남아 있다. |

## SUB UI Task 진행 시트

SUB UI Task 상태는 자산 존재 여부와 다를 수 있다. Main Task에서 임시 또는 통합 WBP를 만들었더라도 SUB Task의 완료 조건을 통과한 것으로 간주하지 않는다.

| SUB Task | 계획 상태 | 현재 자산·연결 | 실제 잔여 항목 |
| --- | --- | --- | --- |
| S03-1 LAN Host/Join UI | 예정 | `WBP_MainMenu`와 C++ 자동 버튼 바인딩이 존재한다. | 세션 목록·상태를 포함한 SUB Task 전체 체크리스트는 미완료다. |
| S03-2 로비 UI | 예정 | `WBP_Lobby`와 `ULobbyWidget` 연결 기본틀이 존재한다. | 이름·팀·준비 목록과 Host 시작 UI의 정식 구현·검증이 남아 있다. |
| S04-1 상호작용 UI | 예정 | 아웃라인과 게임플레이 상태 함수는 존재한다. | 상호작용 안내, 가능한 행동, 제작 진행 UI는 미구현이다. |
| S04-2 조준·충전 UI | 예정 | `WBP_MainHUDWidget`에 조준점과 투척 충전 바를 연결할 C++ 기본틀이 존재한다. | SUB Task 자체는 완료 처리되지 않았으며 작은 눈·큰 눈 표시 구분을 포함한 정식 검증이 남아 있다. |
| S04-3 머리 위 행동 진행도 | 진행중 | `UOverheadTimedActionWidget`과 `WBP_OverheadTimedActionWidget`이 존재하고 눈 제작 진행 표시는 확인했다. | 눈 굴리기 성장률 표시·종료·로컬 전용 결과 확인이 남아 있다. |
| S05-1 관전·부활 UI | 예정 | 구현 없음. | Main Task 05-1 완료 후 시작한다. |
| S05-2 아이템 상태 UI | 예정 | 구현 없음. | Main Task 05-2·05-3 완료 후 시작한다. |
| S05-3 게임플레이 HUD | 예정 | `WBP_MainHUDWidget`, `WBP_HealthBar` 자산과 일부 C++ 데이터 연결이 존재한다. | HUD 전체 구성과 05-4의 남은 데이터, 얼기·관전·아이템 상태 통합 및 결과 확인이 남아 있다. |
| S06-1 결과 UI | 예정 | 구현 없음. | Main Task 06-1·06-2 결과 계약 확정 후 시작한다. |

## 현재 보존할 C++ 책임

### 세션과 로비

- `USnowRumbleSessionSubsystem`: NULL LAN Host·검색·Join 작업 상태와 결과 이벤트를 관리한다.
- `ASnowRumblePlayerState`: 로비 이름, 팀, 준비 상태를 서버에서 확정하고 복제한다.
- `ASnowRumbleLobbyGameState`: 참가자 목록과 경기 시작 가능 조건을 제공한다.
- `ASnowRumbleLobbyGameMode`: Host 시작 요청을 검사하고 게임 맵으로 ServerTravel한다.
- `AMainMenuPlayerController`, `ALobbyPlayerController`: 로컬 WBP 생성과 UI 입력 모드를 담당한다.

### 플레이어와 전투

- `ASnowRumbleCharacter`: 이동·입력·카메라와 여러 게임플레이 컴포넌트의 연결 지점이다.
- `USnowRumbleHealthComponent`: 서버 권한 HP와 얼기 상태를 관리·복제한다.
- 경기 결과에 영향을 주는 피해, HP, 얼기, 눈덩이 상태와 충전 결과는 서버가 확정한다.
- 카메라, FOV, 어깨 위치, 로컬 아웃라인, 로컬 UI와 로컬 눈 VFX는 소유 클라이언트 표현이다.

### 눈덩이 핵심 루프

- `USnowballCreationComponent`: 눈 바닥 판정, 제작 진행과 서버 생성 요청을 담당한다.
- `USnowballEquipmentComponent`: 획득·보유·내려놓기·굴리기와 장비 상태 전환을 담당한다.
- `ASnowballItem`: `Ground`, `Held`, `Rolling`, `Thrown` 상태와 성장·투척·충돌 결과를 담당한다.
- `UOutlineComponent`: 로컬 상호작용 대상의 Primitive Component에 Custom Depth를 적용한다.
- `ASnowballProjectile`: 초기 프로토타입 기록용 레거시 클래스이며 현재 보유 눈덩이 투척 파이프라인과 분리되어 있다.

### UI 연결 기본틀

- `UMainMenuWidget`, `ULobbyWidget`: 메뉴와 로비 WBP가 세션·로비 요청 및 상태를 읽는 부모 클래스다.
- `UMainHUDWidget`, `UHealthBarWidget`: Main HUD와 재사용 체력 바 부모 클래스다.
- `UOverheadTimedActionWidget`: 로컬 캐릭터 머리 위 진행 바를 갱신한다.
- `UEmoteRadialMenuWidget`: `EmoteButton0`~`EmoteButton7`을 선택 인덱스에 연결한다.

## 주요 Blueprint·자산 기준선

| 영역 | 현재 존재하는 주요 자산 |
| --- | --- |
| 맵 | `Content/Maps/L_MainMenu.umap`, `L_Lobby.umap`, `L_Prototype.umap` |
| 플레이어 | `BP_SnowRumbleCharacter`, `ABP_SnowRumbleCharacter`, 캐릭터 Mesh·애니메이션·얼기 표현 자산 |
| 게임 흐름 | `BP_MainMenuGameMode`, `BP_LobbyGameMode`, `BP_SnowRumbleGameMode`, 메뉴·로비 PlayerController Blueprint |
| 눈덩이 | `BP_SnowballItem`, 눈덩이 메시·머티리얼, 눈 충돌 Niagara |
| 입력 | `IMC_Player`와 이동·시점·점프·스프린트·상호작용·조준·행동·내려놓기·이모션 `IA_` 자산 |
| UI | `WBP_MainMenu`, `WBP_Lobby`, `WBP_MainHUDWidget`, `WBP_HealthBar`, `WBP_OverheadTimedActionWidget`, `WBP_EmoteRadialMenuWidget` |
| 환경 표현 | `NS_Snow`, 겨울 환경 에셋, `PP_Outline` |

## 문서·자산 이름 불일치

아래 이름은 Task 문서나 `PLANS.md`의 수동 작업 이름과 실제 저장소 자산 이름이 다르다. 개편 전에 실제 부모 클래스와 참조 상태를 Unreal Editor에서 확인한다.

- 문서의 `WBP_MainHUD`와 실제 `WBP_MainHUDWidget.uasset`
- 문서의 `WBP_OverheadActionProgress`와 실제 `WBP_OverheadTimedActionWidget.uasset`
- 문서의 `WBP_EmoteRadialMenu`와 실제 `WBP_EmoteRadialMenuWidget.uasset`

## 아직 구현되지 않은 MVP 핵심 범위

- 얼어붙은 플레이어의 아군 관전과 핫팩 부활
- 선물상자 생성·등급·공격 개봉·보상
- MVP 기본 아이템
- 큰 눈덩이 광역 피해·넉백
- 눈 쌓인 섬 팀 배치, 얼음물 피해와 영역 축소
- 팀 전원 얼기 승패 판정과 결과 복제
- 결과 화면, 다음 경기 흐름, 사운드와 전체 MVP 통합 검증
- 상호작용·보유 장비·얼기·관전·아이템을 포함한 완성형 HUD

## 상태 불일치와 협업 개편 전 주의점

- `docs/GDD/GDD_PROGRESS.md`의 `현재 구현 상태`와 `알려진 문제`는 초기 프로젝트 문구로 남아 있어 실제 구현보다 오래되었다.
- Task 04-3은 `PLANS.md`에서 완료지만 Task 체크리스트에는 미구현 광역 영향과 미확인 네트워크 결과가 남아 있다.
- Task 06-1은 메인 작업리스트에서 진행중이지만 `PLANS.md`의 현재 작업 배정 표에는 없다.
- `SnowRumbleCharacter.h/.cpp`는 05-4, 07-1, S04-3 작업 배정에 동시에 등장해 현재 단일 수정자 원칙과 충돌한다.
- Main Task에서 UI C++와 WBP 자산 일부를 함께 만들면서 별도 SUB UI Task 상태는 예정으로 남아 있다. 4인 구조에서는 기능 데이터 소유자와 UI 자산 소유자를 다시 분리해야 한다.
- `.uasset`과 `.umap`은 병합 가능한 텍스트로 간주하지 않고 한 시점에 한 담당자만 수정한다.

## 4인 협업 구조 개편 논의안

이 절은 `2026-08-07`까지 사용자와 논의한 4인 협업 방향을 다음 기획·구조 개편에서 참고하기 위한 초안이다.

- 아직 현재 프로젝트의 확정된 역할·소유권 규칙이 아니다.
- 새 기획에서 세 맵의 개발 범위와 우선순위를 확정한 뒤 `COLLABORATION.md`, `WORKFLOW.md`, `PLANS.md`와 Task 구조에 반영한다.
- 이 초안을 근거로 현재 역할, 경로 소유권이나 Task 상태를 선행 변경하지 않는다.
- 팀원 전원이 멀티플레이 개발 경험이 없고 각자 별도 Codex를 사용해 담당 영역을 개발하는 상황을 전제로 한다.

### 논의된 역할 구성

| 역할 | 기본 책임 | 논의된 기본 소유 범위 |
| --- | --- | --- |
| 메인·멀티플레이 통합 | 공용 네트워크 계약, 세션·로비·경기 흐름, 공유 플레이어·전투 기반, Task 배정, 코드 리뷰와 최종 병합 | `Source/Game`, `Source/Online`, 공유 `Source/Player`·`Source/Snowball`, `docs`, `Tasks` |
| UI·UX | 메뉴, 로비, HUD, 관전·아이템·결과 UI의 C++ 부모와 WBP 구성 | `Source/UI`, `Content/WBP`, UI 전용 아이콘·폰트·애니메이션 |
| 레벨 A | 눈 쌓인 섬과 놀이터의 맵 디자인, 맵 전용 C++·Blueprint·환경 로직 | 향후 `Source/World/SnowIsland`, `Source/World/Playground`, 각 맵 전용 `Content/Maps`와 환경 자산 |
| 레벨 B | 얼음 동동 빙하의 맵 디자인, 맵 전용 C++·Blueprint·환경 로직 | 향후 `Source/World/IceFloe`, 해당 맵 전용 `Content/Maps`와 환경 자산 |

세 맵의 현재 기획상 특징은 아래와 같다.

- 눈 쌓인 섬: 얼음물 피해, 수위 상승과 플레이 영역 축소가 핵심이다.
- 얼음 동동 빙하: 미끄러운 바닥, 빙하별 체류시간 누적과 침수가 핵심이다.
- 놀이터: 작은 공간, 놀이기구와 근접 전투 동선이 핵심이다.

눈 쌓인 섬과 놀이터를 레벨 A에, 동적 규칙이 상대적으로 많은 얼음 동동 빙하를 레벨 B에 배정하는 안을 우선 논의했다. 새 기획에서 복잡도와 개발 순서가 바뀌면 맵 배정도 다시 조정한다.

### 역할별 자율 개발 원칙

- 메인이 모든 멀티플레이 C++ 슬롯을 대신 만드는 구조로 고정하지 않는다.
- UI와 두 레벨 담당자는 자기 소유 경로 안에서 Codex를 사용해 C++와 Blueprint를 함께 개발하고 오류도 직접 수정할 수 있다.
- 메인은 다른 담당자의 코드를 대신 작성하는 사람이 아니라 공용 네트워크 계약, 공유 파일 변경, 서버 권한 구조와 통합 결과를 검토하는 담당자다.
- 담당 영역 내부 구현 방식은 담당자가 결정하되 서버 권한, 복제, 파일 소유권과 파트 간 데이터 계약은 공통 규칙을 따른다.
- 다른 역할의 파일 변경이 필요하면 직접 수정하지 않고 해당 소유자에게 필요한 API와 이유를 인계한다.

### 파트 간 책임 경계

#### 메인·멀티플레이 통합

- OnlineSubsystem NULL 세션과 맵 이동
- 공용 `GameMode`, `GameState`, `PlayerState`
- 경기 단계, 팀, 승패와 결과 데이터
- 공용 플레이어·눈덩이·피해 시스템의 공유 계약
- 파트 간 API와 이벤트의 실행 위치 검토
- 중앙 작업 배정과 병합 순서 관리

#### UI·UX

- `Source/UI`와 `Content/WBP` 내부 문제를 직접 구현·수정
- 메인 또는 다른 파트가 제공한 읽기 전용 상태와 요청 함수 사용
- 버튼, 목록, Progress Bar, 아이콘, 텍스트, UI 애니메이션 구성
- UI 로컬 값으로 팀, 피해, 아이템 보상이나 승패를 확정하지 않음
- 게임플레이 데이터가 부족하면 UI에서 중복 판정하지 않고 소유 파트에 API를 요청

#### 레벨 A·B

- 담당 맵의 `.umap` 단독 소유
- 담당 맵 전용 Actor, Component, GameMode 보조 객체와 Blueprint 구현
- 환경 위험, 맵 오브젝트와 공간 변화의 서버 권한·복제 처리
- 담당 맵의 배치, 동선, 조명, VFX, 사운드와 밸런스 조정
- 공용 플레이어 상태를 직접 덮어쓰지 않고 공개된 피해·경기 API 사용
- 주요 게임플레이 로직을 Level Blueprint에 집중하지 않고 배치 가능한 전용 Actor나 Component로 구성

### 세 맵 코드·자산 분리 예시

아래 경로는 구조 개편 시 검토할 예시이며 현재 생성 대상이 아니다.

```text
Source/SnowRumble/World/
├─ Common/                 # 메인 또는 별도 확정 소유자
├─ SnowIsland/             # 레벨 A
├─ Playground/             # 레벨 A
└─ IceFloe/                # 레벨 B

Content/Maps/
├─ SnowIsland/             # 레벨 A
├─ Playground/             # 레벨 A
└─ IceFloe/                # 레벨 B
```

- 공통 경기 수명주기와 데이터 계약만 `World/Common` 후보로 둔다.
- 맵 전용 규칙과 자산은 각 맵 폴더에서 소유한다.
- 두 맵에서 실제 중복이 확인되기 전에는 공통 프레임워크를 미리 과도하게 만들지 않는다.
- 하나의 `.umap`에는 반드시 한 명의 수정 담당자만 둔다.

### 역할별 Task 묶음 초안

새 기획 반영 후 기존 단일 `Tasks/` 목록을 아래 네 역할 묶음으로 재편하는 안을 논의했다.

```text
Tasks/
├─ Main/
│  ├─ ROLE_MAIN.md
│  └─ M-XX_*.md
├─ UI/
│  ├─ ROLE_UI.md
│  └─ U-XX_*.md
├─ LevelA/
│  ├─ ROLE_LEVEL_A.md
│  └─ LA-XX_*.md
└─ LevelB/
   ├─ ROLE_LEVEL_B.md
   └─ LB-XX_*.md
```

- `docs/PLANS.md` 하나만 전체 진행 상태와 의존성의 기준 원본으로 유지한다.
- `PLANS.md`는 메인·통합 담당자가 갱신하고 다른 담당자는 자기 Task 결과와 상태 변경 요청을 인계한다.
- 역할별 `ROLE_*.md`는 허용 경로, 금지 경로, 기본 검증과 인계 대상을 정의한다.
- 개별 Task는 한 번에 구현하고 검증할 수 있는 작은 동작 단위로 작성한다.
- 각 담당자는 별도 브랜치와 별도 worktree 또는 별도 clone에서 자기 Codex를 사용한다.

개별 Task에는 최소한 아래 항목을 기록한다.

- 담당 역할과 검토 담당자
- 선행 Task와 시작 가능 상태
- 수정 가능 경로와 수정 금지 경로
- 단독 선점 파일·`.uasset`·`.umap`
- 로컬 입력·표현, 서버 판정, 복제 상태, Blueprint 책임
- 다른 파트에서 인계받은 실제 클래스·함수·이벤트·프로퍼티 이름
- 구현 항목, 에디터 수동 작업과 호스트·클라이언트 결과 확인
- 완료 후 인계 대상과 병합 순서

### 공용 프로젝트 Skill 논의안

4개의 역할별 Skill을 각각 만들기보다 하나의 공용 Task 실행 Skill을 두는 안을 논의했다.

```text
.agents/skills/snowrumble-run-task/SKILL.md
```

공용 Skill의 역할은 규칙 원본을 복제하는 것이 아니라 현재 역할과 Task를 입력받아 공통 실행 순서를 적용하는 것이다.

1. 역할과 활성 Task를 확인한다.
2. `AGENTS.md`, 담당 `ROLE_*.md`, 현재 Task와 선행 Task를 읽는다.
3. `PLANS.md`의 배정·선점·의존성을 확인한다.
4. 수정 허용·금지 경로와 다른 담당자 소유 대상을 검사한다.
5. 로컬·서버·복제·Blueprint 책임을 구분한다.
6. 기존 승인 절차에 따라 구현 범위를 제시한다.
7. 승인 후 자기 범위만 구현하고 검증·인계 결과를 작성한다.
8. 파트 경계를 넘는 변경은 직접 수행하지 않고 소유자에게 반환한다.

규칙의 책임은 아래처럼 분리한다.

| 기준 | 담당 문서 |
| --- | --- |
| 항상 적용할 프로젝트 규칙과 문서 트리거 | `AGENTS.md` |
| 4인 역할, 소유권과 충돌 대응의 상세 원본 | `docs/COLLABORATION.md` |
| 현재 담당자, 브랜치, 선점과 상태 | `docs/PLANS.md` |
| 담당별 실제 구현 범위와 완료 조건 | 역할별 Task 문서 |
| 위 문서를 읽고 Task를 실행하는 반복 절차 | 공용 `SKILL.md` |

Skill은 지침이며 여러 Codex 사이의 파일 잠금이나 실시간 상태 동기화를 제공하지 않는다. 실제 충돌은 worktree·브랜치 분리, 중앙 배정표, 단일 자산 소유와 메인의 병합 절차로 방지한다.

### 담당별 Codex 실행 예시

각 담당자는 자기 작업 환경에서 공용 Skill을 명시적으로 호출하고 역할과 Task를 전달하는 방식을 검토한다.

```text
$snowrumble-run-task

역할: LevelA
Task: LA-02
```

Codex는 다음 조건에서 구현을 중단하고 해당 소유자 또는 메인에게 인계해야 한다.

- 필요한 공용 API가 아직 없음
- 다른 담당자의 파일이나 자산을 수정해야 함
- GDD에 없는 규칙을 새로 결정해야 함
- 서버와 클라이언트의 실행 위치가 불명확함
- 선점된 `.uasset`이나 `.umap` 수정이 필요함
- 선행 Task 문서와 실제 코드가 일치하지 않음

### 브랜치·병합 초안

```text
main/M-XX
ui/U-XX
level-a/LA-XX
level-b/LB-XX
```

- 각 담당자는 별도 작업 폴더를 사용한다.
- 공용 C++ 계약을 먼저 반영하고, 맵 전용 C++·Blueprint, UI 순으로 의존성을 정한다.
- 서로 독립적인 맵은 레벨 A와 B가 병렬로 개발할 수 있다.
- `.uasset`과 `.umap`은 자동 병합하지 않는다.
- 메인은 담당자 결과를 임의로 재작성하기보다 계약·검증·충돌을 확인한 뒤 통합한다.

### 새 기획에서 확정할 항목

- 세 맵이 모두 MVP인지, 순차 개발·출시 범위인지
- 눈 쌓인 섬·놀이터와 얼음 동동 빙하의 최종 담당 배정
- 공용 플레이어·눈덩이·아이템 시스템의 소유자와 메인의 직접 구현 범위
- 선물상자와 공용 아이템을 메인 또는 어느 레벨 담당이 소유할지
- UI 담당자가 `Source/UI` C++까지 소유할지, WBP만 소유할지
- `Source/World/Common`이 실제로 필요한지와 소유자
- 중앙 `PLANS.md`의 단일 작성자와 상태 갱신 인계 방식
- 공용 `snowrumble-run-task` Skill의 도입 여부와 명시적 호출 방식
- 기존 진행중 Task를 완료·중지·마이그레이션 중 어느 상태로 전환할지
- 현재 `GDD_PROGRESS.md`에서 MVP 이후로 분류된 얼음 동동 빙하와 놀이터의 새 범위

## 기존 Pending Issues

- `UOverheadTimedActionWidget`과 `ESnowRumbleTimedActionState`가 시간 기반이 아닌 굴리기 성장도 함께 표현한다. 진행형 행동이 늘면 이름과 WBP 부모 리디렉션을 검토한다.
- `RollingSnowballCollision`이 `ASnowRumbleCharacter`에 직접 있다. 충돌 규칙이 늘면 전용 컴포넌트 분리를 검토한다.
- 레거시 `ASnowballProjectile`은 현재 파이프라인에서 사용하지 않는다. 참조 확인 없이 삭제하지 않는다.
- `USnowRumbleSessionSubsystem`에 공급자·재접속·종료 생명주기가 추가되면 책임 분리를 검토한다.

## 이 스냅샷의 검증 한계

- 이번 기준선 작성 과정에서는 Unreal Editor, C++ 빌드 또는 새 PIE 검증을 실행하지 않았다.
- 완료와 결과 확인 표시는 기존 Task에 기록된 사용자 확인 결과를 사용했다.
- `.uasset`과 `.umap` 내부 그래프·프로퍼티는 텍스트로 직접 검증하지 않았으며 파일 존재와 Task 기록만 대조했다.
- 기획 개편을 시작할 때 진행중 Task와 이름 불일치 자산은 Unreal Editor에서 다시 확인한다.

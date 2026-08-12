# Task C-25 - PvP 선물상자와 아이템 기본 계약

## 설명

PvP 라운드 중 서버가 맵에 배치된 후보 지점에서 선물상자를 주기적으로 생성하고, 모든 플레이어에게 등장 안내 문구를 보여주며, 플레이어가 가까이에서 `E`로 상자를 열면 서버가 획득자와 랜덤 보상 아이템을 확정하는 첫 아이템 기반을 만든다.

## 상태 전이 기준

- 시작 가능: C-01 기존 기반, C-05 PvP 라운드 시간, C-22 상호작용 안내 UI 계약을 재사용할 수 있고, 선물상자 첫 범위의 보상 효과를 "획득 확정과 로그/알림까지"로 제한한다.
- 완료 가능: 서버 권한 스폰·낙하·상호작용·랜덤 보상 확정, Blueprint 연결 지점, 수동 배치 절차와 호스트/클라이언트 결과 확인 절차가 실제 코드 이름으로 기록되어야 한다.

## 구현 항목

- [x] PvP GameMode가 라운드 시작 후 지정 간격마다 맵의 `TargetPoint` 후보 중 하나를 서버에서 골라 선물상자를 공중 스폰한다.
- [x] 선물상자는 서버 권한 Actor로 복제되고, Blueprint가 모델·VFX·사운드와 낙하 표현을 연결할 수 있는 C++ 부모를 제공한다.
- [x] 상자 생성 시 모든 플레이어에게 `산타가 선물을 흘렸다네` 문구를 표시한다.
- [x] 로컬 플레이어가 상자 근처에 있으면 기존 상호작용 안내 UI에 `E - 선물상자`를 표시한다.
- [x] 플레이어가 `E`로 상자 개봉을 요청하면 서버가 거리, 캐릭터 상태, 라운드 상태, 이미 개봉 여부를 검사해 먼저 성공한 개봉만 확정한다.
- [x] 눈덩이 등 원거리 공격으로 선물상자에 데미지가 들어오면 서버가 선물상자를 연다.
- [x] 상자 하나에서 랜덤 아이템 하나를 선택하고, 보상을 즉시 장착하지 않고 해당 위치에 아이템 Pickup Actor를 스폰한다.
- [x] 플레이어가 스폰된 아이템 근처에서 `E`를 누르면 서버가 획득을 확정하고 획득 로그/알림을 제공한다.
- [x] 등급별 보상 후보, 보상별 Pickup Blueprint 클래스와 상자 Blueprint 클래스를 에디터에서 조정할 수 있게 한다.

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: C++·문서: 최재원(C), 선물상자 Blueprint·모델·맵 TargetPoint 배치: 사용자/S/J 인계
- 생성·변경 후보: `Source/SnowRumble/Item/GiftBox_C.*`, `Source/SnowRumble/Item/GiftBoxItemPickup_C.*`, `Source/SnowRumble/Game/SnowRumbleGameMode.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `docs/ARCHITECTURE.md`, `Tasks/C/C-25_pvp_gift_box_item_foundation.md`
- 공유 확인 대상: 사용자, S/J 맵 담당, S 아이템 모델 담당
- 병합 순서: C-25 공용 C++ 계약 구현 후 각 PvP 맵 담당이 TargetPoint 배치와 선물상자 Blueprint 연결

## 공용 계약과 인계

- 제공받을 계약: C-05 PvP 라운드 시간과 종료 상태, C-22 상호작용 안내 UI 경로, 기존 PlayerController 개인 알림/이벤트 로그 경로
- 제공할 계약: `AGiftBox`, `AGiftBoxItemPickup`, `ESnowRumbleGiftBoxGrade::Red`/`Gold`, `FSnowRumbleGiftBoxReward::PickupClass`, `AGiftBox::CanInteractWith()`, `AGiftBox::TryOpen()`, `AGiftBox::TakeDamage()`, `AGiftBox::OnGiftBoxGradeChanged()`, `AGiftBox::OnGiftBoxLanded()`, `AGiftBox::OnGiftBoxOpened()`, `AGiftBoxItemPickup::TryPickup()`, `AGiftBoxItemPickup::OnItemDataChanged()`, `AGiftBoxItemPickup::OnItemPickedUp()`, `ASnowRumbleGameMode::GiftBoxClass`, `GiftBoxSpawnPointTag`, `FirstGiftBoxSpawnDelaySeconds`, `GiftBoxSpawnIntervalSeconds`, `GoldGiftBoxSpawnChance`
- 인계 대상: 사용자/S/J는 PvP 맵에 선물상자 Spawn Point용 `TargetPoint`를 배치한다. `GiftBoxSpawnPointTag` 기본값은 `GiftBoxSpawn`이며, 해당 태그가 붙은 TargetPoint가 없으면 맵의 모든 TargetPoint를 후보로 사용한다. 사용자/S는 `AGiftBox` 기반 선물상자 Blueprint 모델·낙하 표현·개봉 연출을 연결한다.

## 범위 밖

- 핫팩, 핫초코, 에너지 드링크, 부츠, 패딩, 장갑, 눈삽, 눈오리 제작기, 황금 아이템의 실제 능력 효과 적용
- 아이템 장비 외형, 내구도, `G` 내려놓기, 인벤토리성 보관 규칙
- 마지막 5분 30초 금색 상자와 뒤처진 팀 보정 확률
- 공격으로 상자를 여는 규칙
- 맵 `.umap` 직접 수정과 Unreal Editor 자산 생성

## 사전 전제

- C-01 기존 기반 완료
- C-05의 PvP GameMode/GameState 라운드 시간과 종료 상태를 재사용한다.
- C-22의 상호작용 안내 위젯 경로를 재사용한다.

## 결정 필요

- 이번 첫 구현의 랜덤 보상 후보는 실제 효과 없이 "아이템 이름 확정"까지로 둔다.
- 일반 상자의 정확한 등급별 확률과 실제 아이템 효과는 후속 Task에서 확정한다.

## 변경 기록

- 2026-08-12: 사용자가 PvP 내부 아이템 기능의 첫 단계로 선물상자를 요청해 C-25를 추가했다.
- 2026-08-12: TargetPoint는 레벨 담당자가 배치하고, 선물상자는 빨간색과 황금색 두 등급만 사용하며 등급별 등장 아이템 후보가 다르다는 기준을 반영해 구현했다.
- 2026-08-12: 사용자가 선물상자 개봉 후 즉시 장착이 아니라 아이템 BP가 등장하고, 그 아이템을 `E`로 획득하는 구조를 요청했다. 선물상자는 `E` 또는 데미지로 열리고, 보상별 `PickupClass`를 스폰하도록 변경했다.
- 2026-08-12: 사용자가 아이템 Pickup이 약간 둥둥 떠다니는 표현을 요청해 `AGiftBoxItemPickup`의 `ItemMeshComponent`에 로컬 위아래 부유 Tick을 추가했다.
- 2026-08-12: 아이템 Pickup 기본 `FloatSpeed`를 5로 조정하고, 선물상자 등급별 `RedGiftBoxMaterial`/`GoldGiftBoxMaterial` 슬롯과 개봉 시 `OpenedEffect` Niagara 슬롯을 추가했다.
- 2026-08-12: 아이템 BP 내부 연결용 `ESnowRumbleGiftItemType` enum을 추가했다. 선물상자 보상 후보의 `ItemType`이 스폰된 `AGiftBoxItemPickup`에 복제되고, Pickup BP는 `GetItemType()` 또는 `OnItemDataChanged(NewItemType, NewItemId, NewDisplayName)`로 아이템 종류를 구분할 수 있다.

## 수동 작업 (구현 후 구체화)

1. `AGiftBox`를 부모로 하는 선물상자 Blueprint를 만든다.
2. 선물상자 Blueprint에서 `GiftBoxMeshComponent`에 모델을 연결하고, `OnGiftBoxGradeChanged`에서 빨간색/황금색 외관을 구분한다.
3. `AGiftBoxItemPickup`을 부모로 하는 아이템 Pickup Blueprint를 만든다.
4. Pickup Blueprint에서 `ItemMeshComponent`에 아이템 모델을 연결하고, `OnItemDataChanged`에서 `ItemId` 또는 표시 이름에 따라 모델·색·텍스트를 바꾼다.
5. 선물상자 Blueprint의 `RedBoxRewards`와 `GoldBoxRewards` 배열에서 등급별 보상 후보 이름과 `PickupClass`를 지정한다. 공통 Pickup BP 하나를 쓰려면 `DefaultPickupClass`에 지정한다.
6. PvP GameMode Blueprint 또는 클래스 기본값에서 `GiftBoxClass`에 선물상자 Blueprint를 지정한다.
7. PvP 맵마다 레벨 담당자가 `TargetPoint`를 배치한다. 선물상자 전용 후보만 쓰려면 Actor Tag에 `GiftBoxSpawn`을 추가한다.
8. 필요하면 `FirstGiftBoxSpawnDelaySeconds`, `GiftBoxSpawnIntervalSeconds`, `GiftBoxSpawnHeightOffset`, `GoldGiftBoxSpawnChance`를 조정한다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드·Blueprint·자산 변경 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료
- [x] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [x] 공용 계약과 캡슐화 규칙 위반 없음
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인 (구현 후 구체화)

- [ ] 호스트와 클라이언트로 PvP 라운드에 들어가고, 로딩/카운트다운 이후 `GiftBoxSpawn` 태그가 붙은 TargetPoint 위 공중에서 선물상자가 떨어지는지 확인한다.
- [ ] 호스트와 클라이언트 화면 모두에 `산타가 선물을 흘렸다네` 문구가 표시되는지 확인한다.
- [ ] 상자 가까이에서 `E - 선물상자` 안내가 표시되는지 확인한다.
- [ ] 호스트가 `E`로 열었을 때 상자가 한 번만 열리고 빨간색 또는 황금색 등급 후보 중 하나의 아이템 Pickup BP가 해당 위치에 스폰되는지 확인한다.
- [ ] 클라이언트가 `E`로 열었을 때 서버가 같은 방식으로 아이템 Pickup BP를 스폰하는지 확인한다.
- [ ] 눈덩이를 선물상자에 맞췄을 때 데미지로 상자가 열리고 아이템 Pickup BP가 스폰되는지 확인한다.
- [ ] 호스트와 클라이언트가 스폰된 아이템 근처에서 `E`로 획득했을 때 획득 로그/알림이 보이고 아이템 Pickup이 사라지는지 확인한다.

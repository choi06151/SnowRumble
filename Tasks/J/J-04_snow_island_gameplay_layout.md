# Task J-04 - 눈섬 게임플레이 배치

## 설명

눈섬에 팀 시작 지점, 기본 눈덩이, 상자·모닥불 후보를 배치하고, 경기 후반 기존 Water Actor가 천천히 상승해 플레이어를 침수시키면 기존 Damage 시스템으로 지속 피해를 적용하는 플레이 가능한 맵으로 완성한다.

## 상태 전이 기준

- 시작 가능: J-03, C-04, C-05, C-14, K-02, K-08 완료 또는 J-04 최소 구현에 필요한 인계 확인
- 완료 가능: 눈섬 Gameplay 배치, 물 상승, 침수 판정, 서버 Damage, 호스트·클라이언트 결과 확인

## 구현 항목

- [ ] 팀별 시작 지점과 5초 소개 공간을 배치한다.
- [ ] 기본 눈덩이, 선물상자 Spawn Point와 기본 모닥불 후보를 배치한다.
- [x] 기존 눈섬 Water Actor를 재사용해 경기 후반 물이 천천히 상승하게 한다.
- [x] 0~240초는 물 높이 변화와 Water Damage를 비활성화한다.
- [x] 240~300초는 외곽 낮은 지형이 잠기도록 물을 보간 상승시킨다.
- [x] 300~360초는 안전 지역이 중앙으로 축소되도록 물을 추가 보간 상승시킨다.
- [x] 360초 이후 연장전에는 `OvertimeRiseSpeed` 값에 따라 물을 계속 상승시킬 수 있게 한다.
- [x] 서버에서 Character Capsule 하단 기준으로 침수 여부를 판정한다.
- [x] 침수 중인 플레이어에게 Character별 유효 침수시간 1초가 누적될 때마다 8 Damage를 1회 적용한다.
- [x] 물 상승 수위가 모닥불 위치에 닿으면 `ACampfire::ExtinguishFromWater()`로 모닥불을 즉시 끈다.
- [x] 침수 중인 플레이어가 물속에서 약하게 떠오르도록 서버에서 부력 이동 보정을 적용하고, 물속에서는 점프 입력을 차단한다.
- [x] Damage는 `UGameplayStatics::ApplyDamage`를 통해 기존 `ASnowRumbleCharacter::TakeDamage()`와 `USnowRumbleHealthComponent::ApplyDamage()` 흐름을 재사용한다.
- [x] Damage 호출은 `RequestHazardDamage(AActor* Target, float DamageAmount)` 한 함수에 격리해 C-07 공용 Damage 계약 완료 후 교체 지점을 명확히 둔다.
- [x] 물 상태 표현에 필요한 최소 상태만 복제하고 HP는 기존 HealthComponent 복제를 사용한다.
- [x] 핵심 Gameplay 판정과 Damage를 Level Blueprint나 Blueprint Event Graph에 중복 구현하지 않는다.
- [ ] 6분 이후에도 중앙 전투가 지속될 최소 공간과 연장전 압박을 제공한다.

## 작업 배정

- 담당자: 정다영(J)
- 기능 소유자: 정다영(J)
- 계약 소유자: 레벨·눈섬 환경 로직은 정다영(J), 공용 피해·경기 계약은 최재원(C), 아이템 배치는 강혜원(K)
- 자산 수정자: 정다영(J)
- 생성 파일: `Source/SnowRumble/Map/SnowIslandWaterPressureActor_J.h`, `Source/SnowRumble/Map/SnowIslandWaterPressureActor_J.cpp`
- 변경 후보: `Tasks/J/PLAN_J.md`, `Tasks/J/J-04_snow_island_gameplay_layout.md`, `Content/Maps/L_SnowIsland_J.umap`
- 선점 대상: `Content/Maps/L_SnowIsland_J.umap`은 기존 J-03 기록에 따라 정다영(J) 단독 수정 자산으로 유지한다.
- 제외 대상: `Content/Game/BP_LobbyGameMode.uasset`은 J-04 대상이 아니며 수정·stage·restore·commit에 포함하지 않는다.
- 공유 확인 대상: C-04, C-05, C-07, C-14, K-02, K-08, S-04
- 병합 순서: J-03 → J-04 최소 C++·맵 배치 → C·K 필요 계약 확인 → J-05

## 공용 계약과 인계

- 제공받을 계약:
  - C-04 랜덤 PvP 맵 진입
  - C-05 `ASnowRumbleGameState::GetRoundElapsedSeconds()` 경기 시간, 기존 HUD 시간 표시, 라운드 종료 흐름
  - C-06 기존 `ASnowRumbleCharacter::TakeDamage()`와 `USnowRumbleHealthComponent::ApplyDamage()` HP·얼기 흐름
  - C-14 팀 시작 지점과 시작 연출 계약
  - K-02, K-08 상자·아이템·모닥불 배치 대상
- 현재 재사용할 Damage 경로: `UGameplayStatics::ApplyDamage` → `ASnowRumbleCharacter::TakeDamage()` → `USnowRumbleHealthComponent::ApplyDamage()` → `CurrentHealth` 복제
- 제공할 계약:
  - `ASnowIslandWaterPressureActor` 배치 Actor와 Editor 조정값
  - Runtime에서 움직일 단일 기존 `Water` Actor/Component 참조 방식
  - 회의 확정 시간표 0~240 / 240~300 / 300~360 / 360초 이후 단계
  - `RequestHazardDamage(AActor* Target, float DamageAmount)` C-07 교체 경계
- 인계 대상: C-12, K-10, S-04, J-05

## 범위 밖

- 신규 HP 시스템, J 전용 HealthComponent, HP 중복 변수, 별도 HP replication
- Character 또는 HealthComponent 내부의 눈섬 전용 로직
- 범용 Hazard Framework, 범용 Environment Base Class, J-02까지 미리 고려한 공통 Framework
- 새로운 Water Mesh, WaterVisual Component, 필수 Blueprint Child 생성
- 공용 GameMode/GameState/MatchSubsystem 공개 구조 변경
- C 소유 GDD와 `docs/ARCHITECTURE.md` 직접 수정

## 사전 전제

- J-03 눈섬 블록아웃
- C-04, C-05의 PvP 맵 진입과 경기 시간 계약
- 기존 Damage/HP 흐름

## 결정 필요

- 기존 GDD의 `눈섬 = 자기장` 표현과 회의 결정 `눈섬 = 물 상승` 사이 정합성은 C 통합 시 중앙 문서에서 확인한다.
- 240/300/360초 시간표를 HUD 카운트다운에 표시하려면 공용 GameState/HUD 최소 변경이 필요한지 C와 별도 확인한다. 이번 3일 최소 구현의 blocker로 두지 않는다.
- 실제 `L_SnowIsland_J.umap`에서 기존 Water Actor의 최종 목표 위치와 연장전 상승 속도는 Editor에서 튜닝한다.

## 변경 기록

- 2026-08-13: 신규 J-06을 만들지 않고 J-04 안에서 눈섬 물 상승, 침수 판정과 1초 8 Damage 구현을 진행하기로 확정했다.
- 2026-08-13: J-04의 핵심 Gameplay는 C++ 중심으로 구현하고 Blueprint는 필요 시 Material, VFX, Sound, 시각 경고와 프리셋을 다듬는 용도로 제한했다.
- 2026-08-13: 기존 HP/Damage/동결/사망/라운드 종료 흐름을 재사용하고, C-07 완료 전에는 `RequestHazardDamage()` 내부에서 기존 `UGameplayStatics::ApplyDamage` 경로를 사용하기로 정리했다.
- 2026-08-13: Water Damage는 전역 Timer 순간 판정이 아니라 서버 0.1초 체크에서 Character별 `DamageProgressSeconds`를 누적하고 1초마다 8 Damage를 1회 적용하는 방식으로 조정했다. `ExitGraceSeconds` 기본값은 0.5초다.
- 2026-08-13: Runtime Water는 실제 `Water` 하나만 제어하고 `Water2`/`Water3`는 목표 World Z 참고용으로만 사용하기로 정리했다. 물 상승과 침수 판정은 World Z 기준이다.
- 2026-08-19: 최재원(C) 통합 보정으로 침수 중인 캐릭터에 서버 부력 보정을 추가했다. `bApplyWaterBuoyancy`, `BuoyancyTargetSubmersionDepth`, `BuoyancyCorrectionSpeed`, `BuoyancyMinimumUpwardVelocity`, `BuoyancyMaximumUpwardVelocity`로 물속 둥둥 뜨는 정도를 조정하고, `ASnowRumbleCharacter::SetWaterSubmergedFromServer()` 복제 상태로 물속 점프 입력을 차단한다.
- 2026-08-19: 물속에서 조금 더 통통 튀는 느낌을 주기 위해 `bApplyWaterBounce`, `WaterBounceFrequency`, `WaterBounceUpwardVelocity`를 추가했다. 기본 부력 속도 위에 서버 시간 기반 상승 펄스를 더하며, 캐릭터별 위상을 조금 달리해 여러 플레이어가 같은 박자로 튀지 않게 했다.
- 2026-08-21: C-25 모닥불 계약과 연동해 `ASnowIslandWaterPressureActor`가 서버 Damage Timer에서 물에 닿은 `ACampfire`를 찾아 `ExtinguishFromWater()`로 즉시 끄게 했다. 물에 닿은 모닥불은 Actor와 Mesh가 남고 회복·충돌·VFX만 비활성화된다. `git diff --check`와 충돌 표식 검색은 통과했고, `SnowRumbleEditor Win64 Development` 빌드는 Live Coding 활성화로 보류됐다.

## 구현 현황

- 2026-08-13 `782dbd1`에서 `ASnowIslandWaterPressureActor` C++ 구현(`Source/SnowRumble/Map/SnowIslandWaterPressureActor_J.h`, `Source/SnowRumble/Map/SnowIslandWaterPressureActor_J.cpp`)을 추가했다.
- 이 Actor는 기존 Water Actor를 움직이는 눈섬 전용 환경 압박 Actor다. 복제 상태는 물 단계와 현재 수위(`CurrentWaterStage`, `CurrentWaterZ`)로 제한하고, 클라이언트는 복제된 수위를 기존 Water Actor 표현에 적용한다.
- Runtime 제어 대상은 `ControlledWaterActor` 하나다. `Water2`와 `Water3`는 `OuterFloodWaterZ`, `CentralFloodWaterZ`를 정할 때 쓰는 높이 참고용이며, 런타임 이동 대상이 아니다.
- Water Component는 `WaterComponentName`으로 찾는다. 기본 이름은 `StaticMeshComponent0`이며, 해당 Component를 찾지 못하면 `RootComponent`를 사용한다.
- 시작 수위는 기본적으로 `ControlledWaterActor`의 현재 World Z를 `InitialWaterZ`로 사용한다. 필요하면 `bUseControlledWaterInitialZ`를 끄고 `ManualInitialWaterZ`로 직접 지정할 수 있다.
- 수위는 `ASnowRumbleGameState::GetRoundElapsedSeconds()`를 기준으로 World Z에서 계산한다. 0~240초는 안정 구간, 240~300초는 외곽 침수, 300~360초는 중앙 방향 침수, 360초 이후는 overtime 상승 구간이다.
- Water 이동은 X/Y를 유지하고 Z만 갱신한다.
- 침수 판정은 서버에서 Character Capsule 하단 Sample Z와 현재 Water Z를 비교한다.
- Water Damage는 캐릭터별 침수 상태를 따로 누적하고, 1초의 유효 침수마다 기존 Damage 경로로 8 Damage를 적용한다.
- 모닥불 침수는 캐릭터 Damage 누적과 분리해 서버 Damage Timer에서 `ACampfire` 위치 Z와 현재 Water Z를 비교하고, 물에 닿으면 즉시 `ACampfire::ExtinguishFromWater()`를 호출한다.
- 침수 중인 캐릭터는 서버에서 수면 근처 목표 높이까지 Z 속도를 보정해 물에 둥둥 뜨는 느낌을 주며, 물속 상태가 복제된 동안 점프 입력은 무시된다.
- 캐릭터별 누적 상태는 `SubmersionStates`에 저장하며, 유효 침수 progress와 이탈 시간만 관리한다.
- Damage 호출은 `UGameplayStatics::ApplyDamage()`에서 기존 `ASnowRumbleCharacter::TakeDamage()`와 `USnowRumbleHealthComponent::ApplyDamage()`로 이어진다. 별도의 HP 시스템, J 전용 HealthComponent, 별도 HP replication은 만들지 않았다.

## Gameplay 흐름

1. GameState의 라운드 경과 시간이 갱신된다.
2. `ASnowIslandWaterPressureActor`가 시간 구간에 맞는 Water World Z를 계산하고 기존 Water Actor에 적용한다.
3. 서버 Damage Timer가 Character Capsule 하단 기준으로 침수 여부를 확인한다.
4. 침수 중인 Character는 유효 침수 시간이 누적된다. 짧은 이탈은 grace 동안 누적 상태를 유지하고, grace 이상 이탈하면 progress를 초기화한다.
5. 유효 침수 시간이 1초에 도달하면 기존 Damage 경로로 8 Damage를 적용한다.
6. Frozen/Dead Character와 더 이상 유효하지 않은 Character는 Water Damage 대상에서 제외하고 누적 상태를 정리한다.

## 주요 설정값

| 항목 | 기본값 | 의미 |
| --- | --- | --- |
| `InitialWaterZ` | Runtime 산정 | 기본적으로 `ControlledWaterActor`의 현재 World Z를 사용한다. |
| `ManualInitialWaterZ` | `0.0` | 자동 시작 수위를 쓰지 않을 때 사용할 수동 시작 수위. |
| `StableEndSeconds` | `240.0` | 안정 구간 종료. 이 전에는 수위 변화와 Water Damage를 적용하지 않는다. |
| `OuterFloodEndSeconds` | `300.0` | 외곽 침수 목표 수위(`OuterFloodWaterZ`)에 도달하는 시간. |
| `CentralFloodEndSeconds` | `360.0` | 중앙 침수 목표 수위(`CentralFloodWaterZ`)에 도달하는 시간. |
| `OvertimeRiseSpeed` | `0.0` | 360초 이후 초당 추가 상승량. 실제 연장전 압박은 Editor에서 튜닝한다. |
| `SubmersionSampleOffsetZ` | `0.0` | Capsule 하단 침수 판정 위치를 보정하는 값. |
| `RequiredSubmersionDepth` | `0.0` | Damage 판정에 필요한 최소 침수 깊이. |
| `DamageCheckIntervalSeconds` | `0.1` | 서버가 Character별 침수 상태를 확인하는 주기. |
| `DamageApplyIntervalSeconds` | `1.0` | Damage 1회 적용에 필요한 유효 침수 시간. |
| `DamagePerTick` | `8.0` | 유효 침수 1회마다 적용하는 Damage 값. |
| `ExitGraceSeconds` | `0.5` | 물 밖으로 나간 뒤 침수 progress를 유지하는 grace 시간. |
| `bApplyWaterBuoyancy` | `true` | 침수 중인 캐릭터에게 서버 부력 보정을 적용할지 여부. |
| `BuoyancyTargetSubmersionDepth` | `45.0` | Capsule 하단이 수면 아래 유지될 목표 깊이. |
| `BuoyancyCorrectionSpeed` | `3.5` | 목표 높이까지 올라가는 보정 강도. |
| `BuoyancyMinimumUpwardVelocity` | `80.0` | 부력 적용 시 최소 상승 속도. |
| `BuoyancyMaximumUpwardVelocity` | `360.0` | 부력 적용 시 최대 상승 속도. |
| `bApplyWaterBounce` | `true` | 기본 부력 위에 통통 튀는 상승 펄스를 추가할지 여부. |
| `WaterBounceFrequency` | `1.35` | 물속 통통 튐 반복 속도. |
| `WaterBounceUpwardVelocity` | `120.0` | 통통 튐으로 추가되는 최대 상승 속도. |

## 구현 결정 및 주의사항

- Snow Island Water timing의 240/300/360초 기준은 J-04 전용 환경 압박 시간표다.
- `USnowRumbleMatchSubsystem::GetMapShrinkIntervalSeconds()`의 게임 속도별 90/60/30초 값은 공용 map shrink interval이며, Snow Island Water timing과 다른 개념으로 취급한다.
- 이유를 확인하지 않은 상태에서 공용 `SnowRumbleMatchSubsystem_C`의 90/60/30초 값을 240/300/360초로 교체하지 않는다.
- 핵심 Gameplay logic은 Level Blueprint에 넣지 않고 `ASnowIslandWaterPressureActor` C++에 둔다.
- C++는 서버 판정, 물 상태 복제, Damage timing을 담당하고 Blueprint는 Material, Niagara/VFX, Sound, 시각 경고와 디자인 프리셋을 다듬는 용도로 사용한다.
- `RequestHazardDamage(AActor* Target, float DamageAmount)`는 C-07 이후 공용 Damage 계약으로 바꿀 수 있는 경계 함수로 유지한다.

## 검증 메모

- 현재 코드 기준으로 물 단계/수위 복제, 시간 기반 Lerp, 서버 침수 판정, 캐릭터별 침수 누적, Frozen/Dead skip, 기존 Damage/HealthComponent 경로 재사용을 확인했다.
- `bLogWaterDamageDebug`를 켜면 `[WaterDamage]` 로그로 Character, 침수 여부, 침수 progress, 이탈 시간, Water/Sample Z, Frozen/Dead 여부, 적용 Damage, Damage 전후 HP를 확인할 수 있다.
- [확인 필요] 실제 `L_SnowIsland_J.umap`에서 `ControlledWaterActor`, `OuterFloodWaterZ`, `CentralFloodWaterZ`, Water Component Mobility가 올바르게 설정되었는지 Editor에서 확인해야 한다.
- [확인 필요] Listen Server / Client에서 물 상승과 Damage가 실제로 관찰되었다는 로그 파일은 현재 저장소와 `Saved` 검색에서 확인하지 못했다. 결과 확인 체크리스트는 아직 미완료로 유지한다.
- [확인 필요] `ASnowIslandWaterPressureActor`는 자체 RootComponent를 생성하지 않는다. RootComponent 또는 network relevancy 관련 경고가 있었는지는 현재 코드와 Git 기록만으로 해결 완료를 확정할 수 없다.

## 수동 작업

1. C++ 빌드 성공 후 Unreal Editor에서 `Content/Maps/L_SnowIsland_J.umap`을 연다.
2. 실제 Runtime Water Actor인 `Water`의 World Z를 확인한다.
3. `ASnowIslandWaterPressureActor`를 맵에 직접 배치한다.
4. `ControlledWaterActor`에 실제 `Water` 하나를 직접 지정한다.
5. `Water2`의 World Z를 참고해 `OuterFloodWaterZ`를 입력한다.
6. `Water3`의 World Z를 참고해 `CentralFloodWaterZ`를 입력한다.
7. 첫 기능 검증에서는 `StableEndSeconds`/`OuterFloodEndSeconds`/`CentralFloodEndSeconds`를 5/10/15초처럼 줄여 테스트한다.
8. 물 상승과 침수 Damage 확인 후 시간값을 기본 240/300/360초로 되돌린다.
9. 침수 Sample Offset, Required Submersion Depth, Damage Check Interval, Damage Apply Interval, Exit Grace, Damage Per Tick을 확인한다. 기본값은 0.1초 체크, 1초 누적마다 8 Damage, Exit Grace 0.5초다.
10. 물속 움직임이 너무 튀면 `BuoyancyMaximumUpwardVelocity`, `BuoyancyCorrectionSpeed`, `WaterBounceUpwardVelocity`를 낮추고, 너무 밋밋하면 `WaterBounceFrequency`나 `WaterBounceUpwardVelocity`를 조금 올린다. 너무 가라앉으면 `BuoyancyTargetSubmersionDepth`를 낮추거나 `BuoyancyMinimumUpwardVelocity`를 올린다.
11. 필요할 때만 얇은 Blueprint Child를 만들어 Material, Niagara/VFX, Sound, 시각 경고와 디자인 프리셋을 연결한다.

## 완료 조건

### 에이전트 확인

- [x] `ASnowIslandWaterPressureActor` C++ 구현 완료
- [x] 기존 Water Actor/Component 재사용 구조 확인
- [x] 물 이동 Tick과 서버 Damage Timer 분리 완료
- [x] 0~240 / 240~300 / 300~360 / 연장전 단계 계산 완료
- [x] 서버 침수 판정 완료
- [x] 1초마다 8 Damage 적용 완료
- [x] 침수 중 서버 부력 보정과 점프 차단 상태 복제 추가
- [x] 기존 Damage/HP 흐름 재사용 확인
- [x] HP 중복 변수와 별도 HP replication 없음
- [x] Level Blueprint 또는 Blueprint Event Graph에 핵심 Gameplay 중복 없음
- [x] 관련 C++ 빌드 통과
- [x] `Content/Game/BP_LobbyGameMode.uasset` 제외 유지

### 결과 확인

- [ ] 0~240초 구간에는 Water Damage가 들어가지 않는다.
- [ ] 240~300초 구간에는 기존 Water Actor가 외곽 침수 목표까지 천천히 상승한다.
- [ ] 300~360초 구간에는 기존 Water Actor가 중앙 축소 목표까지 천천히 상승한다.
- [ ] 360초 이후에는 연장전 속도로 추가 상승한다.
- [ ] 물에 잠기지 않은 플레이어는 Damage를 받지 않는다.
- [ ] 물에 잠긴 플레이어는 서버 기준 1초마다 8 Damage를 받는다.
- [ ] 물 상승 수위가 설치된 모닥불 위치에 닿으면 모닥불이 즉시 꺼지고 사라지지는 않는다.
- [ ] 물에 잠긴 플레이어는 수면 근처에서 약하게 떠오르고 점프 입력이 동작하지 않는다.
- [ ] 클라이언트 HUD의 HP는 기존 `CurrentHealth` 복제로 감소한다.
- [ ] HP가 0이 되면 기존 얼기/사망/라운드 종료 흐름이 동작한다.
- [ ] 호스트와 클라이언트에서 물 위치 표현과 Damage 결과가 어긋나지 않는다.

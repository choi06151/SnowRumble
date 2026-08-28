# Task C-27 - 눈길 RenderTarget 계약

## 설명

눈 덮인 맵에서 캐릭터가 밟은 위치를 런타임 RenderTarget 마스크로 누적하고, 지형 머티리얼이 그 마스크를 읽어 눈길처럼 보이게 하는 C++/Blueprint 계약을 제공한다.

## 상태 전이 기준

- 시작 가능: C-26 발걸음 표면 trace 계약이 존재하고, 새 맵 환경 표현 Actor를 C가 생성할 수 있음.
- 완료 가능: C++ Manager와 캐릭터 서버/멀티캐스트 stamp 경로가 구현되고, 사용자가 지형 머티리얼과 stamp 그리기 Blueprint를 연결해 호스트·클라이언트에서 같은 눈길을 확인함.

## 구현 항목

- [x] 맵 배치용 눈길 RenderTarget Manager Actor 제공
- [x] Manager가 RenderTarget 생성, 월드 좌표 to UV 변환, C++ stamp material 그리기와 Blueprint 확장 이벤트를 제공
- [x] 캐릭터 발걸음에서 서버 검증 후 모든 클라이언트에 눈길 stamp를 전달
- [x] 애니메이션 에셋에서 바로 붙일 수 있는 눈 밟힘 AnimNotify 제공
- [x] 지형 머티리얼과 stamp Blueprint 연결에 필요한 수동 작업과 결과 확인 절차 기록

## 작업 배정

- 담당자: 최재원(C)
- 기능 소유자: 최재원(C)
- 계약 소유자: 최재원(C)
- 자산 수정자: C++·문서 최재원(C), 지형 머티리얼·Stamp 머티리얼·맵 배치 사용자 또는 S/J
- 생성·변경 후보: `Tasks/C/C-27_snow_trail_render_target.md`, `Source/SnowRumble/Environment/SnowTrailRenderTargetManager_C.*`, `Source/SnowRumble/Player/AnimNotify_SnowFootstep_C.*`
- 변경 후보: `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`, `docs/ARCHITECTURE.md`, `docs/PENDING_ISSUES.md`
- 공유 확인 대상: 사용자 또는 S/J
- 병합 순서: C++ 계약 반영 후 맵별 Manager BP 배치와 지형 머티리얼 연결

## 공용 계약과 인계

- 제공받을 계약: C-26 `RequestSnowFootstepEffect`, `SnowSurface` 태그 표면 trace
- 제공할 계약: `ASnowTrailRenderTargetManager`, `SnowTrailStampMaterial`, `StampSnowTrailAtWorldLocation`, `OnSnowTrailStampRequested`, `OnDrawSnowTrailStamp`, `UAnimNotify_SnowFootstep`, 캐릭터 서버 검증/멀티캐스트 stamp 경로
- 인계 대상: 사용자 또는 S/J가 Manager BP, Stamp Material, 지형 머티리얼, 맵 배치를 완성

## 범위 밖

- 실제 Landscape/지형 머티리얼 자산 직접 수정
- 맵 `.umap` 배치 직접 수정
- 눈길 마스크를 이용한 이동속도, 은신, 추적 같은 게임 판정
- 대형 월드용 타일 RenderTarget 스트리밍

## 사전 전제

- 눈길을 만들 맵에는 `ASnowTrailRenderTargetManager` 기반 Blueprint가 1개 배치된다.
- 눈 표면 Actor에는 `SnowSurface` 태그가 붙는다.

## 결정 필요

- 없음

## 변경 기록

- 2026-08-14: 사용자가 Decal 대신 지형 머티리얼을 실시간으로 파내는 눈길을 요청해 C-27을 추가하고 진행중으로 전환함.
- 2026-08-14: `ASnowTrailRenderTargetManager`를 누적 stamp 목록 기반으로 구현함. `OnSnowTrailStampRequested`는 stamp 수신/머티리얼 파라미터 갱신용, `OnDrawSnowTrailStamp`는 RenderTarget 갱신 중 Canvas에 stamp를 다시 그리는 용도로 인계함.
- 2026-08-14: `SnowRumbleEditor Win64 Development` 빌드는 UHT와 C++ 컴파일, `.lib` 생성까지 통과했으나 실행 중인 Unreal Editor가 `UnrealEditor-SnowRumble.dll`을 사용 중이라 최종 링크는 `LNK1104`로 보류됨.
- 2026-08-14: 사용자가 stamp 그리기를 Blueprint 대신 C++로 처리할 수 있는지 확인해, Manager가 `SnowTrailStampMaterial`을 Canvas에 직접 그리는 기본 경로를 추가함. `OnDrawSnowTrailStamp`는 추가 보정·연출용 선택 이벤트로 유지함.
- 2026-08-14: C++ Canvas stamp 경로 추가 후 `SnowRumbleEditor Win64 Development` 빌드는 UHT와 C++ 컴파일, `.lib` 생성까지 통과했으나 실행 중인 Unreal Editor DLL 잠금으로 최종 링크는 `LNK1104`로 보류됨.
- 2026-08-14: 애니메이션 에셋에서 직접 사용할 `UAnimNotify_SnowFootstep`을 추가함. Notify의 `FootSocketName`을 왼발·오른발 socket으로 지정하면 캐릭터의 `RequestSnowFootstepEffect`를 호출한다.
- 2026-08-14: `UAnimNotify_SnowFootstep` 추가 후 `SnowRumbleEditor Win64 Development` 빌드가 최종 링크까지 성공함.
- 2026-08-14: 사전 지정 `SnowTrailRenderTarget` 사용 시 Canvas 업데이트 콜백이 연결되지 않아 stamp가 그려지지 않던 경로를 수정함. 자동 생성 RT와 사전 지정 RT 모두 `OnCanvasRenderTargetUpdate`를 연결한다. UHT와 C++ 컴파일, `.lib` 생성은 통과했으나 실행 중인 Unreal Editor DLL 잠금으로 최종 링크는 `LNK1104`로 보류됨.
- 2026-08-14: 사전 지정 `SnowTrailRenderTarget` 사용 시 BeginPlay에서 stamp 목록과 RT 픽셀을 초기화하고 `OnSnowTrailRenderTargetReady`로 지형 머티리얼 파라미터 재연결 시점을 제공함. `SnowRumbleEditor Win64 Development` 빌드가 최종 링크까지 성공함.
- 2026-08-14: 발걸음 stamp가 점처럼 끊겨 보여 같은 캐릭터의 이전 stamp와 현재 stamp 사이를 보간해 중간 stamp를 추가하는 옵션을 Manager에 추가함. `bInterpolateTrailBetweenStamps`, `InterpolatedStampSpacingWorld`, `MaxInterpolatedStampDistanceWorld`로 조정한다. C++ 컴파일과 `.lib` 생성은 통과했으나 실행 중인 Unreal Editor DLL 잠금으로 최종 링크는 `LNK1104`로 보류됨.
- 2026-08-14: AnimNotify가 눈 표면을 확인한 뒤 캐릭터가 눈 표면 위를 이동하는 동안 거리 기준으로 연속 stamp를 요청하는 경로를 추가함. `bEnableDistanceBasedSnowTrailStamps`, `SnowTrailDistanceStampInterval`, `SnowTrailDistanceStampMinimumSpeed`로 조정한다. 이번 변경 대상 파일의 `git diff --check`는 통과했고, UHT와 C++ 컴파일, `.lib` 생성은 통과했으나 실행 중인 Unreal Editor DLL 잠금으로 최종 링크는 `LNK1104`로 보류됨.
- 2026-08-19: `L_snowisland`처럼 Lobby와 다른 Landscape UV 배치에서 RT 마스크가 다른 위치에 보이는 문제를 보정하기 위해 Manager에 맵별 `SnowTrailUVScale`, `SnowTrailUVOffset`, `bSnowTrailFlipU`, `bSnowTrailFlipV` 조정값을 추가함. Manager는 머티리얼 파라미터 `SnowTrailUVScale`/`TrailUVScale`, `SnowTrailUVOffset`/`TrailUVOffset`, `SnowTrailFlipU`/`TrailFlipU`, `SnowTrailFlipV`/`TrailFlipV`를 자동 적용한다.
- 2026-08-19: `SnowTrailMaterialActors`, `SnowTrailMaterialComponents`, 또는 `SnowSurface` 태그 자동 적용 대상의 Bounds에서 `TrailWorldCenter`와 `TrailWorldSize`를 계산하는 `bAutoFitTrailWorldAreaFromMaterialBounds` 옵션을 추가함. `TrailWorldBoundsPadding`으로 가장자리 여유를 조정하고, 유효 Bounds가 없으면 기존 수동 값을 유지한다.

## 수동 작업

- `BP_SnowTrailRenderTargetManager_C`를 만들고 부모를 `ASnowTrailRenderTargetManager`로 설정한다.
- 눈길을 만들 맵에 Manager BP를 1개 배치하고 `TrailWorldCenter`, `TrailWorldSize`, `RenderTargetSize`를 맵 크기에 맞춘다.
- `L_snowisland`처럼 Landscape 크기와 위치를 수동으로 맞추기 어려운 맵은 Manager BP에서 `bAutoFitTrailWorldAreaFromMaterialBounds`를 켜고, Landscape Actor를 `SnowTrailMaterialActors`에 넣거나 Landscape 컴포넌트를 `SnowTrailMaterialComponents`에 넣는다. 가장자리가 잘리면 `TrailWorldBoundsPadding`을 늘린다.
- 걷기/달리기 애니메이션 에셋의 왼발·오른발이 바닥에 닿는 프레임에 `Snow Rumble Snow Footstep` AnimNotify를 추가하고, `FootSocketName`을 실제 Skeleton socket 이름으로 지정한다.
- 캐릭터 BP에서 `bEnableDistanceBasedSnowTrailStamps`를 켜고, `SnowTrailDistanceStampInterval`은 35~45cm, `SnowTrailDistanceStampMinimumSpeed`는 20cm/s 전후로 시작한다.
- Manager BP의 `OnSnowTrailStampRequested`에서 눈 지형 머티리얼 인스턴스에 `TargetRenderTarget`, `TrailWorldCenter`, `TrailWorldSize`를 전달하거나 필요한 파라미터 갱신을 수행한다.
- 사전 생성 RT를 사용할 때 이전 플레이 흔적을 지우려면 Manager BP의 `bClearRenderTargetOnBeginPlay`를 켜둔다.
- 지형 머티리얼 파라미터 연결이 BeginPlay 이후 다시 필요하면 Manager BP의 `OnSnowTrailRenderTargetReady`에서 `TargetRenderTarget`, `WorldCenter`, `WorldSize`를 동적 머티리얼 인스턴스에 설정한다.
- Manager BP의 `SnowTrailStampMaterial`에 발자국/눈길 stamp material을 지정한다. `bDrawStampsInCpp`가 켜져 있으면 C++가 `StampData.TrailUV`와 `StampData.RadiusPixels` 기준으로 Canvas에 직접 그린다.
- `OnDrawSnowTrailStamp`는 기본 C++ stamp 위에 추가 보정이나 별도 레이어가 필요할 때만 구현한다. Manager가 기존 stamp 목록을 보관하고 RenderTarget 갱신 때 전부 다시 그리므로 Blueprint는 누적 목록을 따로 저장하지 않는다.
- 눈 지형 머티리얼에 Manager의 `SnowTrailRenderTarget`을 Texture Parameter로 전달하고, `AbsoluteWorldPosition.xy`를 Manager의 `TrailWorldCenter`/`TrailWorldSize`와 같은 방식으로 UV 변환해 마스크를 샘플링한다.
- 맵별 Landscape UV가 Lobby와 다르면 기본 UV 계산 뒤 `SnowTrailUVScale`, `SnowTrailUVOffset`, `SnowTrailFlipU`, `SnowTrailFlipV`를 적용한다. Lobby Manager는 기본값 `Scale=(1,1)`, `Offset=(0,0)`, Flip 꺼짐으로 두고, `L_snowisland` Manager BP에서만 보정값을 조정한다.
- 마스크 값이 높은 곳은 BaseColor를 살짝 어둡게, Normal/Roughness/Height 표현을 눌린 눈처럼 조정한다.

## 완료 조건

### 에이전트 확인

- [x] 관련 코드·Blueprint·자산 변경 완료
- [x] 로컬 정적 점검 또는 프로젝트 구조 기준 확인 완료
- [x] 역할·소유권·담당자 이니셜 규칙 위반 없음
- [x] 공용 계약과 캡슐화 규칙 위반 없음
- [x] 현재 Task 문서가 실제 구현 기준으로 갱신됨

### 결과 확인

- [ ] 눈 표면을 밟으면 Manager RenderTarget의 해당 UV에 마스크가 누적됨
- [ ] 눈 지형 머티리얼이 마스크를 읽어 밟은 길이 눈길처럼 보임
- [ ] Listen Server 호스트와 클라이언트가 같은 위치의 눈길 stamp를 받음
- [ ] `SnowSurface` 태그가 없는 바닥에서는 눈길 stamp가 생기지 않음

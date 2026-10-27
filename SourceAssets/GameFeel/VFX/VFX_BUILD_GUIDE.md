# 게임필 VFX 제작 가이드

이 폴더의 텍스처로 게임필 코드에 연결할 Niagara 4종을 만드는 순서다.
모든 수치는 시작값이니, 실제 화면을 보고 조정한다.

## 0. 텍스처 임포트

`Content/VFX/Textures/GameFeel` 폴더를 만들고 아래 PNG를 드래그해 임포트한다.

| 파일 | 용도 | 아틀라스 |
|---|---|---|
| `T_FX_SnowPuff` | 눈가루 연기 | 1×1 |
| `T_FX_SnowChunks_2x2` | 튀는 눈덩이 조각 | 2×2 |
| `T_FX_IceShards_2x2` | 얼음 파편 | 2×2 |
| `T_FX_Ring` | 퍼지는 충격파 링 | 1×1 |
| `T_FX_Sparkle` | 반짝임 | 1×1 |

모든 텍스처의 설정은 그대로 두고(Default, sRGB 켜짐), **Texture Group만 `Effects`**로 바꾼다.

## 1. 공용 머티리얼 `M_FX_Sprite` (한 번만)

`Content/VFX/Materials/M_FX_Sprite` 생성.

- Details: **Blend Mode = Translucent**, **Shading Model = Unlit**, **Used with Niagara Sprites 체크**
- 노드
  1. `TextureObjectParameter` 이름 `SpriteTexture` (기본값 T_FX_SnowPuff)
  2. `Particle SubUV` 노드 → Tex 입력에 1번 연결
  3. `Particle Color` 노드
  4. Emissive Color = `SubUV.RGB × ParticleColor.RGB`
  5. Opacity = `SubUV.A × ParticleColor.A × DepthFade`
     - `DepthFade` 노드: Fade Distance 상수 **30** (바닥에 박힐 때 딱 잘리는 선을 없앤다)

머티리얼 인스턴스 5개 생성: `MI_FX_SnowPuff`, `MI_FX_SnowChunks`, `MI_FX_IceShards`, `MI_FX_Ring`, `MI_FX_Sparkle` — 각각 `SpriteTexture`에 같은 이름 텍스처를 넣는다.

## 2. Niagara 공통 규칙

- 새 시스템은 **Empty** 템플릿에서 시작하고, 이미터도 **Empty**로 추가한다.
- 모든 이미터 **Emitter Properties → Local Space 체크**.
  코드가 넘기는 크기 배율(`...EffectScale`)과 충돌 방향 회전은 Local Space일 때만 적용된다.
- **Emitter State**: Life Cycle Mode = Self, Loop Behavior = **Once**.
- 터짐은 전부 **Spawn Burst Instantaneous**로 한 번에 생성한다.
- 2×2 아틀라스를 쓰는 이미터: Sprite Renderer → **Sub Image Size = (2,2)**, 그리고 Particle Spawn에 **SubUV Animation** 모듈 추가 → Animation Mode = **Random**, Random Start Frame 체크.
- 터짐 이펙트는 코드가 **충돌면 방향을 X축**으로 넘긴다. 그래서 위로 튀어야 할 방향은 로컬 **+X**다.

## 3. `NS_SnowImpact_Small` — 작은 눈덩이 터짐

연결: 눈덩이 BP → `SmallImpactNiagaraEffect`

| 이미터 | 설정 |
|---|---|
| **Chunks** (MI_FX_SnowChunks, 2×2) | Burst 10~14 · Lifetime 0.35~0.6 · Sprite Size 6~14 · **Add Velocity in Cone**: Cone Axis (1,0,0), Angle 70, Speed 250~450 · Gravity Force (0,0,-980) · Drag 2 · Scale Sprite Size 커브: 1 → 0.6 · Sprite Rotation Rate -360~360 |
| **Puff** (MI_FX_SnowPuff) | Burst 3 · Lifetime 0.4~0.6 · Sprite Size 25~40 · Cone Axis (1,0,0) Angle 60 Speed 60~120 · Drag 4 · Scale Sprite Size 커브 0.6 → 1.6 · Scale Color 알파 0.7 → 0 |

## 4. `NS_SnowImpact_Large` — 큰 눈덩이 폭발

연결: 눈덩이 BP → `LargeImpactNiagaraEffect` (코드 기본 크기 배율 2)

`NS_SnowImpact_Small`을 복제한 뒤:

| 이미터 | 변경 |
|---|---|
| **Chunks** | Burst 28~36 · Size 10~24 · Speed 400~750 · Angle 85 · Lifetime 0.5~0.9 |
| **Puff** | Burst 8 · Size 50~90 · Speed 120~260 · Angle 90 · Lifetime 0.6~1.0 |
| **Ring** 추가 (MI_FX_Ring) | Burst 1 · Lifetime 0.45 · Sprite Renderer → **Facing Mode = Custom Facing Vector**, Custom Facing Vector (1,0,0) (충돌면과 나란히 퍼짐) · Sprite Size 30 · Scale Sprite Size 커브 1 → 14 (Ease Out) · Scale Color 알파 0.9 → 0 |

## 5. `NS_GrabImpact` — 잡는 순간 손맛

연결: 캐릭터 BP의 `PlayerGrabComponent` → `GrabImpactEffect` (눈덩이 잡기 전용 `SnowballGrabImpactEffect`는 비워 두면 이걸 같이 쓴다)

잡기 지점은 손 위치라 작고 짧아야 한다. 회전은 넘기지 않으므로 방향은 월드 기준.

| 이미터 | 설정 |
|---|---|
| **Puff** (MI_FX_SnowPuff) | Burst 2 · Lifetime 0.25 · Size 12~18 · Shape Location Sphere 반경 4 · Scale Sprite Size 커브 0.6 → 1.4 · Scale Color 알파 0.6 → 0 |
| **Chunks** (MI_FX_SnowChunks, 2×2) | Burst 5 · Lifetime 0.25~0.4 · Size 3~6 · **Add Velocity from Point** Speed 120~220 · Gravity (0,0,-600) |
| **Sparkle** (MI_FX_Sparkle) | Burst 1 · Lifetime 0.15 · Size 18 · Scale Sprite Size 커브 1.4 → 0 |

## 6. `NS_FreezeBurst` — 얼어붙는 순간

연결: 캐릭터 BP → `FreezeBurstEffect` (짧은 멈춤이 끝나는 순간 캐릭터 위치에서 재생)

캐릭터 위치는 캡슐 중심이므로, 모든 이미터의 **Shape Location Offset을 Z -40 정도**로 내려 발 쪽에서 퍼지게 한다.

| 이미터 | 설정 |
|---|---|
| **Shards** (MI_FX_IceShards, 2×2) | Burst 10 · Lifetime 0.5~0.8 · Size 18~34 · Shape Location: **Ring/Disc** 반경 30 · **Add Velocity from Point** Speed 180~320 + Add Velocity (0,0,150~300) · Gravity (0,0,-700) · Sprite Renderer **Alignment = Velocity Aligned** (파편이 날아가는 방향을 향함) · Color (0.75, 0.92, 1.0) |
| **Ring** (MI_FX_Ring) | Burst 1 · Lifetime 0.4 · Facing = Custom Facing Vector (0,0,1) · Sprite Size 40 · Scale Sprite Size 커브 1 → 6.5 (Ease Out) · Color (0.55, 0.85, 1.0), Scale Color 알파 0.8 → 0 |
| **Sparkles** (MI_FX_Sparkle) | Burst 10 · Lifetime 0.3~0.6 · Size 6~14 · Shape Location Sphere 반경 60 · Scale Sprite Size 커브 1 → 0 · Color (0.7, 0.95, 1.0) |
| **Mist** (MI_FX_SnowPuff) | Burst 4 · Lifetime 0.6~0.9 · Size 50~80 · Velocity Sphere Speed 40 · Scale Sprite Size 커브 0.7 → 1.5 · Color (0.85, 0.95, 1.0), Scale Color 알파 0.5 → 0 |

## 7. 확인 순서

1. 각 Niagara를 레벨에 끌어다 놓고 **재생 버튼으로 한 번씩** 모양을 본다(Loop Once라 끝나면 사라진다).
2. 해당 BP 슬롯에 연결한다.
3. PIE에서 실제 상황으로 확인한다.
   - 작은/큰 눈덩이를 바닥·벽·캐릭터에 던진다.
   - 눈덩이와 벽, 상대를 잡아본다.
   - 상대를 얼린다.
4. 크기가 안 맞으면 Niagara를 열지 말고 먼저 BP의 `...EffectScale` 값으로 맞춘다.

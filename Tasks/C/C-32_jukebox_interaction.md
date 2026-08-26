# C-32 주크박스 상호작용

## 목표

Static Mesh와 Box Collision을 가진 주크박스에 기존 E 상호작용을 연결한다. 상호작용 시 Blueprint에 지정한 Sound를 주크박스 위치에서 재생하고, Sound가 끝날 때까지 범위 안의 플레이어를 반복 점프시킨다.

## 범위

- `AJukeboxActor` C++ 부모 클래스 추가
- 기존 outline 및 상호작용 안내에 `E - 노래틀기` 후보 추가
- 서버 검증, Sound Multicast, Sound duration 기반 점프 타이머
- 재생 중 상호작용 후보 숨김, 종료 후 자동 복구
- 안내 위치는 액터 전체 Bounds가 아닌 RootComponent 위치를 기준으로 계산
- 인스턴스에서 Spotlight 배열을 지정하고 재생 중 랜덤 Spotlight를 순환
- Sound 후보 배열 중 하나를 랜덤 재생
- Spotlight 전환마다 색상 후보 중 하나 또는 랜덤 색상 적용

## 수동 작업

- 주크박스 Blueprint를 `AJukeboxActor` 기반으로 생성한다.
- `JukeboxMeshComponent`에 Static Mesh를 지정한다.
- `JumpBoxComponent`의 Box Extent를 설정한다.
- `JukeboxSounds` 배열에 재생할 Sound 후보를 지정한다.
- 필요 시 `JukeboxSoundAttenuation`을 지정한다.
- `JukeboxSpotlights` 배열에 맵에 배치한 `SpotLight` 액터를 지정한다.
- 필요하면 `SpotlightColors` 배열에 색상 후보를 지정한다. 비워두면 랜덤 색상을 사용한다.
- 필요 시 `SpotlightChangeInterval`로 랜덤 전환 속도를 조정한다.
- 맵에 배치한 뒤 호스트와 클라이언트에서 E 안내, 사운드, 반복 점프, outline 복구를 확인한다.

## 결과 확인

- 호스트가 E를 누르면 모든 클라이언트에서 주크박스 위치의 Sound가 들린다.
- 클라이언트가 E를 눌러도 서버에서 한 번만 재생된다.
- Box Collision 안의 호스트·클라이언트 캐릭터가 Sound 종료까지 반복 점프한다.
- 재생 중에는 outline과 `E - 노래틀기` 안내가 사라진다.
- Sound 종료 후 점프가 멈추고 outline과 안내가 다시 나타난다.

## 상태

진행중

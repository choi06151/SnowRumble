# C-32 주크박스 상호작용

## 목표

Static Mesh와 Box Collision을 가진 주크박스에 기존 E 상호작용을 연결한다. 상호작용 시 Blueprint에 지정한 Sound를 주크박스 위치에서 재생하고, Sound가 끝날 때까지 참여 중인 범위 안의 플레이어를 반복 점프시킨다. 재생 중에는 E로 참여 여부를 토글한다.

## 범위

- `AJukeboxActor` C++ 부모 클래스 추가
- 재생 전 주크박스 outline 및 `E - 노래틀기` 안내 표시, 재생 후 outline 없이 참여 토글 문구 표시
- 서버 검증, Sound Multicast, Sound duration 기반 점프 타이머
- 재생 중 기본 참여 상태, 캐릭터별 참여 해제 목록 복제 및 종료 후 초기화
- 참여 중에는 `E - 참여 안하기`, 해제 중에는 `E - 참여하기` 표시
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
- 맵에 배치한 뒤 호스트와 클라이언트에서 E 안내, 사운드, 반복 점프, 참여 토글을 확인한다.

## 결과 확인

- 호스트가 E를 누르면 모든 클라이언트에서 주크박스 위치의 Sound가 들린다.
- 클라이언트가 E를 눌러도 서버에서 한 번만 재생된다.
- Box Collision 안의 참여 중 호스트·클라이언트 캐릭터가 Sound 종료까지 반복 점프한다.
- 재생 전 범위 안에서 주크박스 outline과 `E - 노래틀기` 안내가 표시된다.
- 재생 중 범위에 들어온 캐릭터는 outline 없이 `E - 참여 안하기` 안내가 표시되고 점프한다.
- 참여 중 E를 누르면 `E - 참여하기`로 바뀌고 해당 캐릭터의 점프가 멈춘다.
- 해제 중 E를 다시 누르면 `E - 참여 안하기`로 바뀌고 다음 점프부터 다시 참여한다.
- Sound 종료 후 점프가 멈추고 다음 재생 시 참여 상태가 초기화된다.

## 상태

진행중

## 변경 기록

- 2026-08-27: 재생 중 캐릭터별 참여 토글을 추가했다. 기본 참여 상태에서는 점프하고 `E - 참여 안하기`를 표시하며, 해제 상태에서는 점프하지 않고 `E - 참여하기`를 표시한다. 주크박스 outline은 제거했다.

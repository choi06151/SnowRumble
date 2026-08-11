# Task C-11 - 커스터마이징 데이터 계약

## 설명

드로잉과 얼굴 표정 결과를 저장하고 메인 로비와 경기 캐릭터에 동일하게 적용할 데이터 계약을 제공한다.

## 상태 전이 기준
- 시작 가능: C-02 완료, S-01의 모델·표현 요구 인계와 드로잉 방식 결정
- 완료 가능: 저장·불러오기·멀티플레이 표현 계약과 S-01·S-02 인계 확인

## 현재 하위 범위

- 메인메뉴에서 커스터마이징 레벨로 이동하는 첫 진입 경로를 제공한다.
- 커스터마이징 레벨 전용 GameMode, PlayerController, UI 부모를 제공한다.
- 커스터마이징 레벨에 들어오면 UI를 항상 표시하고 마우스 커서와 클릭 입력을 유지한다.
- 레벨에 배치한 태그 기반 카메라를 ViewTarget으로 삼아 캐릭터를 바라보는 구조를 제공한다.
- 커스터마이징 UI는 WidgetSwitcher로 메인, 시점변경, 색칠하기 화면을 전환한다.
- 시점변경 화면은 카메라를 돌리지 않고 프리뷰 캐릭터를 좌/우 버튼 press 동안 계속 회전한다.
- 커스터마이징 레벨의 프리뷰 캐릭터는 커마 방 전용 애니메이션 에셋을 지정하거나 현재 애니메이션을 정지 상태로 둘 수 있다.
- 첫 외형 데이터는 몸 색상 `BodyColor`로 제한해 로컬 저장, 서버 복제, 캐릭터 머티리얼 적용 경로를 제공한다.
- 메쉬 직접 드로잉 1차 범위는 커스터마이징 레벨 프리뷰에서 검정 브러쉬로 그리고, Stroke 단위 이전과 전체 초기화를 지원한다.
- 드로잉, 얼굴 표정, 장비 외형 분리는 후속 범위에서 결정 후 구현한다.

## 구현 항목
- [x] 메인메뉴 `CustomizationButton`으로 커스터마이징 레벨 이동을 요청한다.
- [x] 메인메뉴 PlayerController BP가 이동할 커스터마이징 레벨 URL을 설정할 수 있게 한다.
- [x] 커스터마이징 레벨 전용 GameMode와 PlayerController를 제공한다.
- [x] 커스터마이징 레벨 진입 시 WBP를 자동 생성하고 마우스 UI 입력을 유지한다.
- [x] 레벨 카메라 태그 기준으로 커스터마이징 카메라 ViewTarget을 적용한다.
- [x] 커스터마이징 WBP의 WidgetSwitcher 기반 화면 전환 계약을 제공한다.
- [x] 좌/우 버튼을 누르고 있는 동안 프리뷰 캐릭터를 회전한다.
- [x] 커스터마이징 레벨 프리뷰 캐릭터의 오버헤드 이름표 위젯을 숨긴다.
- [x] 커스터마이징 레벨 프리뷰 캐릭터 전용 애니메이션 에셋과 정지 설정을 제공한다.
- [x] 로컬 플레이어의 몸 색상 선택 결과를 저장·불러온다.
- [x] 방 참가 뒤 몸 색상 데이터를 서버가 검증해 다른 참가자에게 전달한다.
- [x] 캐릭터 Mesh 머티리얼 파라미터에 복제된 몸 색상을 적용하는 공개 지점을 제공한다.
- [x] 커스터마이징 레벨에서 캐릭터 Mesh UV에 검정 브러쉬 선을 그리는 1차 프리뷰를 제공한다.
- [x] 드로잉 Stroke 단위 이전과 전체 초기화를 제공한다.
- [x] 페인트 trace 마우스 좌표에 UI DPI 스케일을 반영해 커서보다 왼쪽에 그려지는 현상을 보정한다.
- [x] 페인트 trace X/Y 보정값을 PlayerController BP에서 픽셀 단위로 지정할 수 있게 한다.
- [x] 로컬 플레이어의 드로잉 결과를 저장·복제한다.
- [x] 페인트 화면에서 `BackButton`과 `Ctrl+Z`가 마지막 완료 stroke를 하나씩 누적 undo하게 한다.
- [x] 색칠하기 화면이 아닐 때 좌클릭으로 페인트 stroke가 생성되지 않게 한다.
- [ ] 로컬 플레이어의 얼굴 표정 선택 결과를 저장·불러온다.
- [ ] 로비 외형과 경기 장비 외형 데이터를 분리한다.
- [ ] S가 제작한 드로잉·얼굴 표정 자산에 결과를 연결할 공개 지점을 제공한다.

## 작업 배정
- 담당자·기능·계약 소유자: 최재원(C)
- 자산 수정자: C++·문서 최재원(C), 메인메뉴 WBP 버튼과 커스터마이징 레벨 자산은 사용자/S
- 생성 파일: `Source/SnowRumble/Game/SnowRumbleCustomizationGameMode_C.*`, `Source/SnowRumble/UI/CustomizationPlayerController_C.*`, `Source/SnowRumble/UI/CustomizationWidget_C.*`, `Source/SnowRumble/Player/SnowRumbleCustomizationData_C.h`, `Source/SnowRumble/Player/SnowRumbleCustomizationSubsystem_C.*`
- 변경 파일: `Source/SnowRumble/UI/MainMenuWidget.*`, `Source/SnowRumble/UI/MainMenuPlayerController.*`, `Source/SnowRumble/Game/SnowRumblePlayerState.*`, `Source/SnowRumble/Player/SnowRumbleCharacter.*`, `Source/SnowRumble/UI/LobbyWidget.*`, `Tasks/C/C-11_customization_contract.md`, `Tasks/C/PLAN_C.md`, `docs/PLANS.md`
- 공유 확인 대상: S-01, S-02, S-08
- 병합 순서: C-02와 S-01 원본 준비 후

## 공용 계약과 인계
- 제공받을 계약: S-01 드로잉 가능 모델·표현 요구
- 제공할 계약:
  - `UMainMenuWidget::CustomizationButton`: 메인메뉴에서 커스터마이징 레벨 이동을 요청하는 선택 버튼
  - `AMainMenuPlayerController::CustomizationLevelUrl`: 이동할 커스터마이징 맵 URL
  - `AMainMenuPlayerController::TravelToCustomizationLevel()`: 로컬 메인메뉴에서 커스터마이징 레벨로 이동하는 함수
  - `ASnowRumbleCustomizationGameMode`: 커스터마이징 레벨 전용 GameMode
  - `ACustomizationPlayerController`: 커스터마이징 레벨 전용 PlayerController
  - `ACustomizationPlayerController::CustomizationWidgetClass`: 자동 생성할 커스터마이징 WBP 클래스
  - `ACustomizationPlayerController::CustomizationCameraTag`: ViewTarget으로 삼을 레벨 카메라 태그. 기본값은 `CustomizationCamera`
  - `ACustomizationPlayerController::MainMenuTravelUrl`: 커스터마이징 화면에서 돌아갈 메인메뉴 URL
  - `ACustomizationPlayerController::PreviewAnimationAsset`: 커스터마이징 방에서 프리뷰 캐릭터에 적용할 단일 애니메이션 에셋
  - `ACustomizationPlayerController::bPausePreviewAnimation`: 커스터마이징 방에서 프리뷰 캐릭터 애니메이션을 정지 상태로 둘지 결정
  - `ACustomizationPlayerController::PreviewAnimationPositionSeconds`: 단일 애니메이션 에셋을 적용할 때 고정할 재생 위치
  - `UCustomizationWidget`: 커스터마이징 WBP 부모
  - `UCustomizationWidget::CustomizationContentSwitcher`: 0 메인, 1 시점변경, 2 색칠하기 화면을 담는 WidgetSwitcher
  - `UCustomizationWidget::PaintModeButton`: 메인 화면에서 색칠하기 화면으로 이동
  - `UCustomizationWidget::RotateLeftButton`: 누르고 있는 동안 프리뷰 캐릭터를 왼쪽으로 회전
  - `UCustomizationWidget::RotateRightButton`: 누르고 있는 동안 프리뷰 캐릭터를 오른쪽으로 회전
  - `UCustomizationWidget::ReturnToLobbyButton`: 메인메뉴 URL로 복귀
  - `UCustomizationWidget::BackButton`: 하위 화면에서 메인 화면으로 복귀
  - `UCustomizationWidget::ApplyButton`: 현재 프리뷰 커스터마이징 데이터를 로컬 저장소에 저장하고 현재 하위 화면 적용 이벤트 호출
  - `UCustomizationWidget::ResetButton`: 프리뷰 커스터마이징 데이터를 기본값으로 되돌리고 현재 하위 화면 초기화 이벤트 호출
  - `UCustomizationWidget::SetPreviewBodyColor(FLinearColor)`: WBP 색상 버튼이나 컬러 피커가 프리뷰 몸 색을 변경할 때 호출
  - `UCustomizationWidget::GetPreviewCustomizationData()`: 현재 프리뷰 데이터를 반환
  - `FSnowRumbleCustomizationData`: 로비와 PvP 캐릭터에 공유할 커스터마이징 데이터. 현재 `BodyColor`만 포함
  - `FSnowRumbleCustomizationData::PaintStrokes`: 커스터마이징 드로잉 UV stroke 배열. 각 stroke는 맞은 SkeletalMeshComponent 이름을 함께 저장하며, 커스터마이징 방에서 그리는 즉시 로컬 저장소에 반영되고 로비 PlayerState를 통해 복제된다.
  - `FSnowRumbleCustomizationData::bFlipPaintUvY`: 커스터마이징 방에서 RenderTarget에 그릴 때 사용한 UV Y축 flip 기준. 로비/PvP 캐릭터가 같은 기준으로 stroke를 재생성한다.
  - `USnowRumbleCustomizationSubsystem`: 로컬 GameInstance에 커스터마이징 데이터를 저장·조회·초기화
  - `ASnowRumblePlayerState::RequestSetCustomizationData(...)`: 소유 클라이언트가 저장된 커마 데이터를 서버 PlayerState에 제출
  - `ASnowRumblePlayerState::GetCustomizationData()`: 복제된 커마 데이터 조회
  - `ASnowRumblePlayerState::OnCustomizationChanged`: 복제된 커마 데이터 변경 이벤트
  - `ASnowRumbleCharacter::ApplyCustomizationData(...)`: 캐릭터 Mesh 머티리얼에 커마 데이터를 즉시 적용
  - `ASnowRumbleCharacter::SetCustomizationPaintTexture(UTexture*)`: 드로잉 RenderTarget을 캐릭터 Mesh 머티리얼에 적용
  - `ASnowRumbleCharacter::CustomizationPaintRenderTargetSize`: 저장된 stroke를 로비/PvP 캐릭터에서 다시 그릴 RenderTarget 크기
  - `ASnowRumbleCharacter::CustomizationPaintStrokeThickness`: 저장된 stroke를 로비/PvP 캐릭터에서 다시 그릴 선 두께
  - `ASnowRumbleCharacter::CustomizationPaintBrushColor`: 저장된 stroke를 로비/PvP 캐릭터에서 다시 그릴 색상
  - `ASnowRumbleCharacter::bFlipCustomizationPaintUvY`: 저장된 stroke를 로비/PvP 캐릭터에서 다시 그릴 때 Y축을 뒤집을지 여부
  - `ASnowRumbleCharacter::CustomizationMaterialIndex`: 커마 색을 적용할 Mesh 머티리얼 슬롯
  - `ASnowRumbleCharacter::CustomizationBodyColorParameterName`: 몸 색 Vector Parameter 이름. 기본값은 `BodyColor`
  - `ASnowRumbleCharacter::CustomizationPaintTextureParameterName`: 드로잉 Texture Parameter 이름. 기본값은 `PaintTexture`
  - `ACustomizationPlayerController::UndoLastPaintStroke()`: 마지막 완료 Stroke 제거
  - `ACustomizationPlayerController::Ctrl+Z`: 페인트 화면에서 `UndoLastPaintStroke()`와 같은 누적 undo 단축키로 동작
  - `ACustomizationPlayerController::ResetPaintStrokes()`: 모든 Stroke 제거
  - `ACustomizationPlayerController::GetPaintRenderTarget()`: 현재 드로잉 RenderTarget 조회
  - `ACustomizationPlayerController::PaintCursorScreenOffset`: 페인트 trace 화면 좌표에 더하는 픽셀 단위 X/Y 보정값. X 양수는 오른쪽, Y 양수는 아래쪽으로 trace를 옮긴다.
  - `ACustomizationPlayerController::PreviewRotationSpeedDegrees`: 회전 버튼 press 중 프리뷰 캐릭터가 초당 회전하는 각도
- 인계 대상: S-01, S-02, S-08

## 범위 밖
- 드로잉 UI 레이아웃과 모델 제작

## 사전 전제
- C-02
- S-01 모델·표현 요구 인계

## 결정 필요
- 드로잉 입력 방식, 저장 형식·용량과 네트워크 공유 범위
- 얼굴 표정과 드로잉의 적용 우선순위

## 변경 기록

- 2026-08-11: 메인메뉴에서 커스터마이징 레벨로 이동하는 첫 진입 경로를 추가했다. WBP는 `CustomizationButton` 이름의 버튼을 배치하면 C++이 자동 바인딩하고, `AMainMenuPlayerController::CustomizationLevelUrl` 기본값 또는 BP 설정값으로 `ClientTravel()`을 수행한다.
- 2026-08-11: 커스터마이징 레벨 전용 `ASnowRumbleCustomizationGameMode`, `ACustomizationPlayerController`, `UCustomizationWidget` 부모를 추가했다. 레벨 진입 시 커스터마이징 WBP를 자동 표시하고, `CustomizationCamera` 태그가 붙은 카메라를 ViewTarget으로 설정하며, WidgetSwitcher로 메인/시점변경/색칠하기 화면을 전환한다.
- 2026-08-11: 몸 색상 커스터마이징 첫 세로 슬라이스를 추가했다. `FSnowRumbleCustomizationData`와 `USnowRumbleCustomizationSubsystem`이 로컬 저장을 맡고, 로비 입장 시 `ASnowRumblePlayerState`에 서버 제출·복제되며, `ASnowRumbleCharacter`가 `BodyColor` 머티리얼 파라미터로 적용한다. `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-11: 커스터마이징 레벨 프리뷰용 메쉬 직접 드로잉 1차 구현을 추가했다. 좌클릭 중 캐릭터 Mesh UV 위치를 Stroke로 저장하고 검정 선을 `CanvasRenderTarget2D`에 그리며, 페인트 화면의 `BackButton`은 마지막 Stroke 이전, `ResetButton`은 전체 초기화로 동작한다. `SnowRumbleEditor Win64 Development` 빌드가 성공했다.
- 2026-08-11: 커스터마이징 방 전용 프리뷰 애니메이션 설정을 추가했다. `PreviewAnimationAsset`을 지정하면 프리뷰 캐릭터의 모든 SkeletalMeshComponent를 단일 애니메이션 모드로 전환하고, 기본값으로 정지 상태를 적용한다. C++ 컴파일은 통과했으나 에디터 DLL 잠금으로 최종 링크는 보류됐다.
- 2026-08-11: 페인트 trace에 쓰는 마우스 좌표를 `UWidgetLayoutLibrary::GetMousePositionOnViewport()`와 viewport scale 기준으로 계산하게 보정했다. DPI 스케일 환경에서 `DeprojectScreenPositionToWorld()` trace가 실제 커서보다 왼쪽으로 나가던 문제를 줄인다. C++ 컴파일은 통과했으나 에디터 DLL 잠금으로 최종 링크는 보류됐다.
- 2026-08-11: `PaintCursorScreenOffset`을 추가해 커스터마이징 PlayerController BP에서 페인트 trace X/Y 보정값을 픽셀 단위로 조정할 수 있게 했다.
- 2026-08-11: 커스터마이징 드로잉 stroke를 `FSnowRumbleCustomizationData::PaintStrokes`에 저장·복제하게 했다. 커스터마이징 방에서 그리는 즉시 로컬 저장소에 반영되고, 로비와 PvP 캐릭터는 복제된 stroke 배열로 RenderTarget을 다시 그려 `PaintTexture` 머티리얼 파라미터에 적용한다.
- 2026-08-11: 머리에 그린 선이 로비에서 몸에 나타나는 문제를 막기 위해 stroke마다 맞은 SkeletalMeshComponent 이름을 저장하고, 로비/PvP 캐릭터에서는 컴포넌트별 RenderTarget을 따로 만들어 해당 메쉬에만 적용하게 했다.
- 2026-08-11: 같은 SkeletalMesh에서 머리/몸 위치가 뒤집혀 보이는 문제에 대응해 커스터마이징 방의 UV Y축 flip 기준을 `FSnowRumbleCustomizationData::bFlipPaintUvY`로 함께 저장하고, 로비/PvP 캐릭터가 이 값을 사용해 stroke를 재생성하게 했다.
- 2026-08-11: 페인트 화면에서 `Ctrl+Z` 입력을 `UndoLastPaintStroke()`에 연결했다. `BackButton`과 같은 누적 undo 스택을 사용해 마지막 완료 stroke를 하나씩 제거한다.
- 2026-08-11: 색칠하기 화면에 들어가지 않아도 좌클릭으로 선이 그려지던 문제를 수정했다. 페인트 입력은 `PaintMode` 페이지에서만 처리하고, 다른 페이지에서는 진행 중 stroke를 종료한다.
- 2026-08-11: 시점변경 화면을 카메라 회전 대신 프리뷰 캐릭터 회전으로 정리했다. WBP의 `RotateLeftButton`/`RotateRightButton`은 press 중 `PreviewRotationSpeedDegrees` 기준으로 캐릭터 yaw를 계속 회전한다.
- 2026-08-11: 별도 시점변경 화면 없이 `RotateLeftButton`/`RotateRightButton`을 누르면 현재 페이지에서 바로 프리뷰 캐릭터가 회전하게 했다.
- 2026-08-11: 회전 버튼 좌우 방향을 실제 화면 기준에 맞게 반대로 조정하고, 커스터마이징 레벨 프리뷰 캐릭터의 `OverheadNameplateComponent`를 숨기게 했다.

## 수동 작업

- 메인메뉴 WBP에 커스터마이징 버튼을 배치하고 이름을 `CustomizationButton`으로 맞춘다.
- 커스터마이징 레벨 자산을 만든다.
- 메인메뉴 PlayerController BP의 `CustomizationLevelUrl`에 실제 커스터마이징 레벨 경로를 지정한다. 기본값은 `/Game/Maps/L_Customization`이다.
- 커스터마이징 레벨 GameMode BP는 `ASnowRumbleCustomizationGameMode`를 부모로 만든다.
- 커스터마이징 레벨 PlayerController BP는 `ACustomizationPlayerController`를 부모로 만들고, GameMode BP의 PlayerControllerClass로 지정한다.
- 커스터마이징 WBP는 `UCustomizationWidget`을 부모로 만든다.
- 커스터마이징 PlayerController BP의 `CustomizationWidgetClass`에 커스터마이징 WBP를 지정한다.
- 페인트 위치가 커서와 어긋나면 커스터마이징 PlayerController BP의 `PaintCursorScreenOffset`을 조정한다. X 양수는 오른쪽, Y 양수는 아래쪽으로 trace를 옮긴다.
- 커스터마이징 PlayerController BP의 `PreviewAnimationAsset`에 커마 방에서 보여줄 포즈/애니메이션 에셋을 지정한다. 비워두면 기존 애니메이션 상태를 멈춘다.
- 프리뷰를 완전히 고정하려면 `bPausePreviewAnimation`을 켜고, 특정 프레임 포즈를 쓰려면 `PreviewAnimationPositionSeconds`를 조정한다.
- 레벨에 캐릭터와 카메라를 배치하고, 카메라 액터 태그에 `CustomizationCamera`를 추가한다.
- 커스터마이징 WBP에 `CustomizationContentSwitcher`를 배치하고 자식 순서를 0 메인, 1 시점변경, 2 색칠하기로 맞춘다. 시점변경 화면을 쓰지 않으면 1번 자식은 비워도 된다.
- 메인 화면 버튼 이름을 `PaintModeButton`, `ReturnToLobbyButton`으로 맞춘다.
- 회전 버튼 두 개를 배치하고 이름을 `RotateLeftButton`, `RotateRightButton`으로 맞춘다.
- 회전 속도는 커스터마이징 PlayerController BP의 `PreviewRotationSpeedDegrees`에서 조정한다.
- 하위 화면 공통 버튼 이름을 `BackButton`, `ApplyButton`, `ResetButton`으로 맞춘다.
- 색칠하기 화면의 색상 버튼이나 컬러 피커에서 `SetPreviewBodyColor`를 호출한다.
- 캐릭터 머티리얼에 Vector Parameter `BodyColor`를 만들거나, BP 캐릭터의 `CustomizationBodyColorParameterName`을 실제 파라미터명으로 바꾼다.
- 캐릭터 머티리얼에 Texture Parameter `PaintTexture`를 만들고, 투명한 부분은 기존 색/텍스처를 보이고 검정 알파가 있는 부분은 그려지도록 머티리얼 그래프를 연결한다.
- 색을 적용할 Mesh 슬롯이 0번이 아니면 BP 캐릭터의 `CustomizationMaterialIndex`를 실제 슬롯 번호로 바꾼다.
- 커스터마이징 방과 로비/PvP 캐릭터의 stroke 재생성이 다르게 보이면 BP 캐릭터에서 `CustomizationPaintRenderTargetSize`, `CustomizationPaintStrokeThickness`, `CustomizationPaintBrushColor`, `bFlipCustomizationPaintUvY`를 커스터마이징 PlayerController의 페인트 설정과 맞춘다.
- 캐릭터 Mesh가 마우스 trace에 맞고 UV를 반환해야 하므로 커마 레벨 테스트 전 에디터 재시작 후 `Support UV From Hit Results` 설정이 적용됐는지 확인한다.

## 완료 조건
### 에이전트 확인
- [x] 메인메뉴 커스터마이징 버튼 바인딩 코드 변경 완료
- [x] 커스터마이징 레벨 이동 URL 설정값 코드 변경 완료
- [x] 커스터마이징 전용 GameMode/PlayerController/Widget 부모 코드 변경 완료
- [x] 커스터마이징 카메라 태그 ViewTarget 코드 변경 완료
- [x] WidgetSwitcher 기반 커스터마이징 UI 전환 코드 변경 완료
- [x] 프리뷰 캐릭터 좌/우 회전 코드 변경 완료
- [x] 커스터마이징 프리뷰 캐릭터 애니메이션 설정 코드 변경 완료
- [x] 몸 색상 저장·복제·머티리얼 적용 계약 완료
- [x] 커스터마이징 레벨 검정 브러쉬 메쉬 드로잉 1차 코드 변경 완료
- [x] 드로잉 stroke 저장·복제·로비/PvP 캐릭터 재적용 경로 완료
- [ ] 장비 외형 데이터와 분리 확인
- [ ] S 인계 완료
### 결과 확인

- [ ] 메인메뉴에서 `CustomizationButton`을 누르면 지정된 커스터마이징 레벨로 이동한다.
- [ ] `CustomizationLevelUrl`을 다른 맵 경로로 바꾸면 해당 맵으로 이동한다.
- [ ] 커스터마이징 레벨 진입 시 커스터마이징 WBP가 표시되고 마우스 커서와 클릭 입력을 사용할 수 있다.
- [ ] `CustomizationCamera` 태그가 붙은 카메라 시점으로 캐릭터가 보인다.
- [ ] 커스터마이징 방 프리뷰 캐릭터가 지정한 `PreviewAnimationAsset`의 지정 시점 포즈로 정지한다.
- [ ] `RotateLeftButton`을 누르고 있는 동안 프리뷰 캐릭터가 왼쪽으로 회전한다.
- [ ] `RotateRightButton`을 누르고 있는 동안 프리뷰 캐릭터가 오른쪽으로 회전한다.
- [ ] 좌/우 회전 버튼에서 손을 떼면 프리뷰 캐릭터 회전이 멈춘다.
- [ ] 커스터마이징 레벨 프리뷰 캐릭터 머리 위 이름표가 보이지 않는다.
- [ ] `PaintModeButton`을 누르면 색칠하기 화면으로 전환된다.
- [ ] 색칠하기 화면이 아닌 상태에서 캐릭터 Mesh를 좌클릭해도 선이 그려지지 않는다.
- [ ] `BackButton`을 누르면 메인 화면으로 돌아간다.
- [ ] `ReturnToLobbyButton`을 누르면 메인메뉴 URL로 돌아간다.
- [ ] 색칠하기 화면에서 `SetPreviewBodyColor`를 호출하면 커마 캐릭터 색이 즉시 바뀐다.
- [ ] `ApplyButton`을 누른 뒤 로비에 들어가면 로컬 캐릭터와 다른 화면의 해당 캐릭터 색이 유지된다.
- [ ] PvP 이동 뒤에도 같은 `BodyColor`가 캐릭터에 적용된다.
- [ ] 색칠하기 화면에서 캐릭터 Mesh를 좌클릭 드래그하면 검정 선이 그려진다.
- [ ] 색칠하기 화면에서 커서 위치와 검정 선 시작점이 좌우로 어긋나지 않는다.
- [ ] 색칠하기 화면에서 선을 그린 뒤 Apply 없이 메인메뉴로 돌아갔다가 다시 커스터마이징에 들어와도 선이 유지된다.
- [ ] 선을 그린 뒤 로비에 들어가면 로컬 캐릭터와 다른 화면의 해당 캐릭터에 같은 선이 보인다.
- [ ] PvP 이동 뒤에도 같은 선이 캐릭터에 적용된다.
- [ ] 색칠하기 화면에서 `BackButton`을 누르면 마지막으로 완성한 선 하나가 사라진다.
- [ ] 색칠하기 화면에서 `Ctrl+Z`를 누를 때마다 마지막으로 완성한 선이 하나씩 사라진다.
- [ ] 색칠하기 화면에서 `ResetButton`을 누르면 모든 선이 사라진다.

# Task S-01 - 캐릭터 모델과 드로잉 외형

## 설명

목도리 없는 기본 인간형 캐릭터 모델과 얼굴 표정·드로잉 커스터마이징이 적용될 표현 자산을 만든다.

## 상태 전이 기준
- 시작 가능: 즉시, 드로잉 표현 방식 세부 결정은 연결 전 확정
- 완료 가능: 모델·표현 영역과 C-11 연결 요구 인계 완료

## 구현 항목
- [x] 귀엽고 둥근 기본 인간형 캐릭터 실루엣을 제작한다.
- [x] 목도리를 사용하지 않고 드로잉과 얼굴 표정이 읽히는 표면을 제공한다.
- [ ] 부츠·패딩·장갑·핫팩을 별도 경기 장비 외형으로 연결할 영역을 구분한다.
- [ ] 메인 로비와 경기에서 같은 커스터마이징 결과를 표현할 수 있게 한다.

## 작업 배정
- 담당자·기능·자산 수정자: 서유정(S)
- 계약 소유자: 외형 자산은 S, 저장·복제는 최재원(C)
- 생성·변경 자산:
  - `Content/Characters/MainModel/HandFix.uasset`
  - `Content/Characters/MainModel/HandFix_PhysicsAsset.uasset`
  - `Content/Characters/MainModel/HandFix_Skeleton.uasset`
  - `Content/Characters/MainModel/M_Modeling.uasset`
- 공유 확인 대상: C-11, S-08
- 병합 순서: 원본 모델 → C-11 계약 → 로비·경기 연결

## 공용 계약과 인계
- 제공받을 계약: 없음
- 제공할 계약: 드로잉·표정·장비 외형 적용 요구
- 인계 대상: C-11, S-02, S-08

## 범위 밖
- 저장·복제 C++와 아이템 능력 로직

## 사전 전제
- 없음

## 결정 필요
- 드로잉 입력 방식과 표정 우선순위는 C-11 연결 전 확정

## 수동 작업 (구현 후 구체화)
- Unreal Editor에서 `Content/Characters/MainModel/HandFix` Skeletal Mesh를 열어 Skeleton과 Physics Asset 참조가 정상인지 확인한다.
- `M_Modeling` 머티리얼이 캐릭터 표면에 의도한 외형으로 적용되는지 확인한다.
- C-11 연결 전, 드로잉 입력 방식과 표정 우선순위를 확정한다.

## 완료 조건
### 에이전트 확인
- [x] 캐릭터·표현 자산 초안 추가
- [x] 목도리 없는 캐릭터 모델 자산 추가
- [ ] C-11·S-08 인계 완료
### 결과 확인 (구현 후 구체화)
- Unreal Editor에서 `HandFix` Skeletal Mesh가 열리고 메시, 스켈레톤, 피직스 에셋 오류가 없는지 확인한다.
- 메인 로비와 경기 캐릭터 Blueprint에 연결했을 때 커스터마이징 표면과 기본 실루엣이 유지되는지 확인한다.
- 장갑·핫팩·부츠 등 경기 장비 외형 연결이 필요한 영역을 S-08 또는 C-11 인계 메모로 확정한다.

## 변경 기록
- 2026-08-30: `HandFix` 캐릭터 Skeletal Mesh, Skeleton, Physics Asset과 `M_Modeling` 머티리얼 자산을 추가했다. C-11·S-08 인계와 실제 Blueprint 연결 확인 전이므로 Task는 진행중으로 유지한다.

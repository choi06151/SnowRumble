// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AudioCaptureCore.h"
#include "SnowRumbleAudioUserWidget.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"
#include "Styling/SlateTypes.h"
#include "../Player/SnowRumbleUserSettingsSubsystem_C.h"
#include "OptionsWidget_C.generated.h"

class UButton;
class UComboBoxString;
class UOptionsKeyBindingRowWidget;
class UPanelWidget;
class USoundClass;
class USoundMix;
class USlider;
class UTextBlock;
class UWidgetSwitcher;

USTRUCT(BlueprintType)
struct FSnowRumbleKeyBindingViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnowRumble|UI|Options")
	FName BindingId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnowRumble|UI|Options")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnowRumble|UI|Options")
	FKey CurrentKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnowRumble|UI|Options")
	FKey DefaultKey;
};

UENUM(BlueprintType)
enum class ESnowRumbleOptionsCategory : uint8
{
	Sensitivity,
	Audio,
	KeyBinding,
	Microphone
};

UCLASS(Abstract, Blueprintable)
class SNOWRUMBLE_API UOptionsWidget : public USnowRumbleAudioUserWidget
{
	GENERATED_BODY()

public:
	UOptionsWidget(const FObjectInitializer& ObjectInitializer);

	/** C++ 소유 메뉴가 옵션 닫기 요청을 받을 때 사용하는 델리게이트다. */
	FSimpleMulticastDelegate OnOptionsCloseRequestedNative;

	/** 옵션 카테고리를 선택하고 하단 WidgetSwitcher 페이지를 전환한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Options")
	void SetOptionsCategory(ESnowRumbleOptionsCategory NewCategory);

	/** 현재 선택된 옵션 카테고리를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Options")
	ESnowRumbleOptionsCategory GetCurrentOptionsCategory() const;

	/** 현재 키 설정 패널에 표시할 기본 조작 목록을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Options|Key Binding")
	const TArray<FSnowRumbleKeyBindingViewData>& GetKeyBindingRows() const;

	/** WBP에서 임시 변경 여부를 알려 적용 버튼 활성 상태를 갱신한다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Options")
	void SetHasPendingOptionChanges(bool bNewHasPendingChanges);

	/** 저장되지 않은 임시 옵션 변경이 있는지 반환한다. */
	UFUNCTION(BlueprintPure, Category = "SnowRumble|UI|Options")
	bool HasPendingOptionChanges() const;

	/** 적용하지 않고 옵션을 닫을 때 임시 변경을 폐기하고 저장된 값으로 되돌린다. */
	UFUNCTION(BlueprintCallable, Category = "SnowRumble|UI|Options")
	void DiscardPendingOptionChanges();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(
		const FGeometry& MyGeometry,
		float InDeltaTime) override;
	virtual FReply NativeOnPreviewKeyDown(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	/** 카테고리가 바뀔 때 Blueprint가 버튼 상태와 패널 표현을 갱신할 수 있다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options")
	void OnOptionsCategoryChanged(ESnowRumbleOptionsCategory NewCategory);

	/** 닫기 버튼이 눌렸을 때 호출된다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options")
	void OnOptionsCloseRequested();

	/** 적용 버튼이 눌렸을 때 호출된다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options")
	void OnOptionsApplyRequested();

	/** 초기화 버튼이 눌렸을 때 호출된다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options")
	void OnOptionsResetRequested();

	/** 현재 선택된 카테고리의 옵션 초기화가 요청됐을 때 호출된다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options")
	void OnOptionsCategoryResetRequested(ESnowRumbleOptionsCategory Category);

	/** 키 설정 행 목록이 갱신될 때 Blueprint 표현을 보강할 수 있다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options|Key Binding")
	void OnKeyBindingRowsRefreshed(
		const TArray<FSnowRumbleKeyBindingViewData>& Rows);

	/** 키 변경 버튼이 눌려 다음 키 입력 대기 상태가 됐을 때 호출된다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options|Key Binding")
	void OnKeyRebindRequested(FName BindingId);

	/** 키가 실제로 변경됐을 때 호출된다. 실제 Enhanced Input 적용은 후속 저장·적용 단계에서 붙인다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options|Key Binding")
	void OnKeyBindingChanged(FName BindingId, FKey NewKey);

	/** 키 변경 대기가 취소됐을 때 호출된다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options|Key Binding")
	void OnKeyRebindCanceled(FName BindingId);

	/** 키 초기화 버튼이 눌렸을 때 호출된다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options|Key Binding")
	void OnKeyBindingResetRequested(FName BindingId);

	/** 마이크 방식이 임시로 바뀔 때 Blueprint가 버튼 선택 표시를 갱신할 수 있다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options|Microphone")
	void OnMicrophoneModeChanged(ESnowRumbleMicrophoneMode NewMode);

	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options|Microphone")
	void OnMicrophoneDeviceChanged(const FString& DeviceId, const FString& DeviceName);

	/** 마이크 테스트 상태와 현재 입력 레벨을 Blueprint가 표시할 수 있는 이벤트다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "SnowRumble|UI|Options|Microphone")
	void OnMicrophoneTestStateChanged(
		bool bIsTesting,
		bool bInputDetected,
		float InputLevel,
		const FText& StatusText);

	/** 상단 감도 카테고리 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options")
	TObjectPtr<UButton> SensitivityCategoryButton;

	/** 상단 사운드 카테고리 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options")
	TObjectPtr<UButton> AudioCategoryButton;

	/** 상단 키 설정 카테고리 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options")
	TObjectPtr<UButton> KeyBindingCategoryButton;

	/** 상단 마이크 카테고리 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options")
	TObjectPtr<UButton> MicrophoneCategoryButton;

	/** 하단 옵션 내용 패널을 전환하는 WidgetSwitcher다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options")
	TObjectPtr<UWidgetSwitcher> OptionsContentSwitcher;

	/** 옵션 메뉴를 닫는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options")
	TObjectPtr<UButton> CloseButton;

	/** 현재 옵션 값을 적용하는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options")
	TObjectPtr<UButton> ApplyButton;

	/** 옵션 값을 기본값으로 되돌리는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options")
	TObjectPtr<UButton> ResetButton;

	/** 마우스/카메라 감도를 조절하는 슬라이더다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Sensitivity")
	TObjectPtr<USlider> SensitivitySlider;

	/** 현재 감도 값을 퍼센트로 표시하는 텍스트다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Sensitivity")
	TObjectPtr<UTextBlock> SensitivityValueText;

	/** 전체 볼륨을 조절하는 슬라이더다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Audio")
	TObjectPtr<USlider> MasterVolumeSlider;

	/** 전체 볼륨 값을 퍼센트로 표시하는 텍스트다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Audio")
	TObjectPtr<UTextBlock> MasterVolumeValueText;

	/** 배경음악 볼륨을 조절하는 슬라이더다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Audio")
	TObjectPtr<USlider> BgmVolumeSlider;

	/** 배경음악 볼륨 값을 퍼센트로 표시하는 텍스트다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Audio")
	TObjectPtr<UTextBlock> BgmVolumeValueText;

	/** 효과음 볼륨을 조절하는 슬라이더다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Audio")
	TObjectPtr<USlider> SfxVolumeSlider;

	/** 효과음 볼륨 값을 퍼센트로 표시하는 텍스트다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Audio")
	TObjectPtr<UTextBlock> SfxVolumeValueText;

	/** 보이스 볼륨을 조절하는 슬라이더다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Audio")
	TObjectPtr<USlider> VoiceVolumeSlider;

	/** 보이스 볼륨 값을 퍼센트로 표시하는 텍스트다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Audio")
	TObjectPtr<UTextBlock> VoiceVolumeValueText;

	/** 지정하면 적용 시 배경음악 SoundClass 볼륨에 저장값을 반영한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Options|Audio")
	TObjectPtr<USoundClass> BgmSoundClass;

	/** 지정하면 적용 시 효과음 SoundClass 볼륨에 저장값을 반영한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Options|Audio")
	TObjectPtr<USoundClass> SfxSoundClass;

	/** 지정하면 적용 시 보이스 SoundClass 볼륨에 저장값을 반영한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Options|Audio")
	TObjectPtr<USoundClass> VoiceSoundClass;

	/** 옵션 메뉴가 열려 있는 동안만 사용하는 라이브 오디오 프리뷰 믹스다. */
	UPROPERTY(Transient)
	TObjectPtr<USoundMix> LiveAudioPreviewSoundMix;

	/** 마이크 입력 음량을 조절하는 슬라이더다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Microphone")
	TObjectPtr<USlider> MicrophoneVolumeSlider;

	/** 마이크 입력 음량 값을 퍼센트로 표시하는 텍스트다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Microphone")
	TObjectPtr<UTextBlock> MicrophoneVolumeValueText;

	/** 누르고 있을 때만 말하기 방식을 선택하는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Microphone")
	TObjectPtr<UButton> MicrophonePushToTalkButton;

	/** 항상 말하기 방식을 선택하는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Microphone")
	TObjectPtr<UButton> MicrophoneAlwaysOnButton;

	/** 현재 사용할 마이크 장치를 선택하는 ComboBoxString이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Microphone")
	TObjectPtr<UComboBoxString> MicrophoneDeviceComboBox;

	/** 선택한 장치의 로컬 입력 테스트를 시작하거나 중지하는 버튼이다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Microphone")
	TObjectPtr<UButton> MicrophoneTestButton;

	/** 마이크 테스트 상태를 표시하는 텍스트다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Microphone")
	TObjectPtr<UTextBlock> MicrophoneTestStatusText;

	/** 마이크 테스트 입력 레벨을 0~1로 표시하는 ProgressBar다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Microphone")
	TObjectPtr<class UProgressBar> MicrophoneInputLevelProgressBar;

	/** 키 설정 행을 자동 생성할 패널이다. VerticalBox 또는 ScrollBox를 쓸 수 있다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Key Binding")
	TObjectPtr<UPanelWidget> KeyBindingListBox;

	/** 키 변경 대기 상태 같은 짧은 안내를 표시할 선택 텍스트다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "SnowRumble|UI|Options|Key Binding")
	TObjectPtr<UTextBlock> KeyBindingStatusText;

	/** 있으면 키 설정 행을 이 WBP 클래스로 생성한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SnowRumble|UI|Options|Key Binding")
	TSubclassOf<UOptionsKeyBindingRowWidget> KeyBindingRowWidgetClass;

private:
	UFUNCTION()
	void HandleSensitivityCategoryButtonClicked();

	UFUNCTION()
	void HandleAudioCategoryButtonClicked();

	UFUNCTION()
	void HandleKeyBindingCategoryButtonClicked();

	UFUNCTION()
	void HandleMicrophoneCategoryButtonClicked();

	UFUNCTION()
	void HandleCloseButtonClicked();

	UFUNCTION()
	void HandleApplyButtonClicked();

	UFUNCTION()
	void HandleResetButtonClicked();

	UFUNCTION()
	void HandleSensitivitySliderValueChanged(float NewValue);

	UFUNCTION()
	void HandleMasterVolumeSliderValueChanged(float NewValue);

	UFUNCTION()
	void HandleBgmVolumeSliderValueChanged(float NewValue);

	UFUNCTION()
	void HandleSfxVolumeSliderValueChanged(float NewValue);

	UFUNCTION()
	void HandleVoiceVolumeSliderValueChanged(float NewValue);

	UFUNCTION()
	void HandleMicrophoneVolumeSliderValueChanged(float NewValue);

	UFUNCTION()
	void HandleMicrophonePushToTalkButtonClicked();

	UFUNCTION()
	void HandleMicrophoneAlwaysOnButtonClicked();

	UFUNCTION()
	void HandleMicrophoneDeviceSelectionChanged(
		FString SelectedItem,
		ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleMicrophoneTestButtonClicked();

	void HandleKeyRowRebindRequested(FName BindingId);
	void HandleKeyRowResetRequested(FName BindingId);

	/** 임시 옵션 변경을 실제 로컬 설정에 저장하고 게임 입력에 반영한다. */
	void ApplyPendingOptionChanges();

	/** 현재 선택된 카테고리의 설정값을 기본값으로 되돌린다. */
	void ResetCurrentOptionsCategory();

	/** 키 설정 패널의 모든 행과 저장값을 기본값으로 되돌린다. */
	void ResetAllKeyBindingsToDefault();

	/** 감도 슬라이더와 텍스트를 저장된 설정 기준으로 구성한다. */
	void InitializeSensitivitySetting();

	/** 소리 슬라이더와 텍스트를 저장된 설정 기준으로 구성한다. */
	void InitializeAudioSettings();

	/** 저장된 BGM/SFX 볼륨을 지정된 SoundClass에 반영한다. */
	void ApplyAudioVolumeSettings() const;

	/** 현재 임시 오디오 값을 활성 사운드 믹스에 바로 반영한다. */
	void ApplyAudioPreviewSoundMix();

	/** 마이크 슬라이더와 방식 버튼을 저장된 설정 기준으로 구성한다. */
	void InitializeMicrophoneSettings();

	/** 운영체제에서 사용 가능한 마이크 장치 목록을 조회해 ComboBox를 구성한다. */
	void RefreshMicrophoneDeviceList();

	/** 현재 선택 장치의 오디오 캡처 테스트를 시작한다. */
	void StartMicrophoneTest();

	/** 오디오 캡처 테스트를 중지하고 스트림을 닫는다. */
	void StopMicrophoneTest();

	/** 오디오 캡처 콜백에서 입력 레벨을 축적한다. */
	void HandleMicrophoneCapture(
		const void* AudioData,
		int32 NumFrames,
		int32 NumChannels,
		int32 SampleRate,
		double StreamTime,
		bool bOverFlow);

	/** 테스트 버튼과 상태 텍스트·레벨 표시를 갱신한다. */
	void RefreshMicrophoneTestDisplay(float InDeltaTime);

	/** 선택 바인딩된 버튼 클릭 이벤트를 연결한다. */
	void BindOptionButtons();

	/** 선택 바인딩된 버튼 클릭 이벤트를 해제한다. */
	void UnbindOptionButtons();

	/** 카테고리에 대응하는 WidgetSwitcher 인덱스를 반환한다. */
	int32 GetSwitcherIndexForCategory(ESnowRumbleOptionsCategory Category) const;

	/** 키 설정 패널 표시 데이터를 구성한다. */
	void InitializeDefaultKeyBindingRows();

	/** KeyBindingListBox가 있으면 키 설정 행을 자동 생성한다. */
	void RefreshKeyBindingPanel();

	/** 적용 버튼을 현재 임시 변경 여부에 맞춰 활성화하거나 비활성화한다. */
	void RefreshApplyButtonEnabled();

	/** 키 설정 행 값과 저장된 사용자 설정이 다른지 확인한다. */
	bool HasPendingKeyBindingChanges() const;

	/** 현재 임시 옵션 값과 저장된 설정이 다른지 확인한다. */
	bool HasAnyPendingOptionChanges() const;

	/** 감도 설정이 저장값과 다른지 확인한다. */
	bool HasPendingSensitivityChanges() const;

	/** 소리 설정이 저장값과 다른지 확인한다. */
	bool HasPendingAudioChanges() const;

	/** 마이크 설정이 저장값과 다른지 확인한다. */
	bool HasPendingMicrophoneChanges() const;

	/** 감도 슬라이더의 0~1 값을 실제 감도 값으로 변환한다. */
	float ConvertSliderValueToSensitivity(float SliderValue) const;

	/** 실제 감도 값을 슬라이더 0~1 값으로 변환한다. */
	float ConvertSensitivityToSliderValue(float Sensitivity) const;

	/** 감도 표시 텍스트를 현재 임시값 기준으로 갱신한다. */
	void RefreshSensitivityValueText();

	/** 소리 표시 텍스트를 현재 임시값 기준으로 갱신한다. */
	void RefreshAudioValueText();

	/** 마이크 표시 텍스트를 현재 임시값 기준으로 갱신한다. */
	void RefreshMicrophoneValueText();

	/** 현재 로컬 플레이어의 배경음악 프리뷰 볼륨을 갱신한다. */
	void ApplyBackgroundMusicPreviewVolume() const;

	/** 버튼 기본 스타일을 보관한 뒤 선택 상태에서는 Pressed 스타일을 유지한다. */
	void SetButtonSelectedVisual(UButton* Button, bool bSelected);

	/** 현재 선택된 옵션 카테고리 버튼의 눌림 표시를 갱신한다. */
	void RefreshCategoryButtonSelection();

	/** 현재 선택된 마이크 방식 버튼의 눌림 표시를 갱신한다. */
	void RefreshMicrophoneModeButtonSelection();

	/** 키 변경 대기를 시작한다. */
	void BeginKeyRebind(FName BindingId);

	/** 키 변경 대기를 취소한다. */
	void CancelKeyRebind();

	/** 대기 중인 키바인딩에 새 키를 적용한다. */
	bool ApplyCapturedKey(FKey NewKey);

	/** 입력 캡처 대상으로 쓸 수 있는 키인지 확인한다. */
	bool IsBindableKey(FKey Key) const;

	ESnowRumbleOptionsCategory CurrentOptionsCategory =
		ESnowRumbleOptionsCategory::Sensitivity;

	UPROPERTY(Transient)
	TArray<FSnowRumbleKeyBindingViewData> KeyBindingRows;

	FName PendingKeyBindingId = NAME_None;

	float PendingMouseSensitivity = 1.0f;

	float PendingMasterVolume = 1.0f;

	float PendingBgmVolume = 1.0f;

	float PendingSfxVolume = 1.0f;

	float PendingVoiceVolume = 1.0f;

	float PendingMicrophoneVolume = 1.0f;

	ESnowRumbleMicrophoneMode PendingMicrophoneMode =
		ESnowRumbleMicrophoneMode::PushToTalk;

	FString PendingMicrophoneDeviceId;
	FString PendingMicrophoneDeviceName;

	bool bIsUpdatingSensitivitySlider = false;

	bool bIsUpdatingAudioSliders = false;

	bool bIsUpdatingMicrophoneSlider = false;

	bool bIsUpdatingMicrophoneDeviceComboBox = false;

	bool bHasPendingOptionChanges = false;

	TMap<FString, FString> MicrophoneDeviceIdsByName;

	TMap<FString, int32> MicrophoneDeviceIndicesByName;

	TUniquePtr<Audio::FAudioCapture> MicrophoneTestCapture;

	FCriticalSection MicrophoneTestCaptureCriticalSection;

	float PendingMicrophoneInputLevel = 0.0f;

	float DisplayedMicrophoneInputLevel = 0.0f;

	bool bIsMicrophoneTestActive = false;

	TMap<UButton*, FButtonStyle> DefaultButtonStyles;

};

// Copyright Epic Games, Inc. All Rights Reserved.

#include "LatestPhotoScreenshotWidget_C.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void ULatestPhotoScreenshotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (bRefreshOnConstruct)
	{
		RefreshLatestScreenshot();
	}
	else
	{
		UpdateScreenshotNameText();
	}

	if (!ScreenshotProcessedDelegateHandle.IsValid())
	{
		ScreenshotProcessedDelegateHandle =
			FScreenshotRequest::OnScreenshotRequestProcessed().AddUObject(
				this,
				&ULatestPhotoScreenshotWidget::HandleScreenshotRequestProcessed);
	}
}

void ULatestPhotoScreenshotWidget::NativeDestruct()
{
	if (ScreenshotProcessedDelegateHandle.IsValid())
	{
		FScreenshotRequest::OnScreenshotRequestProcessed().Remove(
			ScreenshotProcessedDelegateHandle);
		ScreenshotProcessedDelegateHandle.Reset();
	}

	Super::NativeDestruct();
}

bool ULatestPhotoScreenshotWidget::RefreshLatestScreenshot()
{
	FString ScreenshotPath;
	if (!FindLatestScreenshotFile(ScreenshotPath))
	{
		LatestScreenshotTexture = nullptr;
		LatestScreenshotPath.Empty();
		ApplyScreenshotTexture(nullptr);
		UpdateScreenshotNameText();
		return false;
	}

	UTexture2D* LoadedTexture =
		FImageUtils::ImportFileAsTexture2D(ScreenshotPath);
	if (!LoadedTexture)
	{
		LatestScreenshotTexture = nullptr;
		LatestScreenshotPath = ScreenshotPath;
		ApplyScreenshotTexture(nullptr);
		UpdateScreenshotNameText();
		return false;
	}

	LatestScreenshotTexture = LoadedTexture;
	LatestScreenshotPath = ScreenshotPath;
	ApplyScreenshotTexture(LatestScreenshotTexture);
	UpdateScreenshotNameText();
	return true;
}

const FString& ULatestPhotoScreenshotWidget::GetLatestScreenshotPath() const
{
	return LatestScreenshotPath;
}

UTexture2D* ULatestPhotoScreenshotWidget::GetLatestScreenshotTexture() const
{
	return LatestScreenshotTexture;
}

bool ULatestPhotoScreenshotWidget::FindLatestScreenshotFile(
	FString& OutScreenshotPath) const
{
	const FString ScreenshotRoot =
		FPaths::ConvertRelativePathToFull(FPaths::ScreenShotDir());
	const FString FilenamePattern =
		bOnlyPhotoScreenshots ? TEXT("Photo_*.png") : TEXT("*.png");

	TArray<FString> ScreenshotFiles;
	IFileManager::Get().FindFilesRecursive(
		ScreenshotFiles,
		*ScreenshotRoot,
		*FilenamePattern,
		true,
		false,
		false);

	FDateTime LatestTimestamp = FDateTime::MinValue();
	bool bFoundFile = false;
	for (const FString& ScreenshotFile : ScreenshotFiles)
	{
		const FDateTime FileTimestamp =
			IFileManager::Get().GetTimeStamp(*ScreenshotFile);
		if (!bFoundFile || FileTimestamp > LatestTimestamp)
		{
			LatestTimestamp = FileTimestamp;
			OutScreenshotPath = ScreenshotFile;
			bFoundFile = true;
		}
	}

	return bFoundFile;
}

void ULatestPhotoScreenshotWidget::ApplyScreenshotTexture(
	UTexture2D* ScreenshotTexture)
{
	if (!LatestScreenshotImage)
	{
		return;
	}

	if (!ScreenshotTexture)
	{
		LatestScreenshotImage->SetBrushFromTexture(nullptr);
		return;
	}

	LatestScreenshotImage->SetBrushFromTexture(ScreenshotTexture, true);
}

void ULatestPhotoScreenshotWidget::UpdateScreenshotNameText()
{
	if (!LatestScreenshotNameText)
	{
		return;
	}

	LatestScreenshotNameText->SetText(
		LatestScreenshotPath.IsEmpty()
			? FText::GetEmpty()
			: FText::FromString(FPaths::GetCleanFilename(LatestScreenshotPath)));
}

void ULatestPhotoScreenshotWidget::HandleScreenshotRequestProcessed()
{
	RefreshLatestScreenshot();
}

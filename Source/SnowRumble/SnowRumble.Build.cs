// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class SnowRumble : ModuleRules
{
	public SnowRumble(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
    
		// ★ 여기에 "NavigationSystem"을 추가합니다.
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "OnlineSubsystem", "UMG", "NavigationSystem" });

		// 기존 Private에서 "NavigationSystem"은 제거합니다.
		PrivateDependencyModuleNames.AddRange(new string[] { "AppFramework", "AudioCaptureCore", "MoviePlayer", "Niagara", "OnlineSubsystemUtils", "Slate", "SlateCore" });

		// Steam OSS가 패키징된 실행 파일을 Steam으로 재실행할 수 있도록 테스트 App ID 파일을 함께 배포한다.
		RuntimeDependencies.Add(
			"$(TargetOutputDir)/steam_appid.txt",
			Path.Combine(ModuleDirectory, "..", "..", "steam_appid.txt"));

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}

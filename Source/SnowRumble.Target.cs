// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SnowRumbleTarget : TargetRules
{
	public SnowRumbleTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		// Shipping builds must not compile or emit runtime log output.
		bUseLoggingInShipping = false;
		ExtraModuleNames.Add("SnowRumble");
	}
}

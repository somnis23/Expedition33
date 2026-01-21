// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Expedition33 : ModuleRules
{
	public Expedition33(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore",
			"EnhancedInput", "AudioMixer"});
	}
}

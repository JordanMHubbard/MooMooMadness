// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MooMooMadness : ModuleRules
{
	public MooMooMadness(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}

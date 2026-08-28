// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UTweenProject : ModuleRules
{
	public UTweenProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

		PrivateDependencyModuleNames.AddRange(new string[] { "UTween", "UMG", "Slate", "SlateCore" });
	}
}

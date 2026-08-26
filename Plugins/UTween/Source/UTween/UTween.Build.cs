using UnrealBuildTool;

public class UTween : ModuleRules
{
	public UTween(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		IWYUSupport = IWYUSupport.Full;

		SharedPCHHeaderFile = "Public/UTweenSharedPCH.h";

		PublicIncludePaths.Add(ModuleDirectory + "/Public");

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"UMG",
			"Slate",
			"SlateCore"
		});
	}
}
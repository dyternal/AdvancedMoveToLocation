using UnrealBuildTool;

public class AdvancedMoveToLocation : ModuleRules
{
	public AdvancedMoveToLocation(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"AIModule",
			"NavigationSystem",
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
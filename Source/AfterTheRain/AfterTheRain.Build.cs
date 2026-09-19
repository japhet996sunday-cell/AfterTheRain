using UnrealBuildTool;

public class AfterTheRain : ModuleRules
{
	public AfterTheRain(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayTags",
			"UMG",
			"Slate",
			"SlateCore",
			"LevelSequence",
			"MovieScene",
			"MovieSceneTracks"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Json",
			"JsonUtilities"
		});

		bEnforceIWYU = true;
		bUseUnity = false;
	}
}

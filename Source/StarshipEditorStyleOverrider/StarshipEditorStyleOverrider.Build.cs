// Copyright 2023 aquasilver, Inc. All Rights Reserved.

using UnrealBuildTool;

public class StarshipEditorStyleOverrider : ModuleRules
{
	public StarshipEditorStyleOverrider(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);


		if (!(Target.Version.MajorVersion >= 5 && Target.Version.MinorVersion >= 3))
		{
			PrivateIncludePaths.AddRange(
				new string[] {
					// ... add other private include paths required here ...
					System.IO.Path.Combine(GetModuleDirectory("PropertyEditor"), "Private"),
				}
				);
		}
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"EditorStyle",
				"Settings",
				"ToolWidgets",
				"Kismet",
				"Json",
				"JsonUtilities",
				"InputCore",
				"Projects",
				"PropertyEditor"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}

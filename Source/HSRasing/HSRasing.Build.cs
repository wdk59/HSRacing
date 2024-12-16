// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HSRasing : ModuleRules
{
	public HSRasing(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "PhysXVehicles", "HeadMountedDisplay", "Niagara" });

		PublicDefinitions.Add("HMD_MODULE_INCLUDED=1");
	}
}

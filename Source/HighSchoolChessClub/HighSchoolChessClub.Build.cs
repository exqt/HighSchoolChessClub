// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HighSchoolChessClub : ModuleRules
{
	public HighSchoolChessClub(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// The migrated sources include headers from the module root (for example,
		// "UI/Common/CCGameButton.h"). Modern build settings do not add that
		// directory implicitly.
		PrivateIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"CommonUI",
			"GameplayTags",
			"UMG",
			"ChessCore",
			"ChessBots"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"DeveloperSettings",
			"MoviePlayer",
			"Slate",
			"SlateCore", 
			"CommonInput",
			"GameSettings",
			"AudioMixer",
			"AudioModulation",
			"PropertyPath"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}

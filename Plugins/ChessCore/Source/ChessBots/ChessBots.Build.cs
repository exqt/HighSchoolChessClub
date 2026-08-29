// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class ChessBots : ModuleRules
{
	public ChessBots(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;

		string ThirdPartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty/chess-library"));
		string OpeningBookPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty/chess-openings.tsv"));

		PublicDefinitions.Add("CHESS_NO_EXCEPTIONS=1");

		PrivateIncludePaths.Add(Path.Combine(ThirdPartyPath, "include"));
		RuntimeDependencies.Add("$(PluginDir)/Resources/chess-openings.tsv", OpeningBookPath);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"ChessCore"
			});

		PrivateDependencyModuleNames.Add("Projects");
	}
}

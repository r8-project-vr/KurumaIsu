// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class DemoProjectVR : ModuleRules
{
	public DemoProjectVR(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // 共通
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "BleGoodies", "AIModule", "NavigationSystem" });

        // Windowsだけ
        if (Target.Platform == UnrealTargetPlatform.Win64)
            PublicDependencyModuleNames.AddRange(new string[] { "ASerialCom" });
        // Androidだけ
        else if (Target.Platform == UnrealTargetPlatform.Android)
            PublicDependencyModuleNames.AddRange(new string[] { });
		// IOSだけ
		else if (Target.Platform == UnrealTargetPlatform.IOS)
			PublicDependencyModuleNames.AddRange(new string[] { });

        PrivateDependencyModuleNames.AddRange(new string[] { "TextureCompressor", "UMG", "Slate", "SlateCore" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}

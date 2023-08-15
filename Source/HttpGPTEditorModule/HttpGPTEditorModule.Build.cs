// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

using System.IO;
using UnrealBuildTool;

public class HttpGPTEditorModule : ModuleRules
{
    public HttpGPTEditorModule(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp17;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Engine",
            "CoreUObject",
            "InputCore",
            "Slate",
            "SlateCore",
            "UnrealEd",
            "ToolMenus",
            "EditorStyle",
            "WorkspaceMenuStructure",
            "Projects",
            "AssetRegistry",
            "HttpGPTCommonModule",
            "HttpGPTChatModule",
            "HttpGPTImageModule",
            "Json"
        });
    }
}
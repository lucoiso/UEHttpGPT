// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

using System.IO;
using UnrealBuildTool;

public class HttpGPTEditor : ModuleRules
{
    public HttpGPTEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp17;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "HttpGPT"
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
            "Projects"
        });
    }
}

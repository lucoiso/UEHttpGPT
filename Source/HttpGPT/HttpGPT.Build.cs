// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEHttpGPT

using UnrealBuildTool;

public class HttpGPT : ModuleRules
{
    public HttpGPT(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp17;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "HTTP",
            "Json"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Engine",
            "CoreUObject",
            "DeveloperSettings"
        });

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }
    }
}

// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEHttpGPT

using UnrealBuildTool;

public class HttpGPTImageModule : ModuleRules
{
    public HttpGPTImageModule(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp17;

        if (Target.Platform == UnrealTargetPlatform.HoloLens)
        {
            PrecompileForTargets = PrecompileTargetsType.Any;
        }

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "HTTP",
            "Json",
            "HttpGPTCommonModule"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Engine",
            "CoreUObject"
        });

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }
    }
}
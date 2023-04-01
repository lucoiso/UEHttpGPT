// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "Management/HttpGPTSettings.h"
#include "LogHttpGPT.h"
#include <Runtime/Launch/Resources/Version.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(HttpGPTSettings)
#endif

UHttpGPTSettings::UHttpGPTSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), bUseCustomSystemContext(false), CustomSystemContext(FString()), bEnableInternalLogs(false)
{
	CategoryName = TEXT("Plugins");

	SetToDefaults();
}

const UHttpGPTSettings* UHttpGPTSettings::Get()
{
	const UHttpGPTSettings* const Instance = GetDefault<UHttpGPTSettings>();
	return Instance;
}

FHttpGPTCommonOptions UHttpGPTSettings::GetCommonOptions()
{
	return GetDefault<UHttpGPTSettings>()->CommonOptions;
}

void UHttpGPTSettings::SetCommonOptions(const FHttpGPTCommonOptions& Value)
{
	UHttpGPTSettings* const Settings = GetMutableDefault<UHttpGPTSettings>();
	Settings->CommonOptions = Value;

	Settings->SaveAndReload(GET_MEMBER_NAME_CHECKED(UHttpGPTSettings, CommonOptions));
}

FHttpGPTChatOptions UHttpGPTSettings::GetChatOptions()
{
	return GetDefault<UHttpGPTSettings>()->ChatOptions;
}

void UHttpGPTSettings::SetChatOptions(const FHttpGPTChatOptions& Value)
{
	UHttpGPTSettings* const Settings = GetMutableDefault<UHttpGPTSettings>();
	Settings->ChatOptions = Value;

	Settings->SaveAndReload(GET_MEMBER_NAME_CHECKED(UHttpGPTSettings, ChatOptions));
}

FHttpGPTImageOptions UHttpGPTSettings::GetImageOptions()
{
	return GetDefault<UHttpGPTSettings>()->ImageOptions;
}

void UHttpGPTSettings::SetImageOptions(const FHttpGPTImageOptions& Value)
{
	UHttpGPTSettings* const Settings = GetMutableDefault<UHttpGPTSettings>();
	Settings->ImageOptions = Value;

	Settings->SaveAndReload(GET_MEMBER_NAME_CHECKED(UHttpGPTSettings, ImageOptions));
}

#if WITH_EDITOR
void UHttpGPTSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UHttpGPTSettings, bEnableInternalLogs))
	{
		ToggleInternalLogs();
	}
}
#endif

void UHttpGPTSettings::PostInitProperties()
{
	Super::PostInitProperties();
	ToggleInternalLogs();
}

void UHttpGPTSettings::SetToDefaults()
{
	CommonOptions.APIKey = NAME_None;
	CommonOptions.User = NAME_None;

	ChatOptions.Model = EHttpGPTChatModel::gpt35turbo;
	ChatOptions.MaxTokens = 2048;
	ChatOptions.Temperature = 1.f;
	ChatOptions.TopP = 1.f;
	ChatOptions.Choices = 1;
	ChatOptions.bStream = true;
	ChatOptions.Stop = TArray<FName>();
	ChatOptions.PresencePenalty = 0.f;
	ChatOptions.FrequencyPenalty = 0.f;
	ChatOptions.LogitBias = TMap<int32, float>();

	ImageOptions.ImagesNum = 1;
	ImageOptions.Size = EHttpGPTImageSize::x256;
	ImageOptions.Format = EHttpGPTResponseFormat::url;
}

void UHttpGPTSettings::SaveAndReload(const FName& PropertyName)
{
	SaveConfig();

	uint32 PropagationFlags = 0u;

#if ENGINE_MAJOR_VERSION >= 5
	PropagationFlags = UE::ELoadConfigPropagationFlags::LCPF_PropagateToChildDefaultObjects;
#else
	PropagationFlags = UE4::ELoadConfigPropagationFlags::LCPF_PropagateToChildDefaultObjects;
#endif

	ReloadConfig(GetClass(), *GetDefaultConfigFilename(), PropagationFlags, GetClass()->FindPropertyByName(PropertyName));
}

void UHttpGPTSettings::ToggleInternalLogs()
{
#if !UE_BUILD_SHIPPING
	LogHttpGPT_Internal.SetVerbosity(bEnableInternalLogs ? ELogVerbosity::Display : ELogVerbosity::NoLogging);
#endif
}

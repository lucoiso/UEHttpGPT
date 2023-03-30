// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "Management/HttpGPTSettings.h"
#include "LogHttpGPT.h"
#include <Runtime/Launch/Resources/Version.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(HttpGPTSettings)
#endif

UHttpGPTSettings::UHttpGPTSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), APIKey(NAME_None), bEnableInternalLogs(false)
{
	CategoryName = TEXT("Plugins");

	SetToDefaults();
}

const UHttpGPTSettings* UHttpGPTSettings::Get()
{
	const UHttpGPTSettings* const Instance = GetDefault<UHttpGPTSettings>();
	return Instance;
}

FName UHttpGPTSettings::GetAPIKey()
{
	return GetDefault<UHttpGPTSettings>()->APIKey;
}

void UHttpGPTSettings::SetAPIKey(const FName& Value)
{
	UHttpGPTSettings* const Settings = GetMutableDefault<UHttpGPTSettings>();
	Settings->APIKey = Value;

	Settings->SaveAndReload(GET_MEMBER_NAME_CHECKED(UHttpGPTSettings, APIKey));
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
	APIKey = NAME_None;

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
	ChatOptions.User = NAME_None;
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

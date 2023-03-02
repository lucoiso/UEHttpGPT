// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "HttpGPTSettings.h"
#include "LogHttpGPT.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(HttpGPTSettings)
#endif

UHttpGPTSettings::UHttpGPTSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), APIKey(FString()), MaxTokens(2048), Temperature(1.f), Choices(1), PresencePenalty(0.f), FrequencyPenalty(0.f), bEnableInternalLogs(false)
{
	CategoryName = TEXT("Plugins");
}

const UHttpGPTSettings* UHttpGPTSettings::Get()
{
	const UHttpGPTSettings* const Instance = GetDefault<UHttpGPTSettings>();
	return Instance;
}

void UHttpGPTSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UHttpGPTSettings, bEnableInternalLogs))
	{
		ToggleInternalLogs();
	}
}

void UHttpGPTSettings::PostInitProperties()
{
	Super::PostInitProperties();
	ToggleInternalLogs();
}

void UHttpGPTSettings::ToggleInternalLogs()
{
#if !UE_BUILD_SHIPPING
	LogHttpGPT_Internal.SetVerbosity(bEnableInternalLogs ? ELogVerbosity::Display : ELogVerbosity::NoLogging);
#endif
}

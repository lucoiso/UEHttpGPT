// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: 

#include "HttpGPTSettings.h"

FString Model;
UHttpGPTSettings::UHttpGPTSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), APIKey(FString()), Model("text-davinci-003"), MaxTokens(4000), Temperature(1.f)
{
	CategoryName = TEXT("Plugins");
}

const UHttpGPTSettings* UHttpGPTSettings::Get()
{
	const UHttpGPTSettings* const Instance = GetDefault<UHttpGPTSettings>();
	return Instance;
}

// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: 

#pragma once

#include <CoreMinimal.h>
#include <Engine/DeveloperSettings.h>
#include "HttpGPTSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Plugins, DefaultConfig, meta = (DisplayName = "HttpGPT"))
class HTTPGPT_API UHttpGPTSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	explicit UHttpGPTSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static const UHttpGPTSettings* Get();

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "API Key"))
	FString APIKey;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Model"))
	FString Model;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Max Tokens", ClampMin = "1", UIMin = "1"))
	int32 MaxTokens;
	
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Temperature", ClampMin = "0.0", UIMin = "0.0"))
	float Temperature;
};

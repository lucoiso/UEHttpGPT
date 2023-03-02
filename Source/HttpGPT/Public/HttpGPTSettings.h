// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

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

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Max Tokens", ClampMin = "1", UIMin = "1", ClampMax = "2048", UIMax = "2048"))
	int32 MaxTokens;
	
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Temperature", ClampMin = "0.0", UIMin = "0.0", ClampMax = "2.0", UIMax = "2.0"))
	float Temperature;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Choices", ClampMin = "1", UIMin = "1"))
	int32 Choices;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Presence Penalty", ClampMin = "-2.0", UIMin = "-2.0", ClampMax = "2.0", UIMax = "2.0"))
	float PresencePenalty;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Frequency Penalty", ClampMin = "-2.0", UIMin = "-2.0", ClampMax = "2.0", UIMax = "2.0"))
	float FrequencyPenalty;

	/* Will print extra internal informations in log */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Logging", Meta = (DisplayName = "Enable Internal Logs"))
	bool bEnableInternalLogs;

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void PostInitProperties() override;

private:
	void ToggleInternalLogs();
};

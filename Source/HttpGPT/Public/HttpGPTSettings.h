// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Engine/DeveloperSettings.h>
#include "HttpGPTTypes.h"
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
	FName APIKey;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Default Http GPT Options"))
	FHttpGPTOptions DefaultOptions;

	/* Will print extra internal informations in log */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Logging", Meta = (DisplayName = "Enable Internal Logs"))
	bool bEnableInternalLogs;

	UFUNCTION(BlueprintPure, Category = "AzSpeech", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Get HttpGPT Settings Data: API Key"))
	static FName GetAPIKey();

	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Set HttpGPT Settings Data: API Key"))
	static void SetAPIKey(const FName Value);

	UFUNCTION(BlueprintPure, Category = "AzSpeech", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Get HttpGPT Settings Data: Default Settings"))
	static FHttpGPTOptions GetDefaultSettings();

	UFUNCTION(BlueprintCallable, Category = "AzSpeech", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Set HttpGPT Settings Data: Default Settings"))
	static void SetDefaultSettings(const FHttpGPTOptions& Value);

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void PostInitProperties() override;

private:
	void ToggleInternalLogs();
};

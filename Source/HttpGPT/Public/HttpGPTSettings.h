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

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Default Options"))
	FHttpGPTOptions DefaultOptions;

	/* Will print extra internal informations in log */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Logging", Meta = (DisplayName = "Enable Internal Logs"))
	bool bEnableInternalLogs;

	UFUNCTION(BlueprintPure, Category = "AzSpeech | Settings", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Get Default Options", CompactNodeTitle = "HttpGPT Default Options"))
	static FHttpGPTOptions GetDefaultOptions();

	UFUNCTION(BlueprintCallable, Category = "AzSpeech | Settings", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Set Default Options"))
	static void SetDefaultOptions(const FHttpGPTOptions& Value);

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void PostInitProperties() override;

	virtual void SetToDefaults();

	void SaveAndReload(const FName& PropertyName);

private:
	void ToggleInternalLogs();
};

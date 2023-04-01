// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Engine/DeveloperSettings.h>
#include "Structures/HttpGPTCommonTypes.h"
#include "Structures/HttpGPTChatTypes.h"
#include "Structures/HttpGPTImageTypes.h"
#include "HttpGPTSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Plugins, DefaultConfig, meta = (DisplayName = "HttpGPT"))
class HTTPGPTCOMMONMODULE_API UHttpGPTSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	explicit UHttpGPTSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static const UHttpGPTSettings* Get();

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Default Options", Meta = (DisplayName = "Common Options"))
	FHttpGPTCommonOptions CommonOptions;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Default Options", Meta = (DisplayName = "Chat Options"))
	FHttpGPTChatOptions ChatOptions;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Default Options", Meta = (DisplayName = "Image Options"))
	FHttpGPTImageOptions ImageOptions;

	/* Enable custom system context in HttpGPT Chat Editor Tool */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Editor | HttpGPT Chat", Meta = (DisplayName = "Use Custom System Context"))
	bool bUseCustomSystemContext;

	/* Custom system context to use in HttpGPT Chat Editor Tool */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Editor | HttpGPT Chat", Meta = (DisplayName = "Custom System Context", EditCondition = "bUseCustomSystemContext"))
	FString CustomSystemContext;

	/* Will print extra internal informations in log */
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "Logging", Meta = (DisplayName = "Enable Internal Logs"))
	bool bEnableInternalLogs;

	UFUNCTION(BlueprintPure, Category = "HttpGPT | Settings", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Get Common Options", CompactNodeTitle = "HttpGPT Common Options"))
	static FHttpGPTCommonOptions GetCommonOptions();

	UFUNCTION(BlueprintCallable, Category = "HttpGPT | Settings", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Set Common Options"))
	static void SetCommonOptions(const FHttpGPTCommonOptions& Value);

	UFUNCTION(BlueprintPure, Category = "HttpGPT | Settings", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Get Chat Options", CompactNodeTitle = "HttpGPT Chat Options"))
	static FHttpGPTChatOptions GetChatOptions();

	UFUNCTION(BlueprintCallable, Category = "HttpGPT | Settings", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Set Chat Options"))
	static void SetChatOptions(const FHttpGPTChatOptions& Value);

	UFUNCTION(BlueprintPure, Category = "HttpGPT | Settings", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Get Image Options", CompactNodeTitle = "HttpGPT Image Options"))
	static FHttpGPTImageOptions GetImageOptions();

	UFUNCTION(BlueprintCallable, Category = "HttpGPT | Settings", meta = (HidePin = "Self", DefaultToSelf = "Self", DisplayName = "Set Image Options"))
	static void SetImageOptions(const FHttpGPTImageOptions& Value);

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

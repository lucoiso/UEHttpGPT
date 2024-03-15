// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include "HttpGPTCommonTypes.generated.h"

USTRUCT(BlueprintType, Category = "HttpGPT | Common", Meta = (DisplayName = "HttpGPT Common Error"))
struct HTTPGPTCOMMONMODULE_API FHttpGPTCommonError
{
	GENERATED_BODY()

	FHttpGPTCommonError() = default;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Common")
	FName Type;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Common")
	FName Code;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Common")
	FString Message;
};

USTRUCT(BlueprintType, Category = "HttpGPT | Common", Meta = (DisplayName = "HttpGPT Common Options"))
struct HTTPGPTCOMMONMODULE_API FHttpGPTCommonOptions
{
	GENERATED_BODY()

	FHttpGPTCommonOptions();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Common", Meta = (DisplayName = "API Key"))
	FName APIKey;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Common", Meta = (DisplayName = "User"))
	FName User;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Common", Meta = (DisplayName = "Is Azure OpenAI"))
	bool bIsAzureOpenAI;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Common", Meta = (DisplayName = "Endpoint", EditCondition = "bIsAzureOpenAI"))
	FString Endpoint;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Common",
		Meta = (DisplayName = "Azure OpenAI API Version", EditCondition = "bIsAzureOpenAI"))
	FString AzureOpenAIAPIVersion;

private:
	void SetDefaults();
};

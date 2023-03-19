// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Dom/JsonValue.h>
#include "HttpGPTTypes.generated.h"

UENUM(BlueprintType, Category = "HttpGPT", Meta = (DisplayName = "HttpGPT Role"))
enum class EHttpGPTRole : uint8
{
	User,
	Assistant,
	System
};

USTRUCT(BlueprintType, Category = "HttpGPT", Meta = (DisplayName = "HttpGPT Message"))
struct HTTPGPT_API FHttpGPTMessage
{
	GENERATED_BODY()

	FHttpGPTMessage() = default;
	FHttpGPTMessage(const EHttpGPTRole& Role, const FString& Content) : Role(Role), Content(Content) {}
	FHttpGPTMessage(const FName& Role, const FString& Content);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	EHttpGPTRole Role = EHttpGPTRole::User;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	FString Content;

	TSharedPtr<FJsonValue> GetMessage() const;
};

USTRUCT(BlueprintType, Category = "HttpGPT", Meta = (DisplayName = "HttpGPT Choice"))
struct HTTPGPT_API FHttpGPTChoice
{
	GENERATED_BODY()

	FHttpGPTChoice() = default;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	int32 Index = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	FHttpGPTMessage Message;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	FName FinishReason = NAME_None;
};

USTRUCT(BlueprintType, Category = "HttpGPT", Meta = (DisplayName = "HttpGPT Usage"))
struct HTTPGPT_API FHttpGPTUsage
{
	GENERATED_BODY()

	FHttpGPTUsage() = default;
	FHttpGPTUsage(const int32& PromptTokens, const int32& CompletionTokens, const int32& TotalTokens) : PromptTokens(PromptTokens), CompletionTokens(CompletionTokens), TotalTokens(TotalTokens) {}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	int32 PromptTokens;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	int32 CompletionTokens;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	int32 TotalTokens;
};

USTRUCT(BlueprintType, Category = "HttpGPT", Meta = (DisplayName = "HttpGPT Error"))
struct HTTPGPT_API FHttpGPTError
{
	GENERATED_BODY()

	FHttpGPTError() = default;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	FName Type;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	FName Code;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	FString Message;
};

USTRUCT(BlueprintType, Category = "HttpGPT", Meta = (DisplayName = "HttpGPT Response"))
struct HTTPGPT_API FHttpGPTResponse
{
	GENERATED_BODY()

	FHttpGPTResponse() = default;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	FName ID = NAME_None;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	FName Object = NAME_None;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	int32 Created = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	TArray<FHttpGPTChoice> Choices;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	FHttpGPTUsage Usage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	bool bSuccess = false;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	FHttpGPTError Error;
};

UENUM(BlueprintType, Category = "HttpGPT", Meta = (DisplayName = "HttpGPT Model"))
enum class EHttpGPTModel : uint8
{
	gpt4			UMETA(DisplayName = "gpt-4"),
	gpt432k			UMETA(DisplayName = "gpt-4-32k"),
	gpt35turbo		UMETA(DisplayName = "gpt-3.5-turbo"),
	textdavinci003	UMETA(DisplayName = "text-davinci-003"),
	textdavinci002	UMETA(DisplayName = "text-davinci-002"),
	codedavinci002	UMETA(DisplayName = "code-davinci-002"),
};

USTRUCT(BlueprintType, Category = "HttpGPT", Meta = (DisplayName = "HttpGPT Options"))
struct HTTPGPT_API FHttpGPTOptions
{
	GENERATED_BODY()

	FHttpGPTOptions();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", Meta = (DisplayName = "API Key"))
	FName APIKey;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT", Meta = (DisplayName = "Model"))
	EHttpGPTModel Model;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT", Meta = (DisplayName = "Temperature", ClampMin = "0.0", UIMin = "0.0", ClampMax = "2.0", UIMax = "2.0"))
	float Temperature;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT", Meta = (DisplayName = "TopP", ClampMin = "0.01", UIMin = "0.01", ClampMax = "1.0", UIMax = "1.0"))
	float TopP;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT", Meta = (DisplayName = "Choices", ClampMin = "1", UIMin = "1"))
	int32 Choices;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT", Meta = (DisplayName = "Stream"))
	bool bStream;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT", Meta = (DisplayName = "Stop"))
	TArray<FName> Stop;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT", Meta = (DisplayName = "Presence Penalty", ClampMin = "-2.0", UIMin = "-2.0", ClampMax = "2.0", UIMax = "2.0"))
	float PresencePenalty;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT", Meta = (DisplayName = "Frequency Penalty", ClampMin = "-2.0", UIMin = "-2.0", ClampMax = "2.0", UIMax = "2.0"))
	float FrequencyPenalty;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT", Meta = (DisplayName = "Max Tokens", ClampMin = "1", UIMin = "1", ClampMax = "32768", UIMax = "32768"))
	int32 MaxTokens;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT", Meta = (DisplayName = "Logit Bias"))
	TArray<float> LogitBias;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT", Meta = (DisplayName = "User", ClampMin = "1", UIMin = "1"))
	FName User;

private:
	void SetDefaults();
};
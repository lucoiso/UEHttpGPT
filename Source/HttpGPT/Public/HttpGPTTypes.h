// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Dom/JsonValue.h>
#include "HttpGPTTypes.generated.h"

USTRUCT(BlueprintType, Category = "HttpGPT", Meta = (DisplayName = "HttpGPT Message"))
struct FHttpGPTMessage
{
	GENERATED_BODY()

	FHttpGPTMessage() = default;
	FHttpGPTMessage(const FName& Role, const FString& Content) : Role(Role), Content(Content) {}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	FName Role;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	FString Content;

	TSharedPtr<FJsonValue> GetMessage() const;
};

USTRUCT(BlueprintType, Category = "HttpGPT", Meta = (DisplayName = "HttpGPT Choice"))
struct FHttpGPTChoice
{
	GENERATED_BODY()

	FHttpGPTChoice() = default;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	int32 Index;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	FHttpGPTMessage Message;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	FName FinishReason;
};

USTRUCT(BlueprintType, Category = "HttpGPT", Meta = (DisplayName = "HttpGPT Usage"))
struct FHttpGPTUsage
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

USTRUCT(BlueprintType, Category = "HttpGPT", Meta = (DisplayName = "HttpGPT Response"))
struct FHttpGPTResponse
{
	GENERATED_BODY()

	FHttpGPTResponse() = default;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	FName ID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	FName Object;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	int32 Created;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	TArray<FHttpGPTChoice> Choices;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	FHttpGPTUsage Usage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT")
	bool bSuccess;
};
// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPTChat

// These structures are defined in the common module due to being used in both modules, to avoid cyclic dependencies.

#pragma once

#include <CoreMinimal.h>
#include <Dom/JsonValue.h>
#include "Structures/HttpGPTCommonTypes.h"
#include "HttpGPTChatTypes.generated.h"

UENUM(BlueprintType, Category = "HttpGPT | Chat", Meta = (DisplayName = "HttpGPT Chat Role"))
enum class EHttpGPTChatRole : uint8
{
    User,
    Assistant,
    System
};

USTRUCT(BlueprintType, Category = "HttpGPT | Chat", Meta = (DisplayName = "HttpGPT Chat Message"))
struct HTTPGPTCOMMONMODULE_API FHttpGPTChatMessage
{
    GENERATED_BODY()

    FHttpGPTChatMessage() = default;
    FHttpGPTChatMessage(const EHttpGPTChatRole& Role, const FString& Content) : Role(Role), Content(Content) {}
    FHttpGPTChatMessage(const FName& Role, const FString& Content);

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat")
    EHttpGPTChatRole Role = EHttpGPTChatRole::User;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat")
    FString Content;

    TSharedPtr<FJsonValue> GetMessage() const;
};

USTRUCT(BlueprintType, Category = "HttpGPT | Chat", Meta = (DisplayName = "HttpGPT Chat Choice"))
struct HTTPGPTCOMMONMODULE_API FHttpGPTChatChoice
{
    GENERATED_BODY()

    FHttpGPTChatChoice() = default;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat")
    int32 Index = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat")
    FHttpGPTChatMessage Message;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat")
    FName FinishReason = NAME_None;
};

USTRUCT(BlueprintType, Category = "HttpGPT | Chat", Meta = (DisplayName = "HttpGPT Chat Usage"))
struct HTTPGPTCOMMONMODULE_API FHttpGPTChatUsage
{
    GENERATED_BODY()

    FHttpGPTChatUsage() = default;
    FHttpGPTChatUsage(const int32& PromptTokens, const int32& CompletionTokens, const int32& TotalTokens) : PromptTokens(PromptTokens), CompletionTokens(CompletionTokens), TotalTokens(TotalTokens) {}

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat")
    int32 PromptTokens = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat")
    int32 CompletionTokens = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat")
    int32 TotalTokens = 0;
};

USTRUCT(BlueprintType, Category = "HttpGPT | Chat", Meta = (DisplayName = "HttpGPT Chat Response"))
struct HTTPGPTCOMMONMODULE_API FHttpGPTChatResponse
{
    GENERATED_BODY()

    FHttpGPTChatResponse() = default;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat")
    FName ID = NAME_None;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat")
    FName Object = NAME_None;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat")
    int32 Created = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat")
    TArray<FHttpGPTChatChoice> Choices;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat")
    FHttpGPTChatUsage Usage;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat")
    bool bSuccess = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat")
    FHttpGPTCommonError Error;
};

UENUM(BlueprintType, Category = "HttpGPT | Chat", Meta = (DisplayName = "HttpGPT Chat Model"))
enum class EHttpGPTChatModel : uint8
{
    gpt4			UMETA(DisplayName = "gpt-4"),
    gpt432k			UMETA(DisplayName = "gpt-4-32k"),
    gpt35turbo		UMETA(DisplayName = "gpt-3.5-turbo"),
    textdavinci003	UMETA(DisplayName = "text-davinci-003"),
    textdavinci002	UMETA(DisplayName = "text-davinci-002"),
    codedavinci002	UMETA(DisplayName = "code-davinci-002"),
};

USTRUCT(BlueprintType, Category = "HttpGPT | Chat", Meta = (DisplayName = "HttpGPT Chat Options"))
struct HTTPGPTCOMMONMODULE_API FHttpGPTChatOptions
{
    GENERATED_BODY()

    FHttpGPTChatOptions();

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat", Meta = (DisplayName = "Model"))
    EHttpGPTChatModel Model;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat", Meta = (DisplayName = "Temperature", ClampMin = "0.0", UIMin = "0.0", ClampMax = "2.0", UIMax = "2.0"))
    float Temperature;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat", Meta = (DisplayName = "TopP", ClampMin = "0.01", UIMin = "0.01", ClampMax = "1.0", UIMax = "1.0"))
    float TopP;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat", Meta = (DisplayName = "Choices", ClampMin = "1", UIMin = "1"))
    int32 Choices;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat", Meta = (DisplayName = "Stream"))
    bool bStream;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat", Meta = (DisplayName = "Stop"))
    TArray<FName> Stop;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat", Meta = (DisplayName = "Presence Penalty", ClampMin = "-2.0", UIMin = "-2.0", ClampMax = "2.0", UIMax = "2.0"))
    float PresencePenalty;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat", Meta = (DisplayName = "Frequency Penalty", ClampMin = "-2.0", UIMin = "-2.0", ClampMax = "2.0", UIMax = "2.0"))
    float FrequencyPenalty;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat", Meta = (DisplayName = "Max Tokens", ClampMin = "1", UIMin = "1", ClampMax = "32768", UIMax = "32768"))
    int32 MaxTokens;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Chat", Meta = (DisplayName = "Logit Bias"))
    TMap<int32, float> LogitBias;

private:
    void SetDefaults();
};
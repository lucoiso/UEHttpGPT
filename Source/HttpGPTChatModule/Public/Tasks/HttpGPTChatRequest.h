// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Tasks/HttpGPTBaseTask.h>
#include <Structures/HttpGPTCommonTypes.h>
#include <Structures/HttpGPTChatTypes.h>
#include <Kismet/BlueprintFunctionLibrary.h>
#include "HttpGPTChatRequest.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHttpGPTChatResponseDelegate, const FHttpGPTChatResponse&, Response);

/**
 * 
 */
UCLASS(NotPlaceable, Category = "HttpGPT | Chat", meta = (ExposedAsyncProxy = AsyncTask))
class HTTPGPTCHATMODULE_API UHttpGPTChatRequest : public UHttpGPTBaseTask
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "HttpGPT | Chat")
	FHttpGPTChatResponseDelegate ProcessCompleted;

	UPROPERTY(BlueprintAssignable, Category = "HttpGPT | Chat")
	FHttpGPTChatResponseDelegate ProgressUpdated;

	UPROPERTY(BlueprintAssignable, Category = "HttpGPT | Chat")
	FHttpGPTChatResponseDelegate ProgressStarted;

	UPROPERTY(BlueprintAssignable, Category = "HttpGPT | Chat")
	FHttpGPTChatResponseDelegate ErrorReceived;
	
#if WITH_EDITOR
	static UHttpGPTChatRequest* EditorTask(const TArray<FHttpGPTChatMessage>& Messages, const FHttpGPTChatOptions Options);
#endif

	UFUNCTION(BlueprintCallable, Category = "HttpGPT | Chat | Default", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Send Message with Default Options"))
	static UHttpGPTChatRequest* SendMessage_DefaultOptions(UObject* WorldContextObject, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "HttpGPT | Chat | Default", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Send Messages with Default Options"))
	static UHttpGPTChatRequest* SendMessages_DefaultOptions(UObject* WorldContextObject, const TArray<FHttpGPTChatMessage>& Messages);

	UFUNCTION(BlueprintCallable, Category = "HttpGPT | Chat | Custom", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Send Message with Custom Options"))
	static UHttpGPTChatRequest* SendMessage_CustomOptions(UObject* WorldContextObject, const FString& Message, const FHttpGPTCommonOptions CommonOptions, const FHttpGPTChatOptions ChatOptions);

	UFUNCTION(BlueprintCallable, Category = "HttpGPT | Chat | Custom", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Send Messages with Custom Options"))
	static UHttpGPTChatRequest* SendMessages_CustomOptions(UObject* WorldContextObject, const TArray<FHttpGPTChatMessage>& Messages, const FHttpGPTCommonOptions CommonOptions, const FHttpGPTChatOptions ChatOptions);

	UFUNCTION(BlueprintPure, Category = "HttpGPT | Chat")
	const FHttpGPTChatOptions GetChatOptions() const;

protected:
	TArray<FHttpGPTChatMessage> Messages;
	FHttpGPTChatOptions ChatOptions;

	virtual bool CanActivateTask() const override;
	virtual bool CanBindProgress() const override;
	virtual FString GetEndpointURL() const override;

	virtual FString SetRequestContent() override;
	virtual void OnProgressUpdated(const FString& Content, int32 BytesSent, int32 BytesReceived) override;
	virtual void OnProgressCompleted(const FString& Content, const bool bWasSuccessful) override;

	TArray<FString> GetDeltasFromContent(const FString& Content) const;

	void DeserializeStreamedResponse(const TArray<FString>& Deltas);
	void DeserializeSingleResponse(const FString& Content);

private:
	FHttpGPTChatResponse Response;
};

UCLASS(NotPlaceable, Category = "HttpGPT | Chat", Meta = (DisplayName = "HttpGPT Chat Helper"))
class HTTPGPTCHATMODULE_API UHttpGPTChatHelper final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "HttpGPT | Chat", Meta = (DisplayName = "Cast to HttpGPT Chat Request"))
	static UHttpGPTChatRequest* CastToHttpGPTChatRequest(UObject* Object);
};

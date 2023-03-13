// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintAsyncActionBase.h>
#include "HttpGPTTypes.h"
#include "HttpGPTRequest.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHttpGPTGenericDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHttpGPTResponseDelegate, const FHttpGPTResponse&, Response);

/**
 * 
 */
UCLASS(NotPlaceable, Category = "HttpGPT")
class HTTPGPT_API UHttpGPTRequest : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "HttpGPT")
	FHttpGPTResponseDelegate ResponseReceived;

	UPROPERTY(BlueprintAssignable, Category = "HttpGPT")
	FHttpGPTResponseDelegate RequestFailed;

	UPROPERTY(BlueprintAssignable, Category = "HttpGPT")
	FHttpGPTGenericDelegate RequestSent;

	UPROPERTY(BlueprintAssignable, Category = "HttpGPT")
	FHttpGPTGenericDelegate RequestNotSent;
	
	UFUNCTION(BlueprintCallable, Category = "HttpGPT", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Send Message to Model"))
	static UHttpGPTRequest* SendMessageToModel(UObject* WorldContextObject, const FString& Message, const FString& Model);

	UFUNCTION(BlueprintCallable, Category = "HttpGPT", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Send Messages to Model"))
	static UHttpGPTRequest* SendMessagesToModel(UObject* WorldContextObject, const TArray<FHttpGPTMessage>& Messages, const FString& Model);

	UFUNCTION(BlueprintCallable, Category = "HttpGPT", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Send Message to GPT"))
	static UHttpGPTRequest* SendMessageToGPT(UObject* WorldContextObject, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "HttpGPT", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Send Messages to GPT"))
	static UHttpGPTRequest* SendMessagesToGPT(UObject* WorldContextObject, const TArray<FHttpGPTMessage>& Messages);

	UFUNCTION(BlueprintCallable, Category = "HttpGPT", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Send Message to Default Model"))
	static UHttpGPTRequest* SendMessageToDefaultModel(UObject* WorldContextObject, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "HttpGPT", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Send Messages to Default Model"))
	static UHttpGPTRequest* SendMessagesToDefaultModel(UObject* WorldContextObject, const TArray<FHttpGPTMessage>& Messages);

	virtual void Activate() override;

protected:
	TArray<FHttpGPTMessage> Messages;
	FName Model;

	mutable FCriticalSection Mutex;

	virtual void ProcessResponse(const FString& Content, const bool bWasSuccessful);
	FHttpGPTResponse GetDesserializedResponse(const FString& Content) const;
};

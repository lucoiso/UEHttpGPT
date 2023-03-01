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
UCLASS(NotPlaceable)
class HTTPGPT_API UHttpGPTRequest : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FHttpGPTResponseDelegate ResponseReceived;

	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FHttpGPTGenericDelegate RequestSent;

	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FHttpGPTGenericDelegate RequestNotSent;

	UPROPERTY(BlueprintAssignable, Category = "AzSpeech")
	FHttpGPTGenericDelegate RequestFailed;
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Send Message to GPT"))
	static UHttpGPTRequest* SendMessageToGPT(UObject* WorldContextObject, const TArray<FHttpGPTMessage>& Messages);

	virtual void Activate() override;

protected:
	TArray<FHttpGPTMessage> Messages;
	mutable FCriticalSection Mutex;

	virtual void ProcessResponse(const FString& Content, const bool bWasSuccessful);
	FHttpGPTResponse GetDesserializedResponse(const FString& Content) const;
};

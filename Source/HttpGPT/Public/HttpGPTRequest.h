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
UCLASS(NotPlaceable, Category = "HttpGPT", meta = (ExposedAsyncProxy = AsyncTask))
class HTTPGPT_API UHttpGPTRequest : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "HttpGPT")
	FHttpGPTResponseDelegate ProcessCompleted;

	UPROPERTY(BlueprintAssignable, Category = "HttpGPT")
	FHttpGPTResponseDelegate ProgressUpdated;

	UPROPERTY(BlueprintAssignable, Category = "HttpGPT")
	FHttpGPTResponseDelegate ProgressStarted;

	UPROPERTY(BlueprintAssignable, Category = "HttpGPT")
	FHttpGPTResponseDelegate ErrorReceived;

	UPROPERTY(BlueprintAssignable, Category = "HttpGPT")
	FHttpGPTGenericDelegate RequestFailed;

	UPROPERTY(BlueprintAssignable, Category = "HttpGPT")
	FHttpGPTGenericDelegate RequestSent;
		
	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const FHttpGPTOptions GetTaskOptions() const;
	
	UFUNCTION(BlueprintCallable, Category = "HttpGPT", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Send Message"))
	static UHttpGPTRequest* SendMessage(UObject* WorldContextObject, const FString& Message, const FHttpGPTOptions& Options);

	UFUNCTION(BlueprintCallable, Category = "HttpGPT", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Send Messages"))
	static UHttpGPTRequest* SendMessages(UObject* WorldContextObject, const TArray<FHttpGPTMessage>& Messages, const FHttpGPTOptions& Options);

	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;

protected:
	TArray<FHttpGPTMessage> Messages;
	FHttpGPTOptions TaskOptions;

	mutable FCriticalSection Mutex;

	void SendRequest();

	void OnProgressUpdated(const FString& Content, int32 BytesSent, int32 BytesReceived);
	void OnProgressCompleted(const FString& Content, const bool bWasSuccessful);

	void DeserializeResponse(const FString& Content);

private:
	FHttpGPTResponse Response;
	bool bInitialized = false;
};

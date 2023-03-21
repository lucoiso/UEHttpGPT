// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintAsyncActionBase.h>
#include <Interfaces/IHttpRequest.h>
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
	
	UFUNCTION(BlueprintCallable, Category = "HttpGPT | Default", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Send Message with Default Options"))
	static UHttpGPTRequest* SendMessage_DefaultOptions(UObject* WorldContextObject, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "HttpGPT | Default", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Send Messages with Default Options"))
	static UHttpGPTRequest* SendMessages_DefaultOptions(UObject* WorldContextObject, const TArray<FHttpGPTMessage>& Messages);

	UFUNCTION(BlueprintCallable, Category = "HttpGPT | Custom", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Send Message with Custom Options"))
	static UHttpGPTRequest* SendMessage_CustomOptions(UObject* WorldContextObject, const FString& Message, const FHttpGPTOptions& Options);

	UFUNCTION(BlueprintCallable, Category = "HttpGPT | Custom", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Send Messages with Custom Options"))
	static UHttpGPTRequest* SendMessages_CustomOptions(UObject* WorldContextObject, const TArray<FHttpGPTMessage>& Messages, const FHttpGPTOptions& Options);

	UFUNCTION(BlueprintCallable, Category = "HttpGPT", meta = (DisplayName = "Stop HttpGPT Task"))
	void StopHttpGPTTask();

	const bool IsTaskActive() const;

	UFUNCTION(BlueprintPure, Category = "AzSpeech")
	const FHttpGPTOptions GetTaskOptions() const;

	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;

protected:
	TArray<FHttpGPTMessage> Messages;
	FHttpGPTOptions TaskOptions;

	mutable FCriticalSection Mutex;

	void SendRequest();
	void InitializeRequest();
	void SetRequestContent();
	void BindRequestCallbacks();

	void OnProgressUpdated(const FString& Content, int32 BytesSent, int32 BytesReceived);
	void OnProgressCompleted(const FString& Content, const bool bWasSuccessful);

	TArray<FString> GetDeltasFromContent(const FString& Content) const;

	void DeserializeStreamedResponse(const TArray<FString>& Deltas);
	void DeserializeSingleResponse(const FString& Content);

private:
	TSharedPtr<IHttpRequest> HttpRequest;
	FHttpGPTResponse Response;

	bool bInitialized = false;
	bool bIsReadyToDestroy = false;
	bool bIsActive = false;

#if WITH_EDITOR
	virtual void PrePIEEnded(bool bIsSimulating);

	bool bEndingPIE = false;
#endif
};

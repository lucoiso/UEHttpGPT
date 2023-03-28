// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Interfaces/IHttpRequest.h>
#include <Kismet/BlueprintAsyncActionBase.h>
#include <Kismet/BlueprintFunctionLibrary.h>
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

	friend class UHttpGPTTaskStatus;

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

	UFUNCTION(BlueprintPure, Category = "HttpGPT")
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
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest;
	FHttpGPTResponse Response;

	bool bInitialized = false;
	bool bIsReadyToDestroy = false;
	bool bIsTaskActive = false;

#if WITH_EDITOR
	virtual void PrePIEEnded(bool bIsSimulating);

	bool bEndingPIE = false;
#endif
};

UCLASS(NotPlaceable, Category = "HttpGPT")
class HTTPGPT_API UHttpGPTTaskStatus final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "HttpGPT")
	static bool IsTaskActive(const UHttpGPTRequest* Test);

	UFUNCTION(BlueprintPure, Category = "HttpGPT")
	static bool IsTaskReadyToDestroy(const UHttpGPTRequest* Test);

	UFUNCTION(BlueprintPure, Category = "HttpGPT")
	static bool IsTaskStillValid(const UHttpGPTRequest* Test);
};
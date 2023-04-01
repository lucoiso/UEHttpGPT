// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Interfaces/IHttpRequest.h>
#include <Kismet/BlueprintAsyncActionBase.h>
#include <Kismet/BlueprintFunctionLibrary.h>
#include "Structures/HttpGPTCommonTypes.h"
#include "HttpGPTBaseTask.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHttpGPTGenericDelegate);

/**
 * 
 */
UCLASS(NotPlaceable, Category = "HttpGPT", meta = (ExposedAsyncProxy = AsyncTask))
class HTTPGPTCOMMONMODULE_API UHttpGPTBaseTask : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	friend class UHttpGPTTaskStatus;

public:
	UPROPERTY(BlueprintAssignable, Category = "HttpGPT")
	FHttpGPTGenericDelegate RequestFailed;

	UPROPERTY(BlueprintAssignable, Category = "HttpGPT")
	FHttpGPTGenericDelegate RequestSent;

	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, Category = "HttpGPT", meta = (DisplayName = "Stop HttpGPT Task"))
	void StopHttpGPTTask();

	virtual void SetReadyToDestroy() override;

	UFUNCTION(BlueprintPure, Category = "HttpGPT", Meta = (DisplayName = "Get API Key"))
	const FHttpGPTCommonOptions GetCommonOptions() const;

protected:
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> HttpRequest;
	FHttpGPTCommonOptions CommonOptions;

	mutable FCriticalSection Mutex;

	virtual bool CanActivateTask() const;
	virtual bool CanBindProgress() const;
	virtual FString GetEndpointURL() const;

	void SendRequest();

	/* Return true if contains error */
	const bool CheckError(const TSharedPtr<class FJsonObject>& JsonObject, FHttpGPTCommonError& OutputError) const;

	void InitializeRequest();
	void BindRequestCallbacks();

	virtual void SetRequestContent() {};
	virtual void OnProgressUpdated(const FString& Content, int32 BytesSent, int32 BytesReceived) {};
	virtual void OnProgressCompleted(const FString& Content, const bool bWasSuccessful) {};

	bool bInitialized = false;
	bool bIsReadyToDestroy = false;
	bool bIsTaskActive = false;

#if WITH_EDITOR
	bool bIsEditorTask = false;
	bool bEndingPIE = false;

	virtual void PrePIEEnded(bool bIsSimulating);
#endif
};

UCLASS(NotPlaceable, Category = "HttpGPT")
class HTTPGPTCOMMONMODULE_API UHttpGPTTaskStatus final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "HttpGPT")
	static bool IsTaskActive(const UHttpGPTBaseTask* Test);

	UFUNCTION(BlueprintPure, Category = "HttpGPT")
	static bool IsTaskReadyToDestroy(const UHttpGPTBaseTask* Test);

	UFUNCTION(BlueprintPure, Category = "HttpGPT")
	static bool IsTaskStillValid(const UHttpGPTBaseTask* Test);
};

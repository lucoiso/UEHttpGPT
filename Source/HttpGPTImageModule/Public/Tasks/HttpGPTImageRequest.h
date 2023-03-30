// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Tasks/HttpGPTBaseTask.h>
#include <Structures/HttpGPTCommonTypes.h>
#include <Structures/HttpGPTImageTypes.h>
#include <Kismet/BlueprintFunctionLibrary.h>
#include "HttpGPTImageRequest.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHttpGPTImageResponseDelegate, const FHttpGPTImageResponse&, Response);

/**
 * 
 */
UCLASS(NotPlaceable, Category = "HttpGPT | Image", meta = (ExposedAsyncProxy = AsyncTask))
class HTTPGPTIMAGEMODULE_API UHttpGPTImageRequest : public UHttpGPTBaseTask
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "HttpGPT | Image")
	FHttpGPTImageResponseDelegate ProcessCompleted;

	UPROPERTY(BlueprintAssignable, Category = "HttpGPT | Image")
	FHttpGPTImageResponseDelegate ProgressUpdated;

	UPROPERTY(BlueprintAssignable, Category = "HttpGPT | Image")
	FHttpGPTImageResponseDelegate ProgressStarted;

	UPROPERTY(BlueprintAssignable, Category = "HttpGPT | Image")
	FHttpGPTImageResponseDelegate ErrorReceived;
	
#if WITH_EDITOR
	static UHttpGPTImageRequest* EditorTask(const FString& Prompt, const FHttpGPTImageOptions Options);
#endif

	UFUNCTION(BlueprintCallable, Category = "HttpGPT | Image | Default", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Request Images with Default Options"))
	static UHttpGPTImageRequest* RequestImages_DefaultOptions(UObject* WorldContextObject, const FString& Prompt);

	UFUNCTION(BlueprintCallable, Category = "HttpGPT | Image | Custom", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Request Images with Custom Options"))
	static UHttpGPTImageRequest* RequestImages_CustomOptions(UObject* WorldContextObject, const FString& Prompt, const FHttpGPTCommonOptions CommonOptions, const FHttpGPTImageOptions ImageOptions);

	UFUNCTION(BlueprintPure, Category = "HttpGPT | Image")
	const FHttpGPTImageOptions GetImageOptions() const;
	
	UFUNCTION(BlueprintPure, Category = "HttpGPT | Image")
	const FString GetPrompt() const;

protected:
	FString Prompt;
	FHttpGPTImageOptions ImageOptions;

	virtual bool CanActivateTask() const override;
	virtual bool CanBindProgress() const override;
	virtual FString GetEndpointURL() const override;

	virtual void SetRequestContent() override;
	virtual void OnProgressCompleted(const FString& Content, const bool bWasSuccessful) override;

	void DeserializeResponse(const FString& Content);

private:
	FHttpGPTImageResponse Response;
};

UCLASS(NotPlaceable, Category = "HttpGPT | Image")
class HTTPGPTIMAGEMODULE_API UHttpGPTImageHelper final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "HttpGPT | Image", Meta = (DisplayName = "Cast to HttpGPT Image Request"))
	static UHttpGPTImageRequest* CastToHTTPGPTImageRequest(UObject* Object);
};

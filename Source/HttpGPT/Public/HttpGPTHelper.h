// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>
#include "HttpGPTRequest.h"
#include "HttpGPTTypes.h"
#include "HttpGPTHelper.generated.h"

/**
 * 
 */
UCLASS(NotPlaceable, Category = "HttpGPT")
class HTTPGPT_API UHttpGPTHelper final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "HttpGPT", meta = (DisplayName = "Cast to HttpGPT Request Async Task"))
	static UHttpGPTRequest* CastToHttpGPTRequest(UObject* Object);

	UFUNCTION(BlueprintPure, Category = "HttpGPT", meta = (DisplayName = "Convert HttpGPT Model to Name"))
	static const FName ModelToName(const EHttpGPTModel& Model);

	UFUNCTION(BlueprintPure, Category = "HttpGPT", meta = (DisplayName = "Convert Name to HttpGPT Model"))
	static const EHttpGPTModel NameToModel(const FName Model);

	UFUNCTION(BlueprintPure, Category = "HttpGPT", meta = (DisplayName = "Get Available GPT Models"))
	static const TArray<FName> GetAvailableGPTModels();
		
	UFUNCTION(BlueprintPure, Category = "HttpGPT", meta = (DisplayName = "Get Endpoint for Model"))
	static const FName GetEndpointForModel(const EHttpGPTModel& Model);

	UFUNCTION(BlueprintPure, Category = "HttpGPT", meta = (DisplayName = "Get Endpoint for Model"))
	static const bool ModelSupportsChat(const EHttpGPTModel& Model);
};

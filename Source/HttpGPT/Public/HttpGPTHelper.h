// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>
#include "HttpGPTRequest.h"
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
};

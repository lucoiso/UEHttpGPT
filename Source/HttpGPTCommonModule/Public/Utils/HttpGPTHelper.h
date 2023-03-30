// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>
#include "Structures/HttpGPTChatTypes.h"
#include "HttpGPTHelper.generated.h"

/**
 * 
 */
UCLASS(NotPlaceable, Category = "HttpGPT")
class HTTPGPTCOMMONMODULE_API UHttpGPTHelper final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "HttpGPT | Chat", meta = (DisplayName = "Convert HttpGPT Model to Name"))
	static const FName ModelToName(const EHttpGPTChatModel& Model);

	UFUNCTION(BlueprintPure, Category = "HttpGPT | Chat", meta = (DisplayName = "Convert Name to HttpGPT Model"))
	static const EHttpGPTChatModel NameToModel(const FName Model);

	UFUNCTION(BlueprintPure, Category = "HttpGPT | Chat", meta = (DisplayName = "Convert HttpGPT Role to Name"))
	static const FName RoleToName(const EHttpGPTChatRole& Role);

	UFUNCTION(BlueprintPure, Category = "HttpGPT | Chat", meta = (DisplayName = "Convert Name to HttpGPT Role"))
	static const EHttpGPTChatRole NameToRole(const FName Role);

	UFUNCTION(BlueprintPure, Category = "HttpGPT | Chat", meta = (DisplayName = "Get Available GPT Models"))
	static const TArray<FName> GetAvailableGPTModels();
		
	UFUNCTION(BlueprintPure, Category = "HttpGPT | Chat", meta = (DisplayName = "Get Endpoint for Model"))
	static const FName GetEndpointForModel(const EHttpGPTChatModel& Model);

	UFUNCTION(BlueprintPure, Category = "HttpGPT | Chat", meta = (DisplayName = "Model Supports Chat"))
	static const bool ModelSupportsChat(const EHttpGPTChatModel& Model);
};

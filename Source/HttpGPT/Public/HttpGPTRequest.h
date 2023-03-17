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
	
	UFUNCTION(BlueprintCallable, Category = "HttpGPT", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Send Message", AutoCreateRefTerm = "Options"))
	static UHttpGPTRequest* SendMessage(UObject* WorldContextObject, const FString& Message, const FHttpGPTOptions& Options = FHttpGPTOptions());

	UFUNCTION(BlueprintCallable, Category = "HttpGPT", meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Send Messages", AutoCreateRefTerm = "Options"))
	static UHttpGPTRequest* SendMessages(UObject* WorldContextObject, const TArray<FHttpGPTMessage>& Messages, const FHttpGPTOptions& Options = FHttpGPTOptions());

	virtual void Activate() override;

protected:
	TArray<FHttpGPTMessage> Messages;
	FHttpGPTOptions Options;

	mutable FCriticalSection Mutex;

	virtual void ProcessResponse(const FString& Content, const bool bWasSuccessful);
	FHttpGPTResponse GetDesserializedResponse(const FString& Content) const;

private:
	static inline FName ModelToName(const EHttpGPTModel& Model)
	{
		switch (Model)
		{
			case EHttpGPTModel::gpt4:
				return "gpt-4";

			case EHttpGPTModel::gpt432k:
				return "gpt-4-32k";

			case EHttpGPTModel::gpt35turbo:
				return "gpt-3.5-turbo";

			case EHttpGPTModel::textdavinci003:
				return "text-davinci-003";

			case EHttpGPTModel::textdavinci002:
				return "text-davinci-002";

			case EHttpGPTModel::codedavinci002:
				return "code-davinci-002";

			default: break;
		}

		return NAME_None;
	}
};

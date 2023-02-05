// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "HttpGPTRequest.h"
#include "HttpGPTSettings.h"
#include "LogHttpGPT.h"
#include <HttpModule.h>
#include <Interfaces/IHttpRequest.h>
#include <Interfaces/IHttpResponse.h>

UHttpGPTRequest* UHttpGPTRequest::SendGPTMessageAsync(UObject* WorldContextObject, const FString& Message)
{
	UHttpGPTRequest* const Task = NewObject<UHttpGPTRequest>();
	Task->Message = Message;
	Task->RegisterWithGameInstance(WorldContextObject);

	return Task;
}

void UHttpGPTRequest::Activate()
{
	const UHttpGPTSettings* const Settings = UHttpGPTSettings::Get();

	if (Message.IsEmpty() || !IsValid(Settings) || Settings->APIKey.IsEmpty())
	{
		UE_LOG(LogHttpGPT, Error, TEXT("%s (%d): Request not sent due to invalid params"), *FString(__func__), GetUniqueID());
		
		SetReadyToDestroy();
		return;
	}

	TSharedRef<class IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL("https://api.openai.com/v1/completions");
	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetHeader("Authorization", FString::Format(TEXT("Bearer {0}"), { Settings->APIKey }));

	const TSharedPtr<FJsonObject> JsonRequest = MakeShareable(new FJsonObject);
	JsonRequest->SetStringField("model", Settings->Model.ToLower());
	JsonRequest->SetStringField("prompt", Message);
	JsonRequest->SetNumberField("max_tokens", Settings->MaxTokens);
	JsonRequest->SetNumberField("temperature", Settings->Temperature);

	FString RequestContentString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestContentString);
	FJsonSerializer::Serialize(JsonRequest.ToSharedRef(), Writer);

	HttpRequest->SetContentAsString(RequestContentString);

	const auto ProccessComplete_Lambda = [this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
		FScopeLock Lock(&Mutex);
		
		if (IsValid(this))
		{
			ProcessResponse(Response->GetContentAsString(), bWasSuccessful);
		}
	};

	HttpRequest->OnProcessRequestComplete().BindLambda(ProccessComplete_Lambda);
	HttpRequest->ProcessRequest();
	
	RequestSent.Broadcast();
}

void UHttpGPTRequest::ProcessResponse(const FString& Content, const bool bWasSuccessful)
{
	FScopeLock Lock(&Mutex);

	if (!bWasSuccessful)
	{
		UE_LOG(LogHttpGPT, Error, TEXT("%s (%d): Request failed"), *FString(__func__), GetUniqueID());
		return;
	}

	UE_LOG(LogHttpGPT, Display, TEXT("%s (%d): Response: %s"), *FString(__func__), GetUniqueID(), *Content);

	ResponseReceived.Broadcast(GetDesserializedResponseString(Content));
	SetReadyToDestroy();
}

FString UHttpGPTRequest::GetDesserializedResponseString(const FString& Content) const
{
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
	TSharedPtr<FJsonObject> JsonResponse = MakeShareable(new FJsonObject);
	FJsonSerializer::Deserialize(Reader, JsonResponse);

	if (const TArray<TSharedPtr<FJsonValue>>* ChoicesArr; JsonResponse->TryGetArrayField("choices", ChoicesArr))
	{
		for (auto Iterator = ChoicesArr->begin(); ChoicesArr; ++ChoicesArr)
		{
			if (const TSharedPtr<FJsonObject>* ChoiceObj; !(*Iterator)->TryGetObject(ChoiceObj))
			{
				continue;
			}
			else if (FString Text; (*ChoiceObj)->TryGetStringField("text", Text))
			{
				return *Text;
			}
		}
	}

	return "An error has occurred, check the logs";
}

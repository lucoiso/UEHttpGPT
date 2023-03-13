// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "HttpGPTRequest.h"
#include "HttpGPTSettings.h"
#include "LogHttpGPT.h"
#include <HttpModule.h>
#include <Interfaces/IHttpRequest.h>
#include <Interfaces/IHttpResponse.h>
#include <Dom/JsonObject.h>
#include <Serialization/JsonWriter.h>
#include <Serialization/JsonReader.h>
#include <Serialization/JsonSerializer.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(HttpGPTRequest)
#endif

UHttpGPTRequest* UHttpGPTRequest::SendMessageToModel(UObject* WorldContextObject, const FString& Message, const FString& Model)
{
	UHttpGPTRequest* const Task = NewObject<UHttpGPTRequest>();
	Task->Messages = { FHttpGPTMessage(EHttpGPTRole::User, Message) };
	Task->Model = *Model;

	Task->RegisterWithGameInstance(WorldContextObject);

	return Task;
}

UHttpGPTRequest* UHttpGPTRequest::SendMessageToGPT(UObject* WorldContextObject, const FString& Message)
{
	return SendMessageToModel(WorldContextObject, Message, "gpt-3.5-turbo");
}

UHttpGPTRequest* UHttpGPTRequest::SendMessageToDefaultModel(UObject* WorldContextObject, const FString& Message)
{
	return SendMessageToModel(WorldContextObject, Message, UHttpGPTSettings::Get()->DefaultModel);
}

UHttpGPTRequest* UHttpGPTRequest::SendMessagesToModel(UObject* WorldContextObject, const TArray<FHttpGPTMessage>& Messages, const FString& Model)
{
	UHttpGPTRequest* const Task = NewObject<UHttpGPTRequest>();
	Task->Messages = Messages;
	Task->Model = *Model;

	Task->RegisterWithGameInstance(WorldContextObject);

	return Task;
}

UHttpGPTRequest* UHttpGPTRequest::SendMessagesToGPT(UObject* WorldContextObject, const TArray<FHttpGPTMessage>& Messages)
{
	return SendMessagesToModel(WorldContextObject, Messages, "gpt-3.5-turbo");
}

UHttpGPTRequest* UHttpGPTRequest::SendMessagesToDefaultModel(UObject* WorldContextObject, const TArray<FHttpGPTMessage>& Messages)
{
	return SendMessagesToModel(WorldContextObject, Messages, UHttpGPTSettings::Get()->DefaultModel);
}

void UHttpGPTRequest::Activate()
{
	const UHttpGPTSettings* const Settings = UHttpGPTSettings::Get();

	if (Messages.IsEmpty() || !IsValid(Settings) || Settings->APIKey.IsEmpty())
	{
		UE_LOG(LogHttpGPT, Error, TEXT("%s (%d): Request not sent due to invalid params"), *FString(__func__), GetUniqueID());
		RequestNotSent.Broadcast();
		SetReadyToDestroy();
		return;
	}

	TSharedRef<class IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL("https://api.openai.com/v1/chat/completions");
	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetHeader("Authorization", FString::Format(TEXT("Bearer {0}"), { Settings->APIKey }));

	const TSharedPtr<FJsonObject> JsonRequest = MakeShareable(new FJsonObject);
	JsonRequest->SetStringField("model", Model.ToString().ToLower());
	JsonRequest->SetNumberField("max_tokens", Settings->MaxTokens);
	JsonRequest->SetNumberField("temperature", Settings->Temperature);
	JsonRequest->SetNumberField("n", Settings->Choices);
	JsonRequest->SetNumberField("presence_penalty", Settings->PresencePenalty);
	JsonRequest->SetNumberField("frequency_penalty", Settings->FrequencyPenalty);

	TArray<TSharedPtr<FJsonValue>> MessagesJson;
	for (const FHttpGPTMessage& Section : Messages)
	{
		MessagesJson.Add(Section.GetMessage());
	}

	JsonRequest->SetArrayField("messages", MessagesJson);

	FString RequestContentString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestContentString);
	FJsonSerializer::Serialize(JsonRequest.ToSharedRef(), Writer);

	HttpRequest->SetContentAsString(RequestContentString);

	const auto ProccessComplete_Lambda = [this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) 
	{
		FScopeLock Lock(&Mutex);
		
		if (IsValid(this))
		{
			ProcessResponse(Response->GetContentAsString(), bWasSuccessful);
		}
	};

	HttpRequest->OnProcessRequestComplete().BindLambda(ProccessComplete_Lambda);
	HttpRequest->ProcessRequest();

	UE_LOG(LogHttpGPT, Display, TEXT("%s (%d): Request sent"), *FString(__func__), GetUniqueID());
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

	UE_LOG(LogHttpGPT, Display, TEXT("%s (%d): Response received"), *FString(__func__), GetUniqueID());
	UE_LOG(LogHttpGPT_Internal, Display, TEXT("%s (%d): Response: %s"), *FString(__func__), GetUniqueID(), *Content);

	const FHttpGPTResponse Response = GetDesserializedResponse(Content);

	if (!Response.bSuccess)
	{
		UE_LOG(LogHttpGPT, Error, TEXT("%s (%d): Request failed"), *FString(__func__), GetUniqueID());
	}

	RequestFailed.Broadcast(Response);

	SetReadyToDestroy();
}

FHttpGPTResponse UHttpGPTRequest::GetDesserializedResponse(const FString& Content) const
{
	FHttpGPTResponse Output;

	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
	TSharedPtr<FJsonObject> JsonResponse = MakeShareable(new FJsonObject);
	FJsonSerializer::Deserialize(Reader, JsonResponse);

	if (!JsonResponse.IsValid())
	{
		return Output;
	}

	if (JsonResponse->HasField("error"))
	{
		Output.bSuccess = false;		

		const TSharedPtr<FJsonObject> ErrorObj = JsonResponse->GetObjectField("error");

		FHttpGPTError Error;
		Error.Code = *ErrorObj->GetStringField("code");
		Error.Type = *ErrorObj->GetStringField("type");
		Error.Message = ErrorObj->GetStringField("message");

		Output.Error = Error;

		return Output;
	}

	Output.bSuccess = true;
	Output.ID = *JsonResponse->GetStringField("id");
	Output.Object = *JsonResponse->GetStringField("object");
	Output.Created = JsonResponse->GetNumberField("created");
	TArray<TSharedPtr<FJsonValue>> ChoicesArr = JsonResponse->GetArrayField("choices");

	for (auto Iterator = ChoicesArr.CreateIterator(); Iterator; ++Iterator)
	{
		if (const TSharedPtr<FJsonObject>* ChoiceObj; (*Iterator)->TryGetObject(ChoiceObj))
		{
			FHttpGPTChoice Choice;
			Choice.Index = (*ChoiceObj)->GetNumberField("index");

			TSharedPtr<FJsonObject> MessageObj = (*ChoiceObj)->GetObjectField("message");
			Choice.Message = FHttpGPTMessage(*MessageObj->GetStringField("role"), *MessageObj->GetStringField("content"));

			while (Choice.Message.Content.StartsWith("\n"))
			{
				Choice.Message.Content.RemoveAt(0);
			}

			if (FString FinishReasonStr; (*ChoiceObj)->TryGetStringField("finish_reason", FinishReasonStr))
			{
				Choice.FinishReason = *FinishReasonStr;
			}

			Output.Choices.Add(Choice);
		}
	}

	TSharedPtr<FJsonObject> UsageObj = JsonResponse->GetObjectField("usage");
	Output.Usage = FHttpGPTUsage(UsageObj->GetNumberField("prompt_tokens"), UsageObj->GetNumberField("completion_tokens"), UsageObj->GetNumberField("total_tokens"));

	return Output;
}

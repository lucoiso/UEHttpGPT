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
#include <Misc/ScopeTryLock.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(HttpGPTRequest)
#endif

UHttpGPTRequest* UHttpGPTRequest::SendMessage(UObject* WorldContextObject, const FString& Message, const FHttpGPTOptions& Options)
{
	UHttpGPTRequest* const Task = NewObject<UHttpGPTRequest>();
	Task->Messages = { FHttpGPTMessage(EHttpGPTRole::User, Message) };
	Task->Options = Options;

	Task->RegisterWithGameInstance(WorldContextObject);

	return Task;
}

UHttpGPTRequest* UHttpGPTRequest::SendMessages(UObject* WorldContextObject, const TArray<FHttpGPTMessage>& Messages, const FHttpGPTOptions& Options)
{
	UHttpGPTRequest* const Task = NewObject<UHttpGPTRequest>();
	Task->Messages = Messages;
	Task->Options = Options;

	Task->RegisterWithGameInstance(WorldContextObject);

	return Task;
}

void UHttpGPTRequest::Activate()
{
	const UHttpGPTSettings* const Settings = UHttpGPTSettings::Get();

	if (Messages.IsEmpty() || !IsValid(Settings) || Settings->APIKey.IsNone())
	{
		UE_LOG(LogHttpGPT, Error, TEXT("%s (%d): Request not sent due to invalid params"), *FString(__func__), GetUniqueID());
		RequestFailed.Broadcast();
		SetReadyToDestroy();
		return;
	}

	TSharedRef<class IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL("https://api.openai.com/v1/chat/completions");
	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetHeader("Authorization", FString::Format(TEXT("Bearer {0}"), { Settings->APIKey.ToString() }));

	const TSharedPtr<FJsonObject> JsonRequest = MakeShareable(new FJsonObject);
	JsonRequest->SetStringField("model", ModelToName(Options.Model).ToString().ToLower());
	JsonRequest->SetNumberField("max_tokens", Options.MaxTokens);
	JsonRequest->SetNumberField("temperature", Options.Temperature);
	JsonRequest->SetNumberField("top_p", Options.TopP);
	JsonRequest->SetNumberField("n", Options.Choices);
	JsonRequest->SetBoolField("stream", Options.bStream);
	JsonRequest->SetNumberField("presence_penalty", Options.PresencePenalty);
	JsonRequest->SetNumberField("frequency_penalty", Options.FrequencyPenalty);
	JsonRequest->SetStringField("user", Options.User.ToString());

	if (!Options.Stop.IsEmpty())
	{
		TArray<TSharedPtr<FJsonValue>> StopJson;
		for (const FName& Iterator : Options.Stop)
		{
			StopJson.Add(MakeShareable(new FJsonValueString(Iterator.ToString())));
		}
		JsonRequest->SetArrayField("stop", StopJson);
	}

	if (!Options.Stop.IsEmpty())
	{
		TArray<TSharedPtr<FJsonValue>> LogitBiasJson;
		for (const float& Iterator : Options.LogitBias)
		{
			LogitBiasJson.Add(MakeShareable(new FJsonValueNumber(Iterator)));
		}
		JsonRequest->SetArrayField("logit_bias", LogitBiasJson);
	}

	TArray<TSharedPtr<FJsonValue>> MessagesJson;
	for (const FHttpGPTMessage& Iterator : Messages)
	{
		MessagesJson.Add(Iterator.GetMessage());
	}

	JsonRequest->SetArrayField("messages", MessagesJson);

	FString RequestContentString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestContentString);
	FJsonSerializer::Serialize(JsonRequest.ToSharedRef(), Writer);

	HttpRequest->SetContentAsString(RequestContentString);

	HttpRequest->OnRequestProgress().BindLambda(
		[this, HttpRequest](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived)
		{
			FScopeTryLock Lock(&Mutex);

			if (!Lock.IsLocked())
			{
				HttpRequest->CancelRequest();
				return;
			}

			if (IsValid(this))
			{
				OnProgressUpdated(Request->GetResponse()->GetContentAsString(), BytesSent, BytesReceived);
			}
		}
	);

	HttpRequest->OnProcessRequestComplete().BindLambda(
		[this, HttpRequest](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			FScopeTryLock Lock(&Mutex);

			if (!Lock.IsLocked())
			{
				HttpRequest->CancelRequest();
				return;
			}

			if (IsValid(this))
			{
				OnProgressCompleted(Response->GetContentAsString(), bWasSuccessful);
			}

			SetReadyToDestroy();
		}
	);

	HttpRequest->ProcessRequest();

	UE_LOG(LogHttpGPT, Display, TEXT("%s (%d): Request sent"), *FString(__func__), GetUniqueID());
	RequestSent.Broadcast();
}

void UHttpGPTRequest::OnProgressUpdated(const FString& Content, int32 BytesSent, int32 BytesReceived)
{
	FScopeTryLock Lock(&Mutex);

	if (!Lock.IsLocked() || Content.IsEmpty())
	{
		return;
	}

	TArray<FString> Deltas;
	Content.ParseIntoArray(Deltas, TEXT("data: "));
	const FString LastContent = Deltas.Top();

	UE_LOG(LogHttpGPT, Display, TEXT("%s (%d): Progress Updated"), *FString(__func__), GetUniqueID());
	UE_LOG(LogHttpGPT_Internal, Display, TEXT("%s (%d): Content: %s; Bytes Sent: %d; Bytes Received: %d"), *FString(__func__), GetUniqueID(), *LastContent, BytesSent, BytesReceived);

	DeserializeResponse(LastContent);

	if (!bInitialized)
	{
		bInitialized = true;
		ProgressStarted.Broadcast();
	}

	ProgressUpdated.Broadcast(Response);
}

void UHttpGPTRequest::OnProgressCompleted(const FString& Content, const bool bWasSuccessful)
{
	FScopeTryLock Lock(&Mutex);

	if (!Lock.IsLocked())
	{
		return;
	}

	UE_LOG(LogHttpGPT, Display, TEXT("%s (%d): Process Completed"), *FString(__func__), GetUniqueID());
	UE_LOG(LogHttpGPT_Internal, Display, TEXT("%s (%d): Content: %s"), *FString(__func__), GetUniqueID(), *Content);

	if (!bWasSuccessful || Content.IsEmpty())
	{
		UE_LOG(LogHttpGPT, Error, TEXT("%s (%d): Request failed"), *FString(__func__), GetUniqueID());
		return;
	}

	if (!Options.bStream)
	{
		DeserializeResponse(Content);
	}

	if (Response.bSuccess)
	{
		ProcessCompleted.Broadcast(Response);
	}
	else
	{
		UE_LOG(LogHttpGPT, Error, TEXT("%s (%d): Request failed"), *FString(__func__), GetUniqueID());
		ErrorReceived.Broadcast(Response);
	}
}

void UHttpGPTRequest::DeserializeResponse(const FString& Content)
{
	FScopeTryLock Lock(&Mutex);

	if (!Lock.IsLocked() || Content.IsEmpty() || Content.Contains("[DONE]", ESearchCase::IgnoreCase))
	{
		return;
	}

	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
	TSharedPtr<FJsonObject> JsonResponse = MakeShareable(new FJsonObject);
	FJsonSerializer::Deserialize(Reader, JsonResponse);

	if (JsonResponse->HasField("error"))
	{
		Response.bSuccess = false;

		const TSharedPtr<FJsonObject> ErrorObj = JsonResponse->GetObjectField("error");

		FHttpGPTError Error;
		Error.Code = *ErrorObj->GetStringField("code");
		Error.Type = *ErrorObj->GetStringField("type");
		Error.Message = ErrorObj->GetStringField("message");

		Response.Error = Error;

		return;
	}

	Response.bSuccess = true;

	Response.ID = *JsonResponse->GetStringField("id");
	Response.Object = *JsonResponse->GetStringField("object");
	Response.Created = JsonResponse->GetNumberField("created");

	const TArray<TSharedPtr<FJsonValue>> ChoicesArr = JsonResponse->GetArrayField("choices");

	for (auto Iterator = ChoicesArr.CreateConstIterator(); Iterator; ++Iterator)
	{
		const TSharedPtr<FJsonObject> ChoiceObj = (*Iterator)->AsObject();
		const int32 ChoiceIndex = ChoiceObj->GetIntegerField("index");

		FHttpGPTChoice* Choice = Response.Choices.FindByPredicate(
			[this, ChoiceIndex](const FHttpGPTChoice& Element)
			{
				return Element.Index == ChoiceIndex;
			}
		);

		if (!Choice)
		{
			FHttpGPTChoice NewChoice;
			NewChoice.Index = ChoiceIndex;
			Choice = &Response.Choices.Add_GetRef(NewChoice);
		}

		if (const TSharedPtr<FJsonObject>* MessageObj; ChoiceObj->TryGetObjectField("message", MessageObj))
		{
			Choice->Message = FHttpGPTMessage(*(*MessageObj)->GetStringField("role"), *(*MessageObj)->GetStringField("content"));
		}
		else if (const TSharedPtr<FJsonObject>* DeltaObj; ChoiceObj->TryGetObjectField("delta", DeltaObj))
		{
			if (FString RoleStr; (*DeltaObj)->TryGetStringField("role", RoleStr))
			{
				Choice->Message.Role = RoleStr == "user" ? EHttpGPTRole::User : EHttpGPTRole::Assistant;
			}
			else if (FString ContentStr; (*DeltaObj)->TryGetStringField("content", ContentStr))
			{
				Choice->Message.Content += ContentStr;
			}
		}

		while (Choice->Message.Content.StartsWith("\n"))
		{
			Choice->Message.Content.RemoveAt(0);
		}

		if (FString FinishReasonStr; ChoiceObj->TryGetStringField("finish_reason", FinishReasonStr))
		{
			Choice->FinishReason = *FinishReasonStr;
		}
	}

	if (const TSharedPtr<FJsonObject>* UsageObj; JsonResponse->TryGetObjectField("usage", UsageObj))
	{
		Response.Usage = FHttpGPTUsage((*UsageObj)->GetNumberField("prompt_tokens"), (*UsageObj)->GetNumberField("completion_tokens"), (*UsageObj)->GetNumberField("total_tokens"));
	}
}

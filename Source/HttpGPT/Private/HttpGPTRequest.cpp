// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "HttpGPTRequest.h"
#include "HttpGPTSettings.h"
#include "HttpGPTHelper.h"
#include "LogHttpGPT.h"
#include <HttpModule.h>
#include <Interfaces/IHttpRequest.h>
#include <Interfaces/IHttpResponse.h>
#include <Dom/JsonObject.h>
#include <Serialization/JsonWriter.h>
#include <Serialization/JsonReader.h>
#include <Serialization/JsonSerializer.h>
#include <Misc/ScopeTryLock.h>
#include <Async/Async.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(HttpGPTRequest)
#endif

UHttpGPTRequest* UHttpGPTRequest::SendMessage_DefaultOptions(UObject* WorldContextObject, const FString& Message)
{
	return SendMessage_CustomOptions(WorldContextObject, Message, FHttpGPTOptions());
}

UHttpGPTRequest* UHttpGPTRequest::SendMessages_DefaultOptions(UObject* WorldContextObject, const TArray<FHttpGPTMessage>& Messages)
{
	return SendMessages_CustomOptions(WorldContextObject, Messages, FHttpGPTOptions());
}

UHttpGPTRequest* UHttpGPTRequest::SendMessage_CustomOptions(UObject* WorldContextObject, const FString& Message, const FHttpGPTOptions& Options)
{
	return SendMessages_CustomOptions(WorldContextObject, { FHttpGPTMessage(EHttpGPTRole::User, Message) }, Options);
}

UHttpGPTRequest* UHttpGPTRequest::SendMessages_CustomOptions(UObject* WorldContextObject, const TArray<FHttpGPTMessage>& Messages, const FHttpGPTOptions& Options)
{
	UHttpGPTRequest* const Task = NewObject<UHttpGPTRequest>();
	Task->Messages = Messages;
	Task->TaskOptions = Options;

	Task->RegisterWithGameInstance(WorldContextObject);

	return Task;
}

void UHttpGPTRequest::StopHttpGPTTask()
{
	FScopeLock Lock(&Mutex);

	if (!bIsActive)
	{
		return;
	}

	UE_LOG(LogHttpGPT, Display, TEXT("%s (%d): Stopping task"), *FString(__func__), GetUniqueID());

	bIsActive = false;

	if (HttpRequest.IsValid())
	{
		HttpRequest->CancelRequest();
		HttpRequest.Reset();
	}

	SetReadyToDestroy();
}

const FHttpGPTOptions UHttpGPTRequest::GetTaskOptions() const
{
	return TaskOptions;
}

void UHttpGPTRequest::Activate()
{
	Super::Activate();

	UE_LOG(LogHttpGPT, Display, TEXT("%s (%d): Activating task"), *FString(__func__), GetUniqueID());

	bIsActive = true;

	if (Messages.IsEmpty() || TaskOptions.APIKey.IsNone())
	{
		UE_LOG(LogHttpGPT, Error, TEXT("%s (%d): Failed to activate task: Request not sent due to invalid params"), *FString(__func__), GetUniqueID());
		RequestFailed.Broadcast();
		SetReadyToDestroy();
		return;
	}

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
		[this]
		{
			SendRequest();
		}
	);
}

void UHttpGPTRequest::SetReadyToDestroy()
{
	FScopeLock Lock(&Mutex);

	if (bIsReadyToDestroy)
	{
		return;
	}

	UE_LOG(LogHttpGPT, Display, TEXT("%s (%d): Setting task as Ready to Destroy"), *FString(__func__), GetUniqueID());

	Super::SetReadyToDestroy();

	bIsReadyToDestroy = true;
	bIsActive = false;
}

void UHttpGPTRequest::SendRequest()
{
	FScopeLock Lock(&Mutex);

	InitializeRequest();
	SetRequestContent();
	BindRequestCallbacks();

	if (!HttpRequest.IsValid())
	{
		UE_LOG(LogHttpGPT, Error, TEXT("%s (%d): Failed to send request: Request object is invalid"), *FString(__func__), GetUniqueID());
		RequestFailed.Broadcast();
		SetReadyToDestroy();
		return;
	}

	UE_LOG(LogHttpGPT_Internal, Display, TEXT("%s (%d): Sending request"), *FString(__func__), GetUniqueID());

	HttpRequest->ProcessRequest();

	UE_LOG(LogHttpGPT_Internal, Display, TEXT("%s (%d): Request sent"), *FString(__func__), GetUniqueID());

	AsyncTask(ENamedThreads::GameThread, 
		[this] 
		{ 
			RequestSent.Broadcast(); 
		}
	);	
}

void UHttpGPTRequest::InitializeRequest()
{
	FScopeLock Lock(&Mutex);

	UE_LOG(LogHttpGPT_Internal, Display, TEXT("%s (%d): Initializing request object"), *FString(__func__), GetUniqueID());

	HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL("https://api.openai.com/v1/chat/completions");
	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetHeader("Authorization", FString::Format(TEXT("Bearer {0}"), { TaskOptions.APIKey.ToString() }));
}

void UHttpGPTRequest::SetRequestContent()
{
	FScopeLock Lock(&Mutex);

	if (!HttpRequest.IsValid())
	{
		return;
	}

	UE_LOG(LogHttpGPT_Internal, Display, TEXT("%s (%d): Mounting content"), *FString(__func__), GetUniqueID());

	const TSharedPtr<FJsonObject> JsonRequest = MakeShareable(new FJsonObject);
	JsonRequest->SetStringField("model", UHttpGPTHelper::ModelToName(TaskOptions.Model).ToString().ToLower());
	JsonRequest->SetNumberField("max_tokens", TaskOptions.MaxTokens);
	JsonRequest->SetNumberField("temperature", TaskOptions.Temperature);
	JsonRequest->SetNumberField("top_p", TaskOptions.TopP);
	JsonRequest->SetNumberField("n", TaskOptions.Choices);
	JsonRequest->SetBoolField("stream", TaskOptions.bStream);
	JsonRequest->SetNumberField("presence_penalty", TaskOptions.PresencePenalty);
	JsonRequest->SetNumberField("frequency_penalty", TaskOptions.FrequencyPenalty);

	if (!TaskOptions.User.IsNone())
	{
		JsonRequest->SetStringField("user", TaskOptions.User.ToString());
	}

	if (!TaskOptions.Stop.IsEmpty())
	{
		TArray<TSharedPtr<FJsonValue>> StopJson;
		for (const FName& Iterator : TaskOptions.Stop)
		{
			StopJson.Add(MakeShareable(new FJsonValueString(Iterator.ToString())));
		}

		JsonRequest->SetArrayField("stop", StopJson);
	}

	if (!TaskOptions.LogitBias.IsEmpty())
	{
		TSharedPtr<FJsonObject> LogitBiasJson = MakeShareable(new FJsonObject());
		for (auto Iterator = TaskOptions.LogitBias.CreateConstIterator(); Iterator; ++Iterator)
		{
			LogitBiasJson->SetNumberField(FString::FromInt(Iterator.Key()), Iterator.Value());
		}

		JsonRequest->SetObjectField("logit_bias", LogitBiasJson);
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
}

void UHttpGPTRequest::BindRequestCallbacks()
{
	FScopeLock Lock(&Mutex);

	if (!HttpRequest.IsValid())
	{
		return;
	}

	UE_LOG(LogHttpGPT_Internal, Display, TEXT("%s (%d): Binding callbacks"), *FString(__func__), GetUniqueID());

	if (TaskOptions.bStream)
	{
		HttpRequest->OnRequestProgress().BindLambda(
			[this](FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived)
			{
				FScopeTryLock Lock(&Mutex);

				if (!Lock.IsLocked() || !IsValid(this) || !bIsActive)
				{
					return;
				}

				OnProgressUpdated(Request->GetResponse()->GetContentAsString(), BytesSent, BytesReceived);
			}
		);
	}

	HttpRequest->OnProcessRequestComplete().BindLambda(
		[this](FHttpRequestPtr Request, FHttpResponsePtr RequestResponse, bool bWasSuccessful)
		{
			FScopeTryLock Lock(&Mutex);

			if (!Lock.IsLocked() || !IsValid(this) || !bIsActive)
			{
				return;
			}

			OnProgressCompleted(RequestResponse->GetContentAsString(), bWasSuccessful);
			SetReadyToDestroy();
		}
	);
}

void UHttpGPTRequest::OnProgressUpdated(const FString& Content, int32 BytesSent, int32 BytesReceived)
{
	FScopeLock Lock(&Mutex);

	if (Content.IsEmpty())
	{
		return;
	}

	TArray<FString> Deltas = GetDeltasFromContent(Content);

	UE_LOG(LogHttpGPT_Internal, Display, TEXT("%s (%d): Progress Updated"), *FString(__func__), GetUniqueID());
	UE_LOG(LogHttpGPT_Internal, Display, TEXT("%s (%d): Content: %s; Bytes Sent: %d; Bytes Received: %d"), *FString(__func__), GetUniqueID(), *Deltas.Top(), BytesSent, BytesReceived);

	DeserializeStreamedResponse(Deltas);

	if (!bInitialized)
	{
		bInitialized = true;

		AsyncTask(ENamedThreads::GameThread,
			[this]
			{
				FScopeLock Lock(&Mutex);
				ProgressStarted.Broadcast(Response);
			}
		);
	}

	AsyncTask(ENamedThreads::GameThread,
		[this]
		{
			FScopeLock Lock(&Mutex);
			ProgressUpdated.Broadcast(Response);
		}
	);
}

void UHttpGPTRequest::OnProgressCompleted(const FString& Content, const bool bWasSuccessful)
{
	FScopeLock Lock(&Mutex);	

	if (!bWasSuccessful || Content.IsEmpty())
	{
		UE_LOG(LogHttpGPT, Error, TEXT("%s (%d): Request failed"), *FString(__func__), GetUniqueID());
		AsyncTask(ENamedThreads::GameThread,
			[this]
			{
				RequestFailed.Broadcast();
			}
		);

		return;
	}

	UE_LOG(LogHttpGPT_Internal, Display, TEXT("%s (%d): Process Completed"), *FString(__func__), GetUniqueID());
	UE_LOG(LogHttpGPT_Internal, Display, TEXT("%s (%d): Content: %s"), *FString(__func__), GetUniqueID(), *Content);

	if (!TaskOptions.bStream)
	{
		DeserializeSingleResponse(Content);
	}
	else
	{
		TArray<FString> Deltas = GetDeltasFromContent(Content);
		DeserializeStreamedResponse(Deltas);
	}

	if (Response.bSuccess)
	{
		AsyncTask(ENamedThreads::GameThread,
			[this]
			{
				FScopeLock Lock(&Mutex);

				if (!TaskOptions.bStream)
				{
					ProgressStarted.Broadcast(Response);
				}

				ProcessCompleted.Broadcast(Response);
			}
		);
	}
	else
	{
		UE_LOG(LogHttpGPT, Error, TEXT("%s (%d): Request failed"), *FString(__func__), GetUniqueID());
		AsyncTask(ENamedThreads::GameThread,
			[this]
			{
				FScopeLock Lock(&Mutex);
				ErrorReceived.Broadcast(Response);
			}
		);
	}
}

TArray<FString> UHttpGPTRequest::GetDeltasFromContent(const FString& Content) const
{
	TArray<FString> Deltas;
	Content.ParseIntoArray(Deltas, TEXT("data: "));

	if (Deltas.Top().Contains("[done]", ESearchCase::IgnoreCase))
	{
		Deltas.Pop();
	}

	if (Deltas.IsEmpty())
	{
		Deltas.Add(Content);
	}

	return Deltas;
}

void UHttpGPTRequest::DeserializeStreamedResponse(const TArray<FString>& Deltas)
{
	FScopeLock Lock(&Mutex);

	Response.Choices.Empty(Deltas.Num());
	for (const FString& Delta : Deltas)
	{
		DeserializeSingleResponse(Delta);
	}
}

void UHttpGPTRequest::DeserializeSingleResponse(const FString& Content)
{
	FScopeLock Lock(&Mutex);

	if (Content.IsEmpty())
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

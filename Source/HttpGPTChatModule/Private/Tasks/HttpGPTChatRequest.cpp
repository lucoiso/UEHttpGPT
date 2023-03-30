// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "Tasks/HttpGPTChatRequest.h"
#include <Utils/HttpGPTHelper.h>
#include <Management/HttpGPTSettings.h>
#include <HttpGPTInternalFuncs.h>
#include <LogHttpGPT.h>

#include <HttpModule.h>
#include <Interfaces/IHttpRequest.h>
#include <Interfaces/IHttpResponse.h>
#include <Dom/JsonObject.h>
#include <Serialization/JsonWriter.h>
#include <Serialization/JsonReader.h>
#include <Serialization/JsonSerializer.h>
#include <Misc/ScopeTryLock.h>
#include <Async/Async.h>

#if WITH_EDITOR
#include <Editor.h>
#endif

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(HttpGPTChatRequest)
#endif

#if WITH_EDITOR
UHttpGPTChatRequest* UHttpGPTChatRequest::EditorTask(const TArray<FHttpGPTChatMessage>& Messages, const FHttpGPTChatOptions Options)
{
	UHttpGPTChatRequest* const NewAsyncTask = SendMessages_CustomOptions(GEditor->GetEditorWorldContext().World(), Messages, FHttpGPTCommonOptions(), Options);
	NewAsyncTask->bIsEditorTask = true;

	return NewAsyncTask;
}
#endif

UHttpGPTChatRequest* UHttpGPTChatRequest::SendMessage_DefaultOptions(UObject* WorldContextObject, const FString& Message)
{
	return SendMessage_CustomOptions(WorldContextObject, Message, FHttpGPTCommonOptions(), FHttpGPTChatOptions());
}

UHttpGPTChatRequest* UHttpGPTChatRequest::SendMessages_DefaultOptions(UObject* WorldContextObject, const TArray<FHttpGPTChatMessage>& Messages)
{
	return SendMessages_CustomOptions(WorldContextObject, Messages, FHttpGPTCommonOptions(), FHttpGPTChatOptions());
}

UHttpGPTChatRequest* UHttpGPTChatRequest::SendMessage_CustomOptions(UObject* WorldContextObject, const FString& Message, const FHttpGPTCommonOptions CommonOptions, const FHttpGPTChatOptions ChatOptions)
{
	return SendMessages_CustomOptions(WorldContextObject, { FHttpGPTChatMessage(EHttpGPTChatRole::User, Message) }, CommonOptions, ChatOptions);
}

UHttpGPTChatRequest* UHttpGPTChatRequest::SendMessages_CustomOptions(UObject* WorldContextObject, const TArray<FHttpGPTChatMessage>& Messages, const FHttpGPTCommonOptions CommonOptions, const FHttpGPTChatOptions ChatOptions)
{
	UHttpGPTChatRequest* const NewAsyncTask = NewObject<UHttpGPTChatRequest>();
	NewAsyncTask->Messages = Messages;
	NewAsyncTask->CommonOptions = CommonOptions;
	NewAsyncTask->ChatOptions = ChatOptions;

	NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

	return NewAsyncTask;
}

bool UHttpGPTChatRequest::CanActivateTask() const
{
	return Super::CanActivateTask() && !HttpGPT::Internal::HasEmptyParam(Messages);
}

bool UHttpGPTChatRequest::CanBindProgress() const
{
	return GetChatOptions().bStream;
}

FString UHttpGPTChatRequest::GetEndpointURL() const
{
	return FString::Format(TEXT("https://api.openai.com/{0}"), { UHttpGPTHelper::GetEndpointForModel(GetChatOptions().Model).ToString() });
}

const FHttpGPTChatOptions UHttpGPTChatRequest::GetChatOptions() const
{
	return ChatOptions;
}

void UHttpGPTChatRequest::SetRequestContent()
{
	FScopeLock Lock(&Mutex);

	if (!HttpRequest.IsValid())
	{
		return;
	}

	UE_LOG(LogHttpGPT_Internal, Display, TEXT("%s (%d): Mounting content"), *FString(__func__), GetUniqueID());

	const TSharedPtr<FJsonObject> JsonRequest = MakeShareable(new FJsonObject);
	JsonRequest->SetStringField("model", UHttpGPTHelper::ModelToName(GetChatOptions().Model).ToString().ToLower());
	JsonRequest->SetNumberField("max_tokens", GetChatOptions().MaxTokens);
	JsonRequest->SetNumberField("temperature", GetChatOptions().Temperature);
	JsonRequest->SetNumberField("top_p", GetChatOptions().TopP);
	JsonRequest->SetNumberField("n", GetChatOptions().Choices);
	JsonRequest->SetBoolField("stream", GetChatOptions().bStream);
	JsonRequest->SetNumberField("presence_penalty", GetChatOptions().PresencePenalty);
	JsonRequest->SetNumberField("frequency_penalty", GetChatOptions().FrequencyPenalty);

	if (!HttpGPT::Internal::HasEmptyParam(GetCommonOptions().User))
	{
		JsonRequest->SetStringField("user", GetCommonOptions().User.ToString());
	}

	if (!HttpGPT::Internal::HasEmptyParam(GetChatOptions().Stop))
	{
		TArray<TSharedPtr<FJsonValue>> StopJson;
		for (const FName& Iterator : GetChatOptions().Stop)
		{
			StopJson.Add(MakeShareable(new FJsonValueString(Iterator.ToString())));
		}

		JsonRequest->SetArrayField("stop", StopJson);
	}

	if (!HttpGPT::Internal::HasEmptyParam(GetChatOptions().LogitBias))
	{
		TSharedPtr<FJsonObject> LogitBiasJson = MakeShareable(new FJsonObject());
		for (auto Iterator = GetChatOptions().LogitBias.CreateConstIterator(); Iterator; ++Iterator)
		{
			LogitBiasJson->SetNumberField(FString::FromInt(Iterator.Key()), Iterator.Value());
		}

		JsonRequest->SetObjectField("logit_bias", LogitBiasJson);
	}

	if (UHttpGPTHelper::ModelSupportsChat(GetChatOptions().Model))
	{
		UE_LOG(LogHttpGPT_Internal, Display, TEXT("%s (%d): Selected model supports Chat API. Mounting section history."), *FString(__func__), GetUniqueID());
		TArray<TSharedPtr<FJsonValue>> MessagesJson;
		for (const FHttpGPTChatMessage& Iterator : Messages)
		{
			MessagesJson.Add(Iterator.GetMessage());
		}

		JsonRequest->SetArrayField("messages", MessagesJson);
	}
	else
	{
		UE_LOG(LogHttpGPT_Internal, Display, TEXT("%s (%d): Selected model does not supports Chat API. Using last message as prompt content."), *FString(__func__), GetUniqueID());
		JsonRequest->SetStringField("prompt", Messages.Top().Content);
	}

	FString RequestContentString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestContentString);
	FJsonSerializer::Serialize(JsonRequest.ToSharedRef(), Writer);

	HttpRequest->SetContentAsString(RequestContentString);
}

void UHttpGPTChatRequest::OnProgressUpdated(const FString& Content, int32 BytesSent, int32 BytesReceived)
{
	FScopeLock Lock(&Mutex);

	if (HttpGPT::Internal::HasEmptyParam(Content))
	{
		return;
	}

	TArray<FString> Deltas = GetDeltasFromContent(Content);

	UE_LOG(LogHttpGPT_Internal, Display, TEXT("%s (%d): Progress Updated"), *FString(__func__), GetUniqueID());
	UE_LOG(LogHttpGPT_Internal, Display, TEXT("%s (%d): Content: %s; Bytes Sent: %d; Bytes Received: %d"), *FString(__func__), GetUniqueID(), *Deltas.Top(), BytesSent, BytesReceived);

	DeserializeStreamedResponse(Deltas);

	if (!Response.bSuccess)
	{
		return;
	}

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

void UHttpGPTChatRequest::OnProgressCompleted(const FString& Content, const bool bWasSuccessful)
{
	FScopeLock Lock(&Mutex);	

	if (!bWasSuccessful || HttpGPT::Internal::HasEmptyParam(Content))
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

	if (!GetChatOptions().bStream)
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

				if (!GetChatOptions().bStream)
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

TArray<FString> UHttpGPTChatRequest::GetDeltasFromContent(const FString& Content) const
{
	TArray<FString> Deltas;
	Content.ParseIntoArray(Deltas, TEXT("data: "));

	if (Deltas.Top().Contains("[done]", ESearchCase::IgnoreCase))
	{
		Deltas.Pop();
	}

	if (HttpGPT::Internal::HasEmptyParam(Deltas))
	{
		Deltas.Add(Content);
	}

	return Deltas;
}

void UHttpGPTChatRequest::DeserializeStreamedResponse(const TArray<FString>& Deltas)
{
	FScopeLock Lock(&Mutex);

	Response.Choices.Empty(Deltas.Num());
	for (const FString& Delta : Deltas)
	{
		DeserializeSingleResponse(Delta);
	}
}

void UHttpGPTChatRequest::DeserializeSingleResponse(const FString& Content)
{
	FScopeLock Lock(&Mutex);

	if (HttpGPT::Internal::HasEmptyParam(Content))
	{
		return;
	}

	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
	TSharedPtr<FJsonObject> JsonResponse = MakeShareable(new FJsonObject);
	FJsonSerializer::Deserialize(Reader, JsonResponse);

	if (CheckError(JsonResponse, Response.Error))
	{
		Response.bSuccess = false;
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

		FHttpGPTChatChoice* Choice = Response.Choices.FindByPredicate(
			[this, ChoiceIndex](const FHttpGPTChatChoice& Element)
			{
				return Element.Index == ChoiceIndex;
			}
		);

		if (!Choice)
		{
			FHttpGPTChatChoice NewChoice;
			NewChoice.Index = ChoiceIndex;
			Choice = &Response.Choices.Add_GetRef(NewChoice);
		}

		if (const TSharedPtr<FJsonObject>* MessageObj; ChoiceObj->TryGetObjectField("message", MessageObj))
		{
			Choice->Message = FHttpGPTChatMessage(*(*MessageObj)->GetStringField("role"), *(*MessageObj)->GetStringField("content"));
		}
		else if (const TSharedPtr<FJsonObject>* DeltaObj; ChoiceObj->TryGetObjectField("delta", DeltaObj))
		{
			if (FString RoleStr; (*DeltaObj)->TryGetStringField("role", RoleStr))
			{
				Choice->Message.Role = RoleStr == "user" ? EHttpGPTChatRole::User : EHttpGPTChatRole::Assistant;
			}
			else if (FString ContentStr; (*DeltaObj)->TryGetStringField("content", ContentStr))
			{
				Choice->Message.Content += ContentStr;
			}
		}
		else if (FString MessageText; ChoiceObj->TryGetStringField("text", MessageText))
		{
			Choice->Message.Role = EHttpGPTChatRole::Assistant;
			Choice->Message.Content += MessageText;
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
		Response.Usage = FHttpGPTChatUsage((*UsageObj)->GetNumberField("prompt_tokens"), (*UsageObj)->GetNumberField("completion_tokens"), (*UsageObj)->GetNumberField("total_tokens"));
	}
}

UHttpGPTChatRequest* UHttpGPTChatHelper::CastToHTTPGPTChatRequest(UObject* Object)
{
	return Cast<UHttpGPTChatRequest>(Object);
}
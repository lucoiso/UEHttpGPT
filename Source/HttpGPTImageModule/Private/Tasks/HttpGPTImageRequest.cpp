// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "Tasks/HttpGPTImageRequest.h"
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
#include UE_INLINE_GENERATED_CPP_BY_NAME(HttpGPTImageRequest)
#endif

#if WITH_EDITOR
UHttpGPTImageRequest* UHttpGPTImageRequest::EditorTask(const FString& Prompt, const FHttpGPTImageOptions Options)
{
	UHttpGPTImageRequest* const NewAsyncTask = RequestImages_CustomOptions(GEditor->GetEditorWorldContext().World(), Prompt, FHttpGPTCommonOptions(), Options);
	NewAsyncTask->bIsEditorTask = true;

	return NewAsyncTask;
}
#endif

UHttpGPTImageRequest* UHttpGPTImageRequest::RequestImages_DefaultOptions(UObject* WorldContextObject, const FString& Prompt)
{
	return RequestImages_CustomOptions(WorldContextObject, Prompt, FHttpGPTCommonOptions(), FHttpGPTImageOptions());
}

UHttpGPTImageRequest* UHttpGPTImageRequest::RequestImages_CustomOptions(UObject* WorldContextObject, const FString& Prompt, const FHttpGPTCommonOptions CommonOptions, const FHttpGPTImageOptions ImageOptions)
{
	return nullptr;
}

const FHttpGPTImageOptions UHttpGPTImageRequest::GetImageOptions() const
{
	return ImageOptions;
}

const FString UHttpGPTImageRequest::GetPrompt() const
{
	return Prompt;
}

bool UHttpGPTImageRequest::CanActivateTask() const
{
	return Super::CanActivateTask() && !HttpGPT::Internal::HasEmptyParam(Prompt);
}

bool UHttpGPTImageRequest::CanBindProgress() const
{
	return false;
}

FString UHttpGPTImageRequest::GetEndpointURL() const
{
	return "https://api.openai.com/v1/images/generations";
}

void UHttpGPTImageRequest::SetRequestContent()
{
	FScopeLock Lock(&Mutex);

	if (!HttpRequest.IsValid())
	{
		return;
	}

	UE_LOG(LogHttpGPT_Internal, Display, TEXT("%s (%d): Mounting content"), *FString(__func__), GetUniqueID());

	const TSharedPtr<FJsonObject> JsonRequest = MakeShareable(new FJsonObject);
	JsonRequest->SetStringField("prompt", Prompt);
	JsonRequest->SetNumberField("n", GetImageOptions().ImagesNum);
	JsonRequest->SetStringField("size", UHttpGPTHelper::SizeToName(GetImageOptions().Size).ToString());
	JsonRequest->SetStringField("format", UHttpGPTHelper::FormatToName(GetImageOptions().Format).ToString());

	if (!HttpGPT::Internal::HasEmptyParam(GetCommonOptions().User))
	{
		JsonRequest->SetStringField("user", GetCommonOptions().User.ToString());
	}

	FString RequestContentString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestContentString);
	FJsonSerializer::Serialize(JsonRequest.ToSharedRef(), Writer);

	HttpRequest->SetContentAsString(RequestContentString);
}

void UHttpGPTImageRequest::OnProgressCompleted(const FString& Content, const bool bWasSuccessful)
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

	DeserializeResponse(Content);

	if (Response.bSuccess)
	{
		AsyncTask(ENamedThreads::GameThread,
			[this]
			{
				FScopeLock Lock(&Mutex);
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

void UHttpGPTImageRequest::DeserializeResponse(const FString& Content)
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

	// Work in Progress ...
}

UHttpGPTImageRequest* UHttpGPTImageHelper::CastToHTTPGPTImageRequest(UObject* Object)
{
	return Cast<UHttpGPTImageRequest>(Object);
}

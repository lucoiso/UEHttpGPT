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
#include <Engine/Texture2D.h>
#include <Misc/Base64.h>
#include <ImageUtils.h>

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
	UHttpGPTImageRequest* const NewAsyncTask = NewObject<UHttpGPTImageRequest>();
	NewAsyncTask->Prompt = Prompt;
	NewAsyncTask->CommonOptions = CommonOptions;
	NewAsyncTask->ImageOptions = ImageOptions;

	NewAsyncTask->RegisterWithGameInstance(WorldContextObject);

	return NewAsyncTask;
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
	if (!Super::CanActivateTask())
	{
		return false;
	}

	if (HttpGPT::Internal::HasEmptyParam(Prompt))
	{
		UE_LOG(LogHttpGPT, Error, TEXT("%s (%d): Can't activate task: Invalid Prompt."), *FString(__func__), GetUniqueID());
		return false;
	}

	return true;
}

bool UHttpGPTImageRequest::CanBindProgress() const
{
	return false;
}

FString UHttpGPTImageRequest::GetEndpointURL() const
{
	return "https://api.openai.com/v1/images/generations";
}

FString UHttpGPTImageRequest::SetRequestContent()
{
	FScopeLock Lock(&Mutex);

	if (!HttpRequest.IsValid())
	{
		return FString();
	}

	UE_LOG(LogHttpGPT_Internal, Display, TEXT("%s (%d): Mounting content"), *FString(__func__), GetUniqueID());

	const TSharedPtr<FJsonObject> JsonRequest = MakeShareable(new FJsonObject);
	JsonRequest->SetStringField("prompt", Prompt);
	JsonRequest->SetNumberField("n", GetImageOptions().ImagesNum);
	JsonRequest->SetStringField("size", UHttpGPTHelper::SizeToName(GetImageOptions().Size).ToString());
	JsonRequest->SetStringField("response_format", UHttpGPTHelper::FormatToName(GetImageOptions().Format).ToString());

	if (!HttpGPT::Internal::HasEmptyParam(GetCommonOptions().User))
	{
		JsonRequest->SetStringField("user", GetCommonOptions().User.ToString());
	}

	FString RequestContentString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestContentString);
	FJsonSerializer::Serialize(JsonRequest.ToSharedRef(), Writer);

	HttpRequest->SetContentAsString(RequestContentString);

	return RequestContentString;
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
	Response.Created = JsonResponse->GetNumberField("created");

	const TArray<TSharedPtr<FJsonValue>> DataArray = JsonResponse->GetArrayField("data");
	for (auto Iterator = DataArray.CreateConstIterator(); Iterator; ++Iterator)
	{
		Response.Data.Add(FHttpGPTImageData((*Iterator)->AsObject()->GetStringField(UHttpGPTHelper::FormatToName(GetImageOptions().Format).ToString()), GetImageOptions().Format));
	}
}

UHttpGPTImageRequest* UHttpGPTImageHelper::CastToHttpGPTImageRequest(UObject* Object)
{
	return Cast<UHttpGPTImageRequest>(Object);
}

void UHttpGPTImageHelper::GenerateImage(const FHttpGPTImageData& ImageData, const FHttpGPTImageGenerate& Callback)
{
	switch (ImageData.Format)
	{
		case EHttpGPTResponseFormat::url:
			GenerateImageFromURL(ImageData, Callback);
			break;

		case EHttpGPTResponseFormat::b64_json:
			GenerateImageFromB64(ImageData, Callback);
			break;

		default:
			break;
	}
}

void UHttpGPTImageHelper::GenerateImageFromURL(const FHttpGPTImageData& ImageData, const FHttpGPTImageGenerate& Callback)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(ImageData.Content);
	HttpRequest->SetVerb("GET");
	HttpRequest->OnProcessRequestComplete().BindLambda(
		[=](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			if (bSuccess && Response.IsValid())
			{
				Callback.ExecuteIfBound(FImageUtils::ImportBufferAsTexture2D(Response->GetContent()));
			}
			else
			{
				Callback.ExecuteIfBound(nullptr);
			}
		}
	);
	HttpRequest->ProcessRequest();
}

void UHttpGPTImageHelper::GenerateImageFromB64(const FHttpGPTImageData& ImageData, const FHttpGPTImageGenerate& Callback)
{
	TArray<uint8> DecodedBytes;
	FBase64::Decode(ImageData.Content, DecodedBytes);
	Callback.ExecuteIfBound(FImageUtils::ImportBufferAsTexture2D(DecodedBytes));
}

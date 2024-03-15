// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "HttpGPTImageGetter.h"
#include <Utils/HttpGPTHelper.h>
#include <Widgets/Layout/SScrollBox.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(HttpGPTImageGetter)
#endif

UHttpGPTImageGetter::UHttpGPTImageGetter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), GeneratedImages(0u), DataSize(0u)
{
}

void UHttpGPTImageGetter::RequestSent()
{
	OnStatusChanged.ExecuteIfBound("Request Sent. Waiting for the response...");
}

void UHttpGPTImageGetter::RequestFailed()
{
	OnStatusChanged.ExecuteIfBound("Request Failed. Check the Logs.");
	Destroy();
}

void UHttpGPTImageGetter::ProcessCompleted(const FHttpGPTImageResponse& Response)
{
	if (!Response.bSuccess)
	{
		const FStringFormatOrderedArguments Arguments_ErrorDetails{
			FString("Request Failed."), FString("Please check the logs. (Enable internal logs in Project Settings -> Plugins -> HttpGPT)."),
			FString("Error Details: "), FString("\tError Code: ") + Response.Error.Code.ToString(),
			FString("\tError Type: ") + Response.Error.Type.ToString(), FString("\tError Message: ") + Response.Error.Message
		};

		OnStatusChanged.ExecuteIfBound(FString::Format(TEXT("{0}\n{1}\n\n{2}\n{3}\n{4}\n{5}"), Arguments_ErrorDetails));
		Destroy();
		return;
	}

	DataSize = Response.Data.Num();
	OnStatusChanged.ExecuteIfBound("Request Completed.");

	OnImageGenerated_Internal.BindUFunction(this, TEXT("ImageGenerated"));

	for (const FHttpGPTImageData& Data : Response.Data)
	{
		ProcessImage(Data);
	}
}

void UHttpGPTImageGetter::ProcessImage(const FHttpGPTImageData& Data)
{
	UHttpGPTImageHelper::GenerateImage(Data, OnImageGenerated_Internal);
}

void UHttpGPTImageGetter::ImageGenerated(UTexture2D* const Texture)
{
	OnImageGenerated.ExecuteIfBound(Texture);

	++GeneratedImages;
	if (GeneratedImages >= DataSize)
	{
		if (OutScrollBox.IsValid())
		{
			OutScrollBox->ScrollToEnd();
		}

		Destroy();
	}
}

void UHttpGPTImageGetter::Destroy()
{
	ClearFlags(RF_Standalone);

#if ENGINE_MAJOR_VERSION >= 5
	MarkAsGarbage();
#else
    MarkPendingKill();
#endif
}

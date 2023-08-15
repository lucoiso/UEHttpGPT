// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "HttpGPTMessagingHandler.h"
#include <HttpGPTInternalFuncs.h>
#include <Widgets/Layout/SScrollBox.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(HttpGPTMessagingHandler)
#endif

UHttpGPTMessagingHandler::UHttpGPTMessagingHandler(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UHttpGPTMessagingHandler::RequestSent()
{
    OnMessageContentUpdated.ExecuteIfBound("Waiting for response...");
}

void UHttpGPTMessagingHandler::RequestFailed()
{
    OnMessageContentUpdated.ExecuteIfBound("Request Failed.\nPlease check the logs. (Enable internal logs in Project Settings -> Plugins -> HttpGPT).");
    Destroy();
}

void UHttpGPTMessagingHandler::ProcessUpdated(const FHttpGPTChatResponse& Response)
{
    ProcessResponse(Response);
}

void UHttpGPTMessagingHandler::ProcessCompleted(const FHttpGPTChatResponse& Response)
{
    ProcessResponse(Response);
    Destroy();
}

void UHttpGPTMessagingHandler::ProcessResponse(const FHttpGPTChatResponse& Response)
{
    bool bScrollToEnd = false;
    if (ScrollBoxReference.IsValid())
    {
        bScrollToEnd = FMath::Abs(ScrollBoxReference->GetScrollOffsetOfEnd() - ScrollBoxReference->GetScrollOffset()) <= 8.f;
    }

    if (!Response.bSuccess)
    {
        const FStringFormatOrderedArguments Arguments_ErrorDetails{
            FString("Request Failed."),
            FString("Please check the logs. (Enable internal logs in Project Settings -> Plugins -> HttpGPT)."),
            FString("Error Details: "),
            FString("\tError Code: ") + Response.Error.Code.ToString(),
            FString("\tError Type: ") + Response.Error.Type.ToString(),
            FString("\tError Message: ") + Response.Error.Message
        };

        OnMessageContentUpdated.ExecuteIfBound(FString::Format(TEXT("{0}\n{1}\n\n{2}\n{3}\n{4}\n{5}"), Arguments_ErrorDetails));
    }
    else if (Response.bSuccess && !HttpGPT::Internal::HasEmptyParam(Response.Choices))
    {
        OnMessageContentUpdated.ExecuteIfBound(Response.Choices[0].Message.Content);
    }
    else
    {
        return;
    }

    if (ScrollBoxReference.IsValid() && bScrollToEnd)
    {
        ScrollBoxReference->ScrollToEnd();
    }
}

void UHttpGPTMessagingHandler::Destroy()
{
    ClearFlags(RF_Standalone);

#if ENGINE_MAJOR_VERSION >= 5
    MarkAsGarbage();
#else
    MarkPendingKill();
#endif
}
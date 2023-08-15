// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <UObject/Object.h>
#include "HttpGPTMessagingHandler.generated.h"

DECLARE_DELEGATE_OneParam(FMessageContentUpdated, FString);

UCLASS(MinimalAPI, NotBlueprintable, NotPlaceable, Category = "Implementation")
class UHttpGPTMessagingHandler : public UObject
{
    GENERATED_BODY()

public:
    explicit UHttpGPTMessagingHandler(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    FMessageContentUpdated OnMessageContentUpdated;

    UFUNCTION()
    void RequestSent();

    UFUNCTION()
    void RequestFailed();

    UFUNCTION()
    void ProcessUpdated(const FHttpGPTChatResponse& Response);

    UFUNCTION()
    void ProcessCompleted(const FHttpGPTChatResponse& Response);

    TSharedPtr<class SScrollBox> ScrollBoxReference;

    void Destroy();

private:
    void ProcessResponse(const FHttpGPTChatResponse& Response);
};
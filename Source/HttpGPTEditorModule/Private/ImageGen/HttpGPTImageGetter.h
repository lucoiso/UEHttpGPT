// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Engine/Texture2D.h>
#include <Tasks/HttpGPTImageRequest.h>
#include "HttpGPTImageGetter.generated.h"

DECLARE_DELEGATE_OneParam(FImageGenerated, UTexture2D*);
DECLARE_DELEGATE_OneParam(FImageStatusChanged, FString);

UCLASS(MinimalAPI, NotBlueprintable, NotPlaceable, Category = "Implementation")
class UHttpGPTImageGetter : public UObject
{
    GENERATED_BODY()

public:
    explicit UHttpGPTImageGetter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    FImageGenerated OnImageGenerated;
    FImageStatusChanged OnStatusChanged;

    UFUNCTION()
    void RequestSent();

    UFUNCTION()
    void RequestFailed();

    UFUNCTION()
    void ProcessCompleted(const FHttpGPTImageResponse& Response);

    void Destroy();

    TSharedPtr<class SScrollBox> OutScrollBox;

private:
    void ProcessImage(const FHttpGPTImageData& Data);

    FHttpGPTImageGenerate OnImageGenerated_Internal;

    UFUNCTION()
    void ImageGenerated(UTexture2D* const Texture);

    uint8 GeneratedImages;
    uint8 DataSize;
};
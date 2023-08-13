// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Structures/HttpGPTImageTypes.h>
#include <Tasks/HttpGPTImageRequest.h>
#include <Widgets/Text/STextBlock.h>
#include <Widgets/Text/SMultiLineEditableText.h>
#include <Widgets/Input/STextComboBox.h>
#include <Widgets/Layout/SScrollBox.h>
#include "SHttpGPTImageGenView.generated.h"

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

private:
    void ProcessImage(const FHttpGPTImageData& Data);

    FHttpGPTImageGenerate OnImageGenerated_Internal;

    UFUNCTION()
    void ImageGenerated(UTexture2D* Texture);

    uint8 GeneratedImages = 0u;
    uint8 DataSize = 0u;
};

class SHttpGPTImageGenItemData final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SHttpGPTImageGenItemData) : _Texture()
        {
        }
        SLATE_ARGUMENT(UTexture2D*, Texture)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    FReply HandleSaveButton();
    bool IsSaveEnabled() const;

private:
    TSharedPtr<SImage> Image;
    TSharedPtr<SButton> SaveButton;

    TWeakObjectPtr<UTexture2D> Texture;
};

typedef TSharedPtr<SHttpGPTImageGenItemData> SHttpGPTImageGenItemDataPtr;

class SHttpGPTImageGenItem final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SHttpGPTImageGenItem) : _Prompt(), _Num(), _Size()
        {
        }
        SLATE_ARGUMENT(FString, Prompt)
        SLATE_ARGUMENT(FString, Num)
        SLATE_ARGUMENT(FString, Size)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    ~SHttpGPTImageGenItem();

    TWeakObjectPtr<UHttpGPTImageGetter> HttpGPTImageGetterObject;

private:
    TSharedPtr<STextBlock> Prompt;
    TSharedPtr<STextBlock> Status;
    TSharedPtr<SScrollBox> ItemScrollBox;
    TSharedPtr<SHorizontalBox> ItemViewBox;

    TWeakObjectPtr<class UHttpGPTImageRequest> RequestReference;
};

typedef TSharedPtr<SHttpGPTImageGenItem> SHttpGPTImageGenItemPtr;

class SHttpGPTImageGenView final : public SCompoundWidget
{
public:
    SLATE_USER_ARGS(SHttpGPTImageGenView)
        {
        }
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    FReply HandleSendRequestButton();
    bool IsSendRequestEnabled() const;

    FReply HandleClearViewButton();
    bool IsClearViewEnabled() const;

protected:
    void InitializeImageNumOptions();
    void InitializeImageSizeOptions();

private:
    TSharedPtr<SVerticalBox> ViewBox;
    TSharedPtr<SScrollBox> ViewScrollBox;

    TSharedPtr<SEditableTextBox> InputTextBox;

    TSharedPtr<STextComboBox> ImageNumComboBox;
    TArray<TSharedPtr<FString>> ImageNum;

    TSharedPtr<STextComboBox> ImageSizeComboBox;
    TArray<TSharedPtr<FString>> ImageSize;
};

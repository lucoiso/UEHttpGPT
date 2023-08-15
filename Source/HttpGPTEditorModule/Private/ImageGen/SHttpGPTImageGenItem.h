// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Widgets/SCompoundWidget.h>

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

    TWeakObjectPtr<class UHttpGPTImageGetter> HttpGPTImageGetterObject;

private:
    TSharedRef<SWidget> ConstructContent();

    FString Prompt;

    TSharedPtr<class STextBlock> Status;
    TSharedPtr<class SHorizontalBox> ItemViewBox;

    TWeakObjectPtr<class UHttpGPTImageRequest> RequestReference;
};

typedef TSharedPtr<SHttpGPTImageGenItem> SHttpGPTImageGenItemPtr;
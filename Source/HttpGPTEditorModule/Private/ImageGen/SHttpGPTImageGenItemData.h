// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Widgets/SCompoundWidget.h>

class SHttpGPTImageGenItemData final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SHttpGPTImageGenItemData) : _Texture()
        {
        }
        SLATE_ARGUMENT(class UTexture2D*, Texture)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    FReply HandleSaveButton();
    bool IsSaveEnabled() const;

private:
    TSharedRef<SWidget> ConstructContent();

    TWeakObjectPtr<class UTexture2D> Texture;
};

typedef TSharedPtr<SHttpGPTImageGenItemData> SHttpGPTImageGenItemDataPtr;
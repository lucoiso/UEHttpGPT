// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Widgets/SCompoundWidget.h>

class SHttpGPTImageGenView final : public SCompoundWidget
{
public:
    SLATE_USER_ARGS(SHttpGPTImageGenView)
        {
        }
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    bool IsSendRequestEnabled() const;
    bool IsClearViewEnabled() const;

private:
    TSharedRef<SWidget> ConstructContent();

    FReply HandleSendRequestButton();
    FReply HandleClearViewButton();

    void InitializeImageNumOptions();
    void InitializeImageSizeOptions();

    TSharedPtr<class SVerticalBox> ViewBox;
    TSharedPtr<class SScrollBox> ViewScrollBox;

    TSharedPtr<class SEditableTextBox> InputTextBox;

    TSharedPtr<class STextComboBox> ImageNumComboBox;
    TArray<TSharedPtr<FString>> ImageNum;

    TSharedPtr<class STextComboBox> ImageSizeComboBox;
    TArray<TSharedPtr<FString>> ImageSize;
};

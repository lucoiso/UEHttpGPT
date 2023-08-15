// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Widgets/SCompoundWidget.h>

class SHttpGPTChatView final : public SCompoundWidget
{
public:
    SLATE_USER_ARGS(SHttpGPTChatView)
        {
        }
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    ~SHttpGPTChatView();

    bool IsSendMessageEnabled() const;
    bool IsClearChatEnabled() const;
    FString GetHistoryPath() const;

private:
    TSharedRef<SWidget> ConstructContent();

    FReply HandleSendMessageButton(const EHttpGPTChatRole Role);
    FReply HandleClearChatButton();

    TArray<FHttpGPTChatMessage> GetChatHistory() const;
    FString GetDefaultSystemContext() const;

    void LoadChatHistory();
    void SaveChatHistory() const;

    TSharedPtr<class SVerticalBox> ChatBox;
    TArray<SHttpGPTChatItemPtr> ChatItems;
    TSharedPtr<class SScrollBox> ChatScrollBox;

    TSharedPtr<class SEditableTextBox> InputTextBox;

    TSharedPtr<class STextComboBox> ModelsComboBox;
    TArray<TSharedPtr<FString>> AvailableModels;

    TWeakObjectPtr<class UHttpGPTChatRequest> RequestReference;
};

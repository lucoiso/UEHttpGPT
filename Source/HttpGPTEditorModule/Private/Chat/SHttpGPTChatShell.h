// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Widgets/SCompoundWidget.h>

typedef TSharedPtr<FName> FNamePtr;

class SHttpGPTChatShell final : public SCompoundWidget
{
public:
    SLATE_USER_ARGS(SHttpGPTChatShell)
        {
        }
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    ~SHttpGPTChatShell();

private:
    TSharedRef<SWidget> ConstructContent();

    void InitializeChatSessionOptions();
    void InitializeChatSession(FNamePtr InItem);

    TSharedPtr<class SBox> ShellBox;
    TSharedPtr<class SHttpGPTChatView> CurrentView;

    TSharedPtr<class SListView<FNamePtr>> ChatSessionListView;
    TArray<FNamePtr> ChatSessions;

    TSharedRef<ITableRow> OnGenerateChatSessionRow(FNamePtr InItem, const TSharedRef<STableViewBase>& OwnerTable);

    void OnChatSessionSelectionChanged(FNamePtr InItem, ESelectInfo::Type SelectInfo);
    void OnChatSessionNameChanged(FNamePtr InItem, const FName& NewName);
    void OnChatSessionDoubleClicked(FNamePtr InItem);
    FReply OnChatSessionKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);
};

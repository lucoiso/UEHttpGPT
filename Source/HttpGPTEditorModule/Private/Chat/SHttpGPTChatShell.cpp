// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "SHttpGPTChatShell.h"
#include "SHttpGPTChatView.h"
#include <Widgets/Views/SListView.h>
#include <Widgets/Text/SInlineEditableTextBlock.h>
#include <Widgets/Input/STextEntryPopup.h>

typedef TDelegate<void(FNamePtr, const FName&)> FOnChatSessionNameChanged;

class SHttpGPTChatSessionOption : public STableRow<FNamePtr>
{
public:
    SLATE_BEGIN_ARGS(SHttpGPTChatSessionOption)
        {
        }
        SLATE_EVENT(FOnChatSessionNameChanged, OnNameChanged)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, FNamePtr InItem)
    {
        OnNameChanged = InArgs._OnNameChanged;
        Item = InItem;

        STableRow<FNamePtr>::Construct(STableRow<FNamePtr>::FArguments()
            .Padding(8.f)
            .Content()
            [
                SAssignNew(SessionName, STextBlock)
                    .Text(this, &SHttpGPTChatSessionOption::GetName)
                    .ToolTipText(this, &SHttpGPTChatSessionOption::GetName)
            ], InOwnerTableView);
    }

    FText GetName() const
    {
        return FText::FromName(Item.IsValid() ? *Item : TEXT("Invalid"));
    }

    void EnableEditMode()
    {
        TSharedRef<STextEntryPopup> TextEntry =
            SNew(STextEntryPopup)
            .Label(FText::FromString("Rename Session"))
            .OnTextCommitted(this, &SHttpGPTChatSessionOption::OnNameCommited);

        FSlateApplication& SlateApp = FSlateApplication::Get();

        SlateApp.PushMenu(
            AsShared(),
            FWidgetPath(),
            TextEntry,
            SlateApp.GetCursorPos(),
            FPopupTransitionEffect::TypeInPopup
        );
    }

private:
    FNamePtr Item;
    FOnChatSessionNameChanged OnNameChanged;

    TSharedPtr<STextBlock> SessionName;

    void OnNameCommited(const FText& NewText, ETextCommit::Type CommitInfo)
    {
        if (!SessionName.IsValid())
        {
            return;
        }

        if (CommitInfo == ETextCommit::OnEnter)
        {
            OnNameChanged.ExecuteIfBound(Item, FName(*NewText.ToString()));
            *Item = *NewText.ToString();

            FSlateApplication::Get().DismissAllMenus();
        }
        else if (CommitInfo == ETextCommit::OnCleared)
        {
            FSlateApplication::Get().DismissAllMenus();
        }
    }
};

void SHttpGPTChatShell::Construct([[maybe_unused]] const FArguments&)
{
    ChildSlot
        [
            ConstructContent()
        ];

    InitializeChatSessionOptions();
}

SHttpGPTChatShell::~SHttpGPTChatShell() = default;

TSharedRef<SWidget> SHttpGPTChatShell::ConstructContent()
{
    return SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .FillWidth(0.2f)
        [
            SAssignNew(ChatSessionListView, SListView<FNamePtr>)
                .ListItemsSource(&ChatSessions)
                .OnGenerateRow(this, &SHttpGPTChatShell::OnGenerateChatSessionRow)
                .OnSelectionChanged(this, &SHttpGPTChatShell::OnChatSessionSelectionChanged)
                .SelectionMode(ESelectionMode::Single)
                .ClearSelectionOnClick(false)
                .OnMouseButtonDoubleClick(this, &SHttpGPTChatShell::OnChatSessionDoubleClicked)
                .OnKeyDownHandler(this, &SHttpGPTChatShell::OnChatSessionKeyDown)

        ]
        + SHorizontalBox::Slot()
        .FillWidth(0.8f)
        [
            SAssignNew(ShellBox, SBox)
                .HAlign(HAlign_Fill)
                .VAlign(VAlign_Fill)
        ];
}

void SHttpGPTChatShell::InitializeChatSessionOptions()
{
    ChatSessions.Empty();

    if (const FString SessionsPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("HttpGPT")); FPaths::DirectoryExists(SessionsPath))
    {
        TArray<FString> FoundFiles;
        IFileManager::Get().FindFilesRecursive(FoundFiles, *SessionsPath, TEXT("*.json"), true, false, true);

        TArray<FString> FoundBaseFileNames;
        Algo::Transform(FoundFiles, FoundBaseFileNames, [](const FString& Iterator) { return FPaths::GetBaseFilename(Iterator); });

        for (const FString& FileIt : FoundBaseFileNames)
        {
            ChatSessions.EmplaceAt(FileIt.Equals(NewSessionName.ToString()) ? 0 : ChatSessions.Num(), MakeShared<FName>(FileIt));
        }

        if (FoundBaseFileNames.IsEmpty() || !FoundBaseFileNames.Contains(NewSessionName.ToString()))
        {
            ChatSessions.EmplaceAt(0, MakeShared<FName>(NewSessionName));
        }

        InitializeChatSession(ChatSessions[0]);
    }

    if (ChatSessionListView.IsValid())
    {
        ChatSessionListView->RequestListRefresh();
    }
}

void SHttpGPTChatShell::InitializeChatSession(FNamePtr InItem)
{
    if (!ShellBox.IsValid())
    {
        return;
    }

    ShellBox->SetContent(SAssignNew(CurrentView, SHttpGPTChatView).SessionID(*InItem));

    if (ChatSessionListView.IsValid())
    {
        if (!ChatSessionListView->IsItemSelected(InItem))
        {
            ChatSessionListView->SetSelection(InItem);
        }

        ChatSessionListView->RequestListRefresh();
    }
}

TSharedRef<ITableRow> SHttpGPTChatShell::OnGenerateChatSessionRow(FNamePtr InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(SHttpGPTChatSessionOption, OwnerTable, InItem)
        .OnNameChanged(this, &SHttpGPTChatShell::OnChatSessionNameChanged);
}

void SHttpGPTChatShell::OnChatSessionSelectionChanged(FNamePtr InItem, [[maybe_unused]] ESelectInfo::Type SelectInfo)
{
    if (!InItem.IsValid())
    {
        return;
    }

    InitializeChatSession(InItem);
}

void SHttpGPTChatShell::OnChatSessionNameChanged(FNamePtr InItem, const FName& NewName)
{
    if (!InItem.IsValid())
    {
        return;
    }

    if (InItem->IsEqual(NewSessionName))
    {
        ChatSessions.EmplaceAt(0, MakeShared<FName>(NewSessionName));
    }

    if (const FString SessionPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("HttpGPT"), InItem->ToString()); FPaths::FileExists(SessionPath))
    {
        const FString NewPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("HttpGPT"), NewName.ToString());
        IFileManager::Get().Move(*SessionPath, *NewPath, true, true, false, false);
    }

    if (!CurrentView.IsValid())
    {
        return;
    }

    if (CurrentView.IsValid() && CurrentView->GetSessionID().IsEqual(*InItem))
    {
        CurrentView->SetSessionID(NewName);
    }

    *InItem = NewName;

    if (ChatSessionListView.IsValid())
    {
        ChatSessionListView->RequestListRefresh();
    }
}

void SHttpGPTChatShell::OnChatSessionDoubleClicked(FNamePtr InItem)
{
    if (!InItem.IsValid() || !ChatSessionListView.IsValid())
    {
        return;
    }

    if (const TSharedPtr<ITableRow> Row = ChatSessionListView->WidgetFromItem(InItem); Row.IsValid())
    {
        if (const TSharedPtr<SHttpGPTChatSessionOption> Session = StaticCastSharedPtr<SHttpGPTChatSessionOption>(Row); Session.IsValid())
        {
            Session->EnableEditMode();
            ChatSessionListView->RequestListRefresh();
        }
    }
}

FReply SHttpGPTChatShell::OnChatSessionKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
    if (!ChatSessionListView.IsValid())
    {
        return FReply::Unhandled();
    }

    if (InKeyEvent.GetKey() != EKeys::Delete)
    {
        return FReply::Unhandled();
    }

    if (FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString("Are you sure you want to delete this session?")) == EAppReturnType::No)
    {
        return FReply::Unhandled();
    }

    const FNamePtr SelectedItem = ChatSessionListView->GetNumItemsSelected() == 0 ? nullptr : ChatSessionListView->GetSelectedItems()[0];
    if (!SelectedItem.IsValid())
    {
        return FReply::Unhandled();
    }

    if (CurrentView.IsValid())
    {
        CurrentView->ClearChat();
    }

    if (const FString SessionPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("HttpGPT"), SelectedItem->ToString() + TEXT(".json")); FPaths::FileExists(SessionPath))
    {
        IFileManager::Get().Delete(*SessionPath, true, true);
    }

    if (SelectedItem->IsEqual(NewSessionName) || !ChatSessions.ContainsByPredicate([](const FNamePtr& Item) { return Item->IsEqual(NewSessionName); }))
    {
        ChatSessions.EmplaceAt(0, MakeShared<FName>(NewSessionName));
    }

    ChatSessions.Remove(SelectedItem);

    if (ChatSessionListView.IsValid())
    {
        ChatSessionListView->RequestListRefresh();
        ChatSessionListView->SetSelection(ChatSessions[0]);
    }

    return FReply::Handled();
}
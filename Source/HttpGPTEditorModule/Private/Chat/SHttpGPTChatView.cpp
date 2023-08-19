// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "SHttpGPTChatView.h"
#include "HttpGPTMessagingHandler.h"
#include <Tasks/HttpGPTChatRequest.h>
#include <Management/HttpGPTSettings.h>
#include <Utils/HttpGPTHelper.h>
#include <HttpGPTInternalFuncs.h>
#include <Interfaces/IPluginManager.h>
#include <Dom/JsonObject.h>
#include <Serialization/JsonWriter.h>
#include <Serialization/JsonReader.h>
#include <Serialization/JsonSerializer.h>
#include <Misc/FileHelper.h>
#include <Widgets/Layout/SScrollBox.h>
#include <Widgets/Input/STextComboBox.h>

void SHttpGPTChatView::Construct(const FArguments& InArgs)
{
    SetSessionID(InArgs._SessionID);

    ModelsComboBox = SNew(STextComboBox)
        .OptionsSource(&AvailableModels)
        .ToolTipText(FText::FromString(TEXT("GPT Model")));

    for (const FName& ModelName : UHttpGPTHelper::GetAvailableGPTModels())
    {
        AvailableModels.Add(MakeShared<FString>(ModelName.ToString()));

        if (ModelsComboBox.IsValid() && ModelName.IsEqual(UHttpGPTHelper::ModelToName(EHttpGPTChatModel::gpt35turbo)))
        {
            ModelsComboBox->SetSelectedItem(AvailableModels.Top());
        }
    }

    ChildSlot
        [
            ConstructContent()
        ];

    LoadChatHistory();
}

SHttpGPTChatView::~SHttpGPTChatView()
{
    SaveChatHistory();
}

bool SHttpGPTChatView::IsSendMessageEnabled() const
{
    return (!RequestReference.IsValid() || !UHttpGPTTaskStatus::IsTaskActive(RequestReference.Get())) && !HttpGPT::Internal::HasEmptyParam(InputTextBox->GetText());
}

bool SHttpGPTChatView::IsClearChatEnabled() const
{
    return !HttpGPT::Internal::HasEmptyParam(ChatItems);
}

FString SHttpGPTChatView::GetHistoryPath() const
{
    return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("HttpGPT"), SessionID.ToString() + TEXT(".json"));
}

void SHttpGPTChatView::SetSessionID(const FName& NewSessionID)
{
    const FName NewValidSessionID = *FPaths::MakeValidFileName(NewSessionID.ToString());
    if (SessionID == NewValidSessionID)
    {
        return;
    }

    if (SessionID.IsNone())
    {
        SessionID = NewValidSessionID;
        return;
    }

    if (const FString OldPath = GetHistoryPath(); FPaths::FileExists(OldPath))
    {
        IFileManager::Get().Delete(*OldPath, true, true, true);
    }

    SessionID = NewValidSessionID;
    SaveChatHistory();
}

FName SHttpGPTChatView::GetSessionID() const
{
    return SessionID;
}

void SHttpGPTChatView::ClearChat()
{
    ChatItems.Empty();
    if (ChatBox.IsValid())
    {
        ChatBox->ClearChildren();
    }

    if (RequestReference.IsValid())
    {
        RequestReference->StopHttpGPTTask();
    }
    else
    {
        RequestReference.Reset();
    }
}

TSharedRef<SWidget> SHttpGPTChatView::ConstructContent()
{
    constexpr float SlotPadding = 4.0f;

    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .Padding(SlotPadding)
        .FillHeight(1.f)
        [
            SAssignNew(ChatScrollBox, SScrollBox)
                + SScrollBox::Slot()
                [
                    SAssignNew(ChatBox, SVerticalBox)
                ]
        ]
        + SVerticalBox::Slot()
        .Padding(SlotPadding)
        .AutoHeight()
        [
            SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .Padding(SlotPadding)
                .FillWidth(1.f)
                [
                    SAssignNew(InputTextBox, SEditableTextBox)
                        .AllowContextMenu(true)
                        .IsReadOnly(false)
                        .OnTextCommitted_Lambda(
                            [this]([[maybe_unused]] const FText& Text, ETextCommit::Type CommitType)
                            {
                                if (IsSendMessageEnabled() && CommitType == ETextCommit::OnEnter)
                                {
                                    HandleSendMessageButton(EHttpGPTChatRole::User);
                                }
                            }
                        )
                ]
                + SHorizontalBox::Slot()
                .Padding(SlotPadding)
                .AutoWidth()
                [
                    SNew(SButton)
                        .Text(FText::FromString(TEXT("Send")))
                        .ToolTipText(FText::FromString(TEXT("Send Message")))
                        .OnClicked(this, &SHttpGPTChatView::HandleSendMessageButton, EHttpGPTChatRole::User)
                        .IsEnabled(this, &SHttpGPTChatView::IsSendMessageEnabled)
                ]
                + SHorizontalBox::Slot()
                .Padding(SlotPadding)
                .AutoWidth()
                [
                    SNew(SButton)
                        .Text(FText::FromString(TEXT("System")))
                        .ToolTipText(FText::FromString(TEXT("Send Message as System Context")))
                        .OnClicked(this, &SHttpGPTChatView::HandleSendMessageButton, EHttpGPTChatRole::System)
                        .IsEnabled(this, &SHttpGPTChatView::IsSendMessageEnabled)
                ]
                + SHorizontalBox::Slot()
                .Padding(SlotPadding)
                .AutoWidth()
                [
                    ModelsComboBox.ToSharedRef()
                ]
                + SHorizontalBox::Slot()
                .Padding(SlotPadding)
                .AutoWidth()
                [
                    SNew(SButton)
                        .Text(FText::FromString(TEXT("Clear")))
                        .ToolTipText(FText::FromString(TEXT("Clear Chat History")))
                        .OnClicked(this, &SHttpGPTChatView::HandleClearChatButton)
                        .IsEnabled(this, &SHttpGPTChatView::IsClearChatEnabled)
                ]
        ];
}

FReply SHttpGPTChatView::HandleSendMessageButton(const EHttpGPTChatRole Role)
{
    const SHttpGPTChatItemPtr NewMessage = SNew(SHttpGPTChatItem)
        .MessageRole(Role)
        .InputText(InputTextBox->GetText().ToString());

    ChatBox->AddSlot()
        .AutoHeight()
        [
            NewMessage.ToSharedRef()
        ];
    ChatItems.Add(NewMessage);

    if (Role == EHttpGPTChatRole::System)
    {
        ChatScrollBox->ScrollToEnd();
        InputTextBox->SetText(FText::GetEmpty());

        return FReply::Handled();
    }

    const SHttpGPTChatItemPtr AssistantMessage = SNew(SHttpGPTChatItem)
        .MessageRole(EHttpGPTChatRole::Assistant)
        .ScrollBox(ChatScrollBox);

    FHttpGPTChatOptions Options;
    Options.Model = UHttpGPTHelper::NameToModel(*(*ModelsComboBox->GetSelectedItem().Get()));
    Options.bStream = true;

    RequestReference = UHttpGPTChatRequest::EditorTask(GetChatHistory(), Options);
    RequestReference->ProgressStarted.AddDynamic(AssistantMessage->MessagingHandlerObject.Get(), &UHttpGPTMessagingHandler::ProcessUpdated);
    RequestReference->ProgressUpdated.AddDynamic(AssistantMessage->MessagingHandlerObject.Get(), &UHttpGPTMessagingHandler::ProcessUpdated);
    RequestReference->ProcessCompleted.AddDynamic(AssistantMessage->MessagingHandlerObject.Get(), &UHttpGPTMessagingHandler::ProcessCompleted);
    RequestReference->ErrorReceived.AddDynamic(AssistantMessage->MessagingHandlerObject.Get(), &UHttpGPTMessagingHandler::ProcessCompleted);
    RequestReference->RequestFailed.AddDynamic(AssistantMessage->MessagingHandlerObject.Get(), &UHttpGPTMessagingHandler::RequestFailed);
    RequestReference->RequestSent.AddDynamic(AssistantMessage->MessagingHandlerObject.Get(), &UHttpGPTMessagingHandler::RequestSent);
    RequestReference->Activate();

    ChatBox->AddSlot()
        .AutoHeight()
        [
            AssistantMessage.ToSharedRef()
        ];
    ChatItems.Add(AssistantMessage);

    ChatScrollBox->ScrollToEnd();
    InputTextBox->SetText(FText::GetEmpty());

    return FReply::Handled();
}

FReply SHttpGPTChatView::HandleClearChatButton()
{
    ClearChat();
    return FReply::Handled();
}

TArray<FHttpGPTChatMessage> SHttpGPTChatView::GetChatHistory() const
{
    TArray<FHttpGPTChatMessage> Output
    {
        FHttpGPTChatMessage(EHttpGPTChatRole::System, GetDefaultSystemContext())
    };

    for (const SHttpGPTChatItemPtr& Item : ChatItems)
    {
        FString RoleText = Item->GetRoleText();
        RoleText.RemoveFromEnd(TEXT(":"));
        Output.Add(FHttpGPTChatMessage(*RoleText, Item->GetMessageText()));
    }

    return Output;
}

FString SHttpGPTChatView::GetDefaultSystemContext() const
{
    if (const UHttpGPTSettings* const Settings = UHttpGPTSettings::Get(); Settings->bUseCustomSystemContext)
    {
        return Settings->CustomSystemContext;
    }

    FString SupportedModels;
    for (const FTextDisplayStringPtr& Model : AvailableModels)
    {
        SupportedModels.Append(*Model.Get() + ", ");
    }
    SupportedModels.RemoveFromEnd(", ");

    const TSharedPtr<IPlugin> PluginInterface = IPluginManager::Get().FindPlugin("HttpGPT");

    const FString PluginShortName = "HttpGPT";
    const FString EngineVersion = FString::Printf(TEXT("%d.%d"), ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION);

    const FStringFormatOrderedArguments Arguments_PluginInfo{
        EngineVersion,
        PluginShortName,
        PluginInterface->GetDescriptor().VersionName,
        PluginInterface->GetDescriptor().CreatedBy,
        PluginInterface->GetDescriptor().Description
    };

    const FString PluginInformation = FString::Format(TEXT("You are in the Unreal Engine {0} plugin {1} version {2}, which was developed by {3}. The description of HttpGPT is: \"{4}\""), Arguments_PluginInfo);

    const FStringFormatOrderedArguments Arguments_SupportInfo{
        PluginInterface->GetDescriptor().DocsURL,
        PluginInterface->GetDescriptor().SupportURL,
    };

    const FString PluginSupport = FString::Format(TEXT("You can find the HttpGPT documentation at {0} and support at {1}."), Arguments_SupportInfo);

    const FStringFormatOrderedArguments Arguments_Models{
        *ModelsComboBox->GetSelectedItem().Get(),
        SupportedModels
    };

    const FString ModelsInformation = FString::Format(TEXT("You're using the model {0} and HttpGPT currently supports all these OpenAI Models: {1}."), Arguments_Models);

    const FStringFormatOrderedArguments Arguments_EngineDocumentation{
        EngineVersion
    };

    const FString EngineDocumentation_General = FString::Format(TEXT("You can find the Unreal Engine {0} general documentation at https://docs.unrealengine.com/{0}/en-US/."), Arguments_EngineDocumentation);
    const FString EngineDocumentation_CPP = FString::Format(TEXT("You can find the Unreal Engine {0} API documentation for C++ at https://docs.unrealengine.com/{0}/en-US/API/."), Arguments_EngineDocumentation);
    const FString EngineDocumentation_BP = FString::Format(TEXT("You can find the Unreal Engine {0} API documentation for Blueprints at https://docs.unrealengine.com/{0}/en-US/BlueprintAPI/."), Arguments_EngineDocumentation);

    const FStringFormatOrderedArguments Arguments_SystemContext{
        PluginInformation,
        PluginSupport,
        ModelsInformation,
        EngineDocumentation_General,
        EngineDocumentation_CPP,
        EngineDocumentation_BP
    };

    return FString::Format(TEXT("You are an assistant that will help with the development of projects in Unreal Engine in general.\n{0}\n{1}\n{2}\n{3}\n{4}\n{5}"), Arguments_SystemContext);
}

void SHttpGPTChatView::LoadChatHistory()
{
    if (SessionID.IsNone())
    {
        return;
    }

    if (const FString LoadPath = GetHistoryPath(); FPaths::FileExists(LoadPath))
    {
        FString FileContent;
        if (!FFileHelper::LoadFileToString(FileContent, *LoadPath))
        {
            return;
        }

        TSharedPtr<FJsonObject> JsonParsed;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContent);
        if (FJsonSerializer::Deserialize(Reader, JsonParsed))
        {
            const TArray<TSharedPtr<FJsonValue>> SessionData = JsonParsed->GetArrayField("Data");
            for (const TSharedPtr<FJsonValue>& Item : SessionData)
            {
                if (const TSharedPtr<FJsonObject> MessageItObj = Item->AsObject())
                {
                    if (FString RoleString; MessageItObj->TryGetStringField("role", RoleString))
                    {
                        const EHttpGPTChatRole Role = UHttpGPTHelper::NameToRole(*RoleString);

                        if (FString Message; MessageItObj->TryGetStringField("content", Message))
                        {
                            if (Role == EHttpGPTChatRole::System && Message == GetDefaultSystemContext())
                            {
                                continue;
                            }

                            ChatItems.Emplace(
                                SNew(SHttpGPTChatItem)
                                .MessageRole(Role)
                                .InputText(Message)
                                .ScrollBox(ChatScrollBox)
                            );
                        }
                    }
                }
            }
        }
    }

    for (const SHttpGPTChatItemPtr& Item : ChatItems)
    {
        ChatBox->AddSlot()
            .AutoHeight()
            [
                Item.ToSharedRef()
            ];
    }
}

void SHttpGPTChatView::SaveChatHistory() const
{
    if (SessionID.IsNone())
    {
        return;
    }

    const TSharedPtr<FJsonObject> JsonRequest = MakeShared<FJsonObject>();

    TArray<TSharedPtr<FJsonValue>> Data;
    for (const FHttpGPTChatMessage& Item : GetChatHistory())
    {
        Data.Add(Item.GetMessage());
    }

    JsonRequest->SetArrayField("Data", Data);

    FString RequestContentString;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestContentString);

    if (FJsonSerializer::Serialize(JsonRequest.ToSharedRef(), Writer))
    {
        FFileHelper::SaveStringToFile(RequestContentString, *GetHistoryPath());
    }
}
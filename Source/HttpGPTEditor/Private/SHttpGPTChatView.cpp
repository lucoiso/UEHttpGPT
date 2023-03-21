// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "SHttpGPTChatView.h"
#include <HttpGPTHelper.h>
#include <Interfaces/IPluginManager.h>
#include <Widgets/Layout/SScrollBox.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(SHttpGPTChatView)
#endif

constexpr float Slot_Padding = 4.0f;

UHttpGPTMessagingHandler::UHttpGPTMessagingHandler(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UHttpGPTMessagingHandler::RequestSent()
{
	Message.Content = "Waiting Response...";
}

void UHttpGPTMessagingHandler::RequestFailed()
{
	Message.Content = "Request Failed. Please check the logs. (Enable internal logs in Project Settings -> Plugins -> HttpGPT)";
}

void UHttpGPTMessagingHandler::ResponseReceived(const FHttpGPTResponse& Response)
{
	if (Response.Choices.IsEmpty())
	{
		return;
	}

	if (Response.bSuccess)
	{
		Message = Response.Choices[0].Message;
	}
	else
	{
		Message.Content = Response.Error.Message;
	}
}

void SHttpGPTChatItem::Construct(const FArguments& InArgs)
{
	MessagingHandlerObject = NewObject<UHttpGPTMessagingHandler>();
	MessagingHandlerObject->Message = FHttpGPTMessage(InArgs._MessageRole, InArgs._InputText);

#if ENGINE_MAJOR_VERSION < 5
	using FAppStyle = FEditorStyle;
#endif

	const ISlateStyle& AppStyle = FAppStyle::Get();

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(MessagingHandlerObject->Message.Role == EHttpGPTRole::User ? FMargin(Slot_Padding * 16.f, Slot_Padding, Slot_Padding, Slot_Padding) : FMargin(Slot_Padding, Slot_Padding, Slot_Padding * 16.f, Slot_Padding))
		[
			SNew(SBorder)
			.BorderImage(AppStyle.GetBrush("Menu.Background"))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(Slot_Padding)
				.AutoHeight()
				[
					SNew(STextBlock)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
					.Text(FText::FromString(MessagingHandlerObject->Message.Role == EHttpGPTRole::User ? "User:" : "Assistant:"))
				]
				+ SVerticalBox::Slot()
				.Padding(FMargin(Slot_Padding * 4, Slot_Padding, Slot_Padding, Slot_Padding))
				.FillHeight(1.f)
				[
					SAssignNew(MessageBox, STextBlock)
					.AutoWrapText(true)
					.Text(this, &SHttpGPTChatItem::GetMessageText)
				]
			]
		]
	];
}

FText SHttpGPTChatItem::GetMessageText() const
{
	return FText::FromString(MessagingHandlerObject->Message.Content);
}

void SHttpGPTChatView::Construct([[maybe_unused]] const FArguments&)
{
#if ENGINE_MAJOR_VERSION < 5
	using FAppStyle = FEditorStyle;
#endif

	const ISlateStyle& AppStyle = FAppStyle::Get();

	ModelsComboBox = SNew(STextComboBox)
		.OptionsSource(&AvailableModels)
		.ToolTipText(FText::FromString("Selected GPT Model"));

	InitializeModelsOptions();

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(Slot_Padding)
		.FillHeight(1.f)
		[
			SNew(SBorder)
			.BorderImage(AppStyle.GetBrush("NoBorder"))
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SAssignNew(ChatBox, SVerticalBox)
				]
			]
		]
		+ SVerticalBox::Slot()
		.Padding(Slot_Padding)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(Slot_Padding)
			.FillWidth(1.f)
			[
				SAssignNew(InputTextBox, SEditableTextBox)
			]
			+ SHorizontalBox::Slot()
			.Padding(Slot_Padding)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString("Send"))
				.ToolTipText(FText::FromString("Send Message"))
				.OnClicked(this, &SHttpGPTChatView::HandleSendMessageButton)
				.IsEnabled(this, &SHttpGPTChatView::IsSendMessageEnabled)
			]
			+ SHorizontalBox::Slot()
			.Padding(Slot_Padding)
			.AutoWidth()
			[
				ModelsComboBox.ToSharedRef()
			]
			+ SHorizontalBox::Slot()
			.Padding(Slot_Padding)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString("Clear"))
				.ToolTipText(FText::FromString("Clear Chat History"))
				.OnClicked(this, &SHttpGPTChatView::HandleClearChatButton)
				.IsEnabled(this, &SHttpGPTChatView::IsClearChatEnabled)
			]
		]
	];
}

FReply SHttpGPTChatView::HandleSendMessageButton()
{
	SHttpGPTChatItemPtr UserMessage = SNew(SHttpGPTChatItem).MessageRole(EHttpGPTRole::User).InputText(InputTextBox->GetText().ToString());
	ChatBox->AddSlot().AutoHeight() [ UserMessage.ToSharedRef() ];
	ChatItems.Add(UserMessage);

	SHttpGPTChatItemPtr AssistantMessage = SNew(SHttpGPTChatItem).MessageRole(EHttpGPTRole::Assistant);

	FHttpGPTOptions Options;
	Options.Model = UHttpGPTHelper::NameToModel(*(*ModelsComboBox->GetSelectedItem().Get()));
	Options.bStream = true;

	RequestReference = UHttpGPTRequest::SendMessages_CustomOptions(GEditor->GetEditorWorldContext().World(), GetChatHistory(), Options);

	RequestReference->ProgressStarted.AddDynamic(AssistantMessage->MessagingHandlerObject, &UHttpGPTMessagingHandler::ResponseReceived);
	RequestReference->ProgressUpdated.AddDynamic(AssistantMessage->MessagingHandlerObject, &UHttpGPTMessagingHandler::ResponseReceived);
	RequestReference->ProcessCompleted.AddDynamic(AssistantMessage->MessagingHandlerObject, &UHttpGPTMessagingHandler::ResponseReceived);
	RequestReference->ErrorReceived.AddDynamic(AssistantMessage->MessagingHandlerObject, &UHttpGPTMessagingHandler::ResponseReceived);
	RequestReference->RequestFailed.AddDynamic(AssistantMessage->MessagingHandlerObject, &UHttpGPTMessagingHandler::RequestFailed);
	RequestReference->RequestSent.AddDynamic(AssistantMessage->MessagingHandlerObject, &UHttpGPTMessagingHandler::RequestSent);

	RequestReference->Activate();

	if (RequestReference->IsTaskActive())
	{
		ChatBox->AddSlot().AutoHeight() [AssistantMessage.ToSharedRef()];
		ChatItems.Add(AssistantMessage);
	}

	InputTextBox->SetText(FText::GetEmpty());

	return FReply::Handled();
}

bool SHttpGPTChatView::IsSendMessageEnabled() const
{
	return (!IsValid(RequestReference) || !RequestReference->IsTaskActive()) && !InputTextBox->GetText().IsEmptyOrWhitespace();
}

FReply SHttpGPTChatView::HandleClearChatButton()
{
	ChatItems.Empty();
	ChatBox->ClearChildren();

	if (RequestReference)
	{
		RequestReference->StopHttpGPTTask();
	}

	return FReply::Handled();
}

bool SHttpGPTChatView::IsClearChatEnabled() const
{
	return !ChatItems.IsEmpty();
}

TArray<FHttpGPTMessage> SHttpGPTChatView::GetChatHistory() const
{
	TArray<FHttpGPTMessage> Output
	{
		FHttpGPTMessage(EHttpGPTRole::System, GetSystemContext())
	};

	for (const auto& Item : ChatItems)
	{
		Output.Add(Item->MessagingHandlerObject->Message);
	}

	return Output;
}

FString SHttpGPTChatView::GetSystemContext() const
{
	const TSharedPtr<IPlugin> PluginInterface = IPluginManager::Get().FindPlugin("HttpGPT");

	const FStringFormatOrderedArguments Arguments {
		FString::Printf(TEXT("%d.%d"), ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION),
		"HttpGPT",
		PluginInterface->GetDescriptor().VersionName,
		PluginInterface->GetDescriptor().CreatedBy,
		PluginInterface->GetDescriptor().DocsURL,
		PluginInterface->GetDescriptor().SupportURL
	};

	return FString::Format(TEXT("You are in the Unreal Engine {0} plugin {1} version {2}, which was developed by {3}. You can find the documentation at {4} and support at {5}. You are an assistant that will help with the development of projects in Unreal Engine in general."), Arguments);
}

void SHttpGPTChatView::InitializeModelsOptions()
{
	for (const FName& ModelName : UHttpGPTHelper::GetAvailableGPTModels())
	{
		AvailableModels.Add(MakeShared<FString>(ModelName.ToString()));

		if (ModelsComboBox.IsValid() && ModelName.IsEqual(UHttpGPTHelper::ModelToName(EHttpGPTModel::gpt35turbo)))
		{
			ModelsComboBox->SetSelectedItem(AvailableModels.Top());
		}
	}
}

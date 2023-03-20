// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "SHttpGPTChatView.h"
#include <HttpGPTHelper.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(SHttpGPTChatView)
#endif

constexpr float Slot_Padding = 4.0f;

UHttpGPTMessagingHandler::UHttpGPTMessagingHandler(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
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

void SHttpGPTChatView::Construct([[maybe_unused]] const FArguments&)
{
	ModelsComboBox = SNew(STextComboBox)
		.OptionsSource(&AvailableModels)
		.ToolTipText(FText::FromString("Select Model"));

	InitializeModelsOptions();

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(Slot_Padding)
		.FillHeight(1.f)
		[
			SAssignNew(ListView, SListView<UHttpGPTMessagingHandlerPtr>)
			.ListItemsSource(&ListItems)
			.SelectionMode(ESelectionMode::None)
			.OnGenerateRow(this, &SHttpGPTChatView::OnGenerateRow)
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

TSharedRef<ITableRow> SHttpGPTChatView::OnGenerateRow(UHttpGPTMessagingHandlerPtr Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SHttpGPTChatItem, OwnerTable, Item);
}

FReply SHttpGPTChatView::HandleSendMessageButton()
{
	UHttpGPTMessagingHandlerPtr UserMessage = NewObject<UHttpGPTMessagingHandler>();
	UserMessage->Message = FHttpGPTMessage(EHttpGPTRole::User, InputTextBox->GetText().ToString());

	ListItems.Add(UserMessage);

	UHttpGPTMessagingHandlerPtr NewMessage = NewObject<UHttpGPTMessagingHandler>();
	NewMessage->Message.Role = EHttpGPTRole::Assistant;

	FHttpGPTOptions Options;
	Options.Model = UHttpGPTHelper::NameToModel(*(*ModelsComboBox->GetSelectedItem().Get()));
	Options.bStream = true;

	RequestReference = UHttpGPTRequest::SendMessages_CustomOptions(GEditor->GetEditorWorldContext().World(), GetChatHistory(), Options);

	RequestReference->ProgressUpdated.AddDynamic(NewMessage, &UHttpGPTMessagingHandler::ResponseReceived);
	RequestReference->ProcessCompleted.AddDynamic(NewMessage, &UHttpGPTMessagingHandler::ResponseReceived);
	RequestReference->ErrorReceived.AddDynamic(NewMessage, &UHttpGPTMessagingHandler::ResponseReceived);

	RequestReference->Activate();

	if (RequestReference->IsTaskActive())
	{
		ListItems.Add(NewMessage);
	}

	InputTextBox->SetText(FText::GetEmpty());

	ListView->RequestListRefresh();

	return FReply::Handled();
}

bool SHttpGPTChatView::IsSendMessageEnabled() const
{
	return (!IsValid(RequestReference) || !RequestReference->IsTaskActive()) && !InputTextBox->GetText().IsEmptyOrWhitespace();
}

FReply SHttpGPTChatView::HandleClearChatButton()
{
	ListItems.Empty();

	ListView->RequestListRefresh();

	if (RequestReference)
	{
		RequestReference->StopHttpGPTTask();
	}

	return FReply::Handled();
}

bool SHttpGPTChatView::IsClearChatEnabled() const
{
	return !ListItems.IsEmpty();
}

TArray<FHttpGPTMessage> SHttpGPTChatView::GetChatHistory() const
{
	TArray<FHttpGPTMessage> Output
	{
		FHttpGPTMessage(EHttpGPTRole::System, "You are in an Unreal Engine 5.1 plugin called HttpGPT, which was developed by Lucas Vilas-Boas. You are an assistant that will help with the development of projects in Unreal Engine in general.")
	};

	for (const auto& Item : ListItems)
	{
		Output.Add(Item->Message);
	}

	return Output;
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

void SHttpGPTChatItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, const UHttpGPTMessagingHandlerPtr InMessagingHandlerObject)
{
#if ENGINE_MAJOR_VERSION < 5
	using FAppStyle = FEditorStyle;
#endif

	const ISlateStyle& AppStyle = FAppStyle::Get();

	MessagingHandlerObject = InMessagingHandlerObject;

	STableRow<UHttpGPTMessagingHandlerPtr>::Construct(STableRow<UHttpGPTMessagingHandlerPtr>::FArguments(), InOwnerTableView);

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(MessagingHandlerObject->Message.Role == EHttpGPTRole::User ? FMargin(Slot_Padding * 64.f, Slot_Padding, Slot_Padding, Slot_Padding) : FMargin(Slot_Padding, Slot_Padding, Slot_Padding * 64.f, Slot_Padding))
		[
			SNew(SBorder)
			.BorderImage(AppStyle.GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(Slot_Padding)
				.AutoHeight()
				[
					SNew(STextBlock)
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

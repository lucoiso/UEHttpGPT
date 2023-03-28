// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "SHttpGPTChatView.h"
#include <HttpGPTHelper.h>
#include <HttpGPTInternalFuncs.h>
#include <Interfaces/IPluginManager.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(SHttpGPTChatView)
#endif

constexpr float Slot_Padding = 4.0f;

UHttpGPTMessagingHandler::UHttpGPTMessagingHandler(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UHttpGPTMessagingHandler::RequestSent()
{
	OnMessageContentUpdated.ExecuteIfBound("Waiting for response...");
}

void UHttpGPTMessagingHandler::RequestFailed()
{
	OnMessageContentUpdated.ExecuteIfBound("Request Failed.\nPlease check the logs. (Enable internal logs in Project Settings -> Plugins -> HttpGPT).");
	Destroy();
}

void UHttpGPTMessagingHandler::ProcessUpdated(const FHttpGPTResponse& Response)
{
	ProcessResponse(Response);
}

void UHttpGPTMessagingHandler::ProcessCompleted(const FHttpGPTResponse& Response)
{
	ProcessResponse(Response);
	Destroy();
}

void UHttpGPTMessagingHandler::ProcessResponse(const FHttpGPTResponse& Response)
{	
	if (!Response.bSuccess)
	{
		const FStringFormatOrderedArguments Arguments_ErrorDetails{
			FString("Request Failed."),
			FString("Please check the logs. (Enable internal logs in Project Settings -> Plugins -> HttpGPT)."),
			FString("Error Details: "),
			FString("\tError Code: ") + Response.Error.Code.ToString(),
			FString("\tError Type: ") + Response.Error.Type.ToString(),
			FString("\tError Message: ") + Response.Error.Message
		};

		OnMessageContentUpdated.ExecuteIfBound(FString::Format(TEXT("{0}\n{1}\n\n{2}\n{3}\n{4}\n{5}"), Arguments_ErrorDetails));
	}
	else if (Response.bSuccess && !HttpGPT::Internal::HasEmptyParam(Response.Choices))
	{
		OnMessageContentUpdated.ExecuteIfBound(Response.Choices[0].Message.Content);
	}
	else
	{
		return;
	}

	if (ScrollBoxReference.IsValid())
	{
		ScrollBoxReference->ScrollToEnd();
	}
}

void UHttpGPTMessagingHandler::Destroy()
{
#if ENGINE_MAJOR_VERSION >= 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void SHttpGPTChatItem::Construct(const FArguments& InArgs)
{
	Message = FHttpGPTMessage(InArgs._MessageRole, InArgs._InputText);

	MessagingHandlerObject = NewObject<UHttpGPTMessagingHandler>();
	MessagingHandlerObject->OnMessageContentUpdated.BindLambda(
		[this](FString Content)
		{
			Message.Content = Content;
		}
	);

#if ENGINE_MAJOR_VERSION < 5
	using FAppStyle = FEditorStyle;
#endif

	const ISlateStyle& AppStyle = FAppStyle::Get();

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(Message.Role == EHttpGPTRole::User ? FMargin(Slot_Padding * 16.f, Slot_Padding, Slot_Padding, Slot_Padding) : FMargin(Slot_Padding, Slot_Padding, Slot_Padding * 16.f, Slot_Padding))
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
					.Text(FText::FromString(Message.Role == EHttpGPTRole::User ? "User:" : "Assistant:"))
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
	return FText::FromString(Message.Content);
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
				SAssignNew(ChatScrollBox, SScrollBox)
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
	AssistantMessage->MessagingHandlerObject->ScrollBoxReference = ChatScrollBox;

	FHttpGPTOptions Options;
	Options.Model = UHttpGPTHelper::NameToModel(*(*ModelsComboBox->GetSelectedItem().Get()));
	Options.bStream = true;

	RequestReference = UHttpGPTRequest::SendMessages_CustomOptions(GEditor->GetEditorWorldContext().World(), GetChatHistory(), Options);

	RequestReference->ProgressStarted.AddDynamic(AssistantMessage->MessagingHandlerObject.Get(), &UHttpGPTMessagingHandler::ProcessUpdated);
	RequestReference->ProgressUpdated.AddDynamic(AssistantMessage->MessagingHandlerObject.Get(), &UHttpGPTMessagingHandler::ProcessUpdated);
	RequestReference->ProcessCompleted.AddDynamic(AssistantMessage->MessagingHandlerObject.Get(), &UHttpGPTMessagingHandler::ProcessCompleted);
	RequestReference->ErrorReceived.AddDynamic(AssistantMessage->MessagingHandlerObject.Get(), &UHttpGPTMessagingHandler::ProcessCompleted);
	RequestReference->RequestFailed.AddDynamic(AssistantMessage->MessagingHandlerObject.Get(), &UHttpGPTMessagingHandler::RequestFailed);
	RequestReference->RequestSent.AddDynamic(AssistantMessage->MessagingHandlerObject.Get(), &UHttpGPTMessagingHandler::RequestSent);

	RequestReference->Activate();

	ChatBox->AddSlot().AutoHeight() [AssistantMessage.ToSharedRef()];
	ChatItems.Add(AssistantMessage);

	ChatScrollBox->ScrollToEnd();

	InputTextBox->SetText(FText::GetEmpty());

	return FReply::Handled();
}

bool SHttpGPTChatView::IsSendMessageEnabled() const
{
	return (!RequestReference.IsValid() || !UHttpGPTTaskStatus::IsTaskActive(RequestReference.Get())) && !HttpGPT::Internal::HasEmptyParam(InputTextBox->GetText());
}

FReply SHttpGPTChatView::HandleClearChatButton()
{
	ChatItems.Empty();
	ChatBox->ClearChildren();

	if (RequestReference.IsValid())
	{
		RequestReference->StopHttpGPTTask();
	}
	else
	{
		RequestReference.Reset();
	}

	return FReply::Handled();
}

bool SHttpGPTChatView::IsClearChatEnabled() const
{
	return !HttpGPT::Internal::HasEmptyParam(ChatItems);
}

TArray<FHttpGPTMessage> SHttpGPTChatView::GetChatHistory() const
{
	TArray<FHttpGPTMessage> Output
	{
		FHttpGPTMessage(EHttpGPTRole::System, GetSystemContext())
	};

	for (const auto& Item : ChatItems)
	{
		Output.Add(Item->Message);
	}

	return Output;
}

FString SHttpGPTChatView::GetSystemContext() const
{
	FString SupportedModels;
	for (const TSharedPtr<FString>& Model : AvailableModels)
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
		PluginInterface->GetDescriptor().CreatedBy
	};

	const FString PluginInformation = FString::Format(TEXT("You are in the Unreal Engine {0} plugin {1} version {2}, which was developed by {3}."), Arguments_PluginInfo);

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

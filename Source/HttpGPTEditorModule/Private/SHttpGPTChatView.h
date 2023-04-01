// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Structures/HttpGPTChatTypes.h>
#include <Tasks/HttpGPTChatRequest.h>
#include <Widgets/Text/STextBlock.h>
#include <Widgets/Text/SMultiLineEditableText.h>
#include <Widgets/Input/STextComboBox.h>
#include <Widgets/Layout/SScrollBox.h>
#include "SHttpGPTChatView.generated.h"

DECLARE_DELEGATE_OneParam(FMessageContentUpdated, FString);

UCLASS(MinimalAPI, NotBlueprintable, NotPlaceable, Category = "Implementation")
class UHttpGPTMessagingHandler : public UObject
{
	GENERATED_BODY()

public:
	explicit UHttpGPTMessagingHandler(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	FMessageContentUpdated OnMessageContentUpdated;

	UFUNCTION()
	void RequestSent();

	UFUNCTION()
	void RequestFailed();

	UFUNCTION()
	void ProcessUpdated(const FHttpGPTChatResponse& Response);
	
	UFUNCTION()
	void ProcessCompleted(const FHttpGPTChatResponse& Response);

	TSharedPtr<SScrollBox> ScrollBoxReference;

	void Destroy();

private:
	void ProcessResponse(const FHttpGPTChatResponse& Response);
};

class SHttpGPTChatItem final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SHttpGPTChatItem) : _MessageRole(), _InputText()
	{
	}
	SLATE_ARGUMENT(EHttpGPTChatRole, MessageRole)
	SLATE_ARGUMENT(FString, InputText)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FString GetRoleText() const;
	FString GetMessageText() const;

	TWeakObjectPtr<UHttpGPTMessagingHandler> MessagingHandlerObject;

private:
	TSharedPtr<STextBlock> Role;
	TSharedPtr<SMultiLineEditableText> Message;
};

typedef TSharedPtr<SHttpGPTChatItem> SHttpGPTChatItemPtr;

class SHttpGPTChatView final : public SCompoundWidget
{
public:
	SLATE_USER_ARGS(SHttpGPTChatView)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FReply HandleSendMessageButton();
	bool IsSendMessageEnabled() const;

	FReply HandleClearChatButton();
	bool IsClearChatEnabled() const;

protected:
	TArray<FHttpGPTChatMessage> GetChatHistory() const;
	FString GetSystemContext() const;

	void InitializeModelsOptions();

private:
	TSharedPtr<SVerticalBox> ChatBox;
	TArray<SHttpGPTChatItemPtr> ChatItems;
	TSharedPtr<SScrollBox> ChatScrollBox;

	TSharedPtr<SEditableTextBox> InputTextBox;

	TSharedPtr<STextComboBox> ModelsComboBox;
	TArray<TSharedPtr<FString>> AvailableModels;

	TWeakObjectPtr<class UHttpGPTChatRequest> RequestReference;
};

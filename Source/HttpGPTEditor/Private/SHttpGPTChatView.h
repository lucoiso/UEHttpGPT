// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <HttpGPTTypes.h>
#include <Widgets/Views/SListView.h>
#include <Widgets/Input/SEditableTextBox.h>
#include <Widgets/Input/STextComboBox.h>
#include "SHttpGPTChatView.generated.h"

UCLASS(MinimalAPI, NotBlueprintable, NotPlaceable, Category = "Implementation")
class UHttpGPTMessagingHandler : public UObject
{
	GENERATED_BODY()

public:
	explicit UHttpGPTMessagingHandler(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION()
	void RequestSent();

	UFUNCTION()
	void RequestFailed();

	UFUNCTION()
	void ResponseReceived(const FHttpGPTResponse& Response);

	FHttpGPTMessage Message;
};

typedef UHttpGPTMessagingHandler* UHttpGPTMessagingHandlerPtr;

class SHttpGPTChatView final : public SCompoundWidget
{
public:
	SLATE_USER_ARGS(SHttpGPTChatView)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	TSharedRef<ITableRow> OnGenerateRow(UHttpGPTMessagingHandlerPtr Item, const TSharedRef<STableViewBase>& OwnerTable);

	FReply HandleSendMessageButton();
	bool IsSendMessageEnabled() const;

	FReply HandleClearChatButton();
	bool IsClearChatEnabled() const;

protected:
	TArray<FHttpGPTMessage> GetChatHistory() const;
	FString GetSystemContext() const;

	void InitializeModelsOptions();

private:
	TArray<UHttpGPTMessagingHandlerPtr> ListItems;
	TSharedPtr<SListView<UHttpGPTMessagingHandlerPtr>> ListView;

	TSharedPtr<SEditableTextBox, ESPMode::ThreadSafe> InputTextBox;
	TSharedPtr<STextComboBox, ESPMode::ThreadSafe> ModelsComboBox;

	TArray<TSharedPtr<FString>> AvailableModels;

#if ENGINE_MAJOR_VERSION >= 5
	TObjectPtr<class UHttpGPTRequest> RequestReference;
#else
	class UHttpGPTRequest* RequestReference;
#endif
};

class SHttpGPTChatItem : public STableRow<UHttpGPTMessagingHandlerPtr>
{
public:
	SLATE_BEGIN_ARGS(SHttpGPTChatItem) { }
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, const UHttpGPTMessagingHandlerPtr InMessagingHandlerObject);
	
	FText GetMessageText() const;

private:
	UHttpGPTMessagingHandlerPtr MessagingHandlerObject;
	TSharedPtr<STextBlock, ESPMode::ThreadSafe> MessageBox;
};
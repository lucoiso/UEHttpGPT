// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Structures/HttpGPTChatTypes.h>
#include <Widgets/SCompoundWidget.h>
#include "SHttpGPTChatItem.h"

class SHttpGPTChatView final : public SCompoundWidget
{
public:
	SLATE_USER_ARGS(SHttpGPTChatView)
			: _SessionID(NAME_None)
		{
		}

		SLATE_ARGUMENT(FName, SessionID)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SHttpGPTChatView() override;

	bool IsSendMessageEnabled() const;
	bool IsClearChatEnabled() const;
	FString GetHistoryPath() const;

	void SetSessionID(const FName& NewSessionID);
	FName GetSessionID() const;

	void ClearChat();

private:
	TSharedRef<SWidget> ConstructContent();

	FReply HandleSendMessageButton(const EHttpGPTChatRole Role);
	FReply HandleClearChatButton();

	TArray<FHttpGPTChatMessage> GetChatHistory() const;
	FString GetDefaultSystemContext() const;

	void LoadChatHistory();
	void SaveChatHistory() const;

	FName SessionID;

	TSharedPtr<class SVerticalBox> ChatBox;
	TArray<SHttpGPTChatItemPtr> ChatItems;
	TSharedPtr<class SScrollBox> ChatScrollBox;

	TSharedPtr<class SEditableTextBox> InputTextBox;

	TSharedPtr<class STextComboBox> ModelsComboBox;
	TArray<FTextDisplayStringPtr> AvailableModels;

	TWeakObjectPtr<class UHttpGPTChatRequest> RequestReference;
};

// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Widgets/SCompoundWidget.h>
#include <Structures/HttpGPTChatTypes.h>

static const FName NewSessionName = TEXT("New Session");

class SHttpGPTChatItem final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SHttpGPTChatItem) : _MessageRole(), _InputText(), _ScrollBox()
		{
		}

		SLATE_ARGUMENT(EHttpGPTChatRole, MessageRole)
		SLATE_ARGUMENT(FString, InputText)
		SLATE_ARGUMENT(TSharedPtr<class SScrollBox>, ScrollBox)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FString GetRoleText() const;
	FString GetMessageText() const;

	TWeakObjectPtr<class UHttpGPTMessagingHandler> MessagingHandlerObject;

private:
	TSharedRef<SWidget> ConstructContent();

	EHttpGPTChatRole MessageRole;
	FString InputText;

	TSharedPtr<class STextBlock> Role;
	TSharedPtr<class SMultiLineEditableText> Message;
};

using SHttpGPTChatItemPtr = TSharedPtr<SHttpGPTChatItem>;

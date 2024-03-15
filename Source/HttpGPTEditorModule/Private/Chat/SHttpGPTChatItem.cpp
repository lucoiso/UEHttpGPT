// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "SHttpGPTChatItem.h"
#include "HttpGPTMessagingHandler.h"
#include <Widgets/Text/SMultiLineEditableText.h>

void SHttpGPTChatItem::Construct(const FArguments& InArgs)
{
	MessageRole = InArgs._MessageRole;
	InputText = InArgs._InputText;

	if (MessageRole == EHttpGPTChatRole::Assistant)
	{
		MessagingHandlerObject = NewObject<UHttpGPTMessagingHandler>();
		MessagingHandlerObject->SetFlags(RF_Standalone);
		MessagingHandlerObject->ScrollBoxReference = InArgs._ScrollBox;

		MessagingHandlerObject->OnMessageContentUpdated.BindLambda([this](FString Content)
		{
			if (!Message.IsValid())
			{
				return;
			}

#if ENGINE_MAJOR_VERSION > 5 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
			const FTextSelection SelectedText = Message->GetSelection();
			Message->SetText(FText::FromString(Content));
			Message->SelectText(SelectedText.GetBeginning(), SelectedText.GetEnd());
#else
                Message->SetText(FText::FromString(Content));
#endif
		});
	}

	ChildSlot
	[
		ConstructContent()
	];
}

static FSlateColor& operator*=(FSlateColor& Lhs, const float Rhs)
{
	FLinearColor NewColor = Lhs.GetSpecifiedColor() * Rhs;
	NewColor.A = 1.f;
	Lhs = FSlateColor(NewColor);

	return Lhs;
}

TSharedRef<SWidget> SHttpGPTChatItem::ConstructContent()
{
	constexpr float SlotPadding = 4.0f;
	constexpr float PaddingMultiplier = 32.0f;

	FText RoleText = FText::FromString(TEXT("User:"));
	FMargin BoxMargin(SlotPadding * PaddingMultiplier, SlotPadding, SlotPadding, SlotPadding);
	FSlateColor MessageColor(FLinearColor::White);

	if (MessageRole == EHttpGPTChatRole::Assistant)
	{
		RoleText = FText::FromString(TEXT("Assistant:"));
		BoxMargin = FMargin(SlotPadding, SlotPadding, SlotPadding * PaddingMultiplier, SlotPadding);
		MessageColor *= 0.3f;
	}
	else if (MessageRole == EHttpGPTChatRole::System)
	{
		RoleText = FText::FromString(TEXT("System:"));
		BoxMargin = FMargin(SlotPadding * PaddingMultiplier * 0.5f, SlotPadding);
		MessageColor *= 0.f;
	}

	const FMargin MessageMargin(SlotPadding * 4.f, SlotPadding, SlotPadding, SlotPadding);

#if ENGINE_MAJOR_VERSION < 5
    using FAppStyle = FEditorStyle;
#endif

	return SNew(SBox).Padding(BoxMargin)
		[
			SNew(SBorder).BorderImage(FAppStyle::Get().GetBrush("Menu.Background")).BorderBackgroundColor(MessageColor)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().Padding(SlotPadding).AutoHeight()
				[
					SAssignNew(Role, STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Bold", 10)).Text(RoleText)
				]
				+ SVerticalBox::Slot().Padding(MessageMargin).FillHeight(1.f)
				[
					SAssignNew(Message, SMultiLineEditableText).AllowMultiLine(true).AutoWrapText(true).IsReadOnly(true).AllowContextMenu(true).Text(
						FText::FromString(InputText))
				]
			]
		];
}

FString SHttpGPTChatItem::GetRoleText() const
{
	return Role.IsValid() ? Role->GetText().ToString() : FString();
}

FString SHttpGPTChatItem::GetMessageText() const
{
	return Message.IsValid() ? Message->GetText().ToString() : FString();
}

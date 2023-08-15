// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "SHttpGPTImageGenView.h"
#include "SHttpGPTImageGenItem.h"
#include <Utils/HttpGPTHelper.h>
#include <HttpGPTInternalFuncs.h>
#include <Widgets/Layout/SScrollBox.h>
#include <Widgets/Input/STextComboBox.h>

void SHttpGPTImageGenView::Construct(const FArguments& InArgs)
{
    InitializeImageNumOptions();
    ImageNumComboBox = SNew(STextComboBox)
        .OptionsSource(&ImageNum)
        .InitiallySelectedItem(ImageNum[0])
        .ToolTipText(FText::FromString(TEXT("Number of Generated Images")));

    InitializeImageSizeOptions();
    ImageSizeComboBox = SNew(STextComboBox)
        .OptionsSource(&ImageSize)
        .InitiallySelectedItem(ImageSize[0])
        .ToolTipText(FText::FromString(TEXT("Size of Generated Images")));

    ChildSlot
        [
            ConstructContent()
        ];
}

TSharedRef<SWidget> SHttpGPTImageGenView::ConstructContent()
{
    constexpr float SlotPadding = 4.0f;

    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .Padding(SlotPadding)
        .FillHeight(1.f)
        [
            SAssignNew(ViewScrollBox, SScrollBox)
                + SScrollBox::Slot()
                [
                    SAssignNew(ViewBox, SVerticalBox)
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
                                if (IsSendRequestEnabled() && CommitType == ETextCommit::OnEnter)
                                {
                                    HandleSendRequestButton();
                                }
                            }
                        )
                ]
                + SHorizontalBox::Slot()
                .Padding(SlotPadding)
                .AutoWidth()
                [
                    SNew(SButton)
                        .Text(FText::FromString(TEXT("Generate")))
                        .ToolTipText(FText::FromString(TEXT("Request Images Generation")))
                        .OnClicked(this, &SHttpGPTImageGenView::HandleSendRequestButton)
                        .IsEnabled(this, &SHttpGPTImageGenView::IsSendRequestEnabled)
                ]
                + SHorizontalBox::Slot()
                .Padding(SlotPadding)
                .AutoWidth()
                [
                    ImageNumComboBox.ToSharedRef()
                ]
                + SHorizontalBox::Slot()
                .Padding(SlotPadding)
                .AutoWidth()
                [
                    ImageSizeComboBox.ToSharedRef()
                ]
                + SHorizontalBox::Slot()
                .Padding(SlotPadding)
                .AutoWidth()
                [
                    SNew(SButton)
                        .Text(FText::FromString(TEXT("Clear")))
                        .ToolTipText(FText::FromString(TEXT("Clear Generation History")))
                        .OnClicked(this, &SHttpGPTImageGenView::HandleClearViewButton)
                        .IsEnabled(this, &SHttpGPTImageGenView::IsClearViewEnabled)
                ]
        ];
}

FReply SHttpGPTImageGenView::HandleSendRequestButton()
{
    ViewBox->AddSlot()
        .AutoHeight()
        [
            SNew(SHttpGPTImageGenItem)
                .Prompt(InputTextBox->GetText().ToString())
                .Num(*ImageNumComboBox->GetSelectedItem().Get())
                .Size(*ImageSizeComboBox->GetSelectedItem().Get())
        ];

    ViewScrollBox->ScrollToEnd();
    InputTextBox->SetText(FText::GetEmpty());

    return FReply::Handled();
}

bool SHttpGPTImageGenView::IsSendRequestEnabled() const
{
    return !HttpGPT::Internal::HasEmptyParam(InputTextBox->GetText());
}

FReply SHttpGPTImageGenView::HandleClearViewButton()
{
    ViewBox->ClearChildren();
    return FReply::Handled();
}

bool SHttpGPTImageGenView::IsClearViewEnabled() const
{
    return ViewBox->NumSlots() > 0;
}

void SHttpGPTImageGenView::InitializeImageNumOptions()
{
    constexpr uint8 MaxNum = 10u;
    for (uint8 Iterator = 1u; Iterator <= MaxNum; ++Iterator)
    {
        ImageNum.Add(MakeShared<FString>(FString::FromInt(Iterator)));
    }
}

void SHttpGPTImageGenView::InitializeImageSizeOptions()
{
    ImageSize.Add(MakeShared<FString>(UHttpGPTHelper::SizeToName(EHttpGPTImageSize::x256).ToString()));
    ImageSize.Add(MakeShared<FString>(UHttpGPTHelper::SizeToName(EHttpGPTImageSize::x512).ToString()));
    ImageSize.Add(MakeShared<FString>(UHttpGPTHelper::SizeToName(EHttpGPTImageSize::x1024).ToString()));
}
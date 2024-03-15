// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "SHttpGPTImageGenItem.h"
#include "SHttpGPTImageGenItemData.h"
#include "HttpGPTImageGetter.h"
#include <HttpGPTInternalFuncs.h>
#include <Structures/HttpGPTImageTypes.h>
#include <Tasks/HttpGPTImageRequest.h>
#include <Utils/HttpGPTHelper.h>
#include <Widgets/Layout/SScrollBox.h>

void SHttpGPTImageGenItem::Construct(const FArguments& InArgs)
{
	Prompt = InArgs._Prompt;

	HttpGPTImageGetterObject = NewObject<UHttpGPTImageGetter>();
	HttpGPTImageGetterObject->SetFlags(RF_Standalone);
	HttpGPTImageGetterObject->OutScrollBox = InArgs._OutScrollBox;

	HttpGPTImageGetterObject->OnImageGenerated.BindLambda([this](UTexture2D* const Texture)
	{
		if (Texture && ItemViewBox.IsValid())
		{
			ItemViewBox->AddSlot().AutoWidth()[SNew(SHttpGPTImageGenItemData).Texture(Texture)];
		}
	});

	HttpGPTImageGetterObject->OnStatusChanged.BindLambda([this](const FString& NewStatus)
	{
		if (HttpGPT::Internal::HasEmptyParam(NewStatus) || !Status.IsValid())
		{
			return;
		}

		Status->SetText(FText::FromString(TEXT("Status: ") + NewStatus));
	});

	FHttpGPTImageOptions Options;
	Options.Format = EHttpGPTResponseFormat::b64_json;
	Options.Size = UHttpGPTHelper::NameToSize(*InArgs._Size);
	Options.ImagesNum = FCString::Atoi(*InArgs._Num);

	RequestReference = UHttpGPTImageRequest::EditorTask(Prompt, Options);

	RequestReference->ProcessCompleted.AddDynamic(HttpGPTImageGetterObject.Get(), &UHttpGPTImageGetter::ProcessCompleted);
	RequestReference->ErrorReceived.AddDynamic(HttpGPTImageGetterObject.Get(), &UHttpGPTImageGetter::ProcessCompleted);
	RequestReference->RequestFailed.AddDynamic(HttpGPTImageGetterObject.Get(), &UHttpGPTImageGetter::RequestFailed);
	RequestReference->RequestSent.AddDynamic(HttpGPTImageGetterObject.Get(), &UHttpGPTImageGetter::RequestSent);

	RequestReference->Activate();

	ChildSlot
	[
		ConstructContent()
	];
}

SHttpGPTImageGenItem::~SHttpGPTImageGenItem()
{
	if (RequestReference.IsValid())
	{
		RequestReference->StopHttpGPTTask();
	}
}

TSharedRef<SWidget> SHttpGPTImageGenItem::ConstructContent()
{
	constexpr float SlotPadding = 4.0f;

#if ENGINE_MAJOR_VERSION < 5
    using FAppStyle = FEditorStyle;
#endif

	return SNew(SBox).Padding(SlotPadding)
		[
			SNew(SBorder).BorderImage(FAppStyle::Get().GetBrush("Menu.Background"))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().Padding(SlotPadding).AutoHeight()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Bold", 10)).Text(FText::FromString(TEXT("Prompt: ") + Prompt))
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SAssignNew(Status, STextBlock).Text(FText::FromString(TEXT("Status: Sending request...")))
					]
				]
				+ SVerticalBox::Slot().Padding(SlotPadding).FillHeight(1.f)
				[
					SNew(SScrollBox).Orientation(Orient_Horizontal)
					+ SScrollBox::Slot()
					[
						SAssignNew(ItemViewBox, SHorizontalBox)
					]
				]
			]
		];
}

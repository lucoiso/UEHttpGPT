// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "SHttpGPTImageGenView.h"
#include <Utils/HttpGPTHelper.h>
#include <HttpGPTInternalFuncs.h>
#include <Brushes/SlateImageBrush.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(SHttpGPTImageGenView)
#endif

UHttpGPTImageGetter::UHttpGPTImageGetter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UHttpGPTImageGetter::RequestSent()
{
	OnStatusChanged.ExecuteIfBound("Request Sent. Waiting for the response...");
}

void UHttpGPTImageGetter::RequestFailed()
{
	OnStatusChanged.ExecuteIfBound("Request Failed. Check the Logs.");
	Destroy();
}

void UHttpGPTImageGetter::ProcessCompleted(const FHttpGPTImageResponse& Response)
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

		OnStatusChanged.ExecuteIfBound(FString::Format(TEXT("{0}\n{1}\n\n{2}\n{3}\n{4}\n{5}"), Arguments_ErrorDetails));
		Destroy();
		return;
	}

	DataSize = Response.Data.Num();
	OnStatusChanged.ExecuteIfBound("Status: Request Completed.");

	OnImageGenerated_Internal.BindUFunction(this, TEXT("ImageGenerated"));

	for (const FHttpGPTImageData& Data : Response.Data)
	{
		ProcessImage(Data);
	}
}

void UHttpGPTImageGetter::ProcessImage(const FHttpGPTImageData& Data)
{
	UHttpGPTImageHelper::GenerateImage(Data, OnImageGenerated_Internal);
}

void UHttpGPTImageGetter::ImageGenerated(UTexture2D* Texture)
{
	OnImageGenerated.ExecuteIfBound(Texture);

	++GeneratedImages;
	if (GeneratedImages >= DataSize)
	{
		Destroy();
	}
}

void UHttpGPTImageGetter::Destroy()
{
	ClearFlags(RF_Standalone);

#if ENGINE_MAJOR_VERSION >= 5
	MarkAsGarbage();
#else
	MarkPendingKill();
#endif
}

void SHttpGPTImageGenItemData::Construct(const FArguments& InArgs)
{
	Texture = InArgs._Texture;

	constexpr float Slot_Padding = 4.0f;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(Slot_Padding)
		.FillHeight(1.f)
		[
			SAssignNew(Image, SImage)
			.Image(Texture.IsValid() ? new FSlateImageBrush(Texture.Get(), FVector2D(256, 256)) : nullptr)
		]
		+ SVerticalBox::Slot()
		.Padding(Slot_Padding)
		.AutoHeight()
		[
			SAssignNew(SaveButton, SButton)
			.Text(FText::FromString("Save"))
			.HAlign(HAlign_Center)
			.OnClicked(this, &SHttpGPTImageGenItemData::HandleSaveButton)
			.IsEnabled(this, &SHttpGPTImageGenItemData::IsSaveEnabled)
		]
	];
}

FReply SHttpGPTImageGenItemData::HandleSaveButton()
{
	return FReply::Handled();
}

bool SHttpGPTImageGenItemData::IsSaveEnabled() const
{
	return Texture.IsValid();
}

void SHttpGPTImageGenItem::Construct(const FArguments& InArgs)
{
	HttpGPTImageGetterObject = NewObject<UHttpGPTImageGetter>();
	HttpGPTImageGetterObject->SetFlags(RF_Standalone);

	HttpGPTImageGetterObject->OnImageGenerated.BindLambda(
		[this](UTexture2D* Texture)
		{
			if (Texture)
			{
				ItemViewBox->AddSlot().AutoWidth() [SNew(SHttpGPTImageGenItemData).Texture(Texture)];
			}
		}
	);

	HttpGPTImageGetterObject->OnStatusChanged.BindLambda(
		[this](FString NewStatus)
		{
			if (HttpGPT::Internal::HasEmptyParam(NewStatus) || !Status.IsValid())
			{
				return;
			}

			Status->SetText(FText::FromString("Status: " + NewStatus));
		}
	);

	FHttpGPTImageOptions Options;
	Options.Format = EHttpGPTResponseFormat::b64_json;
	Options.Size = UHttpGPTHelper::NameToSize(*InArgs._Size);
	Options.ImagesNum = FCString::Atoi(*InArgs._Num);

	RequestReference = UHttpGPTImageRequest::EditorTask(InArgs._Prompt, Options);

	RequestReference->ProcessCompleted.AddDynamic(HttpGPTImageGetterObject.Get(), &UHttpGPTImageGetter::ProcessCompleted);
	RequestReference->ErrorReceived.AddDynamic(HttpGPTImageGetterObject.Get(), &UHttpGPTImageGetter::ProcessCompleted);
	RequestReference->RequestFailed.AddDynamic(HttpGPTImageGetterObject.Get(), &UHttpGPTImageGetter::RequestFailed);
	RequestReference->RequestSent.AddDynamic(HttpGPTImageGetterObject.Get(), &UHttpGPTImageGetter::RequestSent);

	RequestReference->Activate();

	constexpr float Slot_Padding = 4.0f;

#if ENGINE_MAJOR_VERSION < 5
	using FAppStyle = FEditorStyle;
#endif

	const ISlateStyle& AppStyle = FAppStyle::Get();

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(Slot_Padding)
		[
			SNew(SBorder)
			.BorderImage(AppStyle.GetBrush("Menu.Background"))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(Slot_Padding)
				.AutoHeight()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(Prompt, STextBlock)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
						.Text(FText::FromString("Prompt: " + InArgs._Prompt))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(Status, STextBlock)
						.Text(FText::FromString("Status: Sending request..."))
					]
				]
				+ SVerticalBox::Slot()
				.Padding(Slot_Padding)
				.FillHeight(1.f)
				[
					SAssignNew(ItemScrollBox, SScrollBox)
					.Orientation(EOrientation::Orient_Horizontal)
					+ SScrollBox::Slot()
					[
						SAssignNew(ItemViewBox, SHorizontalBox)
					]
				]
			]
		]
	];
}

SHttpGPTImageGenItem::~SHttpGPTImageGenItem()
{
	if (RequestReference.IsValid())
	{
		RequestReference->StopHttpGPTTask();
	}
}

void SHttpGPTImageGenView::Construct(const FArguments& InArgs)
{
	constexpr float Slot_Padding = 4.0f;

#if ENGINE_MAJOR_VERSION < 5
	using FAppStyle = FEditorStyle;
#endif

	const ISlateStyle& AppStyle = FAppStyle::Get();

	InitializeImageNumOptions();
	ImageNumComboBox = SNew(STextComboBox).OptionsSource(&ImageNum).InitiallySelectedItem(ImageNum[0]).ToolTipText(FText::FromString("Number of Generated Images"));

	InitializeImageSizeOptions();
	ImageSizeComboBox = SNew(STextComboBox).OptionsSource(&ImageSize).InitiallySelectedItem(ImageSize[0]).ToolTipText(FText::FromString("Size of Generated Images"));

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
				SAssignNew(ViewScrollBox, SScrollBox)
				+ SScrollBox::Slot()
				[
					SAssignNew(ViewBox, SVerticalBox)
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
				.Text(FText::FromString("Generate"))
				.ToolTipText(FText::FromString("Request Images Generation"))
				.OnClicked(this, &SHttpGPTImageGenView::HandleSendRequestButton)
				.IsEnabled(this, &SHttpGPTImageGenView::IsSendRequestEnabled)
			]
			+ SHorizontalBox::Slot()
			.Padding(Slot_Padding)
			.AutoWidth()
			[
				ImageNumComboBox.ToSharedRef()
			]
			+ SHorizontalBox::Slot()
			.Padding(Slot_Padding)
			.AutoWidth()
			[
				ImageSizeComboBox.ToSharedRef()
			]
			+ SHorizontalBox::Slot()
			.Padding(Slot_Padding)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString("Clear"))
				.ToolTipText(FText::FromString("Clear Generation History"))
				.OnClicked(this, &SHttpGPTImageGenView::HandleClearViewButton)
				.IsEnabled(this, &SHttpGPTImageGenView::IsClearViewEnabled)
			]
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
		ImageNum.Add(MakeShareable(new FString(FString::FromInt(Iterator))));
	}
}

void SHttpGPTImageGenView::InitializeImageSizeOptions()
{
	ImageSize.Add(MakeShareable(new FString(UHttpGPTHelper::SizeToName(EHttpGPTImageSize::x256).ToString())));
	ImageSize.Add(MakeShareable(new FString(UHttpGPTHelper::SizeToName(EHttpGPTImageSize::x512).ToString())));
	ImageSize.Add(MakeShareable(new FString(UHttpGPTHelper::SizeToName(EHttpGPTImageSize::x1024).ToString())));	
}

// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#pragma once

#include <CoreMinimal.h>
#include <Structures/HttpGPTImageTypes.h>
#include <Tasks/HttpGPTImageRequest.h>
#include <Widgets/Text/STextBlock.h>
#include <Widgets/Text/SMultiLineEditableText.h>
#include <Widgets/Input/STextComboBox.h>
#include <Widgets/Layout/SScrollBox.h>
#include "SHttpGPTImageGenView.generated.h"

DECLARE_DELEGATE_OneParam(FImageGenerated, UTexture2D*);

UCLASS(MinimalAPI, NotBlueprintable, NotPlaceable, Category = "Implementation")
class UHttpGPTImageGetter : public UObject
{
	GENERATED_BODY()

public:
	explicit UHttpGPTImageGetter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	FImageGenerated OnImageGenerated;

	UFUNCTION()
	void RequestSent();

	UFUNCTION()
	void RequestFailed();
	
	UFUNCTION()
	void ProcessCompleted(const FHttpGPTImageResponse& Response);

	void Destroy();

private:
	void ProcessResponse(const FHttpGPTImageResponse& Response);
};

class SHttpGPTImageGenItemData final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SHttpGPTImageGenItemData) : _Texture()
	{
	}
	SLATE_ARGUMENT(UTexture2D*, Texture)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FReply HandleSaveButton();
	bool IsSaveEnabled() const;

private:
	TSharedPtr<SImage> Image;
	TSharedPtr<SButton> SaveButton;

	TWeakObjectPtr<UTexture2D> Texture;
};

typedef TSharedPtr<SHttpGPTImageGenItemData> SHttpGPTImageGenItemDataPtr;

class SHttpGPTImageGenItem final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SHttpGPTImageGenItem) : _Prompt()
	{
	}
	SLATE_ARGUMENT(FString, Prompt)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FString GetPromptText() const;

	TWeakObjectPtr<UHttpGPTImageGetter> HttpGPTImageGetterObject;

private:
	TSharedPtr<STextBlock> Prompt;
	TSharedPtr<STextBlock> Status;
	TSharedPtr<SScrollBox> ItemScrollBox;
	TArray<SHttpGPTImageGenItemDataPtr> Images;
};

typedef TSharedPtr<SHttpGPTImageGenItem> SHttpGPTImageGenItemPtr;

class SHttpGPTImageGenView final : public SCompoundWidget
{
public:
	SLATE_USER_ARGS(SHttpGPTImageGenView)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FReply HandleSendRequestButton();
	bool IsSendRequestEnabled() const;

	FReply HandleClearViewButton();
	bool IsClearViewEnabled() const;

protected:
	void InitializeImageNumOptions();

private:
	TSharedPtr<SVerticalBox> ViewBox;
	TArray<SHttpGPTImageGenItemPtr> PromptResults;
	TSharedPtr<SScrollBox> ViewScrollBox;
	TSharedPtr<SEditableTextBox> InputTextBox;
	TSharedPtr<STextComboBox> ImageNumComboBox;
	TArray<TSharedPtr<FString>> ImageNum;

	TWeakObjectPtr<class UHttpGPTImageRequest> RequestReference;
};

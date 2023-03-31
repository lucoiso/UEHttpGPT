// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "SHttpGPTImageGenView.h"
#include <Utils/HttpGPTHelper.h>
#include <HttpGPTInternalFuncs.h>
#include "SHttpGPTImageGenView.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(SHttpGPTImageGenView)
#endif

UHttpGPTImageGetter::UHttpGPTImageGetter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UHttpGPTImageGetter::RequestSent()
{
}

void UHttpGPTImageGetter::RequestFailed()
{
}

void UHttpGPTImageGetter::ProcessCompleted(const FHttpGPTImageResponse& Response)
{
}

void UHttpGPTImageGetter::Destroy()
{
}

void UHttpGPTImageGetter::ProcessResponse(const FHttpGPTImageResponse& Response)
{
}

void SHttpGPTImageGenItem::Construct(const FArguments& InArgs)
{
	constexpr float Slot_Padding = 4.0f;

}

FString SHttpGPTImageGenItem::GetPromptText() const
{
	return FString();
}

void SHttpGPTImageGenView::Construct(const FArguments& InArgs)
{
	constexpr float Slot_Padding = 4.0f;

}

FReply SHttpGPTImageGenView::HandleSendRequestButton()
{
	return FReply::Handled();
}

bool SHttpGPTImageGenView::IsSendRequestEnabled() const
{
	return false;
}

FReply SHttpGPTImageGenView::HandleClearViewButton()
{
	return FReply::Handled();
}

bool SHttpGPTImageGenView::IsClearViewEnabled() const
{
	return false;
}

void SHttpGPTImageGenView::InitializeImageNumOptions()
{
}

void SHttpGPTImageGenItemData::Construct(const FArguments& InArgs)
{
	constexpr float Slot_Padding = 4.0f;

}

FReply SHttpGPTImageGenItemData::HandleSaveButton()
{
	return FReply::Handled();
}

bool SHttpGPTImageGenItemData::IsSaveEnabled() const
{
	return false;
}

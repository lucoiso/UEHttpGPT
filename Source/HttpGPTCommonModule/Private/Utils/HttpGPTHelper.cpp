// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "Utils/HttpGPTHelper.h"
#include "HttpGPTInternalFuncs.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(HttpGPTHelper)
#endif

const FName UHttpGPTHelper::ModelToName(const EHttpGPTChatModel& Model)
{
	switch (Model)
	{
		case EHttpGPTChatModel::gpt4:
			return "gpt-4";

		case EHttpGPTChatModel::gpt432k:
			return "gpt-4-32k";

		case EHttpGPTChatModel::gpt35turbo:
			return "gpt-3.5-turbo";

		case EHttpGPTChatModel::textdavinci003:
			return "text-davinci-003";

		case EHttpGPTChatModel::textdavinci002:
			return "text-davinci-002";

		case EHttpGPTChatModel::codedavinci002:
			return "code-davinci-002";

		default: break;
	}

	return NAME_None;
}

const EHttpGPTChatModel UHttpGPTHelper::NameToModel(const FName Model)
{
	if (Model.IsEqual("gpt-4", ENameCase::IgnoreCase))
	{
		return EHttpGPTChatModel::gpt4;
	}
	else if (Model.IsEqual("gpt-4-32k", ENameCase::IgnoreCase))
	{
		return EHttpGPTChatModel::gpt432k;
	}
	else if (Model.IsEqual("gpt-3.5-turbo", ENameCase::IgnoreCase))
	{
		return EHttpGPTChatModel::gpt35turbo;
	}
	else if (Model.IsEqual("text-davinci-003", ENameCase::IgnoreCase))
	{
		return EHttpGPTChatModel::textdavinci003;
	}
	else if (Model.IsEqual("text-davinci-002", ENameCase::IgnoreCase))
	{
		return EHttpGPTChatModel::textdavinci002;
	}
	else if (Model.IsEqual("code-davinci-002", ENameCase::IgnoreCase))
	{
		return EHttpGPTChatModel::codedavinci002;
	}

	return EHttpGPTChatModel::gpt35turbo;
}

const FName UHttpGPTHelper::RoleToName(const EHttpGPTChatRole& Role)
{
	switch (Role)
	{
		case EHttpGPTChatRole::Assistant:
			return "assistant";

		case EHttpGPTChatRole::User:
			return "user";

		case EHttpGPTChatRole::System:
			return "system";
	}

	return NAME_None;
}

const EHttpGPTChatRole UHttpGPTHelper::NameToRole(const FName Role)
{
	if (Role.IsEqual("user", ENameCase::IgnoreCase))
	{
		return EHttpGPTChatRole::User;
	}
	else if (Role.IsEqual("assistant", ENameCase::IgnoreCase))
	{
		return  EHttpGPTChatRole::Assistant;
	}
	else if (Role.IsEqual("system", ENameCase::IgnoreCase))
	{
		return EHttpGPTChatRole::System;
	}

	return EHttpGPTChatRole::User;
}

const TArray<FName> UHttpGPTHelper::GetAvailableGPTModels()
{
	TArray<FName> Output;

	for (uint8 Iterator = static_cast<uint8>(EHttpGPTChatModel::gpt4); Iterator <= static_cast<uint8>(EHttpGPTChatModel::codedavinci002); ++Iterator)
	{				
		if (const FName ModelName = ModelToName(static_cast<EHttpGPTChatModel>(Iterator)); !HttpGPT::Internal::HasEmptyParam(ModelName))
		{
			Output.Add(ModelName);
		}
	}

	return Output;
}

const FName UHttpGPTHelper::GetEndpointForModel(const EHttpGPTChatModel& Model)
{
	switch (Model)
	{
		case EHttpGPTChatModel::gpt4:
		case EHttpGPTChatModel::gpt432k:
		case EHttpGPTChatModel::gpt35turbo:
			return "v1/chat/completions";

		case EHttpGPTChatModel::textdavinci003:
		case EHttpGPTChatModel::textdavinci002:
		case EHttpGPTChatModel::codedavinci002:
			return "v1/completions";

		default: break;
	}

	return NAME_None;
}

const bool UHttpGPTHelper::ModelSupportsChat(const EHttpGPTChatModel& Model)
{
	switch (Model)
	{
		case EHttpGPTChatModel::gpt4:
		case EHttpGPTChatModel::gpt432k:
		case EHttpGPTChatModel::gpt35turbo:
			return true;

		case EHttpGPTChatModel::textdavinci003:
		case EHttpGPTChatModel::textdavinci002:
		case EHttpGPTChatModel::codedavinci002:
			return false;

		default: break;
	}

	return false;
}

const FName UHttpGPTHelper::SizeToName(const EHttpGPTImageSize& Size)
{
	switch (Size)
	{
		case EHttpGPTImageSize::x256:
				return "256x256";

		case EHttpGPTImageSize::x512:
			return "512x512";

		case EHttpGPTImageSize::x1024:
			return "1024x1024";

		default:
			break;
	}

	return NAME_None;
}

const EHttpGPTImageSize UHttpGPTHelper::NameToSize(const FName Size)
{
	if (Size.IsEqual("256x256", ENameCase::IgnoreCase))
	{
		return EHttpGPTImageSize::x256;
	}
	else if (Size.IsEqual("512x512", ENameCase::IgnoreCase))
	{
		return EHttpGPTImageSize::x512;
	}
	else if (Size.IsEqual("1024x1024", ENameCase::IgnoreCase))
	{
		return EHttpGPTImageSize::x1024;
	}

	return EHttpGPTImageSize::x256;
}

const FName UHttpGPTHelper::FormatToName(const EHttpGPTResponseFormat& Format)
{
	switch (Format)
	{
		case EHttpGPTResponseFormat::url:
			return "url";

		case EHttpGPTResponseFormat::b64_json:
			return "b64_json";

		default:
			break;
	}

	return NAME_None;
}

const EHttpGPTResponseFormat UHttpGPTHelper::NameToFormat(const FName Format)
{
	if (Format.IsEqual("url", ENameCase::IgnoreCase))
	{
		return EHttpGPTResponseFormat::url;
	}
	else if (Format.IsEqual("b64_json", ENameCase::IgnoreCase))
	{
		return EHttpGPTResponseFormat::b64_json;
	}

	return EHttpGPTResponseFormat::url;
}

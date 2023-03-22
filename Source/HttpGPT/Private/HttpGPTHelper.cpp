// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "HttpGPTHelper.h"
#include <HttpGPTInternalFuncs.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(HttpGPTHelper)
#endif

UHttpGPTRequest* UHttpGPTHelper::CastToHttpGPTRequest(UObject* Object)
{
	return Cast<UHttpGPTRequest>(Object);
}

const FName UHttpGPTHelper::ModelToName(const EHttpGPTModel& Model)
{
	switch (Model)
	{
		case EHttpGPTModel::gpt4:
			return "gpt-4";

		case EHttpGPTModel::gpt432k:
			return "gpt-4-32k";

		case EHttpGPTModel::gpt35turbo:
			return "gpt-3.5-turbo";

		case EHttpGPTModel::textdavinci003:
			return "text-davinci-003";

		case EHttpGPTModel::textdavinci002:
			return "text-davinci-002";

		case EHttpGPTModel::codedavinci002:
			return "code-davinci-002";

		default: break;
	}

	return NAME_None;
}

const EHttpGPTModel UHttpGPTHelper::NameToModel(const FName Model)
{
	if (Model.IsEqual("gpt-4", ENameCase::IgnoreCase))
	{
		return EHttpGPTModel::gpt4;
	}
	else if (Model.IsEqual("gpt-4-32k", ENameCase::IgnoreCase))
	{
		return EHttpGPTModel::gpt432k;
	}
	else if (Model.IsEqual("gpt-3.5-turbo", ENameCase::IgnoreCase))
	{
		return EHttpGPTModel::gpt35turbo;
	}
	else if (Model.IsEqual("text-davinci-003", ENameCase::IgnoreCase))
	{
		return EHttpGPTModel::textdavinci003;
	}
	else if (Model.IsEqual("text-davinci-002", ENameCase::IgnoreCase))
	{
		return EHttpGPTModel::textdavinci002;
	}
	else if (Model.IsEqual("code-davinci-002", ENameCase::IgnoreCase))
	{
		return EHttpGPTModel::codedavinci002;
	}

	return EHttpGPTModel::gpt35turbo;
}

const TArray<FName> UHttpGPTHelper::GetAvailableGPTModels()
{
	TArray<FName> Output;

	for (uint8 Iterator = static_cast<uint8>(EHttpGPTModel::gpt4); Iterator <= static_cast<uint8>(EHttpGPTModel::codedavinci002); ++Iterator)
	{				
		if (const FName ModelName = ModelToName(static_cast<EHttpGPTModel>(Iterator)); !HttpGPT::Internal::HasEmptyParam(ModelName))
		{
			Output.Add(ModelName);
		}
	}

	return Output;
}

const FName UHttpGPTHelper::GetEndpointForModel(const EHttpGPTModel& Model)
{
	switch (Model)
	{
		case EHttpGPTModel::gpt4:
		case EHttpGPTModel::gpt432k:
		case EHttpGPTModel::gpt35turbo:
			return "v1/chat/completions";

		case EHttpGPTModel::textdavinci003:
		case EHttpGPTModel::textdavinci002:
		case EHttpGPTModel::codedavinci002:
			return "v1/completions";

		default: break;
	}

	return NAME_None;
}

const bool UHttpGPTHelper::ModelSupportsChat(const EHttpGPTModel& Model)
{
	switch (Model)
	{
		case EHttpGPTModel::gpt4:
		case EHttpGPTModel::gpt432k:
		case EHttpGPTModel::gpt35turbo:
			return true;

		case EHttpGPTModel::textdavinci003:
		case EHttpGPTModel::textdavinci002:
		case EHttpGPTModel::codedavinci002:
			return false;

		default: break;
	}

	return false;
}

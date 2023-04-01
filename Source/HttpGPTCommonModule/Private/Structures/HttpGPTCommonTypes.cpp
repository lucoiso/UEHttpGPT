// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "Structures/HttpGPTCommonTypes.h"
#include <Management/HttpGPTSettings.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(HttpGPTCommonTypes)
#endif

FHttpGPTCommonOptions::FHttpGPTCommonOptions()
{
	SetDefaults();
}

void FHttpGPTCommonOptions::SetDefaults()
{
	if (const UHttpGPTSettings* const Settings = GetDefault<UHttpGPTSettings>())
	{
		APIKey = Settings->CommonOptions.APIKey;
		User = Settings->CommonOptions.User;
	}
}

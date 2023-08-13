// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "Structures/HttpGPTImageTypes.h"
#include "Management/HttpGPTSettings.h"

FHttpGPTImageOptions::FHttpGPTImageOptions()
{
    SetDefaults();
}

void FHttpGPTImageOptions::SetDefaults()
{
    if (const UHttpGPTSettings* const Settings = GetDefault<UHttpGPTSettings>())
    {
        ImagesNum = Settings->ImageOptions.ImagesNum;
        Size = Settings->ImageOptions.Size;
        Format = Settings->ImageOptions.Format;
    }
}
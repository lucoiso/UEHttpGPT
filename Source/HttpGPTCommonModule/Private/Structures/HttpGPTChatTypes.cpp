// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "Structures/HttpGPTChatTypes.h"
#include "Utils/HttpGPTHelper.h"
#include <Management/HttpGPTSettings.h>
#include <Dom/JsonObject.h>

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(HttpGPTChatTypes)
#endif

TSharedPtr<FJsonValue> FHttpGPTFunction::GetFunction() const
{
    TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
    JsonObject->SetStringField("name", Name.ToString());
    JsonObject->SetStringField("description", Description);

    TSharedPtr<FJsonObject> ParametersObject = MakeShared<FJsonObject>();
    ParametersObject->SetStringField("type", "object");

    TSharedPtr<FJsonObject> PropertiesObject = MakeShared<FJsonObject>();
    for (const FHttpGPTFunctionProperty& PropIt : Properties)
    {
        TSharedPtr<FJsonObject> PropertyObject = MakeShared<FJsonObject>();
        PropertyObject->SetStringField("type", UHttpGPTHelper::PropertyTypeToName(PropIt.Type).ToString().ToLower());
        PropertyObject->SetStringField("description", PropIt.Description);

        TArray<TSharedPtr<FJsonValue>> EnumArr;
        for (const FName& EnumIt : PropIt.Enum)
        {
            EnumArr.Emplace(MakeShared<FJsonValueString>(EnumIt.ToString()));
        }
        PropertyObject->SetArrayField("enum", EnumArr);

        PropertiesObject->SetObjectField(PropIt.Name.ToString(), PropertyObject);
    }

    ParametersObject->SetObjectField("properties", PropertiesObject);

    TArray<TSharedPtr<FJsonValue>> RequiredParams;
    for (const FName& ReqIt : RequiredProperties)
    {
        RequiredParams.Emplace(MakeShared<FJsonValueString>(ReqIt.ToString()));
    }

    ParametersObject->SetArrayField("required", RequiredParams);
    JsonObject->SetObjectField("parameters", ParametersObject);

    return MakeShared<FJsonValueObject>(JsonObject);
}

FHttpGPTChatMessage::FHttpGPTChatMessage(const FName& InRole, const FString& InContent)
{
    Role = UHttpGPTHelper::NameToRole(InRole);
    Content = InContent;
}

TSharedPtr<FJsonValue> FHttpGPTChatMessage::GetMessage() const
{
    TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
    JsonObject->SetStringField("role", UHttpGPTHelper::RoleToName(Role).ToString().ToLower());

    if (Role == EHttpGPTChatRole::Function)
    {
        JsonObject->SetStringField("name", FunctionCall.Name.ToString());
        JsonObject->SetStringField("content", FunctionCall.Arguments);
    }
    else
    {
        JsonObject->SetStringField("content", Content);
    }

    return MakeShared<FJsonValueObject>(JsonObject);
}

FHttpGPTChatOptions::FHttpGPTChatOptions()
{
    SetDefaults();
}

void FHttpGPTChatOptions::SetDefaults()
{
    if (const UHttpGPTSettings* const Settings = GetDefault<UHttpGPTSettings>())
    {
        Model = Settings->ChatOptions.Model;
        MaxTokens = Settings->ChatOptions.MaxTokens;
        Temperature = Settings->ChatOptions.Temperature;
        TopP = Settings->ChatOptions.TopP;
        Choices = Settings->ChatOptions.Choices;
        bStream = Settings->ChatOptions.bStream;
        Stop = Settings->ChatOptions.Stop;
        PresencePenalty = Settings->ChatOptions.PresencePenalty;
        FrequencyPenalty = Settings->ChatOptions.FrequencyPenalty;
        LogitBias = Settings->ChatOptions.LogitBias;
    }
}

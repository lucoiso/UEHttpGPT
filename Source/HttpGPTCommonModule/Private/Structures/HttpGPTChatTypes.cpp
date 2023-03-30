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

FHttpGPTChatMessage::FHttpGPTChatMessage(const FName& Role, const FString& Content)
{
	this->Role = UHttpGPTHelper::NameToRole(Role);
	this->Content = Content;
}

TSharedPtr<FJsonValue> FHttpGPTChatMessage::GetMessage() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("role", UHttpGPTHelper::RoleToName(Role).ToString());
	JsonObject->SetStringField("content", Content);

	return MakeShareable(new FJsonValueObject(JsonObject));
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
		User = Settings->ChatOptions.User;
	}
}

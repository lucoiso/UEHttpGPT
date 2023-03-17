// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "HttpGPTTypes.h"
#include "HttpGPTSettings.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(HttpGPTTypes)
#endif

TSharedPtr<FJsonValue> FHttpGPTMessage::GetMessage() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("role", Role == EHttpGPTRole::User ? "user" : "assistant");
	JsonObject->SetStringField("content", Content);

	return MakeShareable(new FJsonValueObject(JsonObject));
}

FHttpGPTOptions::FHttpGPTOptions()
{
	SetDefaults();
}

void FHttpGPTOptions::SetDefaults()
{
	if (const UHttpGPTSettings* const Settings = UHttpGPTSettings::Get())
	{
		Model = Settings->DefaultOptions.Model;
		MaxTokens = Settings->DefaultOptions.MaxTokens;
		Temperature = Settings->DefaultOptions.Temperature;
		TopP = Settings->DefaultOptions.TopP;
		Choices = Settings->DefaultOptions.Choices;
		bStream = Settings->DefaultOptions.bStream;
		Stop = Settings->DefaultOptions.Stop;
		PresencePenalty = Settings->DefaultOptions.PresencePenalty;
		FrequencyPenalty = Settings->DefaultOptions.FrequencyPenalty;
		LogitBias = Settings->DefaultOptions.LogitBias;
		User = Settings->DefaultOptions.User;
	}
}

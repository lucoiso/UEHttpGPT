// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "HttpGPTTypes.h"
#include "HttpGPTSettings.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(HttpGPTTypes)
#endif

FHttpGPTMessage::FHttpGPTMessage(const FName& Role, const FString& Content)
{
	if (Role.IsEqual("user", ENameCase::IgnoreCase))
	{
		this->Role = EHttpGPTRole::User;
	}
	else if (Role.IsEqual("assistant", ENameCase::IgnoreCase))
	{
		this->Role = EHttpGPTRole::Assistant;
	}
	else if (Role.IsEqual("system", ENameCase::IgnoreCase))
	{
		this->Role = EHttpGPTRole::System;
	}
	else
	{
		this->Role = EHttpGPTRole::User;
	}

	this->Content = Content;
}

TSharedPtr<FJsonValue> FHttpGPTMessage::GetMessage() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	FString MessageRole;
	switch (Role)
	{
		case EHttpGPTRole::User:
			MessageRole = "user";
			break;

		case EHttpGPTRole::Assistant:
			MessageRole = "assistant";
			break;

		case EHttpGPTRole::System:
			MessageRole = "system";
			break;

		default:
			MessageRole = "user";
			break;
	}

	JsonObject->SetStringField("role", MessageRole);
	JsonObject->SetStringField("content", Content);

	return MakeShareable(new FJsonValueObject(JsonObject));
}

FHttpGPTOptions::FHttpGPTOptions()
{
	SetDefaults();
}

void FHttpGPTOptions::SetDefaults()
{
	if (const UHttpGPTSettings* const Settings = GetDefault<UHttpGPTSettings>())
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

// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

#include "HttpGPTTypes.h"

#ifdef UE_INLINE_GENERATED_CPP_BY_NAME
#include UE_INLINE_GENERATED_CPP_BY_NAME(HttpGPTTypes)
#endif

TSharedPtr<FJsonValue> FHttpGPTMessage::GetMessage() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("role", Role.ToString().ToLower());
	JsonObject->SetStringField("content", Content);

	return MakeShareable(new FJsonValueObject(JsonObject));
}

// Author: Lucas Vilas-Boas
// Year: 2023
// Repo: https://github.com/lucoiso/UEHttpGPT

// These structures are defined in the common module due to being used in both modules, to avoid cyclic dependencies.

#pragma once

#include <CoreMinimal.h>
#include "Structures/HttpGPTCommonTypes.h"
#include "HttpGPTImageTypes.generated.h"

UENUM(BlueprintType, Category = "HttpGPT | Image", Meta = (DisplayName = "HttpGPT Image Size"))
enum class EHttpGPTImageSize : uint8
{
	x256		UMETA(DisplayName = "256x256"),
	x512		UMETA(DisplayName = "512x512"),
	x1024		UMETA(DisplayName = "1024x1024"),
};

UENUM(BlueprintType, Category = "HttpGPT | Image", Meta = (DisplayName = "HttpGPT Image Response Format"))
enum class EHttpGPTResponseFormat : uint8
{
	url			UMETA(DisplayName = "URL"),
	b64_json	UMETA(DisplayName = "B64")
};

USTRUCT(BlueprintType, Category = "HttpGPT | Image", Meta = (DisplayName = "HttpGPT Image Response"))
struct HTTPGPTCOMMONMODULE_API FHttpGPTImageResponse
{
	GENERATED_BODY()

	FHttpGPTImageResponse() = default;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Image")
	int32 Created = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Image")
	TArray<FString> Data;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Image")
	bool bSuccess = false;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Image")
	FHttpGPTCommonError Error;
};

USTRUCT(BlueprintType, Category = "HttpGPT | Image", Meta = (DisplayName = "HttpGPT Image Options"))
struct HTTPGPTCOMMONMODULE_API FHttpGPTImageOptions
{
	GENERATED_BODY()

	FHttpGPTImageOptions();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Image", Meta = (DisplayName = "Number of Images", ClampMin = "1", UIMin = "1", ClampMax = "10", UIMax = "10"))
	int32 ImagesNum;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Image", Meta = (DisplayName = "Image Size"))
	EHttpGPTImageSize Size;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HttpGPT | Image", Meta = (DisplayName = "Response Format"))
	EHttpGPTResponseFormat Format;

private:
	void SetDefaults();
};
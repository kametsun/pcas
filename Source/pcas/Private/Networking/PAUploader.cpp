#include "Networking/PAUploader.h"

#include "Constants/PAConstants.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Networking/PAHttpClient.h"

DEFINE_LOG_CATEGORY(LogPAUploader);

void UPAUploader::Initialize(UPAHttpClient* InClient)
{
	HttpClient = InClient;
}

void UPAUploader::UploadImage(const FString& LocationId, const FString& FilePath, FUploadCallback Callback)
{
	if (!HttpClient)
	{
		Callback(false, TEXT("Http client not initialized"));
		return;
	}

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		const FString ErrorText = FString::Printf(TEXT("Failed to load screenshot: %s"), *FilePath);
		UE_LOG(LogPAUploader, Error, TEXT("%s"), *ErrorText);
		Callback(false, ErrorText);
		return;
	}

	const FString FileName = FPaths::GetCleanFilename(FilePath);
	const FString UploadUrl = UPAConstants::GetUploadUrl();
	const FString JsonPayload = FString::Printf(TEXT("{\"locationId\":\"%s\"}"), *LocationId);

	HttpClient->PostMultipart(
		UploadUrl,
		FileData,
		TEXT("file"),
		FileName,
		TEXT("image/png"),
		TEXT("json"),
		JsonPayload,
		[Callback](bool bSuccess, const FString& ErrorMessage)
		{
			Callback(bSuccess, ErrorMessage);
		});
}

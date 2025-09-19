#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "PAHttpClient.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPAHttp, Log, All);

UCLASS()
class PCAS_API UPAHttpClient : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UWorld* InWorld);

	using FJsonResponseCallback = TFunction<void(bool bSuccess, const TSharedPtr<FJsonObject>& JsonObject, const FString& ErrorMessage)>;
	using FRawResponseCallback = TFunction<void(bool bSuccess, const FString& ErrorMessage)>;

	void GetJson(const FString& Url, FJsonResponseCallback Callback);

	void PostMultipart(const FString& Url,
		const TArray<uint8>& FileData,
		const FString& FileFieldName,
		const FString& FileName,
		const FString& FileContentType,
		const FString& JsonFieldName,
		const FString& JsonPayload,
		FRawResponseCallback Callback,
		int32 Attempt = 0);

private:
	void PostMultipartInternal(const FString& Url,
		const TArray<uint8>& FileData,
		const FString& FileFieldName,
		const FString& FileName,
		const FString& FileContentType,
		const FString& JsonFieldName,
		const FString& JsonPayload,
		TSharedRef<FRawResponseCallback> CallbackRef,
		int32 Attempt);

	void HandleJsonResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, const FJsonResponseCallback& Callback);
	void RetryDelayed(const TFunction<void()>& Action, double DelaySeconds);

private:
	TWeakObjectPtr<UWorld> CachedWorld;
};

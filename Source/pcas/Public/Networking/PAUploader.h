#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PAUploader.generated.h"

class UPAHttpClient;

DECLARE_LOG_CATEGORY_EXTERN(LogPAUploader, Log, All);

UCLASS()
class PCAS_API UPAUploader : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UPAHttpClient* InClient);

	typedef TFunction<void(bool bSuccess, const FString& ErrorMessage)> FUploadCallback;

	void UploadImage(const FString& LocationId, const FString& FilePath, FUploadCallback Callback);

private:
	UPAHttpClient* HttpClient = nullptr;
};

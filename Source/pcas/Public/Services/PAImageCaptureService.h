#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PAImageCaptureService.generated.h"

class USceneCaptureComponent2D;

DECLARE_LOG_CATEGORY_EXTERN(LogPAImageCapture, Log, All);

UCLASS()
class PCAS_API UPAImageCaptureService : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(USceneCaptureComponent2D* InCaptureComponent);

	typedef TFunction<void(bool bSuccess, const FString& FilePath, const FString& ErrorMessage)> FCaptureCallback;

	void CaptureScreenshot(const FString& AbsoluteFilePath, FCaptureCallback Callback);

private:
	USceneCaptureComponent2D* CaptureComponent = nullptr;
};

#pragma once

#include "CoreMinimal.h"

class UPCAProjectSettings;

/**
 * 設定値や共通ユーティリティを提供する静的クラス。
 */
class PCAS_API UPAConstants final
{
public:
	static const UPCAProjectSettings* GetSettings();

	static FString GetBackendOrigin();
	static FString MakeBackendUrl(const FString& Path);
	static FString GetTilesListUrl();
	static FString GetRoutesUrl();
	static FString GetUploadUrl();

	static FString GetScreenshotDirectory();

	static double GetTilesLoadGracePeriod();
	static double GetPreCaptureStabilizationSeconds();

	static int32 GetUploadRetryCount();
	static double GetUploadRetryBackoffSeconds();

	static double GetOriginLatitude();
	static double GetOriginLongitude();
	static double GetOriginHeight();

	static double GetDirectionOffsetDegrees();

	static bool ShouldExitApplicationOnFinish();
	static bool ShouldExitEditorOnFinish();

	static FString GetWorldMapName();

	static double GetMovementBlendTimeSeconds();
	static double GetRotationBlendTimeSeconds();

	static FIntPoint GetCaptureResolution();
};

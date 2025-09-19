#include "Config/PAProjectSettings.h"

UPCAProjectSettings::UPCAProjectSettings()
{
	BackendOrigin = TEXT("http://10.123.0.160");
	TilesListPath = TEXT("/tiles/list");
	RoutesPath = TEXT("/routes/real");
	UploadPath = TEXT("/images/upload");

	OriginLatitude = 34.972910;
	OriginLongitude = 138.387616;
	OriginHeight = 50.0; // 既定で地上から少し上に配置

	ScreenshotSubdirectory = TEXT("Screenshots/Auto");
	CaptureResolution = FIntPoint(1920, 1080);

	TilesLoadGracePeriodSeconds = 5.0;
	PreCaptureStabilizationSeconds = 3.0;

	UploadRetryCount = 3;
	UploadRetryBackoffSeconds = 2.0;

	DirectionOffsetDegrees = 0.0;

	bExitApplicationOnFinish = true;
	bExitEditorOnFinish = false;

	DefaultWorldMapName = TEXT("/Game/Maps/World");

	MovementBlendTimeSeconds = 1.0;
	RotationBlendTimeSeconds = 0.5;
}

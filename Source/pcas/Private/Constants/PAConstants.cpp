#include "Constants/PAConstants.h"

#include "Config/PAProjectSettings.h"
#include "Misc/Paths.h"

namespace
{
	FString MakeUrl(const FString& Origin, const FString& Path)
	{
		if (Path.StartsWith(TEXT("http")))
		{
			return Path;
		}

		if (Origin.EndsWith(TEXT("/")) && Path.StartsWith(TEXT("/")))
		{
			return Origin + Path.Mid(1);
		}

		if (!Origin.EndsWith(TEXT("/")) && !Path.StartsWith(TEXT("/")))
		{
			return FString::Printf(TEXT("%s/%s"), *Origin, *Path);
		}

		return Origin + Path;
	}
}

const UPCAProjectSettings* UPAConstants::GetSettings()
{
	return GetDefault<UPCAProjectSettings>();
}

FString UPAConstants::GetBackendOrigin()
{
	return GetSettings()->BackendOrigin;
}

FString UPAConstants::GetTilesListUrl()
{
	const UPCAProjectSettings* Settings = GetSettings();
	return MakeUrl(Settings->BackendOrigin, Settings->TilesListPath);
}

FString UPAConstants::GetRoutesUrl()
{
	const UPCAProjectSettings* Settings = GetSettings();
	return MakeUrl(Settings->BackendOrigin, Settings->RoutesPath);
}

FString UPAConstants::GetUploadUrl()
{
	const UPCAProjectSettings* Settings = GetSettings();
	return MakeUrl(Settings->BackendOrigin, Settings->UploadPath);
}

FString UPAConstants::GetScreenshotDirectory()
{
	const UPCAProjectSettings* Settings = GetSettings();
	const FString BaseDir = FPaths::ProjectSavedDir();
	if (Settings->ScreenshotSubdirectory.IsEmpty())
	{
		return FPaths::Combine(BaseDir, TEXT("Screenshots"));
	}

	return FPaths::Combine(BaseDir, Settings->ScreenshotSubdirectory);
}

double UPAConstants::GetTilesLoadGracePeriod()
{
	return GetSettings()->TilesLoadGracePeriodSeconds;
}

double UPAConstants::GetPreCaptureStabilizationSeconds()
{
	return GetSettings()->PreCaptureStabilizationSeconds;
}

int32 UPAConstants::GetUploadRetryCount()
{
	return GetSettings()->UploadRetryCount;
}

double UPAConstants::GetUploadRetryBackoffSeconds()
{
	return GetSettings()->UploadRetryBackoffSeconds;
}

double UPAConstants::GetOriginLatitude()
{
	return GetSettings()->OriginLatitude;
}

double UPAConstants::GetOriginLongitude()
{
	return GetSettings()->OriginLongitude;
}

double UPAConstants::GetOriginHeight()
{
	return GetSettings()->OriginHeight;
}

double UPAConstants::GetDirectionOffsetDegrees()
{
	return GetSettings()->DirectionOffsetDegrees;
}

bool UPAConstants::ShouldExitApplicationOnFinish()
{
	return GetSettings()->bExitApplicationOnFinish;
}

bool UPAConstants::ShouldExitEditorOnFinish()
{
	return GetSettings()->bExitEditorOnFinish;
}

FString UPAConstants::GetWorldMapName()
{
	return GetSettings()->DefaultWorldMapName;
}

double UPAConstants::GetMovementBlendTimeSeconds()
{
	return GetSettings()->MovementBlendTimeSeconds;
}

double UPAConstants::GetRotationBlendTimeSeconds()
{
	return GetSettings()->RotationBlendTimeSeconds;
}

FIntPoint UPAConstants::GetCaptureResolution()
{
	return GetSettings()->CaptureResolution;
}

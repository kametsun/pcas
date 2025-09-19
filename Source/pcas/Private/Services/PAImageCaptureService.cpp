#include "Services/PAImageCaptureService.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "RenderingThread.h"

DEFINE_LOG_CATEGORY(LogPAImageCapture);

void UPAImageCaptureService::Initialize(USceneCaptureComponent2D* InCaptureComponent)
{
	CaptureComponent = InCaptureComponent;
}

void UPAImageCaptureService::CaptureScreenshot(const FString& AbsoluteFilePath, FCaptureCallback Callback)
{
	if (!CaptureComponent)
	{
		Callback(false, AbsoluteFilePath, TEXT("Capture component not initialized"));
		return;
	}

	UTextureRenderTarget2D* RenderTarget = CaptureComponent->TextureTarget;
	if (!RenderTarget)
	{
		Callback(false, AbsoluteFilePath, TEXT("Render target not available"));
		return;
	}

	CaptureComponent->CaptureScene();
	FlushRenderingCommands();

	FTextureRenderTargetResource* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	if (!RenderTargetResource)
	{
		Callback(false, AbsoluteFilePath, TEXT("Failed to access render target resource"));
		return;
	}

	TArray<FColor> Bitmap;
	if (!RenderTargetResource->ReadPixels(Bitmap))
	{
		Callback(false, AbsoluteFilePath, TEXT("Failed to read pixels"));
		return;
	}

	const int32 Width = RenderTarget->SizeX;
	const int32 Height = RenderTarget->SizeY;
	if (Bitmap.Num() != Width * Height)
	{
		Callback(false, AbsoluteFilePath, TEXT("Pixel count mismatch"));
		return;
	}

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(AbsoluteFilePath), true);

	TArray<uint8> PngData;
	FImageUtils::CompressImageArray(Width, Height, Bitmap, PngData);
	if (PngData.Num() == 0)
	{
		Callback(false, AbsoluteFilePath, TEXT("Failed to compress PNG"));
		return;
	}

	if (!FFileHelper::SaveArrayToFile(PngData, *AbsoluteFilePath))
	{
		Callback(false, AbsoluteFilePath, TEXT("Failed to save screenshot"));
		return;
	}

	UE_LOG(LogPAImageCapture, Log, TEXT("Captured screenshot: %s"), *AbsoluteFilePath);
	Callback(true, AbsoluteFilePath, TEXT(""));
}

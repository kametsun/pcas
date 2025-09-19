#include "App/PAAppOrchestrator.h"

#include "Actors/PAAutoShooterController.h"
#include "App/PAPathPlanner.h"
#include "Cesium/PACesiumBootstrapper.h"
#include "Constants/PAConstants.h"
#include "Engine/Engine.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Misc/Paths.h"
#include "Networking/PAHttpClient.h"
#include "Networking/PAUploader.h"
#include "Services/PAImageCaptureService.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogPAOrchestrator);

void UPAAppOrchestrator::Initialize(UWorld* InWorld,
	APACesiumBootstrapper* InBootstrapper,
	APAAutoShooterController* InController)
{
	CachedWorld = InWorld;
	Bootstrapper = InBootstrapper;
	AutoController = InController;

	HttpClient = NewObject<UPAHttpClient>(this);
	HttpClient->Initialize(InWorld);

	PathPlanner = NewObject<UPAPathPlanner>(this);

	CaptureService = NewObject<UPAImageCaptureService>(this);
	if (AutoController)
	{
		CaptureService->Initialize(AutoController->GetCaptureComponent());
	}

	Uploader = NewObject<UPAUploader>(this);
	Uploader->Initialize(HttpClient);
}

void UPAAppOrchestrator::Start()
{
	State = EPAAppState::RequestTiles;
	RequestTiles();
}

void UPAAppOrchestrator::RequestTiles()
{
	UE_LOG(LogPAOrchestrator, Log, TEXT("Requesting tiles list..."));
	HttpClient->GetJson(UPAConstants::GetTilesListUrl(),
		[this](bool bSuccess, const TSharedPtr<FJsonObject>& Json, const FString& ErrorMessage)
		{
			OnTilesResponse(bSuccess, Json, ErrorMessage);
		});
}

void UPAAppOrchestrator::OnTilesResponse(bool bSuccess, const TSharedPtr<FJsonObject>& Json, const FString& ErrorMessage)
{
	if (!bSuccess)
	{
		FailSequence(FString::Printf(TEXT("Tiles request failed: %s"), *ErrorMessage));
		return;
	}

	FText ParseError;
	if (!TilesResponse.FromJson(Json, ParseError))
	{
		FailSequence(ParseError.ToString());
		return;
	}

	if (!Bootstrapper)
	{
		FailSequence(TEXT("Bootstrapper not available"));
		return;
	}

	Bootstrapper->LoadTilesets(TilesResponse.Items);
	State = EPAAppState::WaitTiles;
	WaitForTilesReady();
}

void UPAAppOrchestrator::WaitForTilesReady()
{
	if (!Bootstrapper)
	{
		FailSequence(TEXT("Bootstrapper missing during wait"));
		return;
	}

	if (Bootstrapper->AreTilesReady())
	{
		RequestRoutes();
		return;
	}

	ScheduleRetry([this]()
	{
		WaitForTilesReady();
	}, 1.0);
}

void UPAAppOrchestrator::RequestRoutes()
{
	State = EPAAppState::RequestRoutes;
	UE_LOG(LogPAOrchestrator, Log, TEXT("Requesting route list..."));
	HttpClient->GetJson(UPAConstants::GetRoutesUrl(),
		[this](bool bSuccess, const TSharedPtr<FJsonObject>& Json, const FString& ErrorMessage)
		{
			OnRoutesResponse(bSuccess, Json, ErrorMessage);
		});
}

void UPAAppOrchestrator::OnRoutesResponse(bool bSuccess, const TSharedPtr<FJsonObject>& Json, const FString& ErrorMessage)
{
	if (!bSuccess)
	{
		FailSequence(FString::Printf(TEXT("Route request failed: %s"), *ErrorMessage));
		return;
	}

	FText ParseError;
	if (!RouteResponse.FromJson(Json, ParseError))
	{
		FailSequence(ParseError.ToString());
		return;
	}

	PathPlanner->Reset(RouteResponse.Locations);
	if (!PathPlanner->HasLocations())
	{
		UE_LOG(LogPAOrchestrator, Warning, TEXT("No locations available"));
		FinishSequence();
		return;
	}

	BeginNextLocation();
}

void UPAAppOrchestrator::BeginNextLocation()
{
	State = EPAAppState::Navigate;
	const FPALocation* Location = PathPlanner->GetCurrentLocation();
	if (!Location)
	{
		StopCaptureCountdown();
		FinishSequence();
		return;
	}

	UE_LOG(LogPAOrchestrator, Log, TEXT("Navigating to location %s (%d/%d)"), *Location->Id, PathPlanner->GetIndex() + 1, PathPlanner->GetTotal());

	CurrentLocationId = Location->Id;
	if (AutoController)
	{
		AutoController->BlendToLocation(*Location, UPAConstants::GetMovementBlendTimeSeconds(), UPAConstants::GetRotationBlendTimeSeconds());
	}

	const double MoveDelay = UPAConstants::GetMovementBlendTimeSeconds();
	const double WaitSeconds = UPAConstants::GetPreCaptureStabilizationSeconds();
	StartCaptureCountdown(FMath::Max(0.0, MoveDelay + WaitSeconds));
	if (MoveDelay <= KINDA_SMALL_NUMBER)
	{
		EnterStabilize();
	}
	else
	{
		GetWorldChecked()->GetTimerManager().SetTimer(MovementTimerHandle, [this]() { EnterStabilize(); }, MoveDelay, false);
	}
}

void UPAAppOrchestrator::EnterStabilize()
{
	State = EPAAppState::Stabilize;
	GetWorldChecked()->GetTimerManager().ClearTimer(MovementTimerHandle);
	const double WaitSeconds = UPAConstants::GetPreCaptureStabilizationSeconds();
	if (WaitSeconds <= KINDA_SMALL_NUMBER)
	{
		HandleCapture();
	}
	else
	{
		GetWorldChecked()->GetTimerManager().SetTimer(StabilizeTimerHandle, [this]() { HandleCapture(); }, WaitSeconds, false);
	}
}

void UPAAppOrchestrator::HandleCapture()
{
	GetWorldChecked()->GetTimerManager().ClearTimer(StabilizeTimerHandle);
	StopCaptureCountdown();

	State = EPAAppState::Capture;
	const FPALocation* Location = PathPlanner->GetCurrentLocation();
	if (!Location)
	{
		FailSequence(TEXT("Location disappeared before capture"));
		return;
	}

	const FString Directory = UPAConstants::GetScreenshotDirectory();
	CurrentScreenshotPath = FPaths::Combine(Directory, FString::Printf(TEXT("%s.png"), *Location->Id));

	if (!CaptureService)
	{
		FailSequence(TEXT("Capture service not initialized"));
		return;
	}

	CaptureService->CaptureScreenshot(CurrentScreenshotPath,
		[this](bool bSuccess, const FString& FilePath, const FString& ErrorMessage)
		{
			if (!bSuccess)
			{
				FailSequence(FString::Printf(TEXT("Capture failed: %s"), *ErrorMessage));
				return;
			}

			HandleUpload(FilePath);
		});
}

void UPAAppOrchestrator::HandleUpload(const FString& ScreenshotPath)
{
	State = EPAAppState::Upload;
	const FPALocation* Location = PathPlanner->GetCurrentLocation();
	if (!Location)
	{
		FailSequence(TEXT("Location disappeared before upload"));
		return;
	}

	if (!Uploader)
	{
		FailSequence(TEXT("Uploader not initialized"));
		return;
	}

	Uploader->UploadImage(Location->Id, ScreenshotPath,
		[this](bool bSuccess, const FString& ErrorMessage)
		{
			if (!bSuccess)
			{
				FailSequence(FString::Printf(TEXT("Upload failed: %s"), *ErrorMessage));
				return;
			}

			State = EPAAppState::Advance;
			PathPlanner->Advance();
			BeginNextLocation();
		});
}

void UPAAppOrchestrator::FinishSequence()
{
	State = EPAAppState::Shutdown;
	UE_LOG(LogPAOrchestrator, Log, TEXT("Sequence completed."));
	StopCaptureCountdown();

	const bool bExitOnFinish = UPAConstants::ShouldExitApplicationOnFinish();
	if (!bExitOnFinish)
	{
		return;
	}

#if WITH_EDITOR
	const bool bIsEditor = GIsEditor;
#else
	const bool bIsEditor = false;
#endif

	if (bIsEditor && !UPAConstants::ShouldExitEditorOnFinish())
	{
		UE_LOG(LogPAOrchestrator, Log, TEXT("Completed in editor - not exiting."));
		return;
	}

	FGenericPlatformMisc::RequestExit(false);
}

void UPAAppOrchestrator::FailSequence(const FString& Reason)
{
	State = EPAAppState::Error;
	UE_LOG(LogPAOrchestrator, Error, TEXT("Sequence failed: %s"), *Reason);
	StopCaptureCountdown();
	FinishSequence();
}

void UPAAppOrchestrator::StartCaptureCountdown(double TotalSeconds)
{
	StopCaptureCountdown();

	if (TotalSeconds <= KINDA_SMALL_NUMBER || !CachedWorld.IsValid())
	{
		CaptureCountdownDeadlineSeconds = 0.0;
		return;
	}

	UWorld* World = GetWorldChecked();
	CaptureCountdownDeadlineSeconds = World->GetTimeSeconds() + TotalSeconds;

	LogCaptureCountdown();

	const double IntervalSeconds = 5.0;
	const double FirstDelay = FMath::Min(IntervalSeconds, TotalSeconds);
	if (FirstDelay <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	World->GetTimerManager().SetTimer(CaptureCountdownTimerHandle, this, &UPAAppOrchestrator::HandleCaptureCountdownTick, IntervalSeconds, true, FirstDelay);
}

void UPAAppOrchestrator::StopCaptureCountdown()
{
	if (CachedWorld.IsValid())
	{
		CachedWorld->GetTimerManager().ClearTimer(CaptureCountdownTimerHandle);
	}
	CaptureCountdownDeadlineSeconds = 0.0;
}

void UPAAppOrchestrator::HandleCaptureCountdownTick()
{
	LogCaptureCountdown();
	if (GetCaptureCountdownRemainingSeconds() <= KINDA_SMALL_NUMBER)
	{
		StopCaptureCountdown();
	}
}

void UPAAppOrchestrator::LogCaptureCountdown() const
{
	if (CurrentLocationId.IsEmpty())
	{
		return;
	}

	const double RemainingSeconds = GetCaptureCountdownRemainingSeconds();
	const int32 Index = PathPlanner ? PathPlanner->GetIndex() + 1 : 0;
	const int32 Total = PathPlanner ? PathPlanner->GetTotal() : 0;

	const TCHAR* Phase = TEXT("pending");
	if (State == EPAAppState::Navigate)
	{
		Phase = TEXT("navigating");
	}
	else if (State == EPAAppState::Stabilize)
	{
		Phase = TEXT("stabilizing");
	}

	if (RemainingSeconds > KINDA_SMALL_NUMBER)
	{
		UE_LOG(LogPAOrchestrator, Log, TEXT("[Countdown][%s] Location %s (%d/%d) capture in %.1f seconds"), Phase, *CurrentLocationId, Index, Total, RemainingSeconds);
	}
	else
	{
		UE_LOG(LogPAOrchestrator, Log, TEXT("[Countdown][%s] Location %s (%d/%d) capture imminent"), Phase, *CurrentLocationId, Index, Total);
	}
}

double UPAAppOrchestrator::GetCaptureCountdownRemainingSeconds() const
{
	if (!CachedWorld.IsValid() || CaptureCountdownDeadlineSeconds <= 0.0)
	{
		return 0.0;
	}

	const double Remaining = CaptureCountdownDeadlineSeconds - CachedWorld->GetTimeSeconds();
	return FMath::Max(0.0, Remaining);
}

void UPAAppOrchestrator::ScheduleRetry(const TFunction<void()>& Action, double DelaySeconds)
{
	if (!CachedWorld.IsValid())
	{
		Action();
		return;
	}

	FTimerDelegate Delegate = FTimerDelegate::CreateLambda([Action]()
	{
		Action();
	});
	FTimerHandle Handle;
	CachedWorld->GetTimerManager().SetTimer(Handle, Delegate, DelaySeconds, false);
}

UWorld* UPAAppOrchestrator::GetWorldChecked() const
{
	UWorld* World = CachedWorld.Get();
	check(World);
	return World;
}

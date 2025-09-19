#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Models/PAJsonModels.h"
#include "PAAppOrchestrator.generated.h"

class APACesiumBootstrapper;
class APAAutoShooterController;
class UPAHttpClient;
class UPAPathPlanner;
class UPAImageCaptureService;
class UPAUploader;

UENUM()
enum class EPAAppState : uint8
{
	Idle,
	RequestTiles,
	WaitTiles,
	RequestRoutes,
	Navigate,
	Stabilize,
	Capture,
	Upload,
	Advance,
	Shutdown,
	Error
};

DECLARE_LOG_CATEGORY_EXTERN(LogPAOrchestrator, Log, All);

UCLASS()
class PCAS_API UPAAppOrchestrator : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UWorld* InWorld,
		APACesiumBootstrapper* InBootstrapper,
		APAAutoShooterController* InController);

	void Start();

private:
	void RequestTiles();
	void OnTilesResponse(bool bSuccess, const TSharedPtr<FJsonObject>& Json, const FString& ErrorMessage);
	void WaitForTilesReady();
	void RequestRoutes();
	void OnRoutesResponse(bool bSuccess, const TSharedPtr<FJsonObject>& Json, const FString& ErrorMessage);
	void BeginNextLocation();
	void EnterStabilize();
	void HandleCapture();
	void HandleUpload(const FString& ScreenshotPath);
	void FinishSequence();
	void FailSequence(const FString& Reason);
	void ScheduleRetry(const TFunction<void()>& Action, double DelaySeconds);
	void StartCaptureCountdown(double TotalSeconds);
	void StopCaptureCountdown();
	void HandleCaptureCountdownTick();
	void LogCaptureCountdown() const;
	double GetCaptureCountdownRemainingSeconds() const;

	UWorld* GetWorldChecked() const;

private:
	EPAAppState State = EPAAppState::Idle;

	UPROPERTY()
	APACesiumBootstrapper* Bootstrapper = nullptr;

	UPROPERTY()
	APAAutoShooterController* AutoController = nullptr;

	UPROPERTY()
	UPAHttpClient* HttpClient = nullptr;

	UPROPERTY()
	UPAPathPlanner* PathPlanner = nullptr;

	UPROPERTY()
	UPAImageCaptureService* CaptureService = nullptr;

	UPROPERTY()
	UPAUploader* Uploader = nullptr;

	TWeakObjectPtr<UWorld> CachedWorld;

	FPATilesListResponse TilesResponse;
	FPARouteResponse RouteResponse;

	FTimerHandle StabilizeTimerHandle;
	FTimerHandle MovementTimerHandle;
	FTimerHandle CaptureCountdownTimerHandle;
	FString CurrentLocationId;
	FString CurrentScreenshotPath;
	double CaptureCountdownDeadlineSeconds = 0.0;
};

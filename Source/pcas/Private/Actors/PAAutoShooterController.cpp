#include "Actors/PAAutoShooterController.h"

#include "Camera/CameraComponent.h"
#include "CesiumGeoreference.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Constants/PAConstants.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Scene.h"
#include "GameFramework/PlayerController.h"

APAAutoShooterController::APAAutoShooterController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Root);

	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComponent"));
	CaptureComponent->SetupAttachment(Root);
	CaptureComponent->bCaptureEveryFrame = false;
	CaptureComponent->bCaptureOnMovement = false;
}

void APAAutoShooterController::Initialize(ACesiumGeoreference* InGeoreference)
{
	Georeference = InGeoreference;
}

void APAAutoShooterController::BeginPlay()
{
	Super::BeginPlay();

	const FIntPoint Resolution = UPAConstants::GetCaptureResolution();
	CaptureRenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("AutoCaptureRenderTarget"));
	CaptureRenderTarget->InitCustomFormat(Resolution.X, Resolution.Y, PF_B8G8R8A8, false);
	CaptureRenderTarget->ClearColor = FLinearColor::Transparent;
	CaptureRenderTarget->TargetGamma = 1.0f; // Tonemapped output is already in display space
	CaptureComponent->TextureTarget = CaptureRenderTarget;
	CaptureComponent->bAlwaysPersistRenderingState = true;
	CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	ConfigureCaptureComponent();
}

void APAAutoShooterController::ApplyLocation(const FPALocation& Location)
{
	LastTargetLocation = FVector::ZeroVector;
	if (Georeference)
	{
		const double AdjustedAltitude = Location.Altitude + UPAConstants::GetAltitudeOffsetMeters();
		LastTargetLocation = Georeference->TransformLongitudeLatitudeHeightPositionToUnreal(
			FVector(Location.Longitude, Location.Latitude, AdjustedAltitude));
	}
	SetActorLocation(LastTargetLocation);
	StartLocation = LastTargetLocation;
	LocationBlendDuration = 0.0;
	ElapsedLocationBlend = 0.0;
}

void APAAutoShooterController::BlendToLocation(const FPALocation& Location, double BlendTimeSeconds, double RotationBlendTimeSeconds)
{
	StartLocation = GetActorLocation();
	if (Georeference)
	{
		const double AdjustedAltitude = Location.Altitude + UPAConstants::GetAltitudeOffsetMeters();
		LastTargetLocation = Georeference->TransformLongitudeLatitudeHeightPositionToUnreal(
			FVector(Location.Longitude, Location.Latitude, AdjustedAltitude));
	}
	else
	{
		LastTargetLocation = FVector::ZeroVector;
	}

	LocationBlendDuration = BlendTimeSeconds;
	ElapsedLocationBlend = 0.0;
	ApplyDirection(Location.Direction, RotationBlendTimeSeconds);
}

void APAAutoShooterController::ApplyDirection(double DirectionDegrees, double BlendTimeSeconds)
{
	const double TargetYaw = DirectionDegrees + UPAConstants::GetDirectionOffsetDegrees();
	StartRotation = GetActorQuat();
	TargetRotation = FQuat(FRotator(0.0f, TargetYaw, 0.0f));
	RotationBlendDuration = BlendTimeSeconds;
	ElapsedRotationBlend = 0.0;
}

void APAAutoShooterController::AttachToPlayerController(APlayerController* PlayerController)
{
	if (PlayerController)
	{
		PlayerController->SetViewTarget(this);
	}
}

void APAAutoShooterController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateInterpolation(DeltaSeconds);
}

void APAAutoShooterController::UpdateInterpolation(float DeltaSeconds)
{
	if (LocationBlendDuration > 0.0)
	{
		ElapsedLocationBlend += DeltaSeconds;
		const double Alpha = FMath::Clamp(ElapsedLocationBlend / LocationBlendDuration, 0.0, 1.0);
		const FVector NewLocation = FMath::Lerp(StartLocation, LastTargetLocation, static_cast<float>(Alpha));
		SetActorLocation(NewLocation);

		if (Alpha >= 1.0)
		{
			LocationBlendDuration = 0.0;
		}
	}

	if (RotationBlendDuration > 0.0)
	{
		ElapsedRotationBlend += DeltaSeconds;
		const double Alpha = FMath::Clamp(ElapsedRotationBlend / RotationBlendDuration, 0.0, 1.0);
		const FQuat NewRotation = FQuat::Slerp(StartRotation, TargetRotation, static_cast<float>(Alpha));
		SetActorRotation(NewRotation);

		if (Alpha >= 1.0)
		{
			RotationBlendDuration = 0.0;
		}
	}
}

void APAAutoShooterController::ConfigureCaptureComponent()
{
	if (!CaptureComponent)
	{
		return;
	}

	if (Camera)
	{
		CaptureComponent->PostProcessSettings = Camera->PostProcessSettings;
		CaptureComponent->PostProcessBlendWeight = Camera->PostProcessBlendWeight;
	}

	if (!UPAConstants::ShouldCaptureUseManualExposure())
	{
		return;
	}

	FPostProcessSettings& Settings = CaptureComponent->PostProcessSettings;
	Settings.bOverride_AutoExposureMethod = true;
	Settings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
	Settings.bOverride_AutoExposureBias = true;
	Settings.AutoExposureBias = UPAConstants::GetCaptureManualExposureBias();
	Settings.bOverride_AutoExposureMinBrightness = true;
	Settings.AutoExposureMinBrightness = 1.0f;
	Settings.bOverride_AutoExposureMaxBrightness = true;
	Settings.AutoExposureMaxBrightness = 1.0f;
}

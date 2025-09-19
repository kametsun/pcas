#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Models/PAJsonModels.h"
#include "PAAutoShooterController.generated.h"

class ACesiumGeoreference;
class UCameraComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

UCLASS()
class PCAS_API APAAutoShooterController : public AActor
{
	GENERATED_BODY()

public:
	APAAutoShooterController();

	void Initialize(ACesiumGeoreference* InGeoreference);

	void ApplyLocation(const FPALocation& Location);
	void BlendToLocation(const FPALocation& Location, double BlendTimeSeconds, double RotationBlendTimeSeconds);

	void ApplyDirection(double DirectionDegrees, double BlendTimeSeconds);

	USceneCaptureComponent2D* GetCaptureComponent() const { return CaptureComponent; }
	UCameraComponent* GetCameraComponent() const { return Camera; }

	void AttachToPlayerController(APlayerController* PlayerController);

	FVector GetLastTargetLocation() const { return LastTargetLocation; }

protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;

private:
	void UpdateInterpolation(float DeltaSeconds);

private:
	UPROPERTY()
	USceneComponent* Root = nullptr;

	UPROPERTY()
	UCameraComponent* Camera = nullptr;

	UPROPERTY()
	USceneCaptureComponent2D* CaptureComponent = nullptr;

	UPROPERTY()
	UTextureRenderTarget2D* CaptureRenderTarget = nullptr;

	UPROPERTY()
	ACesiumGeoreference* Georeference = nullptr;

	FVector LastTargetLocation = FVector::ZeroVector;
	FVector StartLocation = FVector::ZeroVector;
	FQuat StartRotation = FQuat::Identity;
	FQuat TargetRotation = FQuat::Identity;

	double ElapsedLocationBlend = 0.0;
	double LocationBlendDuration = 0.0;
	double ElapsedRotationBlend = 0.0;
	double RotationBlendDuration = 0.0;
};

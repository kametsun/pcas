#include "App/PAAutoShooterGameMode.h"

#include "Actors/PAAutoShooterController.h"
#include "App/PAAppOrchestrator.h"
#include "Cesium/PACesiumBootstrapper.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

APAAutoShooterGameMode::APAAutoShooterGameMode()
{
	DefaultPawnClass = nullptr;
}

void APAAutoShooterGameMode::StartPlay()
{
	Super::StartPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<APACesiumBootstrapper> It(World); It; ++It)
	{
		Bootstrapper = *It;
		break;
	}

	if (!Bootstrapper)
	{
		Bootstrapper = World->SpawnActor<APACesiumBootstrapper>(APACesiumBootstrapper::StaticClass());
	}

	for (TActorIterator<APAAutoShooterController> It(World); It; ++It)
	{
		AutoController = *It;
		break;
	}

	if (!AutoController)
	{
		AutoController = World->SpawnActor<APAAutoShooterController>(APAAutoShooterController::StaticClass());
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		AutoController->AttachToPlayerController(PC);
	}

	Orchestrator = NewObject<UPAAppOrchestrator>(this);
	Orchestrator->Initialize(World, Bootstrapper, AutoController);
	Orchestrator->Start();
}

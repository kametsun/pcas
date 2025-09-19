#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PAAutoShooterGameMode.generated.h"

class APACesiumBootstrapper;
class APAAutoShooterController;
class UPAAppOrchestrator;

UCLASS()
class PCAS_API APAAutoShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APAAutoShooterGameMode();

protected:
	virtual void StartPlay() override;

private:
	UPROPERTY()
	APACesiumBootstrapper* Bootstrapper = nullptr;

	UPROPERTY()
	APAAutoShooterController* AutoController = nullptr;

	UPROPERTY()
	UPAAppOrchestrator* Orchestrator = nullptr;
};

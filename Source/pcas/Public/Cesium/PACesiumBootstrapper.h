#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PAJsonModels.h"
#include "PACesiumBootstrapper.generated.h"

class ACesiumGeoreference;
class ACesiumSunSky;
class ACesium3DTileset;

UCLASS()
class PCAS_API APACesiumBootstrapper : public AActor
{
	GENERATED_BODY()

public:
	APACesiumBootstrapper();

	ACesiumGeoreference* GetGeoreference() const { return Georeference; }

	void SpawnCesiumActors();
	void LoadTilesets(const TArray<FPA3DTilesItem>& Items);
	bool AreTilesReady() const;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	ACesiumGeoreference* Georeference = nullptr;

	UPROPERTY()
	ACesiumSunSky* SunSky = nullptr;

	UPROPERTY()
	TArray<ACesium3DTileset*> Tilesets;

	double TilesSpawnWorldTime = 0.0;
};

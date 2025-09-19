#include "Cesium/PACesiumBootstrapper.h"

#include "Cesium3DTileset.h"
#include "CesiumGeoreference.h"
#include "CesiumSunSky.h"
#include "Constants/PAConstants.h"
#include "Engine/World.h"

APACesiumBootstrapper::APACesiumBootstrapper()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APACesiumBootstrapper::BeginPlay()
{
	Super::BeginPlay();
	SpawnCesiumActors();
}

void APACesiumBootstrapper::SpawnCesiumActors()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (!Georeference)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		Georeference = World->SpawnActor<ACesiumGeoreference>(ACesiumGeoreference::StaticClass(), FTransform::Identity, SpawnParams);
		if (Georeference)
		{
			Georeference->SetOriginLongitudeLatitudeHeight(
				FVector(
					UPAConstants::GetOriginLongitude(),
					UPAConstants::GetOriginLatitude(),
					UPAConstants::GetOriginHeight()));
		}
	}

	if (!SunSky)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SunSky = World->SpawnActor<ACesiumSunSky>(ACesiumSunSky::StaticClass(), FTransform::Identity, SpawnParams);
	}
}

void APACesiumBootstrapper::LoadTilesets(const TArray<FPA3DTilesItem>& Items)
{
	UWorld* World = GetWorld();
	if (!World || !Georeference)
	{
		return;
	}

	for (ACesium3DTileset* Tileset : Tilesets)
	{
		if (Tileset)
		{
			Tileset->Destroy();
		}
	}
	Tilesets.Empty();

	for (const FPA3DTilesItem& Item : Items)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		ACesium3DTileset* Tileset = World->SpawnActor<ACesium3DTileset>(ACesium3DTileset::StaticClass(), FTransform::Identity, SpawnParams);
		if (!Tileset)
		{
			continue;
		}

		Tileset->AttachToActor(Georeference, FAttachmentTransformRules::KeepRelativeTransform);
		Tileset->SetTilesetSource(ETilesetSource::FromUrl);
		Tileset->SetUrl(Item.Url);
		Tilesets.Add(Tileset);
	}

	TilesSpawnWorldTime = World->GetTimeSeconds();
}

bool APACesiumBootstrapper::AreTilesReady() const
{
	if (!GetWorld())
	{
		return false;
	}

	if (Tilesets.Num() == 0)
	{
		return false;
	}

	const double Elapsed = GetWorld()->GetTimeSeconds() - TilesSpawnWorldTime;
	return Elapsed >= UPAConstants::GetTilesLoadGracePeriod();
}

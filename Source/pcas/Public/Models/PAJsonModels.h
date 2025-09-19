#pragma once

#include "CoreMinimal.h"
#include "PAJsonModels.generated.h"

USTRUCT()
struct PCAS_API FPA3DTilesItem
{
	GENERATED_BODY()

	UPROPERTY()
	FString Category;

	UPROPERTY()
	FString Name;

	UPROPERTY()
	FString Url;
};

USTRUCT()
struct PCAS_API FPATilesListResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPA3DTilesItem> Items;

	bool FromJson(const TSharedPtr<FJsonObject>& JsonObject, FText& OutError);
};

USTRUCT()
struct PCAS_API FPALocation
{
	GENERATED_BODY()

	UPROPERTY()
	FString Id;

	UPROPERTY()
	double Latitude = 0.0;

	UPROPERTY()
	double Longitude = 0.0;

	UPROPERTY()
	double Direction = 0.0;

	UPROPERTY()
	double Altitude = 0.0;
};

USTRUCT()
struct PCAS_API FPARouteResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPALocation> Locations;

	bool FromJson(const TSharedPtr<FJsonObject>& JsonObject, FText& OutError);
};

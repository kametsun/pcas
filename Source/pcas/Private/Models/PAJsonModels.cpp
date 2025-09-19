#include "Models/PAJsonModels.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

bool FPATilesListResponse::FromJson(const TSharedPtr<FJsonObject>& JsonObject, FText& OutError)
{
	if (!JsonObject.IsValid())
	{
		OutError = FText::FromString(TEXT("Tiles list response is null"));
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* ItemsJson = nullptr;
	if (!JsonObject->TryGetArrayField(TEXT("items"), ItemsJson) || !ItemsJson)
	{
		OutError = FText::FromString(TEXT("Missing 'items' array"));
		return false;
	}

	Items.Reset();
	for (const TSharedPtr<FJsonValue>& Value : *ItemsJson)
	{
		if (!Value.IsValid())
		{
			continue;
		}

		TSharedPtr<FJsonObject> ItemObject = Value->AsObject();
		if (!ItemObject.IsValid())
		{
			continue;
		}

		FPA3DTilesItem Item;
		Item.Category = ItemObject->GetStringField(TEXT("category"));
		Item.Name = ItemObject->GetStringField(TEXT("name"));
		Item.Url = ItemObject->GetStringField(TEXT("url"));
		Items.Add(Item);
	}

	return true;
}

bool FPARouteResponse::FromJson(const TSharedPtr<FJsonObject>& JsonObject, FText& OutError)
{
	if (!JsonObject.IsValid())
	{
		OutError = FText::FromString(TEXT("Route response is null"));
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* LocationsJson = nullptr;
	if (!JsonObject->TryGetArrayField(TEXT("locations"), LocationsJson) || !LocationsJson)
	{
		OutError = FText::FromString(TEXT("Missing 'locations' array"));
		return false;
	}

	Locations.Reset();
	for (const TSharedPtr<FJsonValue>& Value : *LocationsJson)
	{
		if (!Value.IsValid())
		{
			continue;
		}

		TSharedPtr<FJsonObject> LocationObject = Value->AsObject();
		if (!LocationObject.IsValid())
		{
			continue;
		}

		FPALocation Location;
		Location.Id = LocationObject->GetStringField(TEXT("id"));
		Location.Latitude = LocationObject->GetNumberField(TEXT("latitude"));
		Location.Longitude = LocationObject->GetNumberField(TEXT("longitude"));
		Location.Direction = LocationObject->GetNumberField(TEXT("direction"));
		Location.Altitude = LocationObject->GetNumberField(TEXT("altitude"));
		Locations.Add(Location);
	}

	return true;
}

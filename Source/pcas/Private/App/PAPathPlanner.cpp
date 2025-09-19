#include "App/PAPathPlanner.h"

void UPAPathPlanner::Reset(const TArray<FPALocation>& InLocations)
{
	Locations = InLocations;
	CurrentIndex = 0;
}

const FPALocation* UPAPathPlanner::GetCurrentLocation() const
{
	if (Locations.IsValidIndex(CurrentIndex))
	{
		return &Locations[CurrentIndex];
	}
	return nullptr;
}

const FPALocation* UPAPathPlanner::Advance()
{
	if (Locations.IsValidIndex(CurrentIndex + 1))
	{
		++CurrentIndex;
		return &Locations[CurrentIndex];
	}

	++CurrentIndex;
	return nullptr;
}

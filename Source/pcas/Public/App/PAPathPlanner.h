#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PAJsonModels.h"
#include "PAPathPlanner.generated.h"

UCLASS()
class PCAS_API UPAPathPlanner : public UObject
{
	GENERATED_BODY()

public:
	void Reset(const TArray<FPALocation>& InLocations);

	const FPALocation* GetCurrentLocation() const;
	const FPALocation* Advance();

	bool HasLocations() const { return Locations.Num() > 0; }
	bool IsFinished() const { return Locations.Num() == 0 || CurrentIndex >= Locations.Num(); }
	int32 GetIndex() const { return CurrentIndex; }
	int32 GetTotal() const { return Locations.Num(); }

private:
	TArray<FPALocation> Locations;
	int32 CurrentIndex = 0;
};

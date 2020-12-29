#pragma once
#include "IExamInterface.h"
#include <vector>
#include "structs.h"

class WorldState;
namespace utils {
	vector<HouseInfo> GetHousesInFOV(IExamInterface* pInterface);
	vector<EntityInfo> GetEntitiesInFOV(IExamInterface* pInterface);

	void AddActionProperty(GOAPProperty* pProperty, std::vector<GOAPProperty*>& properties, WorldState* pWorldState, bool defaultValue = false);
}
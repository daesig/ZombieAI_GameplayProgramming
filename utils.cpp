#include "stdafx.h"
#include "utils.h"
#include "WorldState.h"

vector<HouseInfo> utils::GetHousesInFOV(IExamInterface* pInterface)
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> utils::GetEntitiesInFOV(IExamInterface* pInterface)
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}

void utils::AddActionProperty(GOAPProperty* pProperty, std::vector<GOAPProperty*>& properties, WorldState* pWorldState, bool defaultValue)
{
	properties.push_back(pProperty);

	// Make sure the states exist in the world
	if (!pWorldState->DoesStateExist(pProperty->propertyKey))
	{
		// State doesn't exist, add the state with some default starter value
		pWorldState->AddState(pProperty->propertyKey, defaultValue);
	}
}

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

inline bool utils::IsPointInHouse(const Elite::Vector2& point, const HouseInfo& house, float margin)
{
	return true;
}

bool utils::VitalStatisticsAreOk(WorldState* pWorldState)
{
	bool requiresFood{ true };
	bool requiresHealth{ true };
	bool hasFood{ false };
	bool hasMedkit{ false };
	bool success{ true };
	success = pWorldState->GetState("RequiresFood", requiresFood) && pWorldState->GetState("RequiresHealth", requiresHealth)
		&& pWorldState->GetState("HasFood", hasFood) && pWorldState->GetState("HasMedkit", hasMedkit);

	if (!success)
		std::cout << "Error reading vitals!\n";

	bool needFood{false};
	bool needHealth{false};

	if (requiresFood)
	{
		if (hasFood)
		{
			needFood = true;
		}
	}

	if (requiresHealth)
	{
		if (hasMedkit)
		{
			needHealth = true;
		}
	}

	return !needFood || !needHealth;
}

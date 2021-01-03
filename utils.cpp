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

std::vector<GOAPProperty*> utils::GetUnsatisfiedActionEffects(const std::vector<GOAPProperty*>& effects, WorldState* pWorldState)
{
	std::vector<GOAPProperty*> unsatisfiedEffects{};

	for (GOAPProperty* pEffect : effects)
	{
		if (!pWorldState->IsStateMet(pEffect->propertyKey, pEffect->value.bValue))
		{
			unsatisfiedEffects.push_back(pEffect);
		}
	}

	return unsatisfiedEffects;
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
			return false;
		}
	}

	if (requiresHealth)
	{
		if (hasMedkit)
		{
			return false;
		}
	}

	return true;
}

float utils::GetCorrectedOrientationAngleInDeg(float orientationAngleRad)
{
	float orientationAngle = orientationAngleRad * 180.f / float(M_PI);
	// If positive
	if (orientationAngle >= 0.f)
		orientationAngle -= 90.f;
	else
	{
		// If [-180, -90]
		if (orientationAngle <= -90.f)
		{
			orientationAngle = abs(orientationAngle);
			float temp = abs(orientationAngle - 180.f);
			orientationAngle = 90.f + temp;
		}
		// If ]-90, 0[
		else
		{
			orientationAngle = abs(orientationAngle);
			float temp = abs(orientationAngle - 90.f);
			orientationAngle = temp - 180.f;
		}
	}

	return orientationAngle;
}

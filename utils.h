#pragma once
#include "IExamInterface.h"
#include <vector>
#include "structs.h"

class WorldState;
namespace utils {
	vector<HouseInfo> GetHousesInFOV(IExamInterface* pInterface);
	vector<EntityInfo> GetEntitiesInFOV(IExamInterface* pInterface);

	void AddActionProperty(GOAPProperty* pProperty, std::vector<GOAPProperty*>& properties, WorldState* pWorldState, bool defaultValue = false);
	std::vector<GOAPProperty*> GetUnsatisfiedActionEffects(const std::vector<GOAPProperty*>& effects, WorldState* pWorldState);

	inline bool IsPointInHouse(const Elite::Vector2& point, const HouseInfo& house, float margin);

	bool VitalStatisticsAreOk(WorldState* pWorldState);

	float GetCorrectedOrientationAngleInDeg(float orientationAngleRad);
}

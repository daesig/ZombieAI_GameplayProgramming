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

	bool IsPointInRect(const Elite::Vector2& point, const Elite::Vector2 centerPoint, const Elite::Vector2& size, float margin = 3.f);
	bool IsPointInCircle(const Elite::Vector2& point, const Elite::Vector2& circleCenter, float circleRadius);
	bool IsLocationInsideGivenPurgezones(const Elite::Vector2& point, const std::vector<SpottedPurgeZone>& purgezones, SpottedPurgeZone& purgezone);

	bool VitalStatisticsAreOk(WorldState* pWorldState);

	float GetCorrectedOrientationAngleInDeg(float orientationAngleRad);
}

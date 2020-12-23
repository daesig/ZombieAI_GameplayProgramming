#pragma once
#include "IExamInterface.h"
#include <vector>

namespace utils {
	vector<HouseInfo> GetHousesInFOV(IExamInterface* pInterface);
	vector<EntityInfo> GetEntitiesInFOV(IExamInterface* pInterface);
}
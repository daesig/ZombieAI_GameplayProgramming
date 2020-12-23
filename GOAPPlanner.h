#pragma once
#include "GOAPActions.h"
#include <vector>

class WorldState;
class GOAPPlanner
{
public:
	GOAPPlanner();
	void PlanAction();
	GOAPAction* GetAction() const;
	void NextAction();

	void SetWorldState(WorldState* pWorldState);
	void AddAction(GOAPAction* pAction);
private:
	std::vector<GOAPAction*> m_pActions{};
	std::queue<GOAPAction*> m_pActionQueue{};
	WorldState* m_pWorldState;
	int m_CurrentActionIndex = 0;
};
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
	WorldState* GetWorldState();

	void AddAction(GOAPAction* pAction);
	void AddActions(std::vector<GOAPAction*>& m_pActions);
private:
	std::vector<GOAPAction*> m_pActions{};
	std::queue<GOAPAction*> m_pActionQueue{};
	WorldState* m_pWorldState = nullptr;
	int m_CurrentActionIndex = 0;
};
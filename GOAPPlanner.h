#pragma once
#include "GOAPActions.h"
#include <vector>

class GOAPPlanner
{
public:
	void PlanAction();
	GOAPAction* GetAction() const;
	void NextAction();
private:
	std::vector<GOAPAction*> m_pActions;
	int m_CurrentActionIndex;
};


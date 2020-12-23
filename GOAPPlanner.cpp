#include "stdafx.h"
#include "GOAPPlanner.h"

void GOAPPlanner::PlanAction()
{
	// Obtain the best path towards the goal
}

GOAPAction* GOAPPlanner::GetAction() const
{
	if (m_pActions.size() >= m_CurrentActionIndex) {
		return m_pActions[m_CurrentActionIndex];
	}

	return nullptr;
}

void GOAPPlanner::NextAction()
{
	m_CurrentActionIndex++;
}

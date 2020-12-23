#include "stdafx.h"
#include "GOAPPlanner.h"

GOAPPlanner::GOAPPlanner():
	m_pActions{},
	m_CurrentActionIndex{ 0 }
{}

void GOAPPlanner::PlanAction()
{
	// Obtain the best path towards the goal
	m_pActionQueue.empty();
	m_pActionQueue.push(m_pActions[0]);
}

GOAPAction* GOAPPlanner::GetAction() const
{
	if (m_pActionQueue.size() > 0) {
		return m_pActionQueue.front();
	}

	return nullptr;
}

void GOAPPlanner::NextAction()
{
	m_pActionQueue.pop();
}

void GOAPPlanner::SetWorldState(WorldState* pWorldState)
{
	m_pWorldState = pWorldState;
}

void GOAPPlanner::AddAction(GOAPAction* pAction)
{
	m_pActions.push_back(pAction);
}

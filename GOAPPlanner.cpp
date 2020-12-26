#include "stdafx.h"
#include "GOAPPlanner.h"

GOAPPlanner::GOAPPlanner():
	m_pActions{},
	m_CurrentActionIndex{ 0 }
{}

void GOAPPlanner::PlanAction()
{
	// Reset queue of actions
	m_pActionQueue.empty();

	// TODO: Obtain the best path towards the goal according to the current world state

	m_pActionQueue.push(m_pActions[1]);
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

WorldState* GOAPPlanner::GetWorldState()
{
	return m_pWorldState;
}

void GOAPPlanner::AddAction(GOAPAction* pAction)
{
	m_pActions.push_back(pAction);
}

void GOAPPlanner::AddActions(std::vector<GOAPAction*>& m_pActionsToAdd)
{
	for (GOAPAction* pAction : m_pActionsToAdd)
	{
	m_pActions.push_back(pAction);
}
}

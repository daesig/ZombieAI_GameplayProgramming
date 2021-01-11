#include "stdafx.h"
#include "GOAPPlanner.h"
#include "WorldState.h"
#include "ActionSearchAlgorithm.h"

GOAPPlanner::GOAPPlanner(WorldState* pWorldState) :
	m_pActions{},
	m_CurrentActionIndex{ 0 },
	m_pWorldState{ pWorldState }
{
	m_pGoalAction = new GOAPSurvive(this);
	m_pSearchAlgorithm = new ActionSearchAlgorithm(m_pWorldState);
}

GOAPPlanner::~GOAPPlanner()
{
	delete m_pGoalAction;
	m_pGoalAction = nullptr;

	delete m_pSearchAlgorithm;
	m_pSearchAlgorithm = nullptr;
}

bool GOAPPlanner::PlanAction()
{
	// Reset queue of actions
	m_pActionQueue.empty();

	m_pActionQueue = m_pSearchAlgorithm->Search(m_pGoalAction, m_pActions);

	return m_pActionQueue.size() > 0;
}

GOAPAction* GOAPPlanner::GetAction() const
{
	if (m_pActionQueue.size() > 0)
	{
		return m_pActionQueue.front();
	}

	return nullptr;
}

void GOAPPlanner::NextAction()
{
	m_pActionQueue.pop();
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

void GOAPPlanner::SetEncounteredProblem(bool value)
{
	m_EncounteredProblem = value;
}

bool GOAPPlanner::GetEncounteredProblem() const
{
	return m_EncounteredProblem;
}

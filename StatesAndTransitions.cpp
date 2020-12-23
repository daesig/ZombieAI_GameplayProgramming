#include "stdafx.h"
#include "StatesAndTransitions.h"
#include "Agent.h"
#include "GOAPPlanner.h"

// STATES
// -----------

/// IdleState: public FSMState
void IdleState::OnEnter(GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	ResetIdleState();
	// Check if we came from an action
	GOAPAction* pAction = pPlanner->GetAction();
	if (pAction)
	{
		pPlanner->NextAction();
		if (pPlanner->GetAction())
		{
			// Next action exists
			m_HasNext = true;
		}
	}
}
void IdleState::Update(GOAPPlanner* pPlanner, Blackboard* pBlackboard, float deltaTime)
{
	// Don't need a new plan if we have a next action
	if (m_HasNext)
		return;

	// Only plan actions every x seconds
	if (m_ActionTimer <= 0.f)
	{
		// Reset action timer
		m_ActionTimer = m_TimePerActionCheck;

		// Plan the action until one is found
		pPlanner->PlanAction();
	}
	else
		m_ActionTimer -= deltaTime;
}
void IdleState::ResetIdleState()
{
	m_ActionTimer = 0.f;
	m_HasNext = false;
}

void GoToState::OnEnter(GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Entered GoToState\n";
}

// GoToState: public FSMState
void GoToState::Update(GOAPPlanner* pPlanner, Blackboard* pBlackboard, float deltaTime)
{
	AgentInfo* pAgentInfo = nullptr;

	bool obtainedData = pBlackboard->GetData("Agent", pAgentInfo);
	if (!obtainedData)
		return;

	Elite::Vector2 currentAgentPosition = pAgentInfo->Position;

	// TODO: Ask the navmesh for a path and go node to node
}

void PerformState::OnEnter(GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Entered PerformState\n";
}

// Perform state: public FSMState
void PerformState::Update(GOAPPlanner* pPlanner, Blackboard* pBlackboard, float deltaTime)
{
	// Perform until the action is done
	pPlanner->GetAction()->Perform(pBlackboard);
}

// PerformedState: public FSMState
void PerformedState::OnEnter(GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Entered PerformedState\n";
	// The state was performed, go to the next state
	pPlanner->NextAction();
}
/// ------------------



/// Transitions
// GoToTransition: public FSMTransition
bool GoToTransition::ToTransition(GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	// Only start moving towards a point if the action requires movement
	GOAPAction* pAction = pPlanner->GetAction();

	// Action required
	if (!pAction)
		return false;

	// Only transition to goto if movement is required
	if (!pAction->RequiresMovement())
		return false;

	return true;
}

// PerformTransition: public FSMTransition
bool PerformTransition::ToTransition(GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	GOAPAction* pAction = pPlanner->GetAction();

	// Action required
	if (!pAction)
		return false;

	// Only transition to perform if no movement is required
	if (pAction->RequiresMovement())
		return false;

	return true;
}

// PerformCompleteTransition: public FSMTransition
bool PerformedTransition::ToTransition(GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	GOAPAction* pAction = pPlanner->GetAction();

	if (!pAction)
		return false;

	if (pAction->IsDone())
		return true;

	return false;
}


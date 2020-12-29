#include "stdafx.h"
#include "StatesAndTransitions.h"
#include "Agent.h"
#include "GOAPPlanner.h"

// STATES
// ------------------------------------------------------------------------------------------------------------------------------------------------
/// IdleState: public FSMState
void IdleState::OnEnter(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	ResetIdleState();

	// TODO: Priority actions that replan even though other actions aren't done yet?
	bool priorityAction = false;
	bool dataValid = pBlackboard->GetData("PriorityAction", priorityAction);
	if (!dataValid)
	{
		std::cout << "Blackboard data in IdleState::OnEnter is invalid\n";
		return;
	}

	// Don't get the next pending state, re-validate the plan
	if (priorityAction)
		return;

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
void IdleState::Update(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float deltaTime)
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

		std::cout << "Idle update\n";
	}
	else
		m_ActionTimer -= deltaTime;
}
void IdleState::ResetIdleState()
{
	m_ActionTimer = 0.f;
	m_HasNext = false;
}

// GoToState: public FSMState
void GoToState::OnEnter(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	// Debug announcement
	std::cout << "Entered GoToState\n";
	// Setup the action
	pPlanner->GetAction()->Setup(pInterface, pPlanner, pBlackboard);

	// Setup GoTo state
	m_PathRefreshTimer = m_PathRefreshDuration;

	// Get the agent
	Agent* pAgent = nullptr;
	bool foundData = pBlackboard->GetData("Agent", pAgent);
	if (!foundData)
	{
		std::cout << "GoToState::OnEnter, problem fetching data from blackboard\n";
		return;
	}
	// Set agent behavior
	pAgent->SetBehavior(Agent::BehaviorType::SEEK);
}
void GoToState::Update(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float deltaTime)
{
	Agent* pAgent = nullptr;
	bool foundData = pBlackboard->GetData("Agent", pAgent);
	if (!foundData)
	{
		std::cout << "GoToState::Update, problem fetching data from blackboard\n";
		return;
	}

	AgentInfo agentInfo = pInterface->Agent_GetInfo();
	Elite::Vector2 currentAgentPosition = agentInfo.Position;

	// Only fetch a new path every x frames
	if (m_PathRefreshTimer >= m_PathRefreshDuration)
	{
		m_PathRefreshTimer = 0.f;
		Elite::Vector2 closestNode = pInterface->NavMesh_GetClosestPathPoint(pPlanner->GetAction()->GetMoveLocation());
		pAgent->SetSeekPos(closestNode);
	}
	m_PathRefreshTimer += deltaTime;

	pInterface->Draw_SolidCircle(pPlanner->GetAction()->GetMoveLocation(), .5f, Elite::Vector2{}, Elite::Vector3{ 0.f, 1.f, 0.f });
}

// Perform state: public FSMState
void PerformState::OnEnter(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Entered PerformState\n";
	// Setup the action
	pPlanner->GetAction()->Setup(pInterface, pPlanner, pBlackboard);
}
void PerformState::Update(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float deltaTime)
{
	// Perform until the action is done
	pPlanner->GetAction()->Perform(pInterface, pPlanner, pBlackboard, deltaTime);
}
/// ------------------------------------------------------------------------------------------------------------------------------------------------


// TRANSITIONS
// ------------------------------------------------------------------------------------------------------------------------------------------------
// GoToTransition: public FSMTransition
bool GoToTransition::ToTransition(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	// Only start moving towards a point if the action requires movement
	GOAPAction* pAction = pPlanner->GetAction();

	// Action required
	if (!pAction)
		return false;

	// Only transition to goto if movement is required
	if (pAction->RequiresMovement(pInterface, pPlanner, pBlackboard))
		return true;

	return false;

	return true;
}

// PerformTransition: public FSMTransition
bool PerformTransition::ToTransition(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	GOAPAction* pAction = pPlanner->GetAction();

	// Action required
	if (!pAction)
		return false;

	// Only transition to perform if no more movement is required
	if (pAction->RequiresMovement(pInterface, pPlanner, pBlackboard))
		return false;

	return true;
}

// PerformCompleteTransition: public FSMTransition
bool PerformedTransition::ToTransition(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	GOAPAction* pAction = pPlanner->GetAction();

	if (!pAction)
		return false;

	if (pAction->IsDone(pInterface, pPlanner, pBlackboard))
		return true;

	return false;
}
/// ------------------------------------------------------------------------------------------------------------------------------------------------
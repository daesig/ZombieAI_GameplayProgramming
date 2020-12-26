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

		pAction->Plan(pInterface, pPlanner, pBlackboard);
	}
}
void IdleState::Update(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float deltaTime)
{
	std::cout << "Idle update\n";
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

// GoToState: public FSMState
void GoToState::OnEnter(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Entered GoToState\n";
	m_PathRefreshTimer = m_PathRefreshDuration;

	Agent* pAgent = nullptr;
	bool foundData = pBlackboard->GetData("Agent", pAgent);
	if (!foundData)
	{
		std::cout << "GoToState::OnEnter, problem fetching data from blackboard\n";
		return;
}
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
}
void PerformState::Update(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float deltaTime)
{
	// Perform until the action is done
	pPlanner->GetAction()->Perform(pInterface, pPlanner, pBlackboard);
}

// PerformedState: public FSMState
void PerformedState::OnEnter(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Entered PerformedState\n";
	// The state was performed, go to the next state
	pPlanner->NextAction();
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
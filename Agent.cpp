#include "stdafx.h"

#include "Agent.h"
#include "utils.h"
#include "GOAPPlanner.h"
#include "DecisionMaking.h"
#include "Blackboard.h"
#include "FSMState.h"
#include "StatesAndTransitions.h"

Agent::Agent()
{
	Initialize();
}

Agent::~Agent()
{
	DeleteBehaviors();
}

SteeringPlugin_Output Agent::UpdateSteering(IExamInterface* pInterface, float dt)
{
	SteeringPlugin_Output steering{};

	// Update FSM states
	if (m_pDecisionMaking)
	{
		//std::cout << "Updatin...\n";
		m_pDecisionMaking->Update(pInterface, m_pGOAPPlanner, dt);
	}

	AgentInfo agentInfo = pInterface->Agent_GetInfo();
	if (m_pSteeringBehavior)
	{
		steering = m_pSteeringBehavior->CalculateSteering(dt, agentInfo);
	}
	return steering;

	//auto vHousesInFOV = utils::GetHousesInFOV(pInterface);//uses m_pInterface->Fov_GetHouseByIndex(...)
	//auto vEntitiesInFOV = utils::GetEntitiesInFOV(pInterface); //uses m_pInterface->Fov_GetEntityByIndex(...)

	//for (auto& e : vEntitiesInFOV)
	//{
	//	if (e.Type == eEntityType::PURGEZONE)
	//	{
	//		PurgeZoneInfo zoneInfo;
	//		pInterface->PurgeZone_GetInfo(e, zoneInfo);
	//		std::cout << "Purge Zone in FOV:" << e.Location.x << ", " << e.Location.y << " ---EntityHash: " << e.EntityHash << "---Radius: " << zoneInfo.Radius << std::endl;
	//	}
	//}					

}

void Agent::Render(IExamInterface* pExamInterface, float dt) const
{
	//pExamInterface->Draw_SolidCircle(pExam, .7f, { 0,0 }, { 1, 0, 0 });
}

void Agent::SetBehavior(BehaviorType behaviorType)
{
	switch (behaviorType)
	{
	case BehaviorType::WANDER:
		m_pSteeringBehavior = m_pWanderBehavior;
		break;
	}
}

void Agent::Initialize()
{
	// Add the world states
	AddWorldStates();

	// Blackboard
	m_pBlackboard = new Blackboard();

	// Behaviors
	m_pWanderBehavior = new Wander();

	// Initialize GOAP
	InitGOAP();

	// States and transitions
	IdleState* pIdleState = new IdleState();
	GoToState* pGoToState = new GoToState();
	PerformState* pPerformState = new PerformState();
	m_pStates.push_back(pIdleState);
	m_pStates.push_back(pGoToState);
	m_pStates.push_back(pPerformState);
	GoToTransition* pGoToTransition = new GoToTransition();
	PerformTransition* pPerformTransition = new PerformTransition();
	PerformedTransition* performedTransition = new PerformedTransition();
	m_pTransitions.push_back(pGoToTransition);
	m_pTransitions.push_back(pPerformTransition);
	m_pTransitions.push_back(performedTransition);

	// FSM
	m_pFiniteStateMachine = new FiniteStateMachine(pIdleState, nullptr, m_pGOAPPlanner, m_pBlackboard);
	// Transitions that go towards perform
	m_pFiniteStateMachine->AddTransition(pIdleState, pGoToState, pGoToTransition);
	m_pFiniteStateMachine->AddTransition(pIdleState, pPerformState, pPerformTransition);
	m_pFiniteStateMachine->AddTransition(pGoToState, pPerformState, pPerformTransition);
	// Transitions that state completed perform
	m_pFiniteStateMachine->AddTransition(pPerformState, pIdleState, performedTransition);

	m_pDecisionMaking = m_pFiniteStateMachine;
}

void Agent::AddWorldStates()
{
	m_pWorldState = new WorldState();
	m_pWorldState->AddState("HasArrived", false);
}

void Agent::InitGOAP()
{
	// GOAP planner
	m_pGOAPPlanner = new GOAPPlanner();
	m_pGOAPPlanner->SetWorldState(m_pWorldState);

	/// GOAP Actions
	// GOAPMoveTo
	GOAPAction* pGOAPMoveTo = new GOAPMoveTo();
	//...
}

void Agent::DeleteBehaviors()
{
	// Blackboard
	delete m_pBlackboard;
	m_pBlackboard = nullptr;

	// Behaviors
	delete m_pWanderBehavior;
	m_pWanderBehavior = nullptr;

	// States and transitions
	for (FSMState* pState : m_pStates)
	{
		delete pState;
		pState = nullptr;
	}
	for (FSMTransition* pTransition : m_pTransitions)
	{
		delete pTransition;
		pTransition = nullptr;
	}

	// GOAP planner
	delete m_pGOAPPlanner;
	m_pGOAPPlanner = nullptr;
}

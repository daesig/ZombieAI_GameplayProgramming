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
	InitializeBehaviors();
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
		m_pDecisionMaking->Update(m_pGOAPPlanner, dt);
	}









	return steering;


	AgentInfo agentInfo = pInterface->Agent_GetInfo();
	if (m_pSteeringBehavior)
	{
		steering = m_pSteeringBehavior->CalculateSteering(dt, agentInfo);
	}

	return steering;

	//Use the Interface (IAssignmentInterface) to 'interface' with the AI_Framework

	auto vHousesInFOV = utils::GetHousesInFOV(pInterface);//uses m_pInterface->Fov_GetHouseByIndex(...)
	auto vEntitiesInFOV = utils::GetEntitiesInFOV(pInterface); //uses m_pInterface->Fov_GetEntityByIndex(...)

	for (auto& e : vEntitiesInFOV)
	{
		if (e.Type == eEntityType::PURGEZONE)
		{
			PurgeZoneInfo zoneInfo;
			pInterface->PurgeZone_GetInfo(e, zoneInfo);
			std::cout << "Purge Zone in FOV:" << e.Location.x << ", " << e.Location.y << " ---EntityHash: " << e.EntityHash << "---Radius: " << zoneInfo.Radius << std::endl;
		}
	}

	////INVENTORY USAGE DEMO
	////********************
	//if (m_GrabItem)
	//{
	//	ItemInfo item;
	//	//Item_Grab > When DebugParams.AutoGrabClosestItem is TRUE, the Item_Grab function returns the closest item in range
	//	//Keep in mind that DebugParams are only used for debugging purposes, by default this flag is FALSE
	//	//Otherwise, use GetEntitiesInFOV() to retrieve a vector of all entities in the FOV (EntityInfo)
	//	//Item_Grab gives you the ItemInfo back, based on the passed EntityHash (retrieved by GetEntitiesInFOV)
	//	if (pInterface->Item_Grab({}, item))
	//	{
	//		//Once grabbed, you can add it to a specific inventory slot
	//		//Slot must be empty
	//		pInterface->Inventory_AddItem(0, item);
	//	}
	//}

	//if (m_UseItem)
	//{
	//	//Use an item (make sure there is an item at the given inventory slot)
	//	pInterface->Inventory_UseItem(0);
	//}

	//if (m_RemoveItem)
	//{
	//	//Remove an item from a inventory slot
	//	pInterface->Inventory_RemoveItem(0);
	//}

	////Simple Seek Behaviour (towards Target)
	//steering.LinearVelocity = nextTargetPos - agentInfo.Position; //Desired Velocity
	//steering.LinearVelocity.Normalize(); //Normalize Desired Velocity
	//steering.LinearVelocity *= agentInfo.MaxLinearSpeed; //Rescale to Max Speed

	//if (Distance(nextTargetPos, agentInfo.Position) < 2.f)
	//{
	//	steering.LinearVelocity = Elite::ZeroVector2;
	//}

	////steering.AngularVelocity = m_AngSpeed; //Rotate your character to inspect the world while walking
	//steering.AutoOrient = true; //Setting AutoOrientate to TRue overrides the AngularVelocity

	//steering.RunMode = m_CanRun; //If RunMode is True > MaxLinSpd is increased for a limited time (till your stamina runs out)

	//							 //SteeringPlugin_Output is works the exact same way a SteeringBehaviour output



	m_GrabItem = false; //Reset State
	m_UseItem = false;
	m_RemoveItem = false;

	return steering;
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

void Agent::InitializeBehaviors()
{
	// Blackboard
	m_pBlackboard = new Blackboard();

	// Behaviors
	m_pWanderBehavior = new Wander();

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
	m_pFiniteStateMachine = new FiniteStateMachine(pIdleState, m_pGOAPPlanner, m_pBlackboard);
	// Transitions that go towards perform
	m_pFiniteStateMachine->AddTransition(pIdleState, pGoToState, pGoToTransition);
	m_pFiniteStateMachine->AddTransition(pIdleState, pPerformState, pPerformTransition);
	m_pFiniteStateMachine->AddTransition(pGoToState, pPerformState, pPerformTransition);
	// Transitions that state completed perform
	m_pFiniteStateMachine->AddTransition(pPerformState, pIdleState, performedTransition);

	// GOAP planner
	m_pGOAPPlanner = new GOAPPlanner();
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

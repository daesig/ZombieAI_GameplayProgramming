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
	m_ExploredLocationTimer = m_ExploredLocationRefreshTime;
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

	// Steer
	AgentInfo agentInfo = pInterface->Agent_GetInfo();
	if (m_pSteeringBehavior)
	{
		steering = m_pSteeringBehavior->CalculateSteering(pInterface, dt, agentInfo, m_pBlackboard);
	}


	// Update known locations
	if (m_ExploredLocationTimer >= m_ExploredLocationRefreshTime)
	{
		// Get closest navmesh node
		Elite::Vector2 currentPosition{ pInterface->NavMesh_GetClosestPathPoint(agentInfo.Position) };

		// Add the location if it doesn't exist yet
		auto it = std::find(m_ExploredTileLocations.begin(), m_ExploredTileLocations.end(), currentPosition);
		if (it == m_ExploredTileLocations.end())
		{
			m_ExploredTileLocations.push_back(currentPosition);
			//std::cout << "Explored new position: [ " << currentPosition.x << ", " << currentPosition.y << " ]\n";
		}
		m_ExploredLocationTimer = 0.f;
	}
	m_ExploredLocationTimer += dt;

	//auto vHousesInFOV = utils::GetHousesInFOV(pInterface);//uses m_pInterface->Fov_GetHouseByIndex(...)
	auto vEntitiesInFOV = utils::GetEntitiesInFOV(pInterface); //uses m_pInterface->Fov_GetEntityByIndex(...)

	// Reset worldstate
	m_pWorldState->SetState("EnemyInSight", false);
	float closestDistanceSq{ FLT_MAX };
	for (auto& enemyInFov : vEntitiesInFOV)
	{
		float distanceToEnemySq = agentInfo.Position.DistanceSquared(enemyInFov.Location);
		if (distanceToEnemySq < closestDistanceSq) 
		{
			m_LastSeenClosestEnemy = enemyInFov.Location;
			m_pBlackboard->ChangeData("LastEnemyPos", &m_LastSeenClosestEnemy);
			m_pWorldState->SetState("EnemyInSight", true);
		}
	}

	//for (auto& e : vEntitiesInFOV)
	//{
	//	if (e.Type == eEntityType::PURGEZONE)
	//	{
	//		PurgeZoneInfo zoneInfo;
	//		pInterface->PurgeZone_GetInfo(e, zoneInfo);
	//		std::cout << "Purge Zone in FOV:" << e.Location.x << ", " << e.Location.y << " ---EntityHash: " << e.EntityHash << "---Radius: " << zoneInfo.Radius << std::endl;
	//	}
	//}					

	return steering;
}
void Agent::Render(IExamInterface* pExamInterface, float dt) const
{
	//pExamInterface->Draw_SolidCircle(pExam, .7f, { 0,0 }, { 1, 0, 0 });
	for (const Elite::Vector2& exploredLocation : m_ExploredTileLocations)
	{
		pExamInterface->Draw_Circle(exploredLocation, .5f, Elite::Vector3{ 1.f,1.f,1.f });
	}

	if (m_DebugSeek)
		pExamInterface->Draw_SolidCircle(m_pSeekBehavior->GetTarget(), .5f, Elite::Vector2{}, Elite::Vector3{ 1.f, 0.f, 0.f });

	pExamInterface->Draw_Circle(Elite::Vector2{ 0.f,0.f }, 200.f, Elite::Vector3{ 0.f, 0.f, 1.f });
}

// Controlling behaviors
void Agent::ClearBehavior()
{
	m_pSteeringBehavior = nullptr;
}
void Agent::SetBehavior(BehaviorType behaviorType)
{
	m_DebugSeek = false;
	switch (behaviorType)
	{
	case BehaviorType::WANDER:
		m_pSteeringBehavior = m_pWanderBehavior;
		break;
	case BehaviorType::SEEK:
		m_pSteeringBehavior = m_pSeekBehavior;
		m_DebugSeek = true;
		break;
	case BehaviorType::DODGE:
		m_pSteeringBehavior = m_pDodgeBehavior;
		//m_DebugSeek = true;
		break;
	}
}
void Agent::SetSeekPos(Elite::Vector2 seekPos)
{
	m_pSeekBehavior->SetTarget(seekPos);
}

void Agent::Initialize()
{
	// Add the world states
	AddWorldStates();

	// Blackboard
	m_pBlackboard = new Blackboard();
	m_pBlackboard->AddData("Agent", this);
	m_pBlackboard->AddData("LastEnemyPos", &m_LastSeenClosestEnemy);
	m_pBlackboard->AddData("WorldState", m_pWorldState);

	// Behaviors
	m_pWanderBehavior = new Wander();
	m_pSeekBehavior = new Seek();
	m_pDodgeBehavior = new DodgeEnemy();

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
	m_pWorldState->AddState("EnemyInSight", false);
}

void Agent::InitGOAP()
{
	// GOAP planner
	m_pGOAPPlanner = new GOAPPlanner();
	m_pGOAPPlanner->SetWorldState(m_pWorldState);

	/// GOAP Actions
	// GOAPMoveTo
	GOAPAction* pGOAPExploreWorldAction = new GOAPExploreWorldAction(m_pGOAPPlanner);
	GOAPAction* pGOAPFindGeneralHouseLocationsAction = new GOAPFindGeneralHouseLocationsAction(m_pGOAPPlanner);
	GOAPAction* pGOAPEvadeEnemy = new GOAPEvadeEnemy(m_pGOAPPlanner);
	m_pActions.push_back(pGOAPExploreWorldAction);
	m_pActions.push_back(pGOAPFindGeneralHouseLocationsAction);
	m_pActions.push_back(pGOAPEvadeEnemy);
	//...

	// Let the planner know all the action this agent can do
	m_pGOAPPlanner->AddActions(m_pActions);
}

void Agent::DeleteBehaviors()
{
	// Blackboard
	delete m_pBlackboard;
	m_pBlackboard = nullptr;

	// Behaviors
	delete m_pWanderBehavior;
	m_pWanderBehavior = nullptr;
	delete m_pSeekBehavior;
	m_pSeekBehavior = nullptr;

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

	for (GOAPAction* pAction : m_pActions)
	{
		delete pAction;
		pAction = nullptr;
	}

	// GOAP planner
	delete m_pGOAPPlanner;
	m_pGOAPPlanner = nullptr;
}

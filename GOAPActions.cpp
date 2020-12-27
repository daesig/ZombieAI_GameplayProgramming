#include "stdafx.h"
#include "GOAPActions.h"
#include "GOAPPlanner.h"
#include "IExamInterface.h"
#include "Blackboard.h"
#include "WorldState.h"
#include "Agent.h"

// ---------------------------
// Base class GOAPAction
GOAPAction::GOAPAction(GOAPPlanner* pPlanner)
{
}
GOAPAction::~GOAPAction()
{
	Cleanup();
}
void GOAPAction::Cleanup()
{
	for (GOAPProperty* pProperty : m_Preconditions) {
		delete pProperty;
		pProperty = nullptr;
	}
	for (GOAPProperty* pProperty : m_Effects) {
		delete pProperty;
		pProperty = nullptr;
	}
}
// ---------------------------

// Explore world action
GOAPExploreWorldAction::GOAPExploreWorldAction(GOAPPlanner* pPlanner) :
	GOAPAction(pPlanner)
{
	std::cout << "GOAPExploreWorldAction constructed\n";
	m_RequiresMovement = true;

	InitPreConditions(pPlanner);
	InitEffects(pPlanner);
}
bool GOAPExploreWorldAction::Plan(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	return false;
}
void GOAPExploreWorldAction::Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Setting up GOAPExploreWorldAction\n";
	// Plan where to move to considering the tile locations that have already been explored
	// Consider explored houses and a range around the houses for a more optimal search routine

	AgentInfo agentInfo = pInterface->Agent_GetInfo();
	moveTarget.Position = agentInfo.Position + Elite::Vector2(m_ExploreActionRange * 7.f, m_ExploreActionRange * 14.f);
}
bool GOAPExploreWorldAction::RequiresMovement(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	AgentInfo agentInfo = pInterface->Agent_GetInfo();
	Elite::Vector2 agentPosition = agentInfo.Position;
	float distanceToPosition = agentPosition.Distance(moveTarget.Position);

	// Check if the movement if fullfilled
	if (distanceToPosition < m_MovementFulfilledRange)
		return false;

	// Movement required
	return true;
}
void GOAPExploreWorldAction::InitPreConditions(GOAPPlanner* pPlanner)
{
	// Get a reference to the world states
	WorldState* pWorldState = pPlanner->GetWorldState();

	// Create states
	// House in sight state
	GOAPProperty* pHouseInSight = new GOAPProperty{ "HouseInSight", false };
	m_Preconditions.push_back(pHouseInSight);

	// Make sure the states exist in the world
	if (!pWorldState->DoesStateExist(pHouseInSight->propertyKey))
	{
		// State doesn't exist, add the state with some default starter value
		pWorldState->AddState(pHouseInSight->propertyKey, pHouseInSight->value.bValue);
	}
}

// Find general house locations action
GOAPFindGeneralHouseLocationsAction::GOAPFindGeneralHouseLocationsAction(GOAPPlanner* pPlanner) :
	GOAPAction(pPlanner)
{
	std::cout << "GOAPFindGeneralHouseLocationsAction constructed\n";
	m_Cost = -100.f;

	InitPreConditions(pPlanner);
	InitEffects(pPlanner);
}
bool GOAPFindGeneralHouseLocationsAction::Plan(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	return false;
}
void GOAPFindGeneralHouseLocationsAction::Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Setting up GOAPFindGeneralHouseLocationsAction\n";
}
bool GOAPFindGeneralHouseLocationsAction::Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt)
{
	Elite::Vector2 locationToExplore{ cos(m_Angle) * m_ExploreVicinityRadius, sin(m_Angle) * m_ExploreVicinityRadius };
	Elite::Vector2 cornerLocation = pInterface->NavMesh_GetClosestPathPoint(locationToExplore);

	if (locationToExplore.Distance(cornerLocation) >= m_IgnoreLocationDistance)
	{
		m_LocationsOfInterest.push_back(cornerLocation);
	}


	m_Angle = m_Angle + m_AngleIncrement;;

	for (Elite::Vector2& loc : m_LocationsOfInterest)
	{
		pInterface->Draw_Circle(loc, 2.f, Elite::Vector3{ 1.f,0.f,0.f });
	}

	return true;
}
bool GOAPFindGeneralHouseLocationsAction::CheckPreConditions(GOAPPlanner* pPlanner) const
{
	return false;
}
bool GOAPFindGeneralHouseLocationsAction::CheckProceduralPreconditions(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	return false;
}
bool GOAPFindGeneralHouseLocationsAction::RequiresMovement(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	return false;
}
bool GOAPFindGeneralHouseLocationsAction::IsDone(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	// Determine if we found all the house locations
	if (m_Angle > 360.f)
		return true;

	return false;
}
void GOAPFindGeneralHouseLocationsAction::InitPreConditions(GOAPPlanner* pPlanner)
{
	// Get a reference to the world states
	WorldState* pWorldState = pPlanner->GetWorldState();

	// Create states
	// Requires an enemy in sight to run
	GOAPProperty* pInitialHouseScout = new GOAPProperty{ "InitialHouseScoutDone", false };
	m_Preconditions.push_back(pInitialHouseScout);

	// Make sure the states exist in the world
	if (!pWorldState->DoesStateExist(pInitialHouseScout->propertyKey))
	{
		// State doesn't exist, add the state with some default starter value
		pWorldState->AddState(pInitialHouseScout->propertyKey, pInitialHouseScout->value.bValue);
	}
}
void GOAPFindGeneralHouseLocationsAction::InitEffects(GOAPPlanner* pPlanner)
{
	// Get a reference to the world states
	WorldState* pWorldState = pPlanner->GetWorldState();

	// Create states
	// Requires an enemy in sight to run
	GOAPProperty* pInitialHouseScout = new GOAPProperty{ "InitialHouseScoutDone", true };
	m_Effects.push_back(pInitialHouseScout);

	// Make sure the states exist in the world
	if (!pWorldState->DoesStateExist(pInitialHouseScout->propertyKey))
	{
		// State doesn't exist, add the state with some default starter value
		pWorldState->AddState(pInitialHouseScout->propertyKey, pInitialHouseScout->value.bValue);
	}
}
bool GOAPFindGeneralHouseLocationsAction::CheckEffects(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	return false;
}

GOAPEvadeEnemy::GOAPEvadeEnemy(GOAPPlanner* pPlanner) :
	GOAPAction(pPlanner)
{
	std::cout << "GOAPEvadeEnemy constructed\n";
	m_Cost = 2.f;

	InitPreConditions(pPlanner);
	InitEffects(pPlanner);
}
bool GOAPEvadeEnemy::Plan(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	return false;
}
void GOAPEvadeEnemy::Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Setting up GOAPEvadeEnemy\n";
	m_EvadeTimer = 0.f;

	Agent* pAgent = nullptr;
	// Check if agent data is valid
	bool dataValid = pBlackboard->GetData("Agent", pAgent);
	if (!dataValid) return;

	// Set agent to dodge behavior
	pAgent->SetBehavior(Agent::BehaviorType::DODGE);
}
bool GOAPEvadeEnemy::Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt)
{
	m_EvadeTimer += dt;
	return true;
}
bool GOAPEvadeEnemy::IsDone(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	if (m_EvadeTimer >= m_EvadeTime) 
	{
		Agent* pAgent = nullptr;
		// Check if agent data is valid
		bool dataValid = pBlackboard->GetData("Agent", pAgent);
		if (!dataValid) return false;

		// Clear the agents behavior
		pAgent->ClearBehavior();

		return true;
	}

	return false;
}
void GOAPEvadeEnemy::InitPreConditions(GOAPPlanner* pPlanner)
{
	// Get a reference to the world states
	WorldState* pWorldState = pPlanner->GetWorldState();

	// Create states
	// Requires an enemy in sight to run
	GOAPProperty* pEnemyWasInSight = new GOAPProperty{ "EnemyWasInSight", true };
	m_Preconditions.push_back(pEnemyWasInSight);

	// Make sure the states exist in the world
	if (!pWorldState->DoesStateExist(pEnemyWasInSight->propertyKey))
	{
		// State doesn't exist, add the state with some default starter value
		pWorldState->AddState(pEnemyWasInSight->propertyKey, pEnemyWasInSight->value.bValue);
	}
}
void GOAPEvadeEnemy::InitEffects(GOAPPlanner* pPlanner)
{
	// Get a reference to the world states
	WorldState* pWorldState = pPlanner->GetWorldState();

	// Create states
	// Requires an enemy in sight to run
	GOAPProperty* pEnemyWasInSight = new GOAPProperty{ "EnemyWasInSight", false };
	m_Preconditions.push_back(pEnemyWasInSight);

	// Make sure the states exist in the world
	if (!pWorldState->DoesStateExist(pEnemyWasInSight->propertyKey))
	{
		// State doesn't exist, add the state with some default starter value
		pWorldState->AddState(pEnemyWasInSight->propertyKey, pEnemyWasInSight->value.bValue);
	}
}

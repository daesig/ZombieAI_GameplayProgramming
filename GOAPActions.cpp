#include "stdafx.h"
#include "GOAPActions.h"
#include "GOAPPlanner.h"
#include "IExamInterface.h"
#include "Blackboard.h"
#include "WorldState.h"
#include "Agent.h"
#include "utils.h"

// ---------------------------
// Base class GOAPAction
GOAPAction::GOAPAction(GOAPPlanner* pPlanner)
{
	m_pWorldState = pPlanner->GetWorldState();
}
GOAPAction::~GOAPAction()
{
	Cleanup();
}
bool GOAPAction::HasEffect(GOAPProperty* pPrecondition)
{
	bool hasEffect{ false };
	for (GOAPProperty* pEffect : m_Effects)
	{
		if (pEffect->propertyKey == pPrecondition->propertyKey)
		{
			hasEffect = true;
		}
	}
	return hasEffect;
}
void GOAPAction::Cleanup()
{
	for (GOAPProperty* pProperty : m_Preconditions)
	{
		delete pProperty;
		pProperty = nullptr;
	}
	for (GOAPProperty* pProperty : m_Effects)
	{
		delete pProperty;
		pProperty = nullptr;
	}
}
// ---------------------------

// GOAPSurvive, GOAL NODE for planner
// Preconditions: HasMoreThan5Energy(true), HasMoreThan5Health(true)
	// NoEnemiesInSpotted (true), have distant goal of finding weapon and stocking up on items
// Effects: None
GOAPSurvive::GOAPSurvive(GOAPPlanner* pPlanner) :
	GOAPAction(pPlanner)
{
	m_RequiresMovement = true;
	InitPreConditions(pPlanner);
	InitEffects(pPlanner);
}
bool GOAPSurvive::Plan(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	return true;
}
void GOAPSurvive::Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Setting up GOAPSurvive\n";
}
void GOAPSurvive::InitPreConditions(GOAPPlanner* pPlanner)
{
	GOAPProperty* pEnergyCondition = new GOAPProperty{ "HasMoreThan5Energy", true };
	utils::AddActionProperty(pEnergyCondition, m_Preconditions, m_pWorldState, false);

	GOAPProperty* pHealthCondition = new GOAPProperty{ "HasMoreThan5Health", true };
	utils::AddActionProperty(pHealthCondition, m_Preconditions, m_pWorldState, true);

	//GOAPProperty* pTestCondition = new GOAPProperty{ "SurviveTest", true };
	//utils::AddActionProperty(pTestCondition, m_Preconditions, m_pWorldState, false);
}
void GOAPSurvive::InitEffects(GOAPPlanner* pPlanner) {}

// DrinkEnergy
// Preconditions: HasEnergyItem(true)
// Effects: HasMoreThan5Energy(true), HasEnergyItem(false)
GOAPDrinkEnergy::GOAPDrinkEnergy(GOAPPlanner* pPlanner) :
	GOAPAction(pPlanner)
{
	InitPreConditions(pPlanner);
	InitEffects(pPlanner);
}
bool GOAPDrinkEnergy::Plan(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	return true;
}
void GOAPDrinkEnergy::Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Setting up GOAPDrinkEnergy\n";
}
bool GOAPDrinkEnergy::Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt)
{
	return true;
}
void GOAPDrinkEnergy::InitPreConditions(GOAPPlanner* pPlanner)
{
	GOAPProperty* pCondition = new GOAPProperty{ "HasEnergyItem", true };
	utils::AddActionProperty(pCondition, m_Preconditions, m_pWorldState, false);
}
void GOAPDrinkEnergy::InitEffects(GOAPPlanner* pPlanner)
{
	GOAPProperty* pEnergyCondition = new GOAPProperty{ "HasMoreThan5Energy", true };
	utils::AddActionProperty(pEnergyCondition, m_Effects, m_pWorldState, true);
}

// SearchForEnergy
// Preconditions: InitialHouseScoutDone(true) 
// Effects: HasEnergyItem(true)
GOAPSearchForEnergy::GOAPSearchForEnergy(GOAPPlanner* pPlanner) :
	GOAPAction(pPlanner)
{
	InitPreConditions(pPlanner);
	InitEffects(pPlanner);
}
bool GOAPSearchForEnergy::Plan(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	return true;
}
void GOAPSearchForEnergy::Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Setting up GOAPSearchForEnergy\n";
	// Setup behavior to an item search behavior with priority for energy
	bool dataValid = pBlackboard->GetData("HouseCornerLocations", m_pHouseCornerLocations)
		&& pBlackboard->GetData("HouseLocations", m_pHouseLocations)
		&& pBlackboard->GetData("ItemLocations", m_pItemsOnGround);

	if (!dataValid)
	{
		std::cout << "Error obtaining blackboard data in GOAPSearchForEnergy::Setup\n";
	}
}
bool GOAPSearchForEnergy::Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt)
{
	// Arrived at item destination, choose what to do with the item
		// Is item energy type? 
			// Set worldstate to effects
		// No? Decide if we want the item and choose a new movement destination to keep searching
	return true;
}
void GOAPSearchForEnergy::InitPreConditions(GOAPPlanner* pPlanner)
{
	GOAPProperty* pCondition = new GOAPProperty{ "InitialHouseScoutDone", true };
	utils::AddActionProperty(pCondition, m_Preconditions, m_pWorldState, false);
}
void GOAPSearchForEnergy::InitEffects(GOAPPlanner* pPlanner)
{
	GOAPProperty* pCondition = new GOAPProperty{ "HasEnergyItem", true };
	utils::AddActionProperty(pCondition, m_Effects, m_pWorldState, false);
}

// Explore world action
// Preconditions: InitialHouseScoutDone(true) 
	//TODO: IsInHouse(false) 
// Effects: SurviveTest(true)
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
	return true;
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

	// Check if the movement is fullfilled
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
	GOAPProperty* pHouseInSight = new GOAPProperty{ "InitialHouseScoutDone", true };
	m_Preconditions.push_back(pHouseInSight);

	// Make sure the states exist in the world
	if (!pWorldState->DoesStateExist(pHouseInSight->propertyKey))
	{
		// State doesn't exist, add the state with some default starter value
		pWorldState->AddState(pHouseInSight->propertyKey, false);
	}
}
void GOAPExploreWorldAction::InitEffects(GOAPPlanner* pPlanner)
{
	// Get a reference to the world states
	WorldState* pWorldState = pPlanner->GetWorldState();

	// Create states
	// House in sight state
	GOAPProperty* pHouseInSight = new GOAPProperty{ "SurviveTest", true };
	m_Effects.push_back(pHouseInSight);

	// Make sure the states exist in the world
	if (!pWorldState->DoesStateExist(pHouseInSight->propertyKey))
	{
		// State doesn't exist, add the state with some default starter value
		pWorldState->AddState(pHouseInSight->propertyKey, pHouseInSight->value.bValue);
	}
}

// Find general house locations action
// Preconditions: InitialHouseScoutDone(false)
// Effects: InitialHouseScoutDone(true)
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
	return true;
}
void GOAPFindGeneralHouseLocationsAction::Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Setting up GOAPFindGeneralHouseLocationsAction\n";
	pBlackboard->AddData("HouseCornerLocations", &m_HouseCornerLocations);
}
bool GOAPFindGeneralHouseLocationsAction::Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt)
{
	Elite::Vector2 locationToExplore{ cos(m_Angle) * m_ExploreVicinityRadius, sin(m_Angle) * m_ExploreVicinityRadius };
	Elite::Vector2 cornerLocation = pInterface->NavMesh_GetClosestPathPoint(locationToExplore);

	if (locationToExplore.Distance(cornerLocation) >= m_IgnoreLocationDistance)
	{
		m_HouseCornerLocations.push_back(cornerLocation);
	}

	m_Angle = m_Angle + m_AngleIncrement;;

	// TODO: remove debug
	for (Elite::Vector2& loc : m_HouseCornerLocations)
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
	{
		for (GOAPProperty* pEffect : m_Effects)
		{
			m_pWorldState->SetState(pEffect->propertyKey, pEffect->value.bValue);
		}
		return true;
	}

	return false;
}
void GOAPFindGeneralHouseLocationsAction::InitPreConditions(GOAPPlanner* pPlanner)
{
	GOAPProperty* pCondition = new GOAPProperty{ "InitialHouseScoutDone", false };
	utils::AddActionProperty(pCondition, m_Preconditions, m_pWorldState, false);
}
void GOAPFindGeneralHouseLocationsAction::InitEffects(GOAPPlanner* pPlanner)
{
	GOAPProperty* pCondition = new GOAPProperty{ "InitialHouseScoutDone", true };
	utils::AddActionProperty(pCondition, m_Effects, m_pWorldState, false);
}
bool GOAPFindGeneralHouseLocationsAction::CheckEffects(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	return false;
}

// TODO: make this a behavior
// GOAPEvadeEnemy
// Preconditions: EnemyWasInSight(true)
// Effects: EnemyWasInSight(false)
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
	return true;
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
	pAgent->SetBehavior(BehaviorType::DODGE);
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
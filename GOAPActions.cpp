#include "stdafx.h"
#include "GOAPActions.h"
#include "GOAPPlanner.h"
#include "IExamInterface.h"
#include "Blackboard.h"
#include "WorldState.h"

// ---------------------------
// Base class GOAPAction
GOAPAction::GOAPAction(GOAPPlanner* pPlanner)
{
	InitPreConditions(pPlanner);
	InitEffects(pPlanner);
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
	m_RequiresMovement = true;
}
void GOAPExploreWorldAction::Plan(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
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

bool GOAPFindGeneralHouseLocationsAction::Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
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

// Find general house locations action
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
}

bool GOAPFindGeneralHouseLocationsAction::CheckEffects(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	return false;
}

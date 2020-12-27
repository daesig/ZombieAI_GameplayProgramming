#include "stdafx.h"
#include "GOAPPlanner.h"

GOAPPlanner::GOAPPlanner():
	m_pActions{},
	m_CurrentActionIndex{ 0 }
{
	m_pGoalAction = new GOAPSurvive(this);
}

GOAPPlanner::~GOAPPlanner()
{
	delete m_pGoalAction;
	m_pGoalAction = nullptr;
}

void GOAPPlanner::PlanAction()
{
	// Reset queue of actions
	m_pActionQueue.empty();

	// TODO: Obtain the best path towards the goal according to the current world state
	std::vector<NodeRecord> openList{};
	std::vector<NodeRecord> closedList{};

	// Setup the start node
	NodeRecord currentRecord;
	currentRecord.pAction = m_pGoalAction;
	currentRecord.pConnectedAction = nullptr;
	currentRecord.costSoFar = 0.f;
	currentRecord.estimatedTotalCost = GetHeuristicCost(m_pGoalAction);
	openList.push_back(currentRecord);

	// Loop through the open list
	while (!openList.empty()) 
	{
		for (NodeRecord& nr : openList) 
		{

		}
	}

	// TODO: implement AStar pathfinding for GOAP goals
		// Step 1. Start at goal node (what we want to achieve)
		// The thing we want to achieve has effects, it's preconditions have to be satisfied by previous node until those can also get fulfilled
		// GOAL???????????????????????
			// Find a path from the goal node to a previous node
			// Use preconditions and effects to string a path together
				// Is the path more expensive than another path?

	// Goal: Survive
		// Find houses
		// Evade enemies
		// Loot items
		// Shoot enemies
		// Avoid purge zones

	// Vitals to manage:
		// Energy (goes down over time, regen with items)
		// Health (goes down when bitten, regen with items)
		// Stamina (sprint)

	// Action system:
		// Survive
		// Precon: HasMoreThanXEnergy (true), HasMoreThanXHealth (true)
		// Effect: None (final goal action of the AI)
			// Drink energy (cost 1)
			// Precon: HasMoreThanXEnergy (false), HasHasEnergyPotion (true)
			// Effect HasMoreThanXEnergy (true), HasEnergyPotion (false)

			// Eat food (cost 1)
			// Precon: HasMoreThanXHealth (false), HasFood (true)
			// Effect HasMoreThanXHealth (true), HasFood (false)
				// Loot Item (cost 1)
				// Precon: IsInHouse (true), UnlootedItems (.size() > 0)
				// [HasEnergyPotion (true), HasWeapon (true), HasFood (true)]
					// Find House (cost 1)
					// Precon: IsInHouse (false), InitialHouseScoutDone (true), EnemyInSight (false)
					// Effect: IsInHouse (true)
						// Dodge Enemy (cost 0)
						// Precon: EnemyInSight (true), HasWeapon (false)
						// Effect: EnemyInSight (true)

						// Kill Enemy cost (1)
						// Precon: HasGun (true)
						// Effect: EnemyInSight (false)
							// loot item.... find house....

						// Initial House Scout (cost -100)
						// Precon: InitialHouseScoutDone (false)
						// Effect: InitialHouseScoutDone (true)

	for (GOAPAction* pAction : m_pActions) 
	{
		if (pAction->GetCost() == 2.f)
			m_pActionQueue.push(pAction);
	}

	//m_pActionQueue.push(m_pActions[1]);
}

GOAPAction* GOAPPlanner::GetAction() const
{
	if (m_pActionQueue.size() > 0) {
		return m_pActionQueue.front();
	}

	return nullptr;
}

void GOAPPlanner::NextAction()
{
	m_pActionQueue.pop();
}

void GOAPPlanner::SetWorldState(WorldState* pWorldState)
{
	m_pWorldState = pWorldState;
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

float GOAPPlanner::GetHeuristicCost(GOAPAction* pAction)
{
	float heuristicCost = 0.f;
	bool hasPreviousAction = true;
	GOAPAction* pCurrentAction = pAction;

	std::queue<GOAPProperty*> pConditionsToCheck{};
	for(auto precon: pCurrentAction->GetPreconditions())
	{
		pConditionsToCheck.push(precon);
	}

	// As long the current action has pre conditions that have to be fulfilled
	while (!pConditionsToCheck.empty())
	{
		hasPreviousAction = false;
		heuristicCost += 1.f;
		for (GOAPAction* pAction : m_pActions) 
		{
			// Get the next condition in the queue
			GOAPProperty* pCurrentProperty = pConditionsToCheck.front();
			// Remove it, it has been checked
			pConditionsToCheck.pop();

			// Go through the child actions and check all the conditions that it has to fullfil, add them to the queue
			if (pAction->HasEffect(pCurrentProperty)) 
			{
				// As long as we find more conditions, add them to the list
				for (auto precon : pCurrentAction->GetPreconditions())
				{
					pConditionsToCheck.push(precon);
				}
			}
		}
	}

	return heuristicCost;
}

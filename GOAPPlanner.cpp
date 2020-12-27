#include "stdafx.h"
#include "GOAPPlanner.h"

GOAPPlanner::GOAPPlanner():
	m_pActions{},
	m_CurrentActionIndex{ 0 }
{}

void GOAPPlanner::PlanAction()
{
	// Reset queue of actions
	m_pActionQueue.empty();

	// TODO: Obtain the best path towards the goal according to the current world state

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
			// Drink energy
			// Precon: HasEnergyPotion (true)
			// Effect HasMoreThanXEnergy (true), HasEnergyPotion (false)
				// Loot Item
				// Precon: IsInHouse (true), UnlootedItems (.size() > 0)
				// [HasEnergyPotion (true), HasWeapon (true), HasFood (true)]
					// Find House
					// Precon: IsInHouse (false), InitialHouseScoutDone (true)
					// Effect: IsInHouse (true)
						// Initial House Scout
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

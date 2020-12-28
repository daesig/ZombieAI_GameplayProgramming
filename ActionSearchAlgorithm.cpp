#include "stdafx.h"
#include "ActionSearchAlgorithm.h"
#include "WorldState.h"
#include "GOAPActions.h"

ActionSearchAlgorithm::ActionSearchAlgorithm(WorldState* pWorldState) :
	m_pWorldState{ pWorldState }
{}

std::queue<GOAPAction*> ActionSearchAlgorithm::Search(GOAPAction* pGoalAction, std::vector<GOAPAction*> possibleActions)
{
	std::queue<GOAPAction*> actionQueue{};
	// List variables
	std::vector<NodeRecord> openlist{};
	std::vector<NodeRecord> closedlist{};

	// Setup the start node (node we want to reach)
	NodeRecord currentRecord;
	currentRecord.pAction = pGoalAction;
	//currentRecord.pConnectedNode = nullptr;
	currentRecord.costSoFar = 0.f;
	currentRecord.estimatedTotalCost = GetHeuristicCost(pGoalAction);
	openlist.push_back(currentRecord);

	// Loop through the open list
	while (!openlist.empty())
	{
		// TODO: Heuristic cost

		// Check if all the preconditions have been met
		std::vector<GOAPProperty*>& preconditions = currentRecord.pAction->GetPreconditions();
		std::vector<GOAPProperty*> preconditionsToSatifsy{};
		for (GOAPProperty* pProperty : preconditions)
		{
			if (!m_pWorldState->IsStateMet(pProperty->propertyKey, pProperty->value.bValue))
			{
				// Condition still needs to be satisfied
				preconditionsToSatifsy.push_back(pProperty);
			}
		}

		// All conditions have been met
		if (preconditionsToSatifsy.size() == 0)
		{
			closedlist.push_back(currentRecord);
			// Remove action from open list
			for (std::vector<NodeRecord>::iterator t{ openlist.begin() }; t != openlist.end();)
			{
				if (*t == currentRecord)
				{
					// Remove the record
					t = openlist.erase(t);
				}
				else
					++t;
			}
			continue;
		}

		// Path wasn't found... Search for actions that fullfil the unfulfilled preconditions
		std::vector<GOAPAction*> actionsThatSatisfy{};
		std::list<GOAPProperty*> satisfiedPreconditions{};
		for (GOAPAction* pPotentialAction : possibleActions)
		{
			// Don't check with itself
			if (pPotentialAction == currentRecord.pAction)
				continue;

			// Obtain all the effects from the potential action
			std::vector<GOAPProperty*>& effects = pPotentialAction->GetEffects();

			// Go over all the required pre conditions of the current action
			bool meetsAtLeastOneCondition = false;
			for (GOAPProperty* pPrecondition : preconditionsToSatifsy)
			{
				// Go over all the effects and check if they satisfy a precondition
				for (GOAPProperty* pEffect : effects)
				{
					if (pPrecondition->propertyKey == pEffect->propertyKey)
					{
						// This action has an effect that satisfies a precondition
						meetsAtLeastOneCondition = true;
						satisfiedPreconditions.push_back(pPrecondition);
					}
				}
			}

			// The potential action meets at least one precondition of the current action, add to openlist
			if (meetsAtLeastOneCondition)
			{
				actionsThatSatisfy.push_back(pPotentialAction);

				//NodeRecord nr{};
				//nr.pAction = pPotentialAction;
				//nr.pConnectedAction = currentRecord.pAction;
				//nr.costSoFar = currentRecord.costSoFar + pPotentialAction->GetCost();
				//nr.estimatedTotalCost = GetHeuristicCost(pPotentialAction);
				//openlist.push_back(currentRecord);
			}
		}

		// Check if the correct amount of preconditions was satisfied
		if (satisfiedPreconditions.size() == preconditionsToSatifsy.size())
		{
			// Action was correctly satisfied
			closedlist.push_back(currentRecord);

			// Add all the actions that satisfy the preconditions to the openlist for further processing
			std::for_each(actionsThatSatisfy.begin(), actionsThatSatisfy.end(), [&openlist, &currentRecord, this](GOAPAction* pAction)
				{
					NodeRecord nr{};
					nr.pAction = pAction;
					//nr.pConnectedNode = currentRecord;
					nr.costSoFar = currentRecord.costSoFar + pAction->GetCost();
					nr.estimatedTotalCost = GetHeuristicCost(pAction);

					openlist.push_back(nr);
				}
			);
		}

		// Remove currentRecord from the openlist (has been handled)
		for (std::vector<NodeRecord>::iterator t{ openlist.begin() }; t != openlist.end();)
		{
			if (*t == currentRecord)
			{
				// Remove the record
				t = openlist.erase(t);
			}
			else
				++t;
		}

		// Select a new current record
		if (openlist.size() != 0)
		{
			currentRecord = openlist[0];
		}
	}

	for (auto i = closedlist.rbegin(); i != closedlist.rend(); ++i)
	{
		actionQueue.push(i->pAction);
	} 

	return actionQueue;
}

float ActionSearchAlgorithm::GetHeuristicCost(GOAPAction* pAction)
{
	return 1.f;
	//float heuristicCost = 0.f;
	//bool hasPreviousAction = true;
	//GOAPAction* pCurrentAction = pAction;

	//std::queue<GOAPProperty*> pConditionsToCheck{};
	//for (auto precon : pCurrentAction->GetPreconditions())
	//{
	//	pConditionsToCheck.push(precon);
	//}

	//// As long the current action has pre conditions that have to be fulfilled
	//while (!pConditionsToCheck.empty())
	//{
	//	hasPreviousAction = false;
	//	heuristicCost += 1.f;
	//	for (GOAPAction* pAction : m_pActions)
	//	{
	//		// Get the next condition in the queue
	//		GOAPProperty* pCurrentProperty = pConditionsToCheck.front();

	//		// Go through the child actions and check all the conditions that it has to fullfil, add them to the queue
	//		if (pAction->HasEffect(pCurrentProperty))
	//		{
	//			// As long as we find more conditions, add them to the list
	//			for (auto precon : pCurrentAction->GetPreconditions())
	//			{
	//				pConditionsToCheck.push(precon);
	//			}
	//		}
	//	}
	//	pConditionsToCheck.pop();
	//}

	//return heuristicCost;
}

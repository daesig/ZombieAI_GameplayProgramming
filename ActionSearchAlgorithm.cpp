#include "stdafx.h"
#include "ActionSearchAlgorithm.h"
#include "WorldState.h"
#include "GOAPActions.h"
#include "utils.h"

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
			auto removeIt = std::remove_if(openlist.begin(), openlist.end(), [&currentRecord](NodeRecord& nr)
				{
					if (nr == currentRecord)
					{
						return nr == currentRecord;
					}
					return false;
				}
			);

			if (removeIt != openlist.end())
			{
				openlist.erase(removeIt, openlist.end());
			}

			if (openlist.size() > 0)
				currentRecord = openlist[0];
			continue;
		}

		// Path wasn't found... Search for actions that fullfil the unfulfilled preconditions
		std::vector<GOAPAction*> actionsThatSatisfy{};
		std::list<GOAPProperty*> satisfiedPreconditions{};
		for (GOAPAction* pPotentialAction : possibleActions)
		{
			std::list<GOAPProperty*> satisfiedPreconditionsFromAction{};
			// Don't check with itself
			if (pPotentialAction == currentRecord.pAction)
				continue;

			// Obtain all the effects from the potential action
			std::vector<GOAPProperty*>& potentialActionEffects = pPotentialAction->GetEffects();

			// Go over all the required pre conditions of the current action
			for (GOAPProperty* pPrecondition : preconditionsToSatifsy)
			{
				// Go over all the effects and check if they satisfy a precondition
				for (GOAPProperty* pEffect : potentialActionEffects)
				{
					if (pPrecondition->propertyKey == pEffect->propertyKey)
					{
						if (pPrecondition->value.bValue == pEffect->value.bValue)
						{
							// This action has an effect that satisfies a precondition
							satisfiedPreconditions.push_back(pPrecondition); // TODO: fill this at the end if we want the action!
							satisfiedPreconditionsFromAction.push_back(pPrecondition);
						}
					}
				}
			}

			// The potential action meets at least one precondition of the current action
			if (satisfiedPreconditionsFromAction.size() > 0)
			{
				bool markActionForAdd = true;
				//std::cout << "new action: " << pPotentialAction->ToString() << " meets at least 1 condition\n";

				// Check if they try to satisfy the same property
				int index{ 0 };
				for (GOAPAction* pPreviousAction : actionsThatSatisfy)
				{
					std::vector<GOAPProperty*> previousUnsatisfiedEffects{ utils::GetUnsatisfiedActionEffects(pPreviousAction->GetEffects(), m_pWorldState) };
					std::vector<GOAPProperty*> potentialUnsatisfiedEffects{ utils::GetUnsatisfiedActionEffects(pPotentialAction->GetEffects(), m_pWorldState) };
					// The previous action satisfies the same / less unsatisfied effects (potential action can possible be better than the previous action)
					if (potentialUnsatisfiedEffects.size() <= previousUnsatisfiedEffects.size())
					{
						// Check how many of the conditions match
						int matches{ 0 };
						std::for_each(previousUnsatisfiedEffects.begin(), previousUnsatisfiedEffects.end(),
							[&potentialActionEffects, &matches](GOAPProperty* previousProperty)
							{
								for (GOAPProperty* potentialProperty : potentialActionEffects)
								{
									if (potentialProperty->propertyKey == previousProperty->propertyKey)
									{
										if (potentialProperty->value.bValue == previousProperty->value.bValue)
										{
											++matches;
										}
									}
								}
							}
						);

						// The potential action covers all of the previous action's effects
						if (matches == potentialUnsatisfiedEffects.size())
						{
							// We are not adding the action at this point, only replacing
							markActionForAdd = false;

							// Potential action might be better then the previous action
							if (potentialUnsatisfiedEffects.size() >= previousUnsatisfiedEffects.size())
							{
								// Check if this action is cheaper
								float previousActionCost = pPreviousAction->GetCost();
								float potentialActionCost = pPotentialAction->GetCost();
								if (potentialActionCost < previousActionCost)
								{
									// Replace the previous action with this action since it's cheaper
									//std::cout << "Replacing: " << pPreviousAction->ToString() << " with " << pPotentialAction->ToString() << "\n";
									actionsThatSatisfy[index] = pPotentialAction;
								}
								// Else this one is more expensive, ignore the pPotentialAction
								// This action matched all the effects from the previous action, stop searching deeper
								break;
							}
						}
					}
					++index;
				}

				// Action is not overruled by any previous actions
				if (markActionForAdd)
				{
					//std::cout << "Found a new action: " << pPotentialAction->ToString() << "\n";
					actionsThatSatisfy.push_back(pPotentialAction);
				}
			}
		}

		satisfiedPreconditions.sort();
		satisfiedPreconditions.unique();
		// Check if the correct amount of preconditions was satisfied
		if (satisfiedPreconditions.size() >= preconditionsToSatifsy.size())
		{
			// Action was correctly satisfied
			closedlist.push_back(currentRecord);

			// Sort the actions, from expensive to cheaper
			// Satisfied actions get priority over unsatisfied actions and should be put at the end so they get ran first
			std::sort(actionsThatSatisfy.begin(), actionsThatSatisfy.end(), [this](GOAPAction* a, GOAPAction* b)
				{
					// If true, put a before b in the vector
					bool isMoreExpensive = a->GetCost() > b->GetCost();
					auto unsatisfiedPreconditionsA = utils::GetUnsatisfiedActionEffects(a->GetPreconditions(), m_pWorldState);
					auto unsatisfiedPreconditionsB = utils::GetUnsatisfiedActionEffects(b->GetPreconditions(), m_pWorldState);

					// Put A earlier in the list if it requires more conditions to be satisfied
					if (unsatisfiedPreconditionsA.size() > unsatisfiedPreconditionsB.size())
						return true;
					else if (unsatisfiedPreconditionsA.size() < unsatisfiedPreconditionsB.size())
						return false;

					// Both have the same unsatisfied amount, but the most expensive ones first
					return isMoreExpensive;
				}
			);

			// Add all the actions that satisfy the preconditions to the openlist for further processing
			std::for_each(actionsThatSatisfy.begin(), actionsThatSatisfy.end(), [&openlist, &currentRecord, &closedlist, this](GOAPAction* pAction)
				{
					bool exists = false;
					for (NodeRecord& openlistRecord : openlist)
					{
						if (openlistRecord.pAction->ToString() == pAction->ToString())
						{
							//std::cout << "Action already exists!!\n";
							exists = true;
						}
					}
					if (!exists)
					{
						NodeRecord nr{};
						nr.pAction = pAction;
						//nr.pConnectedNode = currentRecord;
						nr.costSoFar = currentRecord.costSoFar + pAction->GetCost();
						nr.estimatedTotalCost = GetHeuristicCost(pAction);
						openlist.push_back(nr);
						//std::cout << "Added to openlist: " << pAction->ToString() << "\n";
					}
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

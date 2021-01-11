#pragma once
#include "GOAPActions.h"
#include <vector>

class ActionSearchAlgorithm;
class WorldState;
class Blackboard;
class GOAPPlanner
{
public:
	GOAPPlanner(WorldState* pWorldState, Blackboard* pBlackboard);
	~GOAPPlanner();

	bool PlanAction();
	GOAPAction* GetAction() const;
	void NextAction();

	WorldState* GetWorldState();

	void AddAction(GOAPAction* pAction);
	void AddActions(std::vector<GOAPAction*>& m_pActions);

	void SetEncounteredProblem(bool value);
	bool GetEncounteredProblem() const;
private:
	std::vector<GOAPAction*> m_pActions{};
	std::queue<GOAPAction*> m_pActionQueue{};
	WorldState* m_pWorldState = nullptr;
	int m_CurrentActionIndex = 0;

	GOAPSurvive* m_pGoalAction = nullptr;
	ActionSearchAlgorithm* m_pSearchAlgorithm = nullptr;

	bool m_EncounteredProblem = false;

	// Debugging
	bool* m_pDebugGOAPPlanner = nullptr;
};
#pragma once
#include <unordered_map>
#include "SteeringHelpers.h"
#include "IExamInterface.h"
#include "structs.h"

class Agent;
class Blackboard;
class GOAPPlanner;
class IExamInterface;

class GOAPAction
{
public:
	virtual bool Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) = 0;

	// Use data from the world state
	virtual bool CheckProceduralPrecondition(GOAPPlanner* pPlanner, Blackboard* pBlackboard) = 0;

	virtual bool RequiresMovement() const = 0;
	virtual bool IsDone() const = 0;
protected:
	std::vector<GOAPProperty*> m_Preconditions;
	std::vector<GOAPProperty*> m_Effects;

	bool m_RequiresMovement;
	TargetData moveTarget{};
};

class GOAPMoveTo final : public GOAPAction
{
public:
	virtual bool Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) { return true; };
	virtual bool CheckProceduralPrecondition(GOAPPlanner* pPlanner, Blackboard* pBlackboard) { return true; };
	virtual bool RequiresMovement() const { return true; };
	virtual bool IsDone() const { return false; };
private:
	float m_MovementFullfilledRange = 1.f;
};

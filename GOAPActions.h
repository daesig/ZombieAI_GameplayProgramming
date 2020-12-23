#pragma once
#include <unordered_map>
#include "SteeringHelpers.h"

class Agent;
class Blackboard;
struct GOAPProperty
{
	std::string propertyKey;

	union value
	{
		bool bValue;
		int iValue;
		float fValue;
	};
};

class GOAPAction
{
public:
	virtual bool Perform(Blackboard* pBlackboard) = 0;
	virtual bool CheckProceduralPrecondition(Blackboard* pBlackboard) = 0;
	virtual bool RequiresMovement() const = 0;
	virtual bool IsDone() const = 0;
protected:
	std::unordered_map<std::string, GOAPProperty> m_Preconditions;
	std::unordered_map<std::string, GOAPProperty> m_Effects;

	bool m_RequiresMovement;
	TargetData moveTarget{};
};

class GOAPMoveTo final : public GOAPAction
{
public:
	virtual bool CheckProceduralPrecondition(Blackboard* pBlackboard);
private:
	float m_MovementFullfilledRange = 1.f;
};

class GOAPExplore final : public GOAPAction
{
public:

private:

};

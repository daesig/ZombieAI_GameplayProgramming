#pragma once
class GOAPPlanner;
class IExamInterface;
class DecisionMaking
{
public:
	DecisionMaking() = default;
	virtual ~DecisionMaking() = default;

	virtual void Update(IExamInterface* pInterface, GOAPPlanner* pPlanner, float deltaT) = 0;
};
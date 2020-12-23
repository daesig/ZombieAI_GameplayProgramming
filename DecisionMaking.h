#pragma once
class GOAPPlanner;
class DecisionMaking
{
public:
	DecisionMaking() = default;
	virtual ~DecisionMaking() = default;

	virtual void Update(GOAPPlanner* pPlanner, float deltaT) = 0;

};
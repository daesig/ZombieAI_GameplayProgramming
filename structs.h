#pragma once
#include <string>

struct GOAPProperty
{
	std::string propertyKey;

	union PropertyValue
	{
		PropertyValue(bool value) { bValue = value; }
		PropertyValue(int value) { iValue = value; }
		PropertyValue(float value) { fValue = value; }
		PropertyValue(Elite::Vector2 value) { position = value; }

		bool bValue;
		int iValue;
		float fValue;
		Elite::Vector2 position;
	} value;
};

struct NodeRecord
{
	GOAPAction* pAction;
	GOAPAction* pConnectedAction;
	float costSoFar = 0.f;
	float estimatedTotalCost = 0.f;

	bool operator==(const NodeRecord& other) const
	{
		return pAction == other.pAction
			&& pConnectedAction == other.pConnectedAction
			&& costSoFar == other.costSoFar
			&& estimatedTotalCost == other.estimatedTotalCost;
	};

	bool operator<(const NodeRecord& other) const
	{
		return estimatedTotalCost < other.estimatedTotalCost;
	};
};
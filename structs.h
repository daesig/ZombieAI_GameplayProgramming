#pragma once
#include <string>
#include "Exam_HelperStructs.h"

class GOAPAction;

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
	NodeRecord* pConnectedNode;
	float costSoFar = 0.f;
	float estimatedTotalCost = 0.f;

	bool operator==(const NodeRecord& other) const
	{
		return pAction == other.pAction
			&& pConnectedNode == other.pConnectedNode
			&& costSoFar == other.costSoFar
			&& estimatedTotalCost == other.estimatedTotalCost;
	};

	bool operator<(const NodeRecord& other) const
	{
		return estimatedTotalCost < other.estimatedTotalCost;
	};
};

struct ExploredHouse
{
	HouseInfo houseInfo;
	float timeSinceExplored;
};

enum class EvadeType
{
	SEEK,
	DODGESEEK
};

enum class BehaviorType
{
	WANDER,
	SEEK,
	SEEKDODGE,
	SEEKITEM,
	KILL,
	NONE
};
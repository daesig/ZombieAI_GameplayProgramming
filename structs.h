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

	bool operator==(const NodeRecord& other) const
	{
		return pAction == other.pAction
			&& pConnectedNode == other.pConnectedNode
			&& costSoFar == other.costSoFar;
	};
};

struct ExploredHouse
{
	HouseInfo houseInfo;
	int itemsLootedSinceExplored;
};

struct Line
{
	Elite::Vector2 pointA{};
	Elite::Vector2 pointB{};
	float lifeTime{ 5.f };
};

struct SpottedPurgeZone
{
	PurgeZoneInfo purgezoneInfo{};
	float timeSinceSpotted{ 0 };
};

enum class EvadeType
{
	SEEK,
	DODGESEEK
};

enum class BehaviorType
{
	SEEK,
	SEEKDODGE,
	SEEKITEM,
	KILL,
	NONE
};
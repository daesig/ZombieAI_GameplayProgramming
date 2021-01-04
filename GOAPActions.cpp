#include "stdafx.h"
#include "GOAPActions.h"
#include "GOAPPlanner.h"
#include "IExamInterface.h"
#include "Blackboard.h"
#include "WorldState.h"
#include "Agent.h"
#include "utils.h"

// ---------------------------
// Base class GOAPAction
GOAPAction::GOAPAction(GOAPPlanner* pPlanner, const std::string& effectName) :
	m_EffectName{ effectName }
{
	m_pWorldState = pPlanner->GetWorldState();
	std::cout << "Constructed action: " << m_EffectName << "\n";
}
GOAPAction::~GOAPAction()
{
	Cleanup();
}
bool GOAPAction::HasEffect(GOAPProperty* pPrecondition)
{
	bool hasEffect{ false };
	for (GOAPProperty* pEffect : m_Effects)
	{
		if (pEffect->propertyKey == pPrecondition->propertyKey)
		{
			hasEffect = true;
		}
	}
	return hasEffect;
}
void GOAPAction::ApplyEffects(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	for (auto& effect : m_Effects)
	{
		m_pWorldState->SetState(effect->propertyKey, effect->value.bValue);
	}
}
void GOAPAction::Cleanup()
{
	for (GOAPProperty* pProperty : m_Preconditions)
	{
		delete pProperty;
		pProperty = nullptr;
	}
	for (GOAPProperty* pProperty : m_Effects)
	{
		delete pProperty;
		pProperty = nullptr;
	}
}
// ---------------------------

// GOAPSurvive, GOAL NODE for planner
// Preconditions: RequiresFood(true), RequiresHealth(true)
	// NoEnemiesInSpotted (true), have distant goal of finding weapon and stocking up on items
// Effects: None
GOAPSurvive::GOAPSurvive(GOAPPlanner* pPlanner) :
	GOAPAction(pPlanner, "GOAPSurvive")
{
	m_RequiresMovement = true;
	InitPreConditions(pPlanner);
	InitEffects(pPlanner);
}
bool GOAPSurvive::Plan(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	return true;
}
void GOAPSurvive::Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Setting up GOAPSurvive\n";
}
void GOAPSurvive::InitPreConditions(GOAPPlanner* pPlanner)
{
	// Food management
	GOAPProperty* pE1 = new GOAPProperty{ "RequiresFood", false };
	utils::AddActionProperty(pE1, m_Preconditions, m_pWorldState, false);
	GOAPProperty* pE2 = new GOAPProperty{ "HasFood", true };
	utils::AddActionProperty(pE2, m_Preconditions, m_pWorldState, false);

	// Health management
	GOAPProperty* pH1 = new GOAPProperty{ "RequiresHealth", false };
	utils::AddActionProperty(pH1, m_Preconditions, m_pWorldState, false);
	GOAPProperty* pH2 = new GOAPProperty{ "HasMedkit", true };
	utils::AddActionProperty(pH2, m_Preconditions, m_pWorldState, false);

	GOAPProperty* pM1 = new GOAPProperty{ "HasGoal", true };
	utils::AddActionProperty(pM1, m_Preconditions, m_pWorldState, false);

	GOAPProperty* pCondition = new GOAPProperty{ "FastScoutAllowed", false };
	utils::AddActionProperty(pCondition, m_Preconditions, m_pWorldState, false);

	// If the agent has fullfilled above preconditions
	//GOAPProperty* pKillCondition = new GOAPProperty{ "AttemptKill", true };
	//utils::AddActionProperty(pKillCondition, m_Preconditions, m_pWorldState, true);

	//GOAPProperty* pTestCondition = new GOAPProperty{ "SurviveTest", true };
	//utils::AddActionProperty(pTestCondition, m_Preconditions, m_pWorldState, false);
}
void GOAPSurvive::InitEffects(GOAPPlanner* pPlanner) {}

// DrinkEnergy: public GOAPAction
// Preconditions: HasFood(true)
// Effects: HasFood(false), RequiresFood(false)
GOAPConsumeFood::GOAPConsumeFood(GOAPPlanner* pPlanner) :
	GOAPAction(pPlanner, "GOAPConsumeFood")
{
	InitPreConditions(pPlanner);
	InitEffects(pPlanner);
	m_Cost = 0.f;
}
void GOAPConsumeFood::Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Setting up GOAPDrinkEnergy\n";
	m_Consumed = false;
}
bool GOAPConsumeFood::Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt)
{
	Agent* pAgent = nullptr;
	bool dataValid = pBlackboard->GetData("Agent", pAgent);
	if (!dataValid)
		return false;

	// Check if the consumption went fine if we did decide to consume
	bool success = true;
	// Make sure we can only consume once
	if (!m_Consumed)
	{
		m_Consumed = pAgent->ConsumeItem(eItemType::FOOD);
		success = m_Consumed;
	}

	return success;
}
bool GOAPConsumeFood::IsDone(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	ApplyEffects(pInterface, pPlanner, pBlackboard);
	return true;
}
void GOAPConsumeFood::InitPreConditions(GOAPPlanner* pPlanner)
{
	GOAPProperty* pCondition = new GOAPProperty{ "HasFood", true };
	utils::AddActionProperty(pCondition, m_Preconditions, m_pWorldState, false);
}
void GOAPConsumeFood::InitEffects(GOAPPlanner* pPlanner)
{
	GOAPProperty* pEnergyCondition = new GOAPProperty{ "RequiresFood", false };
	utils::AddActionProperty(pEnergyCondition, m_Effects, m_pWorldState, false);

	GOAPProperty* pHasFoodCondition = new GOAPProperty{ "HasFood", false };
	utils::AddActionProperty(pHasFoodCondition, m_Effects, m_pWorldState, false);
}
void GOAPConsumeFood::ApplyEffects(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	// Setup behavior to an item search behavior with priority for energy
	int maxInventorySlots = pInterface->Inventory_GetCapacity();
	int index{ 0 };
	bool allFoodUsed = true;
	while (index < maxInventorySlots)
	{
		ItemInfo item;
		bool hasItem = pInterface->Inventory_GetItem(index, item);
		if (hasItem)
		{
			if (item.Type == eItemType::FOOD)
			{
				allFoodUsed = false;
				break;
			}
		}
		++index;
	}


	for (auto& effect : m_Effects)
	{
		// Make sure we only apply this effect when neccessary
		if (effect->propertyKey == "HasFood")
		{
			// Apply the effect if there is no medkit in inventory
			if (allFoodUsed)
				m_pWorldState->SetState(effect->propertyKey, effect->value.bValue);
		}
		else
			m_pWorldState->SetState(effect->propertyKey, effect->value.bValue);
	}
}

// GOAPConsumeMedkit: public GOAPAction
// Preconditions: HasMedkit(true);
// Effects: HasMedkit(false), RequiresHealth(false)
GOAPConsumeMedkit::GOAPConsumeMedkit(GOAPPlanner* pPlanner) :
	GOAPAction(pPlanner, "GOAPConsumeMedkit")
{
	InitPreConditions(pPlanner);
	InitEffects(pPlanner);
	m_Cost = 0.f;
}
void GOAPConsumeMedkit::Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Setting up GOAPConsumeMedkit\n";
	m_Consumed = false;
}
bool GOAPConsumeMedkit::Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt)
{
	Agent* pAgent = nullptr;
	bool dataValid = pBlackboard->GetData("Agent", pAgent);
	if (!dataValid)
		return false;

	// Check if the consumption went fine if we did decide to consume
	bool success = true;
	// Make sure we can only consume once
	if (!m_Consumed)
	{
		m_Consumed = pAgent->ConsumeItem(eItemType::MEDKIT);
		success = m_Consumed;
	}

	return success;
}
bool GOAPConsumeMedkit::IsDone(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	ApplyEffects(pInterface, pPlanner, pBlackboard);
	return true;
}
void GOAPConsumeMedkit::InitPreConditions(GOAPPlanner* pPlanner)
{
	GOAPProperty* pCondition = new GOAPProperty{ "HasMedkit", true };
	utils::AddActionProperty(pCondition, m_Preconditions, m_pWorldState, false);
}
void GOAPConsumeMedkit::InitEffects(GOAPPlanner* pPlanner)
{
	GOAPProperty* pEnergyCondition = new GOAPProperty{ "RequiresHealth", false };
	utils::AddActionProperty(pEnergyCondition, m_Effects, m_pWorldState, false);

	GOAPProperty* pHasFoodCondition = new GOAPProperty{ "HasMedkit", false };
	utils::AddActionProperty(pHasFoodCondition, m_Effects, m_pWorldState, false);
}
void GOAPConsumeMedkit::ApplyEffects(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	// Setup behavior to an item search behavior with priority for energy
	int maxInventorySlots = pInterface->Inventory_GetCapacity();
	int index{ 0 };
	bool allMedkitsUsed = true;
	while (index < maxInventorySlots)
	{
		ItemInfo item;
		bool hasItem = pInterface->Inventory_GetItem(index, item);
		if (hasItem)
		{
			if (item.Type == eItemType::MEDKIT)
			{
				allMedkitsUsed = false;
				break;
			}
		}
		++index;
	}

	for (auto& effect : m_Effects)
	{
		// Make sure we only apply this effect when neccessary
		if (effect->propertyKey == "HasMedkit")
		{
			// Apply the effect if there is no medkit in inventory
			if (allMedkitsUsed)
				m_pWorldState->SetState(effect->propertyKey, effect->value.bValue);
		}
		else
			m_pWorldState->SetState(effect->propertyKey, effect->value.bValue);
	}
}

// GOAPSearchItem: public GOAPAction
// Preconditions: InitialHouseScoutDone(true) 
// Effects: HasGoal(true)
GOAPSearchItem::GOAPSearchItem(GOAPPlanner* pPlanner, const std::string effectName) :
	GOAPAction(pPlanner, effectName)
{
	InitPreConditions(pPlanner);
	InitEffects(pPlanner);
	m_Cost = 0.5f;
}
bool GOAPSearchItem::Plan(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	return true;
}
void GOAPSearchItem::Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Setting up GOAPSearchItem\n";
	// Setup behavior to an item search behavior with priority for energy
	bool dataValid = pBlackboard->GetData("HouseCornerLocations", m_pHouseCornerLocations)
		&& pBlackboard->GetData("HouseLocations", m_pHouseLocations)
		&& pBlackboard->GetData("ItemLocations", m_pItemsOnGround)
		&& pBlackboard->GetData("Agent", m_pAgent);

	if (!dataValid)
	{
		std::cout << "Error obtaining blackboard data in GOAPSearchItem::Setup\n";
		return;
	}

	ChooseSeekLocation(pInterface, pPlanner, pBlackboard);


	//m_pAgent->SetBehavior(BehaviorType::SEEKDODGE);

	// Testing
	m_IsDoneTimer = 0.f;
}
bool GOAPSearchItem::Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt)
{
	m_IsDoneTimer += dt;
	if (!utils::VitalStatisticsAreOk(m_pWorldState))
		return false;

	// Get blackboard data
	pBlackboard->GetData("AgentHouse", m_AgentHouse);

	bool requiresNewSeekPos{ false };

	// Try to loot items on the ground
	if (m_pItemsOnGround->size() > 0)
	{
		std::vector<EntityInfo*> grabbedItems{};
		for (EntityInfo& i : *m_pItemsOnGround)
		{
			eItemType grabbedItemType{};
			bool grabError = false;
			bool itemPickedUp = m_pAgent->GrabItem(i, eItemType::FOOD, grabbedItemType, pInterface, grabError);
			if (itemPickedUp)
			{
				grabbedItems.push_back(&i);

				for (ExploredHouse& h : *m_pHouseLocations)
				{
					++h.itemsLootedSinceExplored;
				}
			}

			if (!grabError)
			{
				// Grab error
			}
		}

		// Remove all the grabbed items from the items on ground list
		m_pItemsOnGround->erase(std::remove_if(m_pItemsOnGround->begin(), m_pItemsOnGround->end(), [&grabbedItems, &requiresNewSeekPos](EntityInfo& e)
			{
				for (EntityInfo* grabbedItem : grabbedItems)
				{
					if (grabbedItem->Location == e.Location)
					{
						requiresNewSeekPos = true;
						return true;
					}
				}
				return false;
			}
		), m_pItemsOnGround->end());
	}

	auto vEntitiesInFov = utils::GetEntitiesInFOV(pInterface);
	auto vHousesInFOV = utils::GetHousesInFOV(pInterface);

	// Get the info from the agent
	const AgentInfo& agentInfo = pInterface->Agent_GetInfo();

	m_PurgesZonesInSight.clear();
	// Check for items and purgezones
	for (EntityInfo& entity : vEntitiesInFov)
	{
		if (entity.Type == eEntityType::ITEM)
		{
			ItemInfo itemInfo;
			// If we got item info
			if (pInterface->Item_GetInfo(entity, itemInfo))
			{
				// See if we already know the item
				auto foundIt = std::find_if(m_pItemsOnGround->begin(), m_pItemsOnGround->end(), [&itemInfo](EntityInfo& item)
					{
						return item.Location == itemInfo.Location;
					}
				);

				// New item found! Add the item to the array
				if (foundIt == m_pItemsOnGround->end())
				{
					std::cout << "Item found!\n";
					m_pItemsOnGround->push_back(entity);
					requiresNewSeekPos = true;
				}
			}
		}
		else if (entity.Type == eEntityType::PURGEZONE)
		{
			PurgeZoneInfo pzi;
			pInterface->PurgeZone_GetInfo(entity, pzi);
			m_PurgesZonesInSight.push_back(pzi);

			// Make the action seek a new position if the agent happened to be inside of the purge zone
			if (utils::IsPointInCircle(agentInfo.Position, pzi.Center, pzi.Radius))
			{
				requiresNewSeekPos = true;
			}
		}
	}


	// Go into kill behavior if we're not in a house and we have a weapon
	if (m_pWorldState->IsStateMet("HasWeapon", true) && !m_AgentHouse)
	{
		m_pAgent->SetBehavior(BehaviorType::KILL);
	}
	// Otherwise revert to the seek and dodge behavior
	else
	{
		m_pAgent->SetBehavior(BehaviorType::SEEKDODGE);
	}

	// Check for new houses
	for (HouseInfo& house : vHousesInFOV)
	{
		// See if we have already memorized the house location
		auto foundIterator = std::find_if(m_pHouseLocations->begin(), m_pHouseLocations->end(), [&house](ExploredHouse& exploredHouse)
			{
				return house.Center == exploredHouse.houseInfo.Center;
			}
		);

		// House does not exist yet
		if (foundIterator == m_pHouseLocations->end())
		{
			// Add the house to the known locations
			m_pHouseLocations->push_back(ExploredHouse{ house, 999 });
			// Remove all corner locations of this house
			RemoveExploredCornerLocations(house);
			// Choose a new seek location
			requiresNewSeekPos = true;
		}
	}

	// Check if we need a new seek location
	if (requiresNewSeekPos)
	{
		ChooseSeekLocation(pInterface, pPlanner, pBlackboard);
	}
	else if (CheckArrival(pInterface, pPlanner, pBlackboard))
	{
		if (m_ChooseSeekLocationTimer > m_ChooseSeekLocationTime)
		{
			ChooseSeekLocation(pInterface, pPlanner, pBlackboard);
			m_ChooseSeekLocationTimer = 0.f;
		}
	}
	m_ChooseSeekLocationTimer += dt;

	/// TODO: remove | Debugging
	// Debug seek location
	pInterface->Draw_SolidCircle(m_pAgent->GetGoalPosition(), 3.f, {}, { 0.f,1.f,0.f });
	// Debug corner locations
	for (const Elite::Vector2& c : *m_pHouseCornerLocations)
	{
		pInterface->Draw_SolidCircle(c, 2.f, {}, { 0.f,0.f,1.f });
	}
	pInterface->Draw_SolidCircle(m_pAgent->GetDistantGoalPosition(), 2.f, {}, { 1.f, 0.f, 0.f });

	return true;
}
bool GOAPSearchItem::IsDone(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	return (m_IsDoneTimer > m_IsDoneTime);
}
void GOAPSearchItem::InitPreConditions(GOAPPlanner* pPlanner)
{
	GOAPProperty* pCondition = new GOAPProperty{ "InitialHouseScoutDone", true };
	utils::AddActionProperty(pCondition, m_Preconditions, m_pWorldState, false);
}
void GOAPSearchItem::InitEffects(GOAPPlanner* pPlanner)
{
	GOAPProperty* pM1 = new GOAPProperty{ "HasGoal", true };
	utils::AddActionProperty(pM1, m_Effects, m_pWorldState, false);
}
void GOAPSearchItem::ChooseSeekLocation(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	const Elite::Vector2& agentPos = pInterface->Agent_GetInfo().Position;
	Elite::Vector2 destination{};
	bool foundPath = false;

	// Find path to item
	float closestItemDistanceFromAgentSquared{ FLT_MAX };
	EntityInfo* pClosestItem = nullptr;
	if (!foundPath)
	{
		if (m_pItemsOnGround->size() > 0)
		{
			for (EntityInfo& i : *m_pItemsOnGround)
			{
				// Check if the item is located inside of a purge zone
				bool isInPurgeZone{ utils::IsLocationInsideGivenPurgezones(i.Location, m_PurgesZonesInSight) };

				// Only go for an item if it's not inside of the purgezone
				if (!isInPurgeZone)
				{
					float distanceToAgentSquared = agentPos.DistanceSquared(i.Location);
					if (distanceToAgentSquared < closestItemDistanceFromAgentSquared)
					{
						closestItemDistanceFromAgentSquared = distanceToAgentSquared;
						pClosestItem = &i;
					}
				}
			}

			// Check if we found a nearby item and set the destination
			if (pClosestItem)
			{
				destination = pInterface->NavMesh_GetClosestPathPoint(pClosestItem->Location);
			}
			else
				std::cout << "Error finding path to house\n";
		}
	}

	// Find path to house
	float closestHouseDistanceFromAgentSquared{ FLT_MAX };
	ExploredHouse* pClosestHouse = nullptr;
	if (!foundPath)
	{
		std::vector<ExploredHouse*> possibleHouses{};
		for (ExploredHouse& h : *m_pHouseLocations)
		{
			// Check if the house is located inside of a purge zone
			bool isInPurgeZone{ utils::IsLocationInsideGivenPurgezones(h.houseInfo.Center, m_PurgesZonesInSight) };

			// Only make the house available if it's not withing a purgezone
			if (!isInPurgeZone)
			{
				if (h.itemsLootedSinceExplored > m_ItemsToLootBeforeHouseRevisit)
					possibleHouses.push_back(&h);
			}
		}

		if (possibleHouses.size() > 0)
		{
			for (ExploredHouse* h : possibleHouses)
			{
				float distanceToAgentSquared = agentPos.DistanceSquared(h->houseInfo.Center);
				if (distanceToAgentSquared < closestHouseDistanceFromAgentSquared)
				{
					closestHouseDistanceFromAgentSquared = distanceToAgentSquared;
					pClosestHouse = h;
				}
			}

			if (pClosestHouse)
			{
				m_HouseGoalPos = pClosestHouse->houseInfo.Center;
				destination = pInterface->NavMesh_GetClosestPathPoint(m_HouseGoalPos);
			}
			else
				std::cout << "Error finding path to house\n";
		}
	}

	if (pClosestItem)
	{
		if (closestItemDistanceFromAgentSquared < closestHouseDistanceFromAgentSquared)
		{
			m_pAgent->SetDistantGoalPosition(pClosestItem->Location);
			destination = pInterface->NavMesh_GetClosestPathPoint(pClosestItem->Location);
		}
		foundPath = true;
	}
	else if (pClosestHouse)
	{
		m_pAgent->SetDistantGoalPosition(pClosestHouse->houseInfo.Center);
		foundPath = true;
	}

	// Found path to a house corner
	if (!foundPath)
	{
		float closestCornerDistanceFromAgentSquared{ FLT_MAX };
		Elite::Vector2 closestCorner{};
		if (m_pHouseCornerLocations->size() > 0)
		{
			for (const Elite::Vector2& cornerLoc : (*m_pHouseCornerLocations))
			{
				// Check if the house is located inside of a purge zone
				bool isInPurgeZone{ utils::IsLocationInsideGivenPurgezones(cornerLoc, m_PurgesZonesInSight) };

				// Only make the corner available if it's not withing a purgezone
				if (!isInPurgeZone)
				{
					float distanceToAgentSquared = agentPos.DistanceSquared(cornerLoc);

					if (distanceToAgentSquared < closestCornerDistanceFromAgentSquared)
					{
						closestCornerDistanceFromAgentSquared = distanceToAgentSquared;
						closestCorner = cornerLoc;

						// Set this corner to the goal location
						m_pAgent->SetDistantGoalPosition(closestCorner);
						destination = pInterface->NavMesh_GetClosestPathPoint(closestCorner);
						foundPath = true;
					}
				}
			}
		}
		else
		{
			// No more corners to discover
			// All explored houses are unavailable
			// TODO: Search for more houses
			std::cout << "no more corners\n";
		}
	}

	// Log error message
	if (!foundPath)
		std::cout << "No path found in GOAPSearchItem::ChooseSeekLocation!\n";

	// Set agent destination
	m_pAgent->SetGoalPosition(destination);
}
bool GOAPSearchItem::CheckArrival(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	const Elite::Vector2& agentPos = pInterface->Agent_GetInfo().Position;

	if (agentPos.DistanceSquared(m_HouseGoalPos) < m_ArrivalRange * m_ArrivalRange)
	{
		// Is he in a house?
		if (m_AgentHouse)
			m_AgentHouse->itemsLootedSinceExplored = 0;
	}

	// Has the agent arrived at it's location
	if (agentPos.DistanceSquared(m_pAgent->GetGoalPosition()) < m_ArrivalRange * m_ArrivalRange)
	{
		return true;
	}

	return false;
}
void GOAPSearchItem::RemoveExploredCornerLocations(HouseInfo& houseInfo)
{
	int found{ 0 };
	int houseIndex{ 0 };
	std::vector<int> indexesToRemove{};
	// Remove the house corner location if it's in the explored house vicinity		
	auto findIt = std::remove_if(m_pHouseCornerLocations->begin(), m_pHouseCornerLocations->end(), [houseInfo, &found, &houseIndex, &indexesToRemove](Elite::Vector2& pos)
		{
			float margin{ 5.f };
			float halfWidth = houseInfo.Size.x / 2.f;
			float halfHeight = houseInfo.Size.y / 2.f;

			// Check if corner location is in the vicinity of a house
			if ((pos.x - margin < houseInfo.Center.x + halfWidth) && (pos.x + margin > houseInfo.Center.x - halfWidth) &&
				(pos.y - margin < houseInfo.Center.y + halfHeight) && (pos.y + margin > houseInfo.Center.y - halfHeight))
			{
				indexesToRemove.push_back(houseIndex);
				++found;
				++houseIndex;
				return true;
			}
			++houseIndex;
			return false;
		}
	);

	if (findIt != m_pHouseCornerLocations->end())
		m_pHouseCornerLocations->erase(findIt, m_pHouseCornerLocations->end());
}

// SearchForFood: public GOAPSearchItem
// Preconditions: InitialHouseScoutDone(true) 
// Effects: HasFood(true)
GOAPSearchForFood::GOAPSearchForFood(GOAPPlanner* pPlanner) :
	GOAPSearchItem(pPlanner, "GOAPSearchForFood")
{
	InitPreConditions(pPlanner);
	InitEffects(pPlanner);
	m_Cost = 1.5f;
}
void GOAPSearchForFood::Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	GOAPSearchItem::Setup(pInterface, pPlanner, pBlackboard);
}
bool GOAPSearchForFood::Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt)
{
	if (GOAPSearchItem::IsDone(pInterface, pPlanner, pBlackboard))
		return false;

	bool searchItemSucceeded = GOAPSearchItem::Perform(pInterface, pPlanner, pBlackboard, dt);
	if (!searchItemSucceeded)
		return false;

	// No problems were encountered
	return true;
}
bool GOAPSearchForFood::IsDone(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	// Check if we found energy item
	//return GOAPSearchItem::IsDone(pInterface, pPlanner, pBlackboard);


	for (GOAPProperty* p : m_Effects)
	{
		if (p->propertyKey == "HasFood")
		{
			return m_pWorldState->IsStateMet(p->propertyKey, p->value.bValue);
		}
	}
	return true;
}
void GOAPSearchForFood::InitPreConditions(GOAPPlanner* pPlanner)
{}
void GOAPSearchForFood::InitEffects(GOAPPlanner* pPlanner)
{
	GOAPProperty* pCondition = new GOAPProperty{ "HasFood", true };
	utils::AddActionProperty(pCondition, m_Effects, m_pWorldState, false);
}

// GOAPSearchForMedkit: public GOAPSearchItem
// Preconditions: InitialHouseScoutDone(true) 
// Effects: HasMedkit(true)
GOAPSearchForMedkit::GOAPSearchForMedkit(GOAPPlanner* pPlanner) :
	GOAPSearchItem(pPlanner, "GOAPSearchForMedkit")
{
	InitPreConditions(pPlanner);
	InitEffects(pPlanner);
	m_Cost = 1.f;
}
void GOAPSearchForMedkit::Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	GOAPSearchItem::Setup(pInterface, pPlanner, pBlackboard);
}
bool GOAPSearchForMedkit::Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt)
{
	if (GOAPSearchItem::IsDone(pInterface, pPlanner, pBlackboard))
		return false;
	bool searchItemSucceeded = GOAPSearchItem::Perform(pInterface, pPlanner, pBlackboard, dt);
	if (!searchItemSucceeded)
		return false;

	// No problems were encountered
	return true;
}
bool GOAPSearchForMedkit::IsDone(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{

	for (GOAPProperty* p : m_Effects)
	{
		if (p->propertyKey == "HasMedkit")
		{
			return m_pWorldState->IsStateMet(p->propertyKey, p->value.bValue);
		}
	}

	return true;
}
void GOAPSearchForMedkit::InitPreConditions(GOAPPlanner* pPlanner)
{}
void GOAPSearchForMedkit::InitEffects(GOAPPlanner* pPlanner)
{
	GOAPProperty* pCondition = new GOAPProperty{ "HasMedkit", true };
	utils::AddActionProperty(pCondition, m_Effects, m_pWorldState, false);
}

// TODO: Ask world information to see how far we can scout
// Find general house locations action
// Preconditions: InitialHouseScoutDone(false)
// Effects: InitialHouseScoutDone(true)
GOAPFindGeneralHouseLocationsAction::GOAPFindGeneralHouseLocationsAction(GOAPPlanner* pPlanner) :
	GOAPAction(pPlanner, "GOAPFindGeneralHouseLocationsAction")
{
	m_Cost = 0.f;

	InitPreConditions(pPlanner);
	InitEffects(pPlanner);
}
void GOAPFindGeneralHouseLocationsAction::Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Setting up GOAPFindGeneralHouseLocationsAction\n";
	std::cout << "Scouting houses, this might take a while...\n";
	pBlackboard->AddData("HouseCornerLocations", &m_HouseCornerLocations);
}
bool GOAPFindGeneralHouseLocationsAction::Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt)
{
	while (m_Angle < 360.f && m_TimesLooped < m_Loops)
	{
		Elite::Vector2 locationToExplore{ cos(m_Angle) * m_ExploreVicinityRadius, sin(m_Angle) * m_ExploreVicinityRadius };
		Elite::Vector2 cornerLocation = pInterface->NavMesh_GetClosestPathPoint(locationToExplore);

		if (locationToExplore.Distance(cornerLocation) >= m_IgnoreLocationDistance)
		{
			bool exists = false;
			auto foundIt = std::find(m_HouseCornerLocations.begin(), m_HouseCornerLocations.end(), cornerLocation);

			// Check if this location was a new location
			if (foundIt == m_HouseCornerLocations.end())
			{
				m_HouseCornerLocations.push_back(cornerLocation);
			}
		}

		m_Angle += m_AngleIncrement;

		if (m_Angle >= 359.99)
		{
			std::cout << "Iteration " << m_TimesLooped + 1 << " of " << m_Loops << "\n";
			++m_TimesLooped;
			m_Angle = 0.f;
			m_ExploreVicinityRadius += m_RangeIncrease;
		}
	}



	return true;
}
bool GOAPFindGeneralHouseLocationsAction::CheckPreConditions(GOAPPlanner* pPlanner) const
{
	return false;
}
bool GOAPFindGeneralHouseLocationsAction::CheckProceduralPreconditions(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	return false;
}
bool GOAPFindGeneralHouseLocationsAction::RequiresMovement(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	return false;
}
bool GOAPFindGeneralHouseLocationsAction::IsDone(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	ApplyEffects(pInterface, pPlanner, pBlackboard);
	return true;
}
void GOAPFindGeneralHouseLocationsAction::InitPreConditions(GOAPPlanner* pPlanner)
{
	GOAPProperty* pCondition = new GOAPProperty{ "InitialHouseScoutDone", false };
	utils::AddActionProperty(pCondition, m_Preconditions, m_pWorldState, false);
}
void GOAPFindGeneralHouseLocationsAction::InitEffects(GOAPPlanner* pPlanner)
{
	GOAPProperty* pCondition = new GOAPProperty{ "InitialHouseScoutDone", true };
	utils::AddActionProperty(pCondition, m_Effects, m_pWorldState, false);
}

GOAPFastHouseScout::GOAPFastHouseScout(GOAPPlanner* pPlanner) :
	GOAPAction(pPlanner, "GOAPFastHouseScout")
{
	InitPreConditions(pPlanner);
	InitEffects(pPlanner);
	m_Cost = -1.f;
}
void GOAPFastHouseScout::Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Setting up GOAPFastHouseScout\n";
	// Setup behavior to an item search behavior with priority for energy
	bool dataValid = pBlackboard->GetData("HouseLocations", m_pHouseLocations) && pBlackboard->GetData("HouseCornerLocations", m_pHouseCornerLocations);
	if (!dataValid)
	{
		std::cout << "Error obtaining blackboard data in GOAPFastHouseScout::Setup\n";
		return;
	}

	pBlackboard->GetData("ScoutedVectors", m_pScoutedVectors);

	m_WorldInfo = pInterface->World_GetInfo();
}
bool GOAPFastHouseScout::Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt)
{
	ExploredHouse* pHouse;
	pBlackboard->GetData("AgentHouse", pHouse);
	if (pHouse)
	{
		return true;
	}

	int cycle{ 0 };
	int positionsChecked{ 0 };
	int positionsPerCycle = m_PositionsToCheck / m_Cycles;
	float angleIncrement = 360.f / positionsPerCycle;
	float angle{ 0.f };

	const Elite::Vector2& agentPos = pInterface->Agent_GetInfo().Position;

	while (positionsChecked < positionsPerCycle)
	{
		// Find a point to explore
		float distance = m_DistanceFromAgent + cycle * m_DistanceIncreasePerCycle;
		Elite::Vector2 locationToExplore{ agentPos.x + cos(angle) * distance, agentPos.y + sin(angle) * distance };
		// Make sure it's inside of the navmesh
		if (utils::IsPointInRect(locationToExplore, m_WorldInfo.Center, m_WorldInfo.Dimensions))
		{
			Elite::Vector2 cornerLocation = pInterface->NavMesh_GetClosestPathPoint(locationToExplore);
			// TODO: remove
			if (m_pScoutedVectors)
			{
				m_pScoutedVectors->push_back(Line{ agentPos, locationToExplore });
			}

			// The new position was far enough to assume there was an obstacle!
			if (locationToExplore.Distance(cornerLocation) >= m_IgnoreLocationDistance)
			{
				bool houseFound{ false };
				for (const ExploredHouse& h : *m_pHouseLocations)
				{
					if (utils::IsPointInRect(cornerLocation, h.houseInfo.Center, h.houseInfo.Size, -3.f))
					{
						houseFound = true;
						break;
					}
				}

				if (!houseFound)
				{
					std::cout << "Found a new unexplored house!\n";
					m_pHouseCornerLocations->push_back(cornerLocation);
				}
			}
		}

		++positionsChecked;
		// Check if we completed the cycle
		if (positionsChecked == positionsPerCycle && cycle < m_Cycles - 1)
		{
			positionsChecked = 0;
			angle = 0.f;
			++cycle;

			// If the cycle has an uneven count
			if (cycle % 2 != 0)
				angle += m_OffcycleAngleOffset;
		}

		angle += angleIncrement;
	}

	return true;
}
bool GOAPFastHouseScout::IsDone(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	ApplyEffects(pInterface, pPlanner, pBlackboard);
	return true;
}
void GOAPFastHouseScout::InitPreConditions(GOAPPlanner* pPlanner)
{
	GOAPProperty* pCondition = new GOAPProperty{ "FastScoutAllowed", true };
	utils::AddActionProperty(pCondition, m_Preconditions, m_pWorldState, false);
}
void GOAPFastHouseScout::InitEffects(GOAPPlanner* pPlanner)
{
	GOAPProperty* pCondition = new GOAPProperty{ "FastScoutAllowed", false };
	utils::AddActionProperty(pCondition, m_Effects, m_pWorldState, false);
}

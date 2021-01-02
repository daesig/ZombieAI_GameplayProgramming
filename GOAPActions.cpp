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
GOAPAction::GOAPAction(GOAPPlanner* pPlanner, const std::string& effectName):
	m_EffectName{ effectName }
{
	m_pWorldState = pPlanner->GetWorldState();
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
// Preconditions: HasMoreThan5Energy(true), HasMoreThan5Health(true)
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
	GOAPProperty* pEnergyCondition = new GOAPProperty{ "HasMoreThan5Energy", true };
	utils::AddActionProperty(pEnergyCondition, m_Preconditions, m_pWorldState, true);

	GOAPProperty* pHealthCondition = new GOAPProperty{ "HasMoreThan5Health", true };
	utils::AddActionProperty(pHealthCondition, m_Preconditions, m_pWorldState, true);

	//GOAPProperty* pTestCondition = new GOAPProperty{ "SurviveTest", true };
	//utils::AddActionProperty(pTestCondition, m_Preconditions, m_pWorldState, false);
}
void GOAPSurvive::InitEffects(GOAPPlanner* pPlanner) {}

// DrinkEnergy
// Preconditions: HasEnergyItem(true)
// Effects: HasMoreThan5Energy(true), HasEnergyItem(false)
GOAPConsumeFood::GOAPConsumeFood(GOAPPlanner* pPlanner) :
	GOAPAction(pPlanner, "GOAPConsumeFood")
{
	InitPreConditions(pPlanner);
	InitEffects(pPlanner);
}
bool GOAPConsumeFood::Plan(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	return true;
}
void GOAPConsumeFood::Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Setting up GOAPDrinkEnergy\n";
}
bool GOAPConsumeFood::Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt)
{
	Agent* pAgent = nullptr;
	bool dataValid = pBlackboard->GetData("Agent", pAgent);
	if (!dataValid)
		return false;

	bool consumedFood = pAgent->ConsumeFood();
	return consumedFood;
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
	GOAPProperty* pEnergyCondition = new GOAPProperty{ "HasMoreThan5Energy", true };
	utils::AddActionProperty(pEnergyCondition, m_Effects, m_pWorldState, true);

	GOAPProperty* pHasFoodCondition = new GOAPProperty{ "HasFood", false };
	utils::AddActionProperty(pHasFoodCondition, m_Effects, m_pWorldState, false);
}

// GOAPSearchItem: public GOAPAction
GOAPSearchItem::GOAPSearchItem(GOAPPlanner* pPlanner, const std::string effectName) :
	GOAPAction(pPlanner, "effectName")
{}
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
	m_pAgent->SetBehavior(BehaviorType::SEEKDODGE);
}
bool GOAPSearchItem::Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt)
{
	auto vEntitiesInFov = utils::GetEntitiesInFOV(pInterface);
	auto vHousesInFOV = utils::GetHousesInFOV(pInterface);

	bool requiresNewSeekPos{ false };

	// Check for items
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
			m_pHouseLocations->push_back(ExploredHouse{ house, FLT_MAX });
			std::cout << "Houses explored: " << m_pHouseLocations->size() << "\n";
			// Remove all corner locations of this house
			RemoveExploredCornerLocations(house);
			// Choose a new seek location
			requiresNewSeekPos = true;
		}
	}

	// Check if we need a new seek location
	if (CheckArrival(pInterface, pPlanner, pBlackboard) || requiresNewSeekPos)
	{
		ChooseSeekLocation(pInterface, pPlanner, pBlackboard);
	}

	/// Debugging
	// Debug seek location
	pInterface->Draw_SolidCircle(m_pAgent->GetGoalPosition(), 3.f, {}, { 0.f,1.f,0.f });
	// Debug corner locations
	for (const Elite::Vector2& c : *m_pHouseCornerLocations)
	{
		pInterface->Draw_SolidCircle(c, 2.f, {}, { 0.f,0.f,1.f });
	}

	pInterface->Draw_SolidCircle(distantGoalPos, 2.f, {}, { 1.f, 0.f, 0.f });

	return true;
}
bool GOAPSearchItem::IsDone(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	return false;
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
				float distanceToAgentSquared = agentPos.DistanceSquared(i.Location);
				if (distanceToAgentSquared < closestItemDistanceFromAgentSquared)
				{
					closestItemDistanceFromAgentSquared = distanceToAgentSquared;
					pClosestItem = &i;
				}
			}

			if (pClosestItem)
			{
				distantGoalPos = pClosestItem->Location;
				destination = pInterface->NavMesh_GetClosestPathPoint(distantGoalPos);
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
			if (h.timeSinceExplored > m_HouseExploreCooldown)
				possibleHouses.push_back(&h);
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
				distantGoalPos = pClosestHouse->houseInfo.Center;
				m_HouseGoalPos = pClosestHouse->houseInfo.Center;
				destination = pInterface->NavMesh_GetClosestPathPoint(pClosestHouse->houseInfo.Center);
			}
			else
				std::cout << "Error finding path to house\n";
		}
	}

	if (pClosestItem)
	{
		if (closestItemDistanceFromAgentSquared < closestHouseDistanceFromAgentSquared)
		{
			destination = pClosestItem->Location;
		}
		foundPath = true;
	}
	else if (pClosestHouse)
	{
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
				float distanceToAgentSquared = agentPos.DistanceSquared(cornerLoc);

				if (distanceToAgentSquared < closestCornerDistanceFromAgentSquared)
				{
					closestCornerDistanceFromAgentSquared = distanceToAgentSquared;
					closestCorner = cornerLoc;
				}
			}

			distantGoalPos = closestCorner;
			destination = pInterface->NavMesh_GetClosestPathPoint(closestCorner);
			foundPath = true;
		}
		else
		{
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
		for (ExploredHouse& h : *m_pHouseLocations)
		{
			float housePadding{ 1.f };
			float marginX{ h.houseInfo.Size.x / housePadding };
			float marginY{ h.houseInfo.Size.y / housePadding };
			float halfWidth = h.houseInfo.Size.x / 2.f;
			float halfHeight = h.houseInfo.Size.y / 2.f;
			// Check if agent location is in the house
			if ((agentPos.x + housePadding < h.houseInfo.Center.x + halfWidth) && (agentPos.x - housePadding > h.houseInfo.Center.x - halfWidth) &&
				(agentPos.y + housePadding < h.houseInfo.Center.y + halfHeight) && (agentPos.y - housePadding > h.houseInfo.Center.y - halfHeight))
			{
				std::cout << "Agent is in house\n";
				h.timeSinceExplored = 0.f;
			}
		}
	}

	// Has the agent arrived at it's location
	if (agentPos.DistanceSquared(m_pAgent->GetGoalPosition()) < m_ArrivalRange * m_ArrivalRange)
	{
		std::cout << "Arrived\n";
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

	std::cout << "Removed corners << " << found << " \n";
	std::cout << "last index:  << " << m_pHouseCornerLocations->size() << " \n";
	for (int index : indexesToRemove)
	{
		std::cout << "removed index: " << index << "\n";
	}

	if (findIt != m_pHouseCornerLocations->end())
		m_pHouseCornerLocations->erase(findIt, m_pHouseCornerLocations->end());
}

// SearchForEnergy
// Preconditions: InitialHouseScoutDone(true) 
// Effects: HasEnergyItem(true)
GOAPSearchForFood::GOAPSearchForFood(GOAPPlanner* pPlanner) :
	GOAPSearchItem(pPlanner, "GOAPSearchForFood")
{
	InitPreConditions(pPlanner);
	InitEffects(pPlanner);
}
bool GOAPSearchForFood::Plan(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	return true;
}
void GOAPSearchForFood::Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	GOAPSearchItem::Setup(pInterface, pPlanner, pBlackboard);
}
bool GOAPSearchForFood::Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt)
{
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
			}

			if (!grabError)
			{
				//Elite::Vector2
				std::cout << "Grab error\n";
			}
		}

		// Remove all the grabbed items from the items on ground list
		auto findIt = std::find_if(m_pItemsOnGround->begin(), m_pItemsOnGround->end(), [&grabbedItems](EntityInfo& e)
			{
				for (EntityInfo* grabbedItem : grabbedItems)
				{
					if (grabbedItem->Location == e.Location)
						return true;
				}
				return false;
			}
		);

		// Erase found items
		if (findIt != m_pItemsOnGround->end())
			m_pItemsOnGround->erase(findIt);
	}
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
	return m_pWorldState->IsStateMet(m_Effects[0]->propertyKey, m_Effects[0]->value.bValue);
}
void GOAPSearchForFood::InitPreConditions(GOAPPlanner* pPlanner)
{
	GOAPProperty* pCondition = new GOAPProperty{ "InitialHouseScoutDone", true };
	utils::AddActionProperty(pCondition, m_Preconditions, m_pWorldState, false);
}
void GOAPSearchForFood::InitEffects(GOAPPlanner* pPlanner)
{
	GOAPProperty* pCondition = new GOAPProperty{ "HasFood", true };
	utils::AddActionProperty(pCondition, m_Effects, m_pWorldState, false);
}

// Explore world action
// Preconditions: InitialHouseScoutDone(true) 
	//TODO: IsInHouse(false) 
// Effects: SurviveTest(true)
GOAPExploreWorldAction::GOAPExploreWorldAction(GOAPPlanner* pPlanner) :
	GOAPAction(pPlanner, "GOAPExploreWorldAction")
{
	std::cout << "GOAPExploreWorldAction constructed\n";
	m_RequiresMovement = true;

	InitPreConditions(pPlanner);
	InitEffects(pPlanner);
}
bool GOAPExploreWorldAction::Plan(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	return true;
}
void GOAPExploreWorldAction::Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Setting up GOAPExploreWorldAction\n";
	// Plan where to move to considering the tile locations that have already been explored
	// Consider explored houses and a range around the houses for a more optimal search routine

	AgentInfo agentInfo = pInterface->Agent_GetInfo();
	moveTarget.Position = agentInfo.Position + Elite::Vector2(m_ExploreActionRange * 7.f, m_ExploreActionRange * 14.f);
}
bool GOAPExploreWorldAction::RequiresMovement(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	AgentInfo agentInfo = pInterface->Agent_GetInfo();
	Elite::Vector2 agentPosition = agentInfo.Position;
	float distanceToPosition = agentPosition.Distance(moveTarget.Position);

	// Check if the movement is fullfilled
	if (distanceToPosition < m_MovementFulfilledRange)
		return false;

	// Movement required
	return true;
}
void GOAPExploreWorldAction::InitPreConditions(GOAPPlanner* pPlanner)
{
	// Get a reference to the world states
	WorldState* pWorldState = pPlanner->GetWorldState();

	// Create states
	// House in sight state
	GOAPProperty* pHouseInSight = new GOAPProperty{ "InitialHouseScoutDone", true };
	m_Preconditions.push_back(pHouseInSight);

	// Make sure the states exist in the world
	if (!pWorldState->DoesStateExist(pHouseInSight->propertyKey))
	{
		// State doesn't exist, add the state with some default starter value
		pWorldState->AddState(pHouseInSight->propertyKey, false);
	}
}
void GOAPExploreWorldAction::InitEffects(GOAPPlanner* pPlanner)
{
	// Get a reference to the world states
	WorldState* pWorldState = pPlanner->GetWorldState();

	// Create states
	// House in sight state
	GOAPProperty* pHouseInSight = new GOAPProperty{ "SurviveTest", true };
	m_Effects.push_back(pHouseInSight);

	// Make sure the states exist in the world
	if (!pWorldState->DoesStateExist(pHouseInSight->propertyKey))
	{
		// State doesn't exist, add the state with some default starter value
		pWorldState->AddState(pHouseInSight->propertyKey, pHouseInSight->value.bValue);
	}
}

// TODO: Ask world information to see how far we can scout
// Find general house locations action
// Preconditions: InitialHouseScoutDone(false)
// Effects: InitialHouseScoutDone(true)
GOAPFindGeneralHouseLocationsAction::GOAPFindGeneralHouseLocationsAction(GOAPPlanner* pPlanner) :
	GOAPAction(pPlanner, "GOAPFindGeneralHouseLocationsAction")
{
	std::cout << "GOAPFindGeneralHouseLocationsAction constructed\n";
	m_Cost = -100.f;

	InitPreConditions(pPlanner);
	InitEffects(pPlanner);
}
bool GOAPFindGeneralHouseLocationsAction::Plan(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	return true;
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

// TODO: make this a behavior
// GOAPEvadeEnemy
// Preconditions: EnemyWasInSight(true)
// Effects: EnemyWasInSight(false)
GOAPEvadeEnemy::GOAPEvadeEnemy(GOAPPlanner* pPlanner) :
	GOAPAction(pPlanner, "GOAPEvadeEnemy")
{
	std::cout << "GOAPEvadeEnemy constructed\n";
	m_Cost = 2.f;

	InitPreConditions(pPlanner);
	InitEffects(pPlanner);
}
bool GOAPEvadeEnemy::Plan(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	return true;
}
void GOAPEvadeEnemy::Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
{
	std::cout << "Setting up GOAPEvadeEnemy\n";
	m_EvadeTimer = 0.f;

	Agent* pAgent = nullptr;
	// Check if agent data is valid
	bool dataValid = pBlackboard->GetData("Agent", pAgent);
	if (!dataValid) return;

	// Set agent to dodge behavior
	pAgent->SetBehavior(BehaviorType::SEEKDODGE);
}
bool GOAPEvadeEnemy::Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt)
{
	m_EvadeTimer += dt;
	return true;
}
bool GOAPEvadeEnemy::IsDone(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const
{
	if (m_EvadeTimer >= m_EvadeTime)
	{
		Agent* pAgent = nullptr;
		// Check if agent data is valid
		bool dataValid = pBlackboard->GetData("Agent", pAgent);
		if (!dataValid) return false;

		// Clear the agents behavior
		pAgent->ClearBehavior();

		return true;
	}

	return false;
}
void GOAPEvadeEnemy::InitPreConditions(GOAPPlanner* pPlanner)
{
	// Get a reference to the world states
	WorldState* pWorldState = pPlanner->GetWorldState();

	// Create states
	// Requires an enemy in sight to run
	GOAPProperty* pEnemyWasInSight = new GOAPProperty{ "EnemyWasInSight", true };
	m_Preconditions.push_back(pEnemyWasInSight);

	// Make sure the states exist in the world
	if (!pWorldState->DoesStateExist(pEnemyWasInSight->propertyKey))
	{
		// State doesn't exist, add the state with some default starter value
		pWorldState->AddState(pEnemyWasInSight->propertyKey, pEnemyWasInSight->value.bValue);
	}
}
void GOAPEvadeEnemy::InitEffects(GOAPPlanner* pPlanner)
{
	// Get a reference to the world states
	WorldState* pWorldState = pPlanner->GetWorldState();

	// Create states
	// Requires an enemy in sight to run
	GOAPProperty* pEnemyWasInSight = new GOAPProperty{ "EnemyWasInSight", false };
	m_Preconditions.push_back(pEnemyWasInSight);

	// Make sure the states exist in the world
	if (!pWorldState->DoesStateExist(pEnemyWasInSight->propertyKey))
	{
		// State doesn't exist, add the state with some default starter value
		pWorldState->AddState(pEnemyWasInSight->propertyKey, pEnemyWasInSight->value.bValue);
	}
}


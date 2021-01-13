#include "stdafx.h"

#include "Agent.h"

#include "utils.h"

#include "GOAPPlanner.h"
#include "DecisionMaking.h"
#include "Blackboard.h"
#include "FSMState.h"
#include "StatesAndTransitions.h"
#include "ConfigManager.h"

Agent::Agent(IExamInterface* pInterface) :
	m_pInterface(pInterface)
{
	DebugOutputManager::GetInstance()->DebugLine("ZombieAI version 1.0\n", DebugOutputManager::DebugType::CONSTRUCTION);
	Initialize();
	m_ExploredLocationTimer = m_ExploredLocationRefreshTime;
	m_MaxInventorySlots = m_pInterface->Inventory_GetCapacity();
}

Agent::~Agent()
{
	DeleteFSM();
	DeleteGOAP();
	DeleteBehaviors();
	DeleteBlackboard();
	DeleteWorldState();

	DebugOutputManager::GetInstance()->DebugLine("Deconstructed agent\n\n\n",
		DebugOutputManager::DebugType::DESTRUCTION);
}

// Update
SteeringPlugin_Output Agent::UpdateSteering(float dt)
{
	SteeringPlugin_Output steering{};

	// Get interface information
	AgentInfo& agentInfo = m_pInterface->Agent_GetInfo();
	auto vEntitiesInFOV = utils::GetEntitiesInFOV(m_pInterface); //uses m_pInterface->Fov_GetEntityByIndex(...)
	// Set the house the agent is in
	SetAgentHouseInBlackboard(agentInfo.Position);

	// Manage bitten status
	if (agentInfo.WasBitten)
	{
		m_BittenTimer = 0.f;
		m_WasBitten = true;
	}
	m_BittenTimer += dt;
	if (m_BittenTimer > m_BittenTime)
	{
		m_WasBitten = false;
	}

	// Manage worldstates
	// Manage vitals
	if (agentInfo.Energy < m_MimimumRequiredFood)
		m_pWorldState->SetState("RequiresFood", true);
	if (agentInfo.Health < m_MinimumRequiredHealth)
		m_pWorldState->SetState("RequiresHealth", true);
	if (agentInfo.Position.Distance(GetGoalPosition()) < m_DistanceToFullfillMovement)
	{
		m_pWorldState->SetState("HasGoal", false);
	}
	// Reset worldstate
	m_pWorldState->SetState("EnemyInSight", false);
	// Manage fast scout timer
	if (m_FastScoutTimer > m_FastScoutTime)
	{
		m_FastScoutTimer = 0.f;
		m_pWorldState->SetState("FastScoutAllowed", true);
	}
	m_FastScoutTimer += dt;

	// Get most nearby enemy and enemy count
	float closestDistanceSq{ FLT_MAX };
	for (auto& enemyInFov : vEntitiesInFOV)
	{
		if (enemyInFov.Type == eEntityType::ENEMY)
		{
			float distanceToEnemySq = agentInfo.Position.DistanceSquared(enemyInFov.Location);
			if (distanceToEnemySq < closestDistanceSq)
			{
				m_LastSeenClosestEnemy = enemyInFov.Location;
				m_pBlackboard->ChangeData("LastEnemyPos", &m_LastSeenClosestEnemy);
				m_pWorldState->SetState("EnemyInSight", true);
			}
		}
	}

	m_EnemyCount = static_cast<int>(vEntitiesInFOV.size());

	// Update FSM states
	if (m_pDecisionMaking)
	{
		m_pDecisionMaking->Update(m_pInterface, m_pGOAPPlanner, dt);
	}

	// Steer
	if (m_pSteeringBehavior)
	{
		steering = m_pSteeringBehavior->CalculateSteering(m_pInterface, dt, agentInfo, m_pBlackboard);
	}

	for (Line& l : m_ScoutedVectors)
	{
		l.lifeTime -= dt;
	}

	m_ScoutedVectors.erase(std::remove_if(m_ScoutedVectors.begin(), m_ScoutedVectors.end(),
		[](Line& l) { return l.lifeTime <= 0.f; }), m_ScoutedVectors.end());

	return steering;
}
// Render
void Agent::Render(IExamInterface* pExamInterface, float dt) const
{
	if (ConfigManager::GetInstance()->GetDebugHouseScoutVectors())
	{
		for (const Line& l : m_ScoutedVectors)
		{
			m_pInterface->Draw_Segment(l.pointA, l.pointB, { 1.f,1.f,1.f });
		}
	}
}

// Controlling behaviors
void Agent::ClearBehavior()
{
	m_pSteeringBehavior = nullptr;
}
void Agent::SetBehavior(BehaviorType behaviorType)
{
	m_DebugSeek = false;
	switch (behaviorType)
	{
	case BehaviorType::SEEK:
		m_pSteeringBehavior = m_pSeekBehavior;
		m_DebugSeek = true;
		break;
	case BehaviorType::SEEKDODGE:
		m_pSteeringBehavior = m_pSeekDodgeBehavior;
		break;
	case BehaviorType::KILL:
		m_pSteeringBehavior = m_pKillBehavior;
		break;
	default:
		m_pSteeringBehavior = nullptr;
		break;
	}
}
void Agent::SetSeekPos(Elite::Vector2 seekPos)
{
	m_pSeekBehavior->SetTarget(seekPos);
}

// Inventory
bool Agent::GrabItem(EntityInfo& i, IExamInterface* pInterface)
{
	// Get info
	ItemInfo itemInfo;
	pInterface->Item_GetInfo(i, itemInfo);
	AgentInfo agentInfo = pInterface->Agent_GetInfo();

	// Check if the item is within grabrange
	float distanceSquared = agentInfo.Position.DistanceSquared(itemInfo.Location);
	if (distanceSquared < agentInfo.GrabRange * agentInfo.GrabRange)
	{
		// Don't pick up garbage
		if (itemInfo.Type == eItemType::GARBAGE)
		{
			bool destroyed = pInterface->Item_Destroy(i);
			return destroyed;
		}

		// Else, try and add the item to the inventory
		return AddInventoryItem(i);
	}

	// No item was grabbed, return false
	return false;
}
bool Agent::ConsumeItem(const eItemType& itemType)
{
	bool success{ false };

	int index{ 0 };
	while (index < m_MaxInventorySlots)
	{
		ItemInfo inventoryItem;
		bool itemFound = m_pInterface->Inventory_GetItem(index, inventoryItem);
		if (itemFound)
		{
			if (inventoryItem.Type == itemType)
			{
				success = m_pInterface->Inventory_UseItem(index);
				m_pInterface->Inventory_RemoveItem(index);
				break;
			}
		}
		++index;
	}

	return success;
}
bool Agent::Shoot()
{
	int index{ 0 };
	int gunsFound{ 0 };
	bool shotGun{ false };
	while (index < m_MaxInventorySlots)
	{
		ItemInfo itemInfo;
		m_pInterface->Inventory_GetItem(index, itemInfo);

		// Item is a gun
		if (itemInfo.Type == eItemType::PISTOL)
		{
			++gunsFound;
			// Shoot if we haven't shot before
			if (!shotGun)
			{
				// Make sure we can only shoot once
				shotGun = true;
				// Shoot
				m_pInterface->Inventory_UseItem(index);
				// Remove then gun from inventory if it has no ammo left
				if (m_pInterface->Weapon_GetAmmo(itemInfo) < 1)
				{
					m_pInterface->Inventory_RemoveItem(index);
					// We have one less gun
					--gunsFound;
				}
			}
		}
		++index;
	}

	// All guns have been used
	if (gunsFound < 1)
	{
		m_pWorldState->SetState("HasWeapon", false);
	}

	return shotGun;
}
bool Agent::WasBitten() const
{
	return m_WasBitten;
}
bool Agent::AddInventoryItem(const EntityInfo& entity)
{
	bool success{ false };

	// Get item info
	ItemInfo lootedItemInfo;
	m_pInterface->Item_GetInfo(entity, lootedItemInfo);
	eItemType lootedItemType{ lootedItemInfo.Type };

	std::vector<int> medkitsFoundIndices{  };
	std::vector<int> foodFoundIndices{  };
	bool requiredItem = lootedItemType == eItemType::FOOD || lootedItemType == eItemType::MEDKIT;

	// Go through the inventory and see if we can add the item
	int index{ 0 };
	while (index < m_MaxInventorySlots)
	{
		ItemInfo itemInCurrentSlot{};
		bool itemFound = m_pInterface->Inventory_GetItem(index, itemInCurrentSlot);

		// Found an empty inventory slot
		if (!itemFound)
		{
			// Grab the item from the ground
			bool grabSuccess = m_pInterface->Item_Grab(entity, lootedItemInfo);
			if (!grabSuccess) return false;

			// Add the grabbed item to the inventory
			success = m_pInterface->Inventory_AddItem(index, lootedItemInfo);
			if (success)
			{
				ProcessItemWorldState(lootedItemInfo.Type);
				break;
			}
			else
			{
				DebugOutputManager::GetInstance()->DebugLine("Failed to add inventory item\n",
					DebugOutputManager::DebugType::INVENTORY);
				return false;
			}
		}
		else
		{
			// Process item counts to make sure we always maintain one of these items if possible
			switch (itemInCurrentSlot.Type)
			{
			case eItemType::MEDKIT:
				medkitsFoundIndices.push_back(index);
				break;
			case eItemType::FOOD:
				foodFoundIndices.push_back(index);
				break;
			}

			// The itemtype we found is the same as the itemtype we looted
			if (itemInCurrentSlot.Type == lootedItemType)
			{
				// Check if the current item stack is smaller
				if (GetItemStackSize(itemInCurrentSlot) < GetItemStackSize(lootedItemInfo))
				{
					// Clear the current inventory slot
					if (RemoveInventoryItem(index, itemInCurrentSlot))
					{
						// Grab the item from the ground
						bool grabSuccess = m_pInterface->Item_Grab(entity, lootedItemInfo);
						if (!grabSuccess) return false;
						// Add the grabbed item to the inventory
						success = m_pInterface->Inventory_AddItem(index, lootedItemInfo);
						if (success)
						{
							ProcessItemWorldState(lootedItemInfo.Type);
							break;
						}
						else
						{
							DebugOutputManager::GetInstance()->DebugLine("Failed to replace add inventory item\n",
								DebugOutputManager::DebugType::INVENTORY);
							return false;
						}
					}
					else
					{
						// Failed to remove inventory item!
						DebugOutputManager::GetInstance()->DebugLine("Failed to remove inventory item\n",
							DebugOutputManager::DebugType::INVENTORY);
						return false;
					}
				}
			}
		}

		++index;
	}

	// The inventory did not have space for the item
	if (!success)
	{
		// Attempt to replace another random item if this item is required
		if (requiredItem)
		{
			// The item is only required if no other instances of it were found in the inventory
			switch (lootedItemType)
			{
			case eItemType::MEDKIT:
				requiredItem = medkitsFoundIndices.size() == 0;
				break;
			case eItemType::FOOD:
				requiredItem = foodFoundIndices.size() == 0;
				break;
			}
		}

		// Final check, the item is now required, replace a random other item
		if (requiredItem)
		{
			// Improvements: Possible to not replace the best food / best medkit if we keep count of how many there are in which index
			bool randomIsValid{ false };
			int random;

			// Find a valid inventory slot to replace the item with
			while (!randomIsValid)
			{
				randomIsValid = true;
				random = Elite::randomInt(m_MaxInventorySlots);

				if (foodFoundIndices.size() > 0)
				{
					if (random == foodFoundIndices[0])
						randomIsValid = false;
				}

				if (medkitsFoundIndices.size() > 0)
				{
					if (random == medkitsFoundIndices[0])
						randomIsValid = false;
				}
			}

			// Random was chosen, replace the item in question
			if (RemoveInventoryItem(random))
			{
				// Grab the item from the ground
				bool grabSuccess = m_pInterface->Item_Grab(entity, lootedItemInfo);
				if (!grabSuccess) return false;
				// Add the grabbed item to the inventory
				success = m_pInterface->Inventory_AddItem(random, lootedItemInfo);
				// Process the world states
				if (success)
					ProcessItemWorldState(lootedItemInfo.Type);
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			// No place for item, destroy it
			success = m_pInterface->Item_Destroy(entity);
		}
	}

	return success;
}
bool Agent::RemoveInventoryItem(int itemIndex)
{
	ItemInfo itemInfo;
	m_pInterface->Inventory_GetItem(itemIndex, itemInfo);
	return RemoveInventoryItem(itemIndex, itemInfo);
}
bool Agent::RemoveInventoryItem(int itemIndex, const ItemInfo& itemInfo)
{
	bool removed = false;
	// Removing food or medkits would be a waste without using them up
	if (itemInfo.Type == eItemType::FOOD || itemInfo.Type == eItemType::MEDKIT)
	{
		m_pInterface->Inventory_UseItem(itemIndex);
		removed = m_pInterface->Inventory_RemoveItem(itemIndex);
		ProcessItemWorldState(itemInfo.Type);
	}
	else
	{
		removed = m_pInterface->Inventory_RemoveItem(itemIndex);
	}

	return removed;
}
int Agent::GetItemStackSize(ItemInfo& itemInfo) const
{
	int stackSize{ 0 };

	switch (itemInfo.Type)
	{
	case eItemType::FOOD:
		stackSize = m_pInterface->Food_GetEnergy(itemInfo);
		break;
	case eItemType::MEDKIT:
		stackSize = m_pInterface->Medkit_GetHealth(itemInfo);
		break;
	case eItemType::PISTOL:
		stackSize = m_pInterface->Weapon_GetAmmo(itemInfo);
		break;
	}

	return stackSize;
}
void Agent::ProcessItemWorldState(const eItemType& itemType)
{
	switch (itemType)
	{
	case eItemType::FOOD:
		m_pWorldState->SetState("HasFood", true);
		break;
	case eItemType::MEDKIT:
		m_pWorldState->SetState("HasMedkit", true);
		break;
	case eItemType::PISTOL:
		m_pWorldState->SetState("HasWeapon", true);
		break;
	}
}
void Agent::SetAgentHouseInBlackboard(const Elite::Vector2& agentPos)
{
	ExploredHouse* pHouse = nullptr;
	for (ExploredHouse& h : m_Houses)
	{
		float housePadding{ 1.f };
		float halfWidth = h.houseInfo.Size.x / 2.f;
		float halfHeight = h.houseInfo.Size.y / 2.f;
		// Check if agent location is in the house
		if ((agentPos.x + housePadding < h.houseInfo.Center.x + halfWidth) && (agentPos.x - housePadding > h.houseInfo.Center.x - halfWidth) &&
			(agentPos.y + housePadding < h.houseInfo.Center.y + halfHeight) && (agentPos.y - housePadding > h.houseInfo.Center.y - halfHeight))
		{
			pHouse = &h;
			break;
		}
	}

	m_pBlackboard->ChangeData("AgentHouse", pHouse);
}

// Initialization
void Agent::Initialize()
{
	// InitializeWorldState
	InitializeWorldState();
	// Blackboard
	InitializeBlackboard();
	// Behaviors
	InitializeBehaviors();
	// Initialize GOAP
	InitializeGOAP();
	// FSM
	InitializeFSM();

	DebugOutputManager::GetInstance()->DebugLine("Initialized agent\n\n\n",
		DebugOutputManager::DebugType::CONSTRUCTION);
}
void Agent::InitializeWorldState()
{
	m_pWorldState = new WorldState();
	m_pWorldState->AddState("EnemyInSight", false);
	m_pWorldState->AddState("HasFood", false);
	m_pWorldState->AddState("HasMedkit", false);
	m_pWorldState->AddState("HasWeapon", false);
}
void Agent::InitializeBlackboard()
{
	m_pBlackboard = new Blackboard();
	m_pBlackboard->AddData("Agent", this);
	m_pBlackboard->AddData("LastEnemyPos", &m_LastSeenClosestEnemy);
	m_pBlackboard->AddData("EnemyCount", &m_EnemyCount);
	m_pBlackboard->AddData("WorldState", m_pWorldState);
	m_pBlackboard->AddData("PriorityAction", false);
	m_pBlackboard->AddData("HouseLocations", &m_Houses);
	m_pBlackboard->AddData("ItemLocations", &m_Items);
	m_pBlackboard->AddData("AgentHouse", m_AgentHouse);
	m_pBlackboard->AddData("AgentInPurgeZone", false);
	m_pBlackboard->AddData("HouseCornerLocations", &m_HouseCornerLocations);

	// Debug
	m_pBlackboard->AddData("ScoutedVectors", &m_ScoutedVectors);
	m_pBlackboard->AddData("DebugNavMeshExploration", &m_DebugNavMeshExploration);
}
void Agent::InitializeBehaviors()
{
	m_pSeekBehavior = new Seek();
	m_pSeekDodgeBehavior = new SeekAndDodge();
	m_pKillBehavior = new KillBehavior();
}
void Agent::InitializeGOAP()
{
	// GOAP planner
	m_pGOAPPlanner = new GOAPPlanner(m_pWorldState);

	// GOAP Actions
	GOAPAction* pGOAPConsumeFood = new GOAPConsumeFood(m_pGOAPPlanner);
	GOAPAction* pGOAPConsumeMedkit = new GOAPConsumeMedkit(m_pGOAPPlanner);
	GOAPAction* pGOAPSearchForFood = new GOAPSearchForFood(m_pGOAPPlanner);
	GOAPAction* pGOAPSearchForMedkit = new GOAPSearchForMedkit(m_pGOAPPlanner);
	GOAPAction* pSearchItem = new GOAPSearchItem(m_pGOAPPlanner);
	GOAPAction* pFastScout = new GOAPFastHouseScout(m_pGOAPPlanner);
	m_pActions.push_back(pGOAPConsumeFood);
	m_pActions.push_back(pGOAPConsumeMedkit);
	m_pActions.push_back(pGOAPSearchForFood);
	m_pActions.push_back(pGOAPSearchForMedkit);
	m_pActions.push_back(pSearchItem);
	m_pActions.push_back(pFastScout);
	//...

	// Let the planner know all the action this agent can do
	m_pGOAPPlanner->AddActions(m_pActions);
}
void Agent::InitializeFSM()
{
	IdleState* pIdleState = new IdleState();
	GoToState* pGoToState = new GoToState();
	PerformState* pPerformState = new PerformState();
	m_pStates.push_back(pIdleState);
	m_pStates.push_back(pGoToState);
	m_pStates.push_back(pPerformState);
	GoToTransition* pGoToTransition = new GoToTransition();
	PerformTransition* pPerformTransition = new PerformTransition();
	PerformedTransition* performedTransition = new PerformedTransition();
	m_pTransitions.push_back(pGoToTransition);
	m_pTransitions.push_back(pPerformTransition);
	m_pTransitions.push_back(performedTransition);

	// FSM
	m_pFiniteStateMachine = new FiniteStateMachine(pIdleState, nullptr, m_pGOAPPlanner, m_pBlackboard);
	// The action requires movement before performing, go to GoTo state
	m_pFiniteStateMachine->AddTransition(pIdleState, pGoToState, pGoToTransition);
	// The action doesn't require any movement, go to perform state
	m_pFiniteStateMachine->AddTransition(pIdleState, pPerformState, pPerformTransition);
	// Once movement is done, perform the action
	m_pFiniteStateMachine->AddTransition(pGoToState, pPerformState, pPerformTransition);
	// Action required more movement while performing, go back to movement state
	m_pFiniteStateMachine->AddTransition(pPerformState, pGoToState, pGoToTransition);
	// Action IsDone, go back to idle to recalculate path or choose the next action
	m_pFiniteStateMachine->AddTransition(pPerformState, pIdleState, performedTransition);

	// Set decision making system to the finite state machine
	m_pDecisionMaking = m_pFiniteStateMachine;
}

// Destruction
void Agent::DeleteFSM()
{
	// States and transitions
	for (FSMState* pState : m_pStates)
	{
		delete pState;
		pState = nullptr;
	}
	m_pStates.clear();
	for (FSMTransition* pTransition : m_pTransitions)
	{
		delete pTransition;
		pTransition = nullptr;
	}
	m_pTransitions.clear();

	delete m_pFiniteStateMachine;
	m_pFiniteStateMachine = nullptr;
}
void Agent::DeleteGOAP()
{
	// GOAP planner
	delete m_pGOAPPlanner;
	m_pGOAPPlanner = nullptr;

	for (GOAPAction* pAction : m_pActions)
	{
		delete pAction;
		pAction = nullptr;
	}
	m_pActions.clear();
}
void Agent::DeleteBehaviors()
{
	delete m_pSeekBehavior;
	m_pSeekBehavior = nullptr;
	delete m_pSeekDodgeBehavior;
	m_pSeekDodgeBehavior = nullptr;
	delete m_pKillBehavior;
	m_pKillBehavior = nullptr;

	m_pSteeringBehavior = nullptr;
}
void Agent::DeleteBlackboard()
{
	delete m_pBlackboard;
	m_pBlackboard = nullptr;
}
void Agent::DeleteWorldState()
{
	delete m_pWorldState;
	m_pWorldState = nullptr;
}

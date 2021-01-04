#include "stdafx.h"

#include "Agent.h"

#include "utils.h"

#include "GOAPPlanner.h"
#include "DecisionMaking.h"
#include "Blackboard.h"
#include "FSMState.h"
#include "StatesAndTransitions.h"

Agent::Agent(IExamInterface* pInterface) :
	m_pInterface(pInterface)
{
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
}

// Update
SteeringPlugin_Output Agent::UpdateSteering(float dt)
{
	SteeringPlugin_Output steering{};

	AgentInfo& agentInfo = m_pInterface->Agent_GetInfo();
	//auto vHousesInFOV = utils::GetHousesInFOV(pInterface);//uses m_pInterface->Fov_GetHouseByIndex(...)
	auto vEntitiesInFOV = utils::GetEntitiesInFOV(m_pInterface); //uses m_pInterface->Fov_GetEntityByIndex(...)

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

	// Manage vitals
	if (agentInfo.Energy < m_MimimumRequiredFood)
		m_pWorldState->SetState("RequiresFood", true);
	if (agentInfo.Health < m_MinimumRequiredHealth)
		m_pWorldState->SetState("RequiresHealth", true);
	if (agentInfo.Position.Distance(GetGoalPosition()) < m_DistanceToFullfillMovement)
	{
		m_pWorldState->SetState("HasGoal", false);
	}

	if (m_DebugTimer > m_DebugTime)
	{
		m_DebugTimer = 0.f;
		// Debug
		GOAPAction* pCurrentAction = m_pGOAPPlanner->GetAction();
		if (pCurrentAction)
		{
			std::cout << "Current action: " << pCurrentAction->ToString() << "\n";
		}
	}
	m_DebugTimer += dt;

	// Reset worldstate
	m_pWorldState->SetState("EnemyInSight", false);

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

	// Debug override
	//if (m_pWorldState->IsStateMet("HasWeapon", true))
	//{
	//	m_LastSeenClosestEnemy = { 0.1f,0.1f };
	//	m_pWorldState->SetState("EnemyInSight", true);
	//	m_pBlackboard->ChangeData("LastEnemyPos", &m_LastSeenClosestEnemy);
	//}

	m_EnemyCount = vEntitiesInFOV.size();

	// Update explored houses time
	for (ExploredHouse& h : m_Houses)
	{
		h.timeSinceExplored += dt;
	}

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

	// Update known locations
	if (m_ExploredLocationTimer >= m_ExploredLocationRefreshTime)
	{
		// Get closest navmesh node
		Elite::Vector2 currentPosition{ m_pInterface->NavMesh_GetClosestPathPoint(agentInfo.Position) };

		// Add the location if it doesn't exist yet
		auto it = std::find(m_ExploredTileLocations.begin(), m_ExploredTileLocations.end(), currentPosition);
		if (it == m_ExploredTileLocations.end())
		{
			//m_ExploredTileLocations.push_back(currentPosition);
			//std::cout << "Explored new position: [ " << currentPosition.x << ", " << currentPosition.y << " ]\n";
		}
		m_ExploredLocationTimer = 0.f;
	}
	m_ExploredLocationTimer += dt;

	//for (const EntityInfo& entity : vEntitiesInFOV)
	//{
	//	ItemInfo item{};
	//	if (entity.Type == eEntityType::ITEM)
	//		pInterface->Item_GetInfo(entity, item);
	//}

	//for (auto& e : vEntitiesInFOV)
	//{
	//	if (e.Type == eEntityType::PURGEZONE)
	//	{
	//		PurgeZoneInfo zoneInfo;
	//		pInterface->PurgeZone_GetInfo(e, zoneInfo);
	//		std::cout << "Purge Zone in FOV:" << e.Location.x << ", " << e.Location.y << " ---EntityHash: " << e.EntityHash << "---Radius: " << zoneInfo.Radius << std::endl;
	//	}
	//}					

	return steering;
}
// Render
void Agent::Render(IExamInterface* pExamInterface, float dt) const
{
	//pExamInterface->Draw_SolidCircle(pExam, .7f, { 0,0 }, { 1, 0, 0 });
	for (const Elite::Vector2& exploredLocation : m_ExploredTileLocations)
	{
		pExamInterface->Draw_Circle(exploredLocation, .5f, Elite::Vector3{ 1.f,1.f,1.f });
	}

	if (m_DebugSeek)
		pExamInterface->Draw_SolidCircle(m_pSeekBehavior->GetTarget(), .5f, Elite::Vector2{}, Elite::Vector3{ 1.f, 0.f, 0.f });

	pExamInterface->Draw_Circle(Elite::Vector2{ 0.f,0.f }, 200.f, Elite::Vector3{ 0.f, 0.f, 1.f });
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
	case BehaviorType::WANDER:
		m_pSteeringBehavior = m_pWanderBehavior;
		break;
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
bool Agent::GrabItem(EntityInfo& i, const eItemType& itemPriority, eItemType& grabbedType, IExamInterface* pInterface, bool& grabError)
{
	// Get info
	ItemInfo itemInfo;
	pInterface->Item_GetInfo(i, itemInfo);
	AgentInfo agentInfo = pInterface->Agent_GetInfo();

	// Check if the item is within grabrange
	float distanceSquared = agentInfo.Position.DistanceSquared(itemInfo.Location);
	if (distanceSquared < agentInfo.GrabRange * agentInfo.GrabRange)
	{
		grabbedType = itemInfo.Type;

		// Don't pick up garbage
		if (grabbedType == eItemType::GARBAGE)
		{
			std::cout << "Garbage!\n";
			pInterface->Item_Destroy(i);
		}
		// Non-garbage. Pick up
		else
		{
			return AddInventoryItem(i, grabError);
		}

		// The item was processed, return true
		return true;
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
bool Agent::AddInventoryItem(const EntityInfo& entity, bool& grabError)
{
	int index{ 0 };
	bool success{ false };

	ItemInfo lootedItemInfo;
	m_pInterface->Item_GetInfo(entity, lootedItemInfo);
	eItemType lootedItemType{ lootedItemInfo.Type };

	std::vector<int> medkitsFoundIndices{  };
	std::vector<int> foodFoundIndices{  };
	bool requiredItem = lootedItemType == eItemType::FOOD || lootedItemType == eItemType::MEDKIT;

	while (index < m_MaxInventorySlots)
	{
		ItemInfo itemInCurrentSlot{};
		bool itemFound = m_pInterface->Inventory_GetItem(index, itemInCurrentSlot);

		// Found an empty inventory slot
		if (!itemFound)
		{
			std::cout << "Added item to inventory slots: " << index << " \n";
			// Grab the item from the ground
			grabError = m_pInterface->Item_Grab(entity, lootedItemInfo);
			// Add the grabbed item to the inventory
			success = grabError && m_pInterface->Inventory_AddItem(index, lootedItemInfo);
			// Process the world states
			if (success)
				ProcessItemWorldState(lootedItemInfo.Type);
			break;
		}
		else
		{
			// Process item counts
			switch (itemInCurrentSlot.Type)
			{
			case eItemType::MEDKIT:
				medkitsFoundIndices.push_back(index);
				break;
			case eItemType::FOOD:
				foodFoundIndices.push_back(index);
				break;
			}

			if (itemInCurrentSlot.Type == lootedItemType)
			{
				std::cout << "Item already exists in inventory\n";
				// Current item stack is smaller
				if (GetItemStackSize(itemInCurrentSlot) < GetItemStackSize(lootedItemInfo))
				{
					// Clear the current inventory slot
					m_pInterface->Inventory_RemoveItem(index);
					// Grab the item from the ground
					grabError = m_pInterface->Item_Grab(entity, lootedItemInfo);
					// Add the grabbed item to the inventory
					success = grabError && m_pInterface->Inventory_AddItem(index, lootedItemInfo);
					// Process the world states
					if (success)
						ProcessItemWorldState(lootedItemInfo.Type);
					break;
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
			// Clear the current inventory slot
			m_pInterface->Inventory_RemoveItem(random);
			// Grab the item from the ground
			grabError = m_pInterface->Item_Grab(entity, lootedItemInfo);
			// Add the grabbed item to the inventory
			success = grabError && m_pInterface->Inventory_AddItem(random, lootedItemInfo);
			// Process the world states
			if (success)
				ProcessItemWorldState(lootedItemInfo.Type);
		}
		else
		{
			// No place for item, destroy it
			success = m_pInterface->Item_Destroy(entity);
		}
	}

	return success;
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
void Agent::ProcessItemWorldState(eItemType& itemType)
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

	std::cout << "Initialized\n\n\n";
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
}
void Agent::InitializeBehaviors()
{
	m_pWanderBehavior = new Wander();
	m_pSeekBehavior = new Seek();
	m_pSeekDodgeBehavior = new SeekAndDodge();
	m_pKillBehavior = new KillBehavior();
}
void Agent::InitializeGOAP()
{
	// GOAP planner
	m_pGOAPPlanner = new GOAPPlanner(m_pWorldState);

	// GOAP Actions
	GOAPAction* pGOAPFindGeneralHouseLocationsAction = new GOAPFindGeneralHouseLocationsAction(m_pGOAPPlanner);
	GOAPAction* pGOAPConsumeFood = new GOAPConsumeFood(m_pGOAPPlanner);
	GOAPAction* pGOAPConsumeMedkit = new GOAPConsumeMedkit(m_pGOAPPlanner);
	GOAPAction* pGOAPSearchForFood = new GOAPSearchForFood(m_pGOAPPlanner);
	GOAPAction* pGOAPSearchForMedkit = new GOAPSearchForMedkit(m_pGOAPPlanner);
	GOAPAction* pSearchItem = new GOAPSearchItem(m_pGOAPPlanner);
	m_pActions.push_back(pGOAPFindGeneralHouseLocationsAction);
	m_pActions.push_back(pGOAPConsumeFood);
	m_pActions.push_back(pGOAPConsumeMedkit);
	m_pActions.push_back(pGOAPSearchForFood);
	m_pActions.push_back(pGOAPSearchForMedkit);
	m_pActions.push_back(pSearchItem);
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
	for (FSMTransition* pTransition : m_pTransitions)
	{
		delete pTransition;
		pTransition = nullptr;
	}

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
}
void Agent::DeleteBehaviors()
{
	delete m_pWanderBehavior;
	m_pWanderBehavior = nullptr;
	delete m_pSeekBehavior;
	m_pSeekBehavior = nullptr;
	delete m_pSeekDodgeBehavior;
	m_pSeekDodgeBehavior = nullptr;
	delete m_pKillBehavior;
	m_pKillBehavior = nullptr;
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

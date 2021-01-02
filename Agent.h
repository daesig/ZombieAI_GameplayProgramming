#pragma once
#include <vector>
#include "IExamInterface.h"
#include "SteeringBehaviors.h"
#include "WorldState.h"
#include <unordered_map>

// Planning
class GOAPPlanner;
class GOAPAction;
class DecisionMaking;
// States
class FSMState;
class FSMTransition;
// Behavior
class FiniteStateMachine;
// Information
class Blackboard;
class Agent
{
public:
	Agent(IExamInterface* pInterface);
	~Agent();

	SteeringPlugin_Output UpdateSteering(float dt);
	void Render(IExamInterface* pExamInterface, float dt) const;

	void ClearBehavior();
	void SetBehavior(BehaviorType behaviorType);
	void SetSeekPos(Elite::Vector2 seekPos);

	bool GrabItem(EntityInfo& i, const eItemType& itemPriority, eItemType& grabbedType, IExamInterface* pInterface, bool& grabError);
	bool ConsumeItem(const eItemType& itemType);

	const Elite::Vector2& GetGoalPosition() const { return m_GoalPosition; };
	const Elite::Vector2& GetDistantGoalPosition() const { return m_DistantGoalPosition; };
	void SetGoalPosition(const Elite::Vector2& goalPosition) { m_GoalPosition = goalPosition; };
	void SetDistantGoalPosition(const Elite::Vector2& goalPosition) { m_DistantGoalPosition = goalPosition; };
private:
	IExamInterface* m_pInterface = nullptr;
	// Decision making 
	std::vector<FSMState*> m_pStates{};
	std::vector<FSMTransition*> m_pTransitions{};
	FiniteStateMachine* m_pFiniteStateMachine;

	// Planner
	GOAPPlanner* m_pGOAPPlanner = nullptr;
	std::vector<GOAPAction*> m_pActions{};

	// Steering behaviors
	ISteeringBehavior* m_pSteeringBehavior = nullptr;
	DecisionMaking* m_pDecisionMaking = nullptr;
	Wander* m_pWanderBehavior = nullptr;
	Seek* m_pSeekBehavior = nullptr;
	SeekAndDodge* m_pSeekDodgeBehavior = nullptr;
	SeekItem* m_pSeekItemBehavior = nullptr;

	// Data
	Blackboard* m_pBlackboard = nullptr;
	WorldState* m_pWorldState = nullptr;
	int m_MaxInventorySlots{-1};

	// Exploration
	std::list<Elite::Vector2> m_ExploredTileLocations{};
	std::vector<ExploredHouse> m_Houses{};
	std::list<EntityInfo> m_Items{};
	Elite::Vector2 m_GoalPosition{ 0.f,0.f };
	Elite::Vector2 m_DistantGoalPosition{ 0.f,0.f };
	float m_ExploredLocationRefreshTime = .1f;
	float m_ExploredLocationTimer = 0.f;

	// Enemy tracking
	Elite::Vector2 m_LastSeenClosestEnemy{};
	float m_EnemyCount{ 0 };

	// Debugging
	bool m_DebugSeek = false;
	float m_DebugTimer{ 2.f };
	float m_DebugTime{ 2.f };

	// Private functions
	bool AddInventoryItem(const EntityInfo& item,bool& grabError);
	int GetItemStackSize(ItemInfo& itemInfo) const;
	void ProcessItemWorldState(eItemType& itemType);

	void Initialize();
	void InitializeBlackboard();
	void InitializeWorldState();
	void InitializeBehaviors();
	void InitializeGOAP();
	void InitializeFSM();

	void DeleteFSM();
	void DeleteGOAP();
	void DeleteBehaviors();
	void DeleteWorldState();
	void DeleteBlackboard();
};


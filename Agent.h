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

	bool GrabItem(EntityInfo& i, IExamInterface* pInterface);
	bool ConsumeItem(const eItemType& itemType);
	bool Shoot();

	const Elite::Vector2& GetGoalPosition() const { return m_GoalPosition; };
	const Elite::Vector2& GetDistantGoalPosition() const { return m_DistantGoalPosition; };
	void SetGoalPosition(const Elite::Vector2& goalPosition) { m_GoalPosition = goalPosition; };
	void SetDistantGoalPosition(const Elite::Vector2& goalPosition) { m_DistantGoalPosition = goalPosition; };

	bool WasBitten() const;
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
	Seek* m_pSeekBehavior = nullptr;
	SeekAndDodge* m_pSeekDodgeBehavior = nullptr;
	KillBehavior* m_pKillBehavior = nullptr;

	// Data
	Blackboard* m_pBlackboard = nullptr;
	WorldState* m_pWorldState = nullptr;
	int m_MaxInventorySlots{-1};

	// Exploration
	std::vector<ExploredHouse> m_Houses{};
	ExploredHouse* m_AgentHouse = nullptr;
	std::list<EntityInfo> m_Items{};
	Elite::Vector2 m_GoalPosition{ 0.f,0.f };
	Elite::Vector2 m_DistantGoalPosition{ 0.f,0.f };
	float m_ExploredLocationRefreshTime = .1f;
	float m_ExploredLocationTimer = 0.f;
	// Fast scout goap action worldstate timer
	float m_FastScoutTimer = 0.f; // Current timer towards doing a fast scout
	float m_FastScoutTime = 3.f; // Required time for doing a fastscout

	// Enemy tracking
	Elite::Vector2 m_LastSeenClosestEnemy{};
	int m_EnemyCount{ 0 };

	// Vitals
	float m_MinimumRequiredHealth{ 9.5f };
	float m_MimimumRequiredFood{ 6.f };
	bool m_WasBitten{ false };
	float m_BittenTimer{ 0.f };
	float m_BittenTime{ 2.f };
	// Other worldstates
	float m_DistanceToFullfillMovement{ 4.f };

	// Debugging
	bool m_DebugSeek = false;
	bool m_DebugNavMeshExploration = false;
	bool m_DebugFSMStates = false;
	bool m_DebugGOAPPlanner = false;
	std::vector<Line> m_ScoutedVectors{};

	// Private functions
	bool AddInventoryItem(const EntityInfo& item);
	bool RemoveInventoryItem(int itemIndex);
	bool RemoveInventoryItem(int itemIndex, const ItemInfo& itemInfo);
	int GetItemStackSize(ItemInfo& itemInfo) const;
	void ProcessItemWorldState(const eItemType& itemType);
	void SetAgentHouseInBlackboard(const Elite::Vector2& agentPos);

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


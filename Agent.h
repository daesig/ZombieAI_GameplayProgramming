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
	Agent();
	~Agent();

	SteeringPlugin_Output UpdateSteering(IExamInterface* pInterface, float dt);
	void Render(IExamInterface* pExamInterface, float dt) const;

	void ClearBehavior();
	void SetBehavior(BehaviorType behaviorType);
	void SetSeekPos(Elite::Vector2 seekPos);

	const Elite::Vector2& GetGoalPosition() const { return m_GoalPosition; };
	void SetGoalPosition(const Elite::Vector2& goalPosition) { m_GoalPosition = goalPosition; };
private:
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
	// Exploration
	std::list<Elite::Vector2> m_ExploredTileLocations{};
	std::vector<ExploredHouse> m_Houses{};
	std::list<ItemInfo> m_Items{};
	Elite::Vector2 m_GoalPosition{-100.f,-1000.f};
	float m_ExploredLocationRefreshTime = .1f;
	float m_ExploredLocationTimer = 0.f;
	// Enemy tracking
	Elite::Vector2 m_LastSeenClosestEnemy{};

	bool m_DebugSeek = false;

	// Private functions
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


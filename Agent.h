#pragma once
#include <vector>
#include "IExamInterface.h"
#include "SteeringBehaviors.h"
#include "WorldState.h"

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

	enum class BehaviorType
	{
		WANDER,
		SEEK,
		NONE
	};

	SteeringPlugin_Output UpdateSteering(IExamInterface* pInterface, float dt);
	void Render(IExamInterface* pExamInterface, float dt) const;

	void SetBehavior(BehaviorType behaviorType);
	void SetSeekPos(Elite::Vector2 seekPos);
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

	// Data
	Blackboard* m_pBlackboard = nullptr;
	WorldState* m_pWorldState = nullptr;
	// Exploration
	std::vector<Elite::Vector2> m_ExploredTileLocations{};
	std::vector<Elite::Vector2> m_ExploredHouseLocations{};
	float m_ExploredLocationRefreshTime = .1f;
	float m_ExploredLocationTimer = 0.f;

	bool m_DebugSeek = false;

	// Private functions
	void Initialize();
	void AddWorldStates();
	void InitGOAP();
	void DeleteBehaviors();
};


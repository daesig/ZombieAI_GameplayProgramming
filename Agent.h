#pragma once
#include <vector>
#include "IExamInterface.h"
#include "SteeringBehaviors.h"
#include "WorldState.h"

// Planning
class GOAPPlanner;
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
		NONE
	};

	SteeringPlugin_Output UpdateSteering(IExamInterface* pInterface, float dt);
	void Render(IExamInterface* pExamInterface, float dt) const;

	void SetBehavior(BehaviorType behaviorType);
private:
	// Decision making 
	std::vector<FSMState*> m_pStates{};
	std::vector<FSMTransition*> m_pTransitions{};
	FiniteStateMachine* m_pFiniteStateMachine;

	// Planner
	GOAPPlanner* m_pGOAPPlanner = nullptr;

	// Steering behaviors
	ISteeringBehavior* m_pSteeringBehavior = nullptr;
	DecisionMaking* m_pDecisionMaking = nullptr;
	Wander* m_pWanderBehavior = nullptr;

	// Data
	Blackboard* m_pBlackboard = nullptr;
	WorldState* m_pWorldState = nullptr;

	// Private functions
	void Initialize();
	void AddWorldStates();
	void InitGOAP();
	void DeleteBehaviors();
};


#pragma once
#include <vector>
#include "IExamInterface.h"
#include "SteeringBehaviors.h"

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
	bool m_CanRun = false; //Demo purpose
	bool m_GrabItem = false; //Demo purpose
	bool m_UseItem = false; //Demo purpose
	bool m_RemoveItem = false; //Demo purpose
	float m_AngSpeed = 0.f; //Demo purpose

	void InitializeBehaviors();
	void DeleteBehaviors();

	//Decision making 
	std::vector<FSMState*> m_pStates{};
	std::vector<FSMTransition*> m_pTransitions{};
	FiniteStateMachine* m_pFiniteStateMachine;

	// Steering behaviors
	ISteeringBehavior* m_pSteeringBehavior = nullptr;
	DecisionMaking* m_pDecisionMaking = nullptr;
	Wander* m_pWanderBehavior = nullptr;

	Blackboard* m_pBlackboard = nullptr;
	GOAPPlanner* m_pGOAPPlanner = nullptr;

};


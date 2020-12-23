#pragma once

//--- Includes ---
#include <vector>
#include <map>
#include "Blackboard.h"
#include "DecisionMaking.h"

class GOAPPlanner;
class FSMState
{
public:
	FSMState() {}
	virtual ~FSMState() = default;

	virtual void OnEnter(GOAPPlanner* pPlanner, Blackboard* pBlackboard) {};
	virtual void OnExit(GOAPPlanner* pPlanner, Blackboard* pBlackboard) {};
	virtual void Update(GOAPPlanner* pPlanner, Blackboard* pBlackboard, float deltaTime) {};
};

class FSMTransition
{
public:
	FSMTransition() = default;
	virtual ~FSMTransition() = default;
	virtual bool ToTransition(GOAPPlanner* pPlanner, Blackboard* pBlackboard) const = 0;
};

class FiniteStateMachine final : public DecisionMaking
{
public:
	FiniteStateMachine(FSMState* startState, GOAPPlanner* pPlanner, Blackboard* pBlackboard);
	virtual ~FiniteStateMachine();

	void AddTransition(FSMState* startState, FSMState* toState, FSMTransition* transition);
	virtual void Update(GOAPPlanner* pPlanner, float deltaTime) override;
	Blackboard* GetBlackboard() const;

private:
	void SetState(GOAPPlanner* pPlanner,FSMState* newState);
private:
	typedef std::pair<FSMTransition*, FSMState*> TransitionStatePair;
	typedef std::vector<TransitionStatePair> Transitions;

	std::map<FSMState*, Transitions> m_Transitions; //Key is the state, value are all the transitions for that current state 
	FSMState* m_pCurrentState;
	Blackboard* m_pBlackboard = nullptr; // takes ownership of the blackboard
};
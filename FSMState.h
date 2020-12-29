#pragma once

//--- Includes ---
#include <vector>
#include <map>
#include "Blackboard.h"
#include "DecisionMaking.h"

class IExamInterface;
class GOAPPlanner;
class FSMState
{
public:
	FSMState() {}
	virtual ~FSMState() = default;

	virtual void OnEnter(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) {};
	virtual void OnExit(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) {};
	virtual void Update(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float deltaTime) {};
};

class FSMTransition
{
public:
	FSMTransition() = default;
	virtual ~FSMTransition() = default;
	virtual bool ToTransition(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const = 0;
};

class FiniteStateMachine final : public DecisionMaking
{
public:
	FiniteStateMachine(FSMState* startState,IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard);
	virtual ~FiniteStateMachine() = default;

	void AddTransition(FSMState* startState, FSMState* toState, FSMTransition* transition);
	virtual void Update(IExamInterface* pInterface, GOAPPlanner* pPlanner, float deltaT);
	Blackboard* GetBlackboard() const;

private:
	void SetState(IExamInterface* pInterface, GOAPPlanner* pPlanner,FSMState* newState);
private:
	typedef std::pair<FSMTransition*, FSMState*> TransitionStatePair;
	typedef std::vector<TransitionStatePair> Transitions;

	std::map<FSMState*, Transitions> m_Transitions; //Key is the state, value are all the transitions for that current state 
	FSMState* m_pCurrentState;
	Blackboard* m_pBlackboard = nullptr; // takes ownership of the blackboard
};
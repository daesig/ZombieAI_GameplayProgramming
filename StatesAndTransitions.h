#pragma once

#include "SteeringBehaviors.h"
#include "Blackboard.h"
#include "FSMState.h"

// STATES
// -----------
class IdleState final: public FSMState 
{
public:
	virtual void OnEnter(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) override;
	virtual void Update(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float deltaTime) override;
private:
	float m_TimePerActionCheck = .5f;
	float m_ActionTimer = 0.f;
	bool m_HasNext = false;

	void ResetIdleState();
};

class GoToState: public FSMState
{
	virtual void OnEnter(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) override;
	virtual void Update(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float deltaTime) override;
};

class PerformState : public FSMState
{
	virtual void OnEnter(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) override;
	virtual void Update(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float deltaTime) override;
};

class PerformedState: public FSMState
{
	virtual void OnEnter(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) override;
};

// TRANSITIONS
// -----------
class GoToTransition : public FSMTransition
{
public:
	GoToTransition() = default;
	virtual bool ToTransition(GOAPPlanner* pPlanner, Blackboard* pBlackboard) const override;
};

class PerformTransition : public FSMTransition
{
public:
	PerformTransition() = default;
	virtual bool ToTransition(GOAPPlanner * pPlanner, Blackboard * pBlackboard) const override;
};

class PerformedTransition : public FSMTransition
{
public:
	PerformedTransition() = default;
	virtual bool ToTransition(GOAPPlanner* pPlanner, Blackboard* pBlackboard) const override;
};
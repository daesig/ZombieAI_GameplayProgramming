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
	bool m_ReplanActions = false;

	void ResetIdleState();
};

class GoToState: public FSMState
{
public:
	virtual void OnEnter(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) override;
	virtual void OnExit(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) override;
	virtual void Update(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float deltaTime) override;
private:
	float m_PathRefreshDuration = 0.1f;
	float m_PathRefreshTimer = 0.f;
};

class PerformState : public FSMState
{
public:
	virtual void OnEnter(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) override;
	virtual void Update(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float deltaTime) override;
};

// TRANSITIONS
// -----------
class GoToTransition : public FSMTransition
{
public:
	GoToTransition() = default;
	virtual bool ToTransition(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const override;
};

class PerformTransition : public FSMTransition
{
public:
	PerformTransition() = default;
	virtual bool ToTransition(IExamInterface* pInterface, GOAPPlanner * pPlanner, Blackboard * pBlackboard) const override;
};

class PerformedTransition : public FSMTransition
{
public:
	PerformedTransition() = default;
	virtual bool ToTransition(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const override;
};
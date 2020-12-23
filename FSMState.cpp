#include "stdafx.h"
#include "FSMState.h"
#include "IExamInterface.h"

FiniteStateMachine::FiniteStateMachine(FSMState* startState, IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard)
	: m_pCurrentState(nullptr),
	m_pBlackboard(pBlackboard)
{
	SetState(pInterface, pPlanner, startState);
}

FiniteStateMachine::~FiniteStateMachine()
{
	SAFE_DELETE(m_pBlackboard);
}

void FiniteStateMachine::AddTransition(FSMState* startState, FSMState* toState, FSMTransition* transition)
{
	auto it = m_Transitions.find(startState);
	if (it == m_Transitions.end())
	{
		m_Transitions[startState] = Transitions();
	}

	m_Transitions[startState].push_back(std::make_pair(transition, toState));
}

void FiniteStateMachine::Update(IExamInterface* pInterface, GOAPPlanner* pPlanner, float deltaTime)
{
	auto it = m_Transitions.find(m_pCurrentState);
	if (it != m_Transitions.end())
	{
		//Since we use a normal for loop to loop over all the transitions
		//the order that you add the transitions is the order of importance
		for (TransitionStatePair& transPair : it->second)
		{
			if (transPair.first->ToTransition(pPlanner, m_pBlackboard))
			{
				SetState(pInterface, pPlanner, transPair.second);
				break;
			}
		}
	}

	if (m_pCurrentState)
		m_pCurrentState->Update(pInterface, pPlanner, m_pBlackboard, deltaTime);
}

Blackboard* FiniteStateMachine::GetBlackboard() const
{
	return m_pBlackboard;
}

void FiniteStateMachine::SetState(IExamInterface* pInterface, GOAPPlanner* pPlanner, FSMState* newState)
{
	if (m_pCurrentState)
		m_pCurrentState->OnExit(pInterface, pPlanner, m_pBlackboard);
	m_pCurrentState = newState;
	if (m_pCurrentState)
	{
		std::cout << "Entering state: " << typeid(*m_pCurrentState).name() << std::endl;
		m_pCurrentState->OnEnter(pInterface, pPlanner, m_pBlackboard);
	}
}

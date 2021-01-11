#pragma once

/*=============================================================================*/
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "SteeringHelpers.h"
#include <Exam_HelperStructs.h>
#include <unordered_map>

class IExamInterface;
class Blackboard;
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringPlugin_Output CalculateSteering(IExamInterface* pInterface, float deltaT, AgentInfo& agentInfo, Blackboard* pBlackboard, bool changeGoal = true) = 0;

	//Seek Functions
	virtual void SetTarget(const Elite::Vector2& target) { m_Target = target; }
	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{
		return static_cast<T*>(this);
	}

protected:
	Elite::Vector2 m_Target;
};

//SEEK
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringPlugin_Output CalculateSteering(IExamInterface* pInterface, float deltaT, AgentInfo& agentInfo, Blackboard* pBlackboard, bool changeGoal = true) override;
	virtual void SetTarget(const Elite::Vector2& target) override;
	Elite::Vector2 GetTarget() const;
};

class SeekAndDodge : public ISteeringBehavior
{
public:
	SeekAndDodge();
	virtual ~SeekAndDodge() = default;

	//Wander Behavior
	SteeringPlugin_Output CalculateSteering(IExamInterface* pInterface, float deltaT, AgentInfo& agentInfo, Blackboard* pBlackboard, bool changeGoal = true) override;
protected:
	float m_AngleToLastEnemy{};
	float m_LastOrientationAngle{};
private:
	float m_DodgeAngle = 35.f; // Angle in degrees
	float m_NavMeshRefreshTime{ 1.f };
	float m_NavMeshRefreshTimer{ 1.f };
};

class KillBehavior : public SeekAndDodge
{
public:
	KillBehavior();
	virtual ~KillBehavior() = default;

	//Wander Behavior
	SteeringPlugin_Output CalculateSteering(IExamInterface * pInterface, float deltaT, AgentInfo & agentInfo, Blackboard * pBlackboard, bool changeGoal = false) override;
private:
	float m_DodgeAngle = 35.f; // Angle in degrees
	float m_NavMeshRefreshTime{ 1.f };
	float m_NavMeshRefreshTimer{ 1.f };
};
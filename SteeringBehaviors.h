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

	virtual SteeringPlugin_Output CalculateSteering(IExamInterface* pInterface, float deltaT, AgentInfo& agentInfo, Blackboard* pBlackboard) = 0;

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
#pragma endregion

//SEEK
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringPlugin_Output CalculateSteering(IExamInterface* pInterface, float deltaT, AgentInfo& agentInfo, Blackboard* pBlackboard) override;
	virtual void SetTarget(const Elite::Vector2& target) override;
	Elite::Vector2 GetTarget() const;
};

class SeekAndDodge : public ISteeringBehavior
{
public:
	SeekAndDodge() = default;
	virtual ~SeekAndDodge() = default;

	//Wander Behavior
	SteeringPlugin_Output CalculateSteering(IExamInterface* pInterface, float deltaT, AgentInfo& agentInfo, Blackboard* pBlackboard) override;
private:
	float m_DodgeAngle = 35.f; // Angle in degrees
};

class SeekItem : public SeekAndDodge
{
public:
	SeekItem();
	virtual ~SeekItem() = default;

	//Wander Behavior
	SteeringPlugin_Output CalculateSteering(IExamInterface* pInterface, float deltaT, AgentInfo& agentInfo, Blackboard* pBlackboard) override;
	void SetItemToSeek(const eItemType& itemType);
private:
	eItemType m_ItemTypeToSeek = eItemType::_LAST;
};

//WANDER
class Wander : public ISteeringBehavior
{
public:
	Wander();
	virtual ~Wander() = default;
	SteeringPlugin_Output CalculateSteering(IExamInterface* pInterface, float deltaT, AgentInfo& agentInfo, Blackboard* pBlackboard) override;
private:
	float m_DistanceFromActor;
	float m_WanderRadius;
	float m_RenewDistance;
};
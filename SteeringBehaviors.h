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

#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo& pAgent) = 0;

	//Seek Functions
	virtual void SetTarget(const TargetData& target) { m_Target = target; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{
		return static_cast<T*>(this);
	}

protected:
	TargetData m_Target;
};
#pragma endregion

//WANDER
class Wander : public ISteeringBehavior
{
public:
	Wander();
	virtual ~Wander() = default;

	//Wander Behavior
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo& pAgent) override;

	virtual void SetTarget(const TargetData& target) override;

private:
	float m_DistanceFromActor;
	float m_WanderRadius;
	float m_RenewDistance;
	TargetData m_WanderTarget;
};